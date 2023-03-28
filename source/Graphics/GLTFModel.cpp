#define TINYGLTF_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "Core/Nyxispch.hpp"
#include "Core/SwapChain.hpp"
#include "Graphics/GLTFModel.hpp"
#include "Graphics/Texture.hpp"

namespace Nyxis
{
	// Bounding Box

	BoundingBox::BoundingBox()
	{
		min = glm::vec3(0.f);
		max = glm::vec3(0.f);
		valid = false;
	}

	BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {};

	BoundingBox BoundingBox::getAABB(glm::mat4 m) {
		glm::vec3 min = glm::vec3(m[3]);
		glm::vec3 max = min;
		glm::vec3 v0, v1;

		glm::vec3 right = glm::vec3(m[0]);
		v0 = right * this->min.x;
		v1 = right * this->max.x;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 up = glm::vec3(m[1]);
		v0 = up * this->min.y;
		v1 = up * this->max.y;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 back = glm::vec3(m[2]);
		v0 = back * this->min.z;
		v1 = back * this->max.z;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		return BoundingBox(min, max);
	}

	// Texture 

	void ModelTexture::updateDescriptor()
	{
		descriptor.imageLayout = imageLayout;
		descriptor.imageView = view;
		descriptor.sampler = sampler;
	}

	void ModelTexture::destroy()
	{
		vkDestroyImageView(device.device(), view, nullptr);
		vkDestroyImage(device.device(), image, nullptr);
		vkFreeMemory(device.device(), deviceMemory, nullptr);
		vkDestroySampler(device.device(), sampler, nullptr);
	}

	void ModelTexture::fromglTFImage(tinygltf::Image& gltfimage, TextureSampler textureSampler)
	{
		unsigned char* buffer = nullptr;
		VkDeviceSize bufferSize = 0;

		bool deleteBuffer = false;
		if (gltfimage.component == 3)
		{
			// TODO: Check the format support on device before transforming
			bufferSize = gltfimage.width * gltfimage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char* rgba = buffer;
			unsigned char* rgb = &gltfimage.image[0];
			for (size_t i = 0; i < gltfimage.width * gltfimage.height; ++i)
			{
				for (size_t j = 0; j < 3; ++j)
				{
					rgba[j] = rgb[j];
				}
				rgba += 4;
				rgb += 3;
			}
			deleteBuffer = true;
		}
		else
		{
			buffer = &gltfimage.image[0];
			bufferSize = gltfimage.image.size();
		}

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		VkFormatProperties formatProperties;
		width = gltfimage.width;
		height = gltfimage.height;
		mipLevels = static_cast<uint32_t> (floor(log2(std::max(width, height))) + 1);

		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = bufferSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(device.device(), &bufferCreateInfo, nullptr, &stagingBuffer);
		vkGetBufferMemoryRequirements(device.device(), stagingBuffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &stagingMemory);
		vkBindBufferMemory(device.device(), stagingBuffer, stagingMemory, 0);

		uint8_t* data;
		vkMapMemory(device.device(), stagingMemory, 0, memReqs.size, 0, (void**)&data);
		memcpy(data, buffer, bufferSize);
		vkUnmapMemory(device.device(), stagingMemory);

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.extent = { width, height, 1 };
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (vkCreateImage(device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to create image");
		}

		vkGetImageMemoryRequirements(device.device(), image, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to allocate memory");
		}

		if (vkBindImageMemory(device.device(), image, deviceMemory, 0) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to bind image memory");
		}

		auto commandBuffer = device.beginSingleTimeCommands();

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;
		bufferCopyRegion.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		device.endSingleTimeCommands(commandBuffer);

		vkFreeMemory(device.device(), stagingMemory, nullptr);
		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);

		commandBuffer = device.beginSingleTimeCommands();
		for (size_t i = 1; i < mipLevels; i++)
		{
			VkImageBlit blit = {};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.layerCount = 1;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcOffsets[1].x = static_cast<int32_t>(width >> (i - 1));
			blit.srcOffsets[1].y = static_cast<int32_t>(height >> (i - 1));
			blit.srcOffsets[1].z = 1;

			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.layerCount = 1;
			blit.dstSubresource.mipLevel = i;
			blit.dstOffsets[1].x = static_cast<int32_t>(width >> i);
			blit.dstOffsets[1].y = static_cast<int32_t>(height >> i);
			blit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			{
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.image = image;
				barrier.subresourceRange = mipSubRange;
				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

			vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			{
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.image = image;
				barrier.subresourceRange = mipSubRange;
				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}
		}

		subresourceRange.levelCount = mipLevels;
		imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = imageLayout;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.image = image;
			barrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		device.endSingleTimeCommands(commandBuffer);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = textureSampler.magFilter;
		samplerInfo.minFilter = textureSampler.minFilter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = textureSampler.addressModeU;
		samplerInfo.addressModeV = textureSampler.addressModeV;
		samplerInfo.addressModeW = textureSampler.addressModeW;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.maxAnisotropy = 8.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.levelCount = mipLevels;
		if (vkCreateImageView(device.device(), &viewInfo, nullptr, &view) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture image view!");

		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout = imageLayout;

		if (deleteBuffer)
			delete[] buffer;
	}

	// Primitive
	Primitive::Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material)
		: firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), material(material)
	{
		hasIndices = indexCount > 0;
	}

	void Primitive::setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Mesh
	Mesh::Mesh(glm::mat4 matrix)
	{
		this->uniformBlock.matrix = matrix;
		device.createBufferWithData(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(uniformBlock),
			&uniformBuffer.buffer,
			&uniformBuffer.memory,
			&uniformBlock);
		vkMapMemory(device.device(), uniformBuffer.memory, 0, sizeof(uniformBlock), 0, &uniformBuffer.mapped);
		uniformBuffer.descriptor = { uniformBuffer.buffer, 0, sizeof(uniformBlock) };
	}

	Mesh::~Mesh()
	{
		for (auto& primitive : primitives)
			delete primitive;
	}

	void Mesh::setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Node
	glm::mat4 Node::localMatrix()
	{
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	}

	glm::mat4 Node::getMatrix()
	{
		glm::mat4 m = localMatrix();
		Node* p = parent;
		while (p) {
			m = p->localMatrix() * m;
			p = p->parent;
		}
		return m;
	}

	void Node::update() {
		if (mesh) {
			glm::mat4 m = getMatrix();
			if (skin) {
				mesh->uniformBlock.matrix = m;
				// Update join matrices
				glm::mat4 inverseTransform = glm::inverse(m);
				size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);
				for (size_t i = 0; i < numJoints; i++) {
					Node* jointNode = skin->joints[i];
					glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
					jointMat = inverseTransform * jointMat;
					mesh->uniformBlock.jointMatrix[i] = jointMat;
				}
				mesh->uniformBlock.jointcount = (float)numJoints;
				memcpy(mesh->uniformBuffer.mapped, &mesh->uniformBlock, sizeof(mesh->uniformBlock));
			}
			else {
				memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
			}
		}

		for (auto& child : children) {
			child->update();
		}
	}

	Node::~Node()
	{
		if (mesh)
			delete mesh;

		for (auto& child : children)
			delete child;
	}

	Model::Model()
	{
		uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			uniformBuffers[i] = std::make_shared<Buffer>(sizeof(UBOMatrices), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			uniformBuffers[i]->map();
		}

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	}

	Model::Model(const std::string& filename, SceneInfo& sceneInfo, std::vector<Ref<Buffer>>& shaderValuesBuffer)
	{
		LOG_INFO("Loading model from {}", filename);
		animationIndex = 0;
		animationTimer = 0.0f;
		auto tStart = std::chrono::high_resolution_clock::now();
		loadFromFile(filename);
		auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		LOG_INFO("Loading took {} ms", tFileLoad);

		uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			uniformBuffers[i] = std::make_shared<Buffer>(sizeof(UBOMatrices), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			uniformBuffers[i]->map();
		}

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		setupDescriptorSet(sceneInfo, shaderValuesBuffer);
	}

	// Model
	void Model::destroy()
	{
		if (vertices.buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device.device(), vertices.buffer, nullptr);
			vkFreeMemory(device.device(), vertices.memory, nullptr);
			vertices.buffer = VK_NULL_HANDLE;
		}
		if (indices.buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device.device(), indices.buffer, nullptr);
			vkFreeMemory(device.device(), indices.memory, nullptr);
			indices.buffer = VK_NULL_HANDLE;
		}
		for (auto texture : textures) {
			texture.destroy();
		}
		textures.resize(0);
		textureSamplers.resize(0);
		for (auto node : nodes) {
			delete node;
		}
		materials.resize(0);
		animations.resize(0);
		nodes.resize(0);
		linearNodes.resize(0);
		extensions.resize(0);
		for (auto skin : skins) {
			delete skin;
		}
		skins.resize(0);
	};

	void Model::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, LoaderInfo& loaderInfo, float globalscale)
	{
		Node* newNode = new Node{};
		newNode->index = nodeIndex;
		newNode->parent = parent;
		newNode->name = node.name;
		newNode->skinIndex = node.skin;
		newNode->matrix = glm::mat4(1.0f);

		// Generate local node matrix
		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3) {
			translation = glm::make_vec3(node.translation.data());
			newNode->translation = translation;
		}
		glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4) {
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->rotation = glm::mat4(q);
		}
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3) {
			scale = glm::make_vec3(node.scale.data());
			newNode->scale = scale;
		}
		if (node.matrix.size() == 16) {
			newNode->matrix = glm::make_mat4x4(node.matrix.data());
		};

		// Node with children
		if (node.children.size() > 0) {
			for (size_t i = 0; i < node.children.size(); i++) {
				loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, loaderInfo, globalscale);
			}
		}

		// Node contains mesh data
		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			Mesh* newMesh = new Mesh(newNode->matrix);
			for (size_t j = 0; j < mesh.primitives.size(); j++) {
				const tinygltf::Primitive& primitive = mesh.primitives[j];
				uint32_t vertexStart = static_cast<uint32_t>(loaderInfo.vertexPos);
				uint32_t indexStart = static_cast<uint32_t>(loaderInfo.indexPos);
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin{};
				glm::vec3 posMax{};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;
				// Vertices
				{
					const float* bufferPos = nullptr;
					const float* bufferNormals = nullptr;
					const float* bufferTexCoordSet0 = nullptr;
					const float* bufferTexCoordSet1 = nullptr;
					const float* bufferColorSet0 = nullptr;
					const void* bufferJoints = nullptr;
					const float* bufferWeights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int uv1ByteStride;
					int color0ByteStride;
					int jointByteStride;
					int weightByteStride;

					int jointComponentType;

					// Position attribute is required
					assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

					const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
					bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = static_cast<uint32_t>(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
						const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// UVs
					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}
					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
						const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					// Vertex colors
					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
						bufferColorSet0 = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// Skinning
					// Joints
					if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
						const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
						bufferJoints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
						jointComponentType = jointAccessor.componentType;
						jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
						const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
						bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
						weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					hasSkin = (bufferJoints && bufferWeights);

					for (size_t v = 0; v < posAccessor.count; v++) {
						Vertex& vert = loaderInfo.vertexBuffer[loaderInfo.vertexPos];
						vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
						vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

						if (hasSkin)
						{
							switch (jointComponentType) {
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
								const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
								const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							default:
								// Not supported by spec
								std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
								break;
							}
						}
						else {
							vert.joint0 = glm::vec4(0.0f);
						}
						vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
						// Fix for all zero weights
						if (glm::length(vert.weight0) == 0.0f) {
							vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
						}
						loaderInfo.vertexPos++;
					}
				}
				// Indices
				if (hasIndices)
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					indexCount = static_cast<uint32_t>(accessor.count);
					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				Primitive* newPrimitive = new Primitive(indexStart, indexCount, vertexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
				newPrimitive->setBoundingBox(posMin, posMax);
				newMesh->primitives.push_back(newPrimitive);
			}
			// Mesh BB from BBs of primitives
			for (auto p : newMesh->primitives) {
				if (p->bb.valid && !newMesh->bb.valid) {
					newMesh->bb = p->bb;
					newMesh->bb.valid = true;
				}
				newMesh->bb.min = glm::min(newMesh->bb.min, p->bb.min);
				newMesh->bb.max = glm::max(newMesh->bb.max, p->bb.max);
			}
			newNode->mesh = newMesh;
		}
		if (parent) {
			parent->children.push_back(newNode);
		}
		else {
			nodes.push_back(newNode);
		}
		linearNodes.push_back(newNode);
	}

	void Model::getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount)
	{
		if (node.children.size() > 0) {
			for (size_t i = 0; i < node.children.size(); i++) {
				getNodeProps(model.nodes[node.children[i]], model, vertexCount, indexCount);
			}
		}
		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				auto primitive = mesh.primitives[i];
				vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
				if (primitive.indices > -1) {
					indexCount += model.accessors[primitive.indices].count;
				}
			}
		}
	}

	void Model::loadSkins(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Skin& source : gltfModel.skins) {
			Skin* newSkin = new Skin{};
			newSkin->name = source.name;

			// Find skeleton root node
			if (source.skeleton > -1) {
				newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
			}

			// Find joint nodes
			for (int jointIndex : source.joints) {
				Node* node = nodeFromIndex(jointIndex);
				if (node) {
					newSkin->joints.push_back(nodeFromIndex(jointIndex));
				}
			}

			// Get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1) {
				const tinygltf::Accessor& accessor = gltfModel.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
				newSkin->inverseBindMatrices.resize(accessor.count);
				memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			}

			skins.push_back(newSkin);
		}
	}

	void Model::loadTextures(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Texture& tex : gltfModel.textures) {
			tinygltf::Image image = gltfModel.images[tex.source];
			TextureSampler textureSampler;
			if (tex.sampler == -1) {
				// No sampler specified, use a default one
				textureSampler.magFilter = VK_FILTER_LINEAR;
				textureSampler.minFilter = VK_FILTER_LINEAR;
				textureSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				textureSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				textureSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
			else {
				textureSampler = textureSamplers[tex.sampler];
			}
			ModelTexture texture;
			texture.fromglTFImage(image, textureSampler);
			textures.push_back(texture);
		}
	}

	VkSamplerAddressMode Model::getVkWrapMode(int32_t wrapMode)
	{
		switch (wrapMode) {
		case -1:
		case 10497:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case 33071:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case 33648:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		}

		std::cerr << "Unknown wrap mode for getVkWrapMode: " << wrapMode << std::endl;
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkFilter Model::getVkFilterMode(int32_t filterMode)
	{
		switch (filterMode) {
		case -1:
		case 9728:
			return VK_FILTER_NEAREST;
		case 9729:
			return VK_FILTER_LINEAR;
		case 9984:
			return VK_FILTER_NEAREST;
		case 9985:
			return VK_FILTER_NEAREST;
		case 9986:
			return VK_FILTER_LINEAR;
		case 9987:
			return VK_FILTER_LINEAR;
		}

		std::cerr << "Unknown filter mode for getVkFilterMode: " << filterMode << std::endl;
		return VK_FILTER_NEAREST;
	}

	void Model::loadTextureSamplers(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Sampler smpl : gltfModel.samplers) {
			TextureSampler sampler{};
			sampler.minFilter = getVkFilterMode(smpl.minFilter);
			sampler.magFilter = getVkFilterMode(smpl.magFilter);
			sampler.addressModeU = getVkWrapMode(smpl.wrapS);
			sampler.addressModeV = getVkWrapMode(smpl.wrapT);
			sampler.addressModeW = sampler.addressModeV;
			textureSamplers.push_back(sampler);
		}
	}

	void Model::loadMaterials(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Material& mat : gltfModel.materials) {
			Material material{};
			material.doubleSided = mat.doubleSided;
			if (mat.values.find("baseColorTexture") != mat.values.end()) {
				material.baseColorTexture = &textures[mat.values["baseColorTexture"].TextureIndex()];
				material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
			}
			if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
				material.metallicRoughnessTexture = &textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
				material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
			}
			if (mat.values.find("roughnessFactor") != mat.values.end()) {
				material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
			}
			if (mat.values.find("metallicFactor") != mat.values.end()) {
				material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
			}
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
			}
			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
				material.normalTexture = &textures[mat.additionalValues["normalTexture"].TextureIndex()];
				material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
				material.emissiveTexture = &textures[mat.additionalValues["emissiveTexture"].TextureIndex()];
				material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
				material.occlusionTexture = &textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
				material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
				tinygltf::Parameter param = mat.additionalValues["alphaMode"];
				if (param.string_value == "BLEND") {
					material.alphaMode = Material::ALPHAMODE_BLEND;
				}
				if (param.string_value == "MASK") {
					material.alphaCutoff = 0.5f;
					material.alphaMode = Material::ALPHAMODE_MASK;
				}
			}
			if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
				material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
			}
			if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
				material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
			}

			// Extensions
			// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (ext->second.Has("specularGlossinessTexture")) {
					auto index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.extension.specularGlossinessTexture = &textures[index.Get<int>()];
					auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
					material.pbrWorkflows.specularGlossiness = true;
				}
				if (ext->second.Has("diffuseTexture")) {
					auto index = ext->second.Get("diffuseTexture").Get("index");
					material.extension.diffuseTexture = &textures[index.Get<int>()];
				}
				if (ext->second.Has("diffuseFactor")) {
					auto factor = ext->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
				if (ext->second.Has("specularFactor")) {
					auto factor = ext->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
			}

			materials.push_back(material);
		}
		// Push a default material at the end of the list for meshes with no material assigned
		materials.push_back(Material());
	}

	void Model::loadAnimations(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Animation& anim : gltfModel.animations) {
			Animation animation{};
			animation.name = anim.name;
			if (anim.name.empty()) {
				animation.name = std::to_string(animations.size());
			}

			// Samplers
			for (auto& samp : anim.samplers) {
				AnimationSampler sampler{};

				if (samp.interpolation == "LINEAR") {
					sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
				}
				if (samp.interpolation == "STEP") {
					sampler.interpolation = AnimationSampler::InterpolationType::STEP;
				}
				if (samp.interpolation == "CUBICSPLINE") {
					sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
				}

				// Read sampler input time values
				{
					const tinygltf::Accessor& accessor = gltfModel.accessors[samp.input];
					const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
					const float* buf = static_cast<const float*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						sampler.inputs.push_back(buf[index]);
					}

					for (auto input : sampler.inputs) {
						if (input < animation.start) {
							animation.start = input;
						};
						if (input > animation.end) {
							animation.end = input;
						}
					}
				}

				// Read sampler output T/R/S values 
				{
					const tinygltf::Accessor& accessor = gltfModel.accessors[samp.output];
					const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

					switch (accessor.type) {
					case TINYGLTF_TYPE_VEC3: {
						const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
						}
						break;
					}
					case TINYGLTF_TYPE_VEC4: {
						const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							sampler.outputsVec4.push_back(buf[index]);
						}
						break;
					}
					default: {
						LOG_WARN("Unknown accessor type", accessor.type);
						break;
					}
					}
				}

				animation.samplers.push_back(sampler);
			}

			// Channels
			for (auto& source : anim.channels) {
				AnimationChannel channel{};

				if (source.target_path == "rotation") {
					channel.path = AnimationChannel::PathType::ROTATION;
				}
				if (source.target_path == "translation") {
					channel.path = AnimationChannel::PathType::TRANSLATION;
				}
				if (source.target_path == "scale") {
					channel.path = AnimationChannel::PathType::SCALE;
				}
				if (source.target_path == "weights") {
					LOG_WARN("weights not yet supported, skipping channel");
					continue;
				}
				channel.samplerIndex = source.sampler;
				channel.node = nodeFromIndex(source.target_node);
				if (!channel.node) {
					continue;
				}

				animation.channels.push_back(channel);
			}

			animations.push_back(animation);
		}
	}

	void Model::loadFromFile(std::string filename, float scale)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;

		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = filename.rfind('.', filename.length());
		if (extpos != std::string::npos) {
			binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
		}

		bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str()) : gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());

		LoaderInfo loaderInfo{};
		size_t vertexCount = 0;
		size_t indexCount = 0;

		if (fileLoaded) {
			loadTextureSamplers(gltfModel);
			loadTextures(gltfModel);
			loadMaterials(gltfModel);

			const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

			// Get vertex and index buffer sizes up-front
			for (size_t i = 0; i < scene.nodes.size(); i++) {
				getNodeProps(gltfModel.nodes[scene.nodes[i]], gltfModel, vertexCount, indexCount);
			}
			loaderInfo.vertexBuffer = new Vertex[vertexCount];
			loaderInfo.indexBuffer = new uint32_t[indexCount];

			// TODO: scene handling with no default scene
			for (size_t i = 0; i < scene.nodes.size(); i++) {
				const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
				loadNode(nullptr, node, scene.nodes[i], gltfModel, loaderInfo, scale);
			}
			if (gltfModel.animations.size() > 0) {
				loadAnimations(gltfModel);
			}
			loadSkins(gltfModel);

			for (auto node : linearNodes) {
				// Assign skins
				if (node->skinIndex > -1) {
					node->skin = skins[node->skinIndex];
				}
				// Initial pose
				if (node->mesh) {
					node->update();
				}
			}
		}
		else {
			// TODO: throw
			std::cerr << "Could not load gltf file: " << error << std::endl;
			return;
		}

		extensions = gltfModel.extensionsUsed;

		size_t vertexBufferSize = vertexCount * sizeof(Vertex);
		size_t indexBufferSize = indexCount * sizeof(uint32_t);

		assert(vertexBufferSize > 0);

		struct StagingBuffer {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging, indexStaging;

		// Create staging buffers
		// Vertex data
		device.createBufferWithData(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBufferSize,
			&vertexStaging.buffer,
			&vertexStaging.memory,
			loaderInfo.vertexBuffer);
		// Index data
		if (indexBufferSize > 0) {
			device.createBufferWithData(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				indexBufferSize,
				&indexStaging.buffer,
				&indexStaging.memory,
				loaderInfo.indexBuffer);
		}

		// Create device local buffers
		// Vertex buffer
		device.createBufferWithData(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBufferSize,
			&vertices.buffer,
			&vertices.memory);
		// Index buffer
		if (indexBufferSize > 0) {
			device.createBufferWithData(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				indexBufferSize,
				&indices.buffer,
				&indices.memory);
		}

		// Copy from staging buffers
		auto copyCmd = device.beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

		if (indexBufferSize > 0) {
			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indices.buffer, 1, &copyRegion);
		}

		device.endSingleTimeCommands(copyCmd);

		vkDestroyBuffer(device.device(), vertexStaging.buffer, nullptr);
		vkFreeMemory(device.device(), vertexStaging.memory, nullptr);
		if (indexBufferSize > 0) {
			vkDestroyBuffer(device.device(), indexStaging.buffer, nullptr);
			vkFreeMemory(device.device(), indexStaging.memory, nullptr);
		}

		delete[] loaderInfo.vertexBuffer;
		delete[] loaderInfo.indexBuffer;

		getSceneDimensions();

	}

	void Model::drawNode(Node* node, VkCommandBuffer commandBuffer)
	{
		if (node->mesh) {
			for (Primitive* primitive : node->mesh->primitives) {
				vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
			}
		}
		for (auto& child : node->children) {
			drawNode(child, commandBuffer);
		}
	}

	void Model::draw(VkCommandBuffer commandBuffer)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		for (auto& node : nodes) {
			drawNode(node, commandBuffer);
		}
	}

	void Model::calculateBoundingBox(Node* node, Node* parent) {
		BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(dimensions.min, dimensions.max);

		if (node->mesh) {
			if (node->mesh->bb.valid) {
				node->aabb = node->mesh->bb.getAABB(node->getMatrix());
				if (node->children.size() == 0) {
					node->bvh.min = node->aabb.min;
					node->bvh.max = node->aabb.max;
					node->bvh.valid = true;
				}
			}
		}

		parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
		parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

		for (auto& child : node->children) {
			calculateBoundingBox(child, node);
		}
	}

	void Model::getSceneDimensions()
	{
		// Calculate binary volume hierarchy for all nodes in the scene
		for (auto node : linearNodes) {
			calculateBoundingBox(node, nullptr);
		}

		dimensions.min = glm::vec3(FLT_MAX);
		dimensions.max = glm::vec3(-FLT_MAX);

		for (auto node : linearNodes) {
			if (node->bvh.valid) {
				dimensions.min = glm::min(dimensions.min, node->bvh.min);
				dimensions.max = glm::max(dimensions.max, node->bvh.max);
			}
		}

		// Calculate scene aabb
		aabb = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.max[0] - dimensions.min[0], dimensions.max[1] - dimensions.min[1], dimensions.max[2] - dimensions.min[2]));
		aabb[3][0] = dimensions.min[0];
		aabb[3][1] = dimensions.min[1];
		aabb[3][2] = dimensions.min[2];
	}

	void Model::updateAnimation(float deltaTime)
	{
		if (animations.size() == 0)
			return;

		animationTimer += deltaTime;
		if (animationTimer > animations[animationIndex].end)
			animationTimer -= animations[animationIndex].end;

		if (animations.empty())
			return;

		if (animationIndex > static_cast<uint32_t>(animations.size()) - 1) {
			std::cout << "No animation with index " << animationIndex << std::endl;
			return;
		}

		Animation& animation = animations[animationIndex];

		bool updated = false;
		for (auto& channel : animation.channels) {
			AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
			if (sampler.inputs.size() > sampler.outputsVec4.size()) 
				continue;

			for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
				if ((animationTimer >= sampler.inputs[i]) && (animationTimer <= sampler.inputs[i + 1])) {
					float u = std::max(0.0f, animationTimer - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
					if (u <= 1.0f) {
						switch (channel.path) {
						case AnimationChannel::PathType::TRANSLATION: {
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							channel.node->translation = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::SCALE: {
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							channel.node->scale = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::ROTATION: {
							glm::quat q1;
							q1.x = sampler.outputsVec4[i].x;
							q1.y = sampler.outputsVec4[i].y;
							q1.z = sampler.outputsVec4[i].z;
							q1.w = sampler.outputsVec4[i].w;
							glm::quat q2;
							q2.x = sampler.outputsVec4[i + 1].x;
							q2.y = sampler.outputsVec4[i + 1].y;
							q2.z = sampler.outputsVec4[i + 1].z;
							q2.w = sampler.outputsVec4[i + 1].w;
							channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
							break;
						}
						}
						updated = true;
					}
				}
			}
		}
		if (updated) {
			for (auto& node : nodes) {
				node->update();
			}
		}
	}

	Node* Model::findNode(Node* parent, uint32_t index) {
		Node* nodeFound = nullptr;
		if (parent->index == index) {
			return parent;
		}
		for (auto& child : parent->children) {
			nodeFound = findNode(child, index);
			if (nodeFound) {
				break;
			}
		}
		return nodeFound;
	}

	Node* Model::nodeFromIndex(uint32_t index) {
		Node* nodeFound = nullptr;
		for (auto& node : nodes) {
			nodeFound = findNode(node, index);
			if (nodeFound) {
				break;
			}
		}
		return nodeFound;
	}

	void Model::updateModelMatrix(RigidBody& rigidBody)
	{
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, rigidBody.translation);
		modelMatrix = glm::scale(modelMatrix, rigidBody.scale);

		// Check if all rotation angles are zero
		if (glm::all(glm::equal(rigidBody.rotation, glm::vec3(0.0f))))
			return;

		modelMatrix = glm::rotate(modelMatrix, glm::radians(rigidBody.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rigidBody.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rigidBody.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void Model::setupDescriptorSet(SceneInfo& sceneInfo, std::vector<Ref<Buffer>>& shaderValuesBuffer)
	{

		struct Layouts
		{
			VkDescriptorSetLayout scene = ModelManager::GetModelDescriptorSetLayout()->getDescriptorSetLayout();
			VkDescriptorSetLayout material = ModelManager::GetMaterialDescriptorSetLayout()->getDescriptorSetLayout();
			VkDescriptorSetLayout node = ModelManager::GetNodeDescriptorSetLayout()->getDescriptorSetLayout();
		} descriptorSetLayouts;

		VkDescriptorPool descriptorPool = ModelManager::GetDescriptorPool()->getDescriptorPool();
		// Scene (matrices and environment maps)
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
			descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
			vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayouts.scene);

			for (auto i = 0; i < descriptorSets.size(); i++) {
		
				VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
				descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocInfo.descriptorPool = descriptorPool;
				descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.scene;
				descriptorSetAllocInfo.descriptorSetCount = 1;
				vkAllocateDescriptorSets(device.device(), &descriptorSetAllocInfo, &descriptorSets[i]);
		
				std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};
		
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = descriptorSets[i];
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].pBufferInfo = uniformBuffers[i]->getDescriptorInfo();
		
				writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[1].descriptorCount = 1;
				writeDescriptorSets[1].dstSet = descriptorSets[i];
				writeDescriptorSets[1].dstBinding = 1;
				writeDescriptorSets[1].pBufferInfo = shaderValuesBuffer[i]->getDescriptorInfo();
		
				writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[2].descriptorCount = 1;
				writeDescriptorSets[2].dstSet = descriptorSets[i];
				writeDescriptorSets[2].dstBinding = 2;
				writeDescriptorSets[2].pImageInfo = &sceneInfo.textures.irradianceCube.m_Descriptor;
		
				writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[3].descriptorCount = 1;
				writeDescriptorSets[3].dstSet = descriptorSets[i];
				writeDescriptorSets[3].dstBinding = 3;
				writeDescriptorSets[3].pImageInfo = &sceneInfo.textures.prefilteredCube.m_Descriptor;
		
				writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[4].descriptorCount = 1;
				writeDescriptorSets[4].dstSet = descriptorSets[i];
				writeDescriptorSets[4].dstBinding = 4;
				writeDescriptorSets[4].pImageInfo = &sceneInfo.textures.lutBrdf.m_Descriptor;
		
				vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
			}
		}
		
		// Material (samplers)
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
			descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
			vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayouts.material);

			// Per-Material descriptor sets
			for (auto& material : materials) {
				VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
				descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocInfo.descriptorPool = descriptorPool;
				descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.material;
				descriptorSetAllocInfo.descriptorSetCount = 1;
				vkAllocateDescriptorSets(device.device(), &descriptorSetAllocInfo, &material.descriptorSet);
		
				std::vector<VkDescriptorImageInfo> imageDescriptors = {
					sceneInfo.textures.empty.m_Descriptor,
					sceneInfo.textures.empty.m_Descriptor,
					material.normalTexture ? material.normalTexture->descriptor : sceneInfo.textures.empty.m_Descriptor,
					material.occlusionTexture ? material.occlusionTexture->descriptor : sceneInfo.textures.empty.m_Descriptor,
					material.emissiveTexture ? material.emissiveTexture->descriptor : sceneInfo.textures.empty.m_Descriptor
				};
		
				// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present
		
				if (material.pbrWorkflows.metallicRoughness) {
					if (material.baseColorTexture) {
						imageDescriptors[0] = material.baseColorTexture->descriptor;
					}
					if (material.metallicRoughnessTexture) {
						imageDescriptors[1] = material.metallicRoughnessTexture->descriptor;
					}
				}
		
				if (material.pbrWorkflows.specularGlossiness) {
					if (material.extension.diffuseTexture) {
						imageDescriptors[0] = material.extension.diffuseTexture->descriptor;
					}
					if (material.extension.specularGlossinessTexture) {
						imageDescriptors[1] = material.extension.specularGlossinessTexture->descriptor;
					}
				}
		
				std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};
				for (size_t i = 0; i < imageDescriptors.size(); i++) {
					writeDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writeDescriptorSets[i].descriptorCount = 1;
					writeDescriptorSets[i].dstSet = material.descriptorSet;
					writeDescriptorSets[i].dstBinding = static_cast<uint32_t>(i);
					writeDescriptorSets[i].pImageInfo = &imageDescriptors[i];
				}
		
				vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
			}
		
			// Model node (matrices)
			{
				std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
					{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
				};
				VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
				descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
				descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
				vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutCI, nullptr, &descriptorSetLayouts.node);
		
				// Per-Node descriptor set
				for (auto& node : nodes) {
					setupNodeDescriptorSet(node);
				}
			}
		}
	}

	void Model::setupNodeDescriptorSet(const Node* node)
	{
		auto descriptorPool = ModelManager::GetDescriptorPool();
		auto descriptorSetLayout = ModelManager::GetNodeDescriptorSetLayout()->getDescriptorSetLayout();
		if (node->mesh) {
			ModelManager::GetDescriptorPool()->allocateDescriptor(descriptorSetLayout, node->mesh->uniformBuffer.descriptorSet);
			DescriptorWriter (ModelManager::GetNodeDescriptorSetLayout(), ModelManager::GetDescriptorPool())
				.writeBuffer(0, &node->mesh->uniformBuffer.descriptor)
				.build(node->mesh->uniformBuffer.descriptorSet);
		}
		for (const auto& child : node->children) {
			setupNodeDescriptorSet(child);
		}
	}

	void Model::updateUniformBuffer(uint32_t index, UBOMatrices* ubo)
	{
		uniformBuffers[index]->writeToBuffer(ubo);
		uniformBuffers[index]->flush();
	}

	Ref<DescriptorPool> ModelManager::GetDescriptorPool()
	{
		if (m_SetupState == false)
		{
			Setup();
			m_SetupState = true;
		}
		return m_DescriptorPool;
	}

	void ModelManager::Setup()
	{
		m_DescriptorPool = DescriptorPool::Builder()
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

		m_ModelDescriptorSetLayout = DescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_MaterialDescriptorSetLayout = DescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_NodeDescriptorSetLayout = DescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
			.build();
	}

	Ref<DescriptorSetLayout> ModelManager::GetModelDescriptorSetLayout()
	{
		if (m_SetupState == false)
		{
			Setup();
			m_SetupState = true;
		}
		return m_ModelDescriptorSetLayout;
	}

	Ref<DescriptorSetLayout> ModelManager::GetMaterialDescriptorSetLayout()
	{
		if (m_SetupState == false)
		{
			Setup();
			m_SetupState = true;
		}
		return m_MaterialDescriptorSetLayout;
	}

	Ref<DescriptorSetLayout> ModelManager::GetNodeDescriptorSetLayout()
	{
		if (m_SetupState == false)
		{
			Setup();
			m_SetupState = true;
		}
		return m_NodeDescriptorSetLayout;
	}
}
