#include "Core/GLTFRenderer.hpp"

#include "Core/Application.hpp"
#include "Core/Pipeline.hpp"
#include "Core/Log.hpp"
#include "Core/SwapChain.hpp"
#include "Core/Renderer.hpp"
#include "Scene/Components.hpp"
#include "Scene/NyxisProject.hpp"

namespace Nyxis
{
	void GLTFRenderer::Init(VkRenderPass renderPass)
	{
		device = &Device::Get();
		s_SceneInfo.shaderValuesParams.lightDir = { 1.0f, 1.0f, 1.0f, 0.5f };
		s_SceneInfo.shaderValuesParams.exposure = 4.5f;
		s_SceneInfo.shaderValuesParams.gamma = 2.2f;
		s_SceneInfo.shaderValuesParams.prefilteredCubeMipLevels = 0.0f;
		s_SceneInfo.shaderValuesParams.scaleIBLAmbient = 1.0f;
		s_SceneInfo.shaderValuesParams.debugViewInputs = 0;
		s_SceneInfo.shaderValuesParams.debugViewEquation = 0;

		// Initialize depth buffer array
		for (int i = 0; i < DEPTH_ARRAY_SCALE; i++)
			objectPicking.depthBufferObject[i] = 0;

		LoadAssets();
		GenerateBRDFLUT();
		PrepareUniformBuffers();
		SetupDescriptorPool();
		SetupDescriptorSets();
		PreparePipelines(renderPass);
	}

	void GLTFRenderer::Shutdown()
	{
		LOG_INFO("[Core] Shutting down GLTF Renderer");
		if(animationThread.joinable())
			animationThread.join();
	}

	void GLTFRenderer::OnUpdate()
	{
		auto scene = Application::GetScene();
		auto frameInfo = Application::GetFrameInfo();

		if(s_ShaderValuesScene.isMouseClicked)
		{
			auto commandBuffer = device->beginSingleTimeCommands();

			// change the layout of the image to a redeable format
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = Renderer::GetIDImage();
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			// change the layout of the image to a redeable format
			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			// copy the image to the buffer
			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { Viewport::GetExtent().width, Viewport::GetExtent().height, 1 };

			Buffer stagingBuffer(Viewport::GetExtent().width * Viewport::GetExtent().height * 4,
								 1,
								 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkCmdCopyImageToBuffer(
				commandBuffer,
				Renderer::GetIDImage(),
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				stagingBuffer.getBuffer(),
				1,
				&region
			);

			device->endSingleTimeCommands(commandBuffer);

			stagingBuffer.map();
			void* mappedData = stagingBuffer.getMappedMemory();

			auto mousePos = Input::GetMousePosition();
			auto viewportPosition = Viewport::GetWindowPos();
			auto x = mousePos.x - viewportPosition.x;
			auto y = mousePos.y - viewportPosition.y;

			uint32_t* pixels = static_cast<uint32_t*>(mappedData);
			uint32_t pixel = pixels[static_cast<size_t>(Viewport::GetExtent().width * y + x)];
			if(pixel != 0)
				EditorLayer::SetSelectedEntity(static_cast<Entity>(pixel));
			else
				EditorLayer::DeselectEntity();

			LOG_TRACE("Pixel {}", pixel);
			LOG_TRACE("Mouse Position: {} {}", x, y);
		}

		if (s_SceneUpdated)
		{
			vkDeviceWaitIdle(device->device());
			LoadEnvironment(s_EnvMapFile);
			FreeDescriptorSets();
			SetupDescriptorSets();

			auto modelView = scene->GetComponentView<Model>();
			for(auto entity : modelView)
			{
				auto& model = scene->GetComponent<Model>(entity);
				model.setupDescriptorSet(s_SceneInfo, s_UniformBuffersParams);
			}

			s_SceneUpdated = false;
		}

        if (s_Animate) {
            animationThread = std::thread([&]{
				UpdateAnimation(Application::GetFrameInfo()->frameTime);
                });
            animationThread.detach();
        }

		if(s_PBRPipelineUpdate)
		{
			Pipes.pbr->Recreate();
			s_PBRPipelineUpdate = false;
		}
		if(s_SkyboxPipelineUpdate)
		{
			Pipes.skybox->Recreate();
			s_SkyboxPipelineUpdate = false;
		}
	}

	void GLTFRenderer::Render()
	{
		auto frameInfo = Application::GetFrameInfo();
		auto scene = Application::GetScene();

		UpdateBuffers();
		vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &skyboxDescriptorSets[frameInfo->frameIndex], 0, nullptr);
		Pipes.skybox->Bind(frameInfo->commandBuffer);
		skybox->draw(frameInfo->commandBuffer);

		auto modelView = scene->GetComponentView<Model>();
		for (auto& model : modelView)
		{
			auto& gltfModel = scene->GetComponent<Model>(model);
			if (!gltfModel.ready)
				continue;
			auto& transform = scene->GetComponent<TransformComponent>(model);

			s_ShaderValuesScene.model = transform.mat4();
			s_ShaderValuesScene.entityID = static_cast<int>(model);
			s_ShaderValuesScene.isMouseClicked = Viewport::IsClicked() && !Viewport::IsHoveredOverGizmo();
			s_ShaderValuesScene.selectedEntityID = static_cast<uint32_t>(EditorLayer::GetSelectedEntity());

			gltfModel.updateUniformBuffer(frameInfo->frameIndex, &s_ShaderValuesScene);
			gltfModel.bind(frameInfo->commandBuffer);

			boundPipeline = VK_NULL_HANDLE;

			for (auto node : gltfModel.nodes)
				RenderNode(node, Material::ALPHAMODE_OPAQUE, gltfModel);
			// Alpha masked primitives
			for (auto node : gltfModel.nodes)
				RenderNode(node, Material::ALPHAMODE_MASK, gltfModel);
			// Transparent primitives
			// TODO: Correct depth sorting
			for (auto node : gltfModel.nodes)
				RenderNode(node, Material::ALPHAMODE_BLEND, gltfModel);
		}
	}
	
	void GLTFRenderer::UpdateAnimation(float dt)
	{
		auto scene = Application::GetScene();

		if (s_Animate) 
		{
			auto view = scene->GetComponentView<Model>();
			for (auto model : view)
			{
				auto& gltfModel = scene->GetComponent<Model>(model);
				gltfModel.updateAnimation(dt);
			}
		}
	}

	void GLTFRenderer::UpdatePipeline(PipelineType pipeline)
	{
		switch (pipeline)
		{
		case PipelineType::PBR:
			s_PBRPipelineUpdate = true;
			break;
		default:
			break;
		}
	}

	void GLTFRenderer::PrepareUniformBuffers()
	{
		skyboxDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		s_SkyboxBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		s_UniformBuffersParams.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		s_ObjectPickingBuffer.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			s_SkyboxBuffers[i] = std::make_shared<Buffer>(sizeof(s_ShaderValuesSkybox), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			s_SkyboxBuffers[i]->map();

			s_UniformBuffersParams[i] = std::make_shared<Buffer>(sizeof(s_SceneInfo.shaderValuesParams), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			s_UniformBuffersParams[i]->map();

			s_ObjectPickingBuffer[i] = std::make_shared<Buffer>(sizeof(ObjectPicking), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			s_ObjectPickingBuffer[i]->map();
		}
	}

	void GLTFRenderer::UpdateBuffers()
	{
		auto frameInfo = Application::GetFrameInfo();
		auto scene = Application::GetScene();
		// Scene
		s_ShaderValuesScene.projection = scene->m_Camera->getProjectionMatrix();
		s_ShaderValuesScene.view = scene->m_Camera->getViewMatrix();

		auto& transform = scene->m_Registry.get<TransformComponent>(scene->m_CameraEntity);
		
		s_ShaderValuesScene.camPos = glm::vec3(
			-transform.translation.z * sin(glm::radians(transform.translation.y)) * cos(glm::radians(transform.translation.x)),
			-transform.translation.z * sin(glm::radians(transform.translation.x)),
			transform.translation.z * cos(glm::radians(transform.translation.y)) * cos(glm::radians(transform.translation.x))
		);

		// Skybox
		s_ShaderValuesSkybox.projection = scene->m_Camera->getProjectionMatrix();
		s_ShaderValuesSkybox.view = scene->m_Camera->getViewMatrix();
		s_ShaderValuesSkybox.model = glm::mat4(glm::mat3(s_ShaderValuesSkybox.view));

		s_UniformBuffersParams[frameInfo->frameIndex]->writeToBuffer(&s_SceneInfo.shaderValuesParams);
		s_SkyboxBuffers[frameInfo->frameIndex]->writeToBuffer(&s_ShaderValuesSkybox);

		s_UniformBuffersParams[frameInfo->frameIndex]->flush();
		s_SkyboxBuffers[frameInfo->frameIndex]->flush();

		objectPicking.selectedEntity = static_cast<uint32_t>(EditorLayer::GetSelectedEntity());
		s_ObjectPickingBuffer[frameInfo->frameIndex]->writeToBuffer(&objectPicking);
		s_ObjectPickingBuffer[frameInfo->frameIndex]->flush();
	}

	void GLTFRenderer::LoadEnvironment(std::string& filename)
	{
		LOG_INFO("[Renderer] Loading environment from {}", filename);
		s_SceneInfo.textures.environmentCube.LoadFromFile(filename, VK_FORMAT_R16G16B16A16_SFLOAT);
		GenerateCubemaps();
	}

	void GLTFRenderer::LoadAssets()
	{
		const std::string assets_path = Application::GetProject()->GetAssetPath();

		s_SceneInfo.textures.empty.LoadFromFile(assets_path + "/textures/empty.ktx", VK_FORMAT_R8G8B8A8_UNORM);
		skybox = std::make_shared<Model>("/models/basic/cube.gltf");

		s_EnvMapFile = assets_path + "/environments/sky.ktx";
		LoadEnvironment(s_EnvMapFile);
	}

	void GLTFRenderer::SetupDescriptorPool()
	{
		/*
			Descriptor Pool
		*/
		// TODO : Calculate the number of descriptors needed without hardcoding them
		uint32_t imageSamplerCount = 5;
		uint32_t materialCount = 5;
		uint32_t meshCount = 5;

		// Environment samplers (radiance, irradiance, brdf lut)
		imageSamplerCount += 3;

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * SwapChain::MAX_FRAMES_IN_FLIGHT},
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * SwapChain::MAX_FRAMES_IN_FLIGHT},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 * SwapChain::MAX_FRAMES_IN_FLIGHT}
		};
		VkDescriptorPoolCreateInfo descriptorPoolCI{};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = 3;
		descriptorPoolCI.pPoolSizes = poolSizes.data();
		descriptorPoolCI.maxSets = (2 + materialCount + meshCount + 100) * SwapChain::MAX_FRAMES_IN_FLIGHT;
		descriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		vkCreateDescriptorPool(device->device(), &descriptorPoolCI, nullptr, &descriptorPool);
	}

	void GLTFRenderer::SetupDescriptorSets()
	{
		// Mouse Map descriptor layout
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{ 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
			};

			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
			descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
			vkCreateDescriptorSetLayout(device->device(), &descriptorSetLayoutCI, nullptr, &depthBufferLayout);
		}

		depthBufferDescriptorSets.resize(s_ObjectPickingBuffer.size());

		for(auto i = 0; i < s_ObjectPickingBuffer.size(); i++)
		{
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorPool;
			descriptorSetAllocInfo.pSetLayouts = &depthBufferLayout;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			vkAllocateDescriptorSets(device->device(), &descriptorSetAllocInfo, &depthBufferDescriptorSets[i]);

			VkWriteDescriptorSet writeDescriptor{};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.dstSet = depthBufferDescriptorSets[i];
			writeDescriptor.dstBinding = 0;
			writeDescriptor.pBufferInfo = s_ObjectPickingBuffer[i]->getDescriptorInfo();

			vkUpdateDescriptorSets(device->device(), 1, &writeDescriptor, 0, nullptr);
		}

		UpdateSkyboxDescriptorSets();
	}

	void GLTFRenderer::FreeDescriptorSets()
	{
		for (auto descriptorSet : skyboxDescriptorSets) {
				vkFreeDescriptorSets(device->device(), descriptorPool, 1, &descriptorSet);
		}
	}


	void GLTFRenderer::UpdateSkyboxDescriptorSets()
	{
		auto layout = ModelDescriptorManager::GetModelDescriptorSetLayout()->getDescriptorSetLayout();
		for (auto i = 0; i < s_SkyboxBuffers.size(); i++) {
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorPool;
			descriptorSetAllocInfo.pSetLayouts = &layout;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			vkAllocateDescriptorSets(device->device(), &descriptorSetAllocInfo, &skyboxDescriptorSets[i]);

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};

			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[0].dstSet = skyboxDescriptorSets[i];
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].pBufferInfo = s_SkyboxBuffers[i]->getDescriptorInfo();

			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[1].dstSet = skyboxDescriptorSets[i];
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].pBufferInfo = s_UniformBuffersParams[i]->getDescriptorInfo();

			writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSets[2].descriptorCount = 1;
			writeDescriptorSets[2].dstSet = skyboxDescriptorSets[i];
			writeDescriptorSets[2].dstBinding = 2;
			writeDescriptorSets[2].pImageInfo = &s_SceneInfo.textures.prefilteredCube.m_Descriptor;

			vkUpdateDescriptorSets(device->device(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void GLTFRenderer::OnEvent(const Event& event)
	{
		if (Viewport::IsClicked() && !Viewport::IsHoveredOverGizmo())
		{
			auto buffer = static_cast<ObjectPicking*>(s_ObjectPickingBuffer[Application::GetFrameInfo()->frameIndex]->getMappedMemory());
			auto depthBuffer = buffer->depthBufferObject;
			for (int i = 0; i < DEPTH_ARRAY_SCALE; i++)
			{
				if (depthBuffer[i] != 0)
				{
					EditorLayer::SetSelectedEntity(static_cast<Entity>(depthBuffer[i]));
					break;
				}
			}
		}
	}

	void GLTFRenderer::RenderNode(Node* node, Material::AlphaMode alphaMode, Model& model)
	{
		auto frameInfo = Application::GetFrameInfo();
		if (node->mesh) {
			// Render mesh primitives
			for (Primitive* primitive : node->mesh->primitives) {
				if (primitive->material.alphaMode == alphaMode) {

					VkPipeline pipeline = VK_NULL_HANDLE;
					switch (alphaMode) {
					case Material::ALPHAMODE_OPAQUE:
					case Material::ALPHAMODE_MASK:
						pipeline = primitive->material.doubleSided ? pipelines.pbrDoubleSided : pipelines.pbr;
						break;
					case Material::ALPHAMODE_BLEND:
						pipeline = pipelines.pbrAlphaBlend;
						break;
					}

					if (pipeline != boundPipeline) {
					}

					Pipes.pbr->Bind(frameInfo->commandBuffer);

					const std::vector<VkDescriptorSet> descriptorsets = {
						model.getDescriptorSet(frameInfo->frameIndex),
						primitive->material.descriptorSet,
						node->mesh->uniformBuffer.descriptorSet,
						depthBufferDescriptorSets[frameInfo->frameIndex]
					};
					vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, static_cast<uint32_t>(descriptorsets.size()), descriptorsets.data(), 0, NULL);

					// Pass material parameters as push constants
					PushConstBlockMaterial pushConstBlockMaterial{};
					pushConstBlockMaterial.emissiveFactor = primitive->material.emissiveFactor;
					// To save push constant space, availabilty and texture coordiante set are combined
					// -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
					pushConstBlockMaterial.colorTextureSet = primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					pushConstBlockMaterial.normalTextureSet = primitive->material.normalTexture != nullptr ? primitive->material.texCoordSets.normal : -1;
					pushConstBlockMaterial.occlusionTextureSet = primitive->material.occlusionTexture != nullptr ? primitive->material.texCoordSets.occlusion : -1;
					pushConstBlockMaterial.emissiveTextureSet = primitive->material.emissiveTexture != nullptr ? primitive->material.texCoordSets.emissive : -1;
					pushConstBlockMaterial.alphaMask = static_cast<float>(primitive->material.alphaMode == Material::ALPHAMODE_MASK);
					pushConstBlockMaterial.alphaMaskCutoff = primitive->material.alphaCutoff;
					pushConstBlockMaterial.nodeID = node->entityID;

					// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

					if (primitive->material.pbrWorkflows.metallicRoughness) {
						// Metallic roughness workflow
						pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
						pushConstBlockMaterial.baseColorFactor = primitive->material.baseColorFactor;
						pushConstBlockMaterial.metallicFactor = primitive->material.metallicFactor;
						pushConstBlockMaterial.roughnessFactor = primitive->material.roughnessFactor;
						pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.metallicRoughnessTexture != nullptr ? primitive->material.texCoordSets.metallicRoughness : -1;
						pushConstBlockMaterial.colorTextureSet = primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					}

					if (primitive->material.pbrWorkflows.specularGlossiness) {
						// Specular glossiness workflow
						pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSINESS);
						pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.extension.specularGlossinessTexture != nullptr ? primitive->material.texCoordSets.specularGlossiness : -1;
						pushConstBlockMaterial.colorTextureSet = primitive->material.extension.diffuseTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
						pushConstBlockMaterial.diffuseFactor = primitive->material.extension.diffuseFactor;
						pushConstBlockMaterial.specularFactor = glm::vec4(primitive->material.extension.specularFactor, 1.0f);
					}

					vkCmdPushConstants(frameInfo->commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstBlockMaterial), &pushConstBlockMaterial);

					if (primitive->hasIndices) {
						vkCmdDrawIndexed(frameInfo->commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
					}
					else {
						vkCmdDraw(frameInfo->commandBuffer, primitive->vertexCount, 1, 0, 0);
					}
				}
			}

		}
		for (auto child : node->children) {
			RenderNode(child, alphaMode, model);
		}
	}

	VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage)
	{
		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;
		shaderStage.pName = "main";
		std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

		if (is.is_open()) {
			size_t size = is.tellg();
			is.seekg(0, std::ios::beg);
			char* shaderCode = new char[size];
			is.read(shaderCode, size);
			is.close();
			assert(size > 0);
			VkShaderModuleCreateInfo moduleCreateInfo{};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = size;
			moduleCreateInfo.pCode = (uint32_t*)shaderCode;
			vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStage.module);
			delete[] shaderCode;
		}
		else {
			std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
			shaderStage.module = VK_NULL_HANDLE;
		}
		assert(shaderStage.module != VK_NULL_HANDLE);
		return shaderStage;
	}

	void GLTFRenderer::PreparePipelines(VkRenderPass renderPass)
	{
		Pipes.skybox = std::make_shared<Pipeline>(
			"../shaders/pbr/skybox.vert.spv",
			"../shaders/pbr/skybox.frag.spv"
		);

		auto& skyboxConfig = Pipes.skybox->GetConfig();
		skyboxConfig.renderPass = renderPass;
		skyboxConfig.colorBlendInfo.attachmentCount = 2;
		skyboxConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; // Cull back faces
		skyboxConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		skyboxConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
		skyboxConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
		skyboxConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		skyboxConfig.depthStencilInfo.back = skyboxConfig.depthStencilInfo.back; // Enable depth test and write
		skyboxConfig.depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		skyboxConfig.bindingDescriptions = { { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
		skyboxConfig.attributeDescriptions = 
		{
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 }
		};

		// Pipeline layout
		std::vector<VkDescriptorSetLayout> setLayouts = {
			ModelDescriptorManager::GetModelDescriptorSetLayout()->getDescriptorSetLayout(),
			ModelDescriptorManager::GetMaterialDescriptorSetLayout()->getDescriptorSetLayout(),
			ModelDescriptorManager::GetNodeDescriptorSetLayout()->getDescriptorSetLayout()
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		pipelineLayoutCI.pSetLayouts = setLayouts.data();
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.size = sizeof(PushConstBlockMaterial);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pipelineLayoutCI.pushConstantRangeCount = 1;
		pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
		vkCreatePipelineLayout(device->device(), &pipelineLayoutCI, nullptr, &pipelineLayout);

		skyboxConfig.pipelineLayout = pipelineLayout;
		skyboxConfig.AddColorBlendAttachment();
		Pipes.skybox->Create();

		// PBR pipeline

		Pipes.pbr = std::make_shared<Pipeline>(
			"../shaders/pbr/pbr.vert.spv",
			"../shaders/pbr/pbr.frag.spv");
		auto& pbrConfig = Pipes.pbr->GetConfig();

		pbrConfig.renderPass = renderPass;
		pbrConfig.colorBlendInfo.attachmentCount = 2;
		pbrConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; // Cull back faces
		pbrConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pbrConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pbrConfig.depthStencilInfo.back = skyboxConfig.depthStencilInfo.back; // Enable depth test and write
		pbrConfig.depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		pbrConfig.bindingDescriptions = { { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
		pbrConfig.attributeDescriptions = 
		{
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 }
		};

		pbrConfig.AddColorBlendAttachment();
		Pipeline::EnableBlending(pbrConfig);

		setLayouts.push_back(depthBufferLayout);
		pipelineLayoutCI.pSetLayouts = setLayouts.data();
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		vkCreatePipelineLayout(device->device(), &pipelineLayoutCI, nullptr, &pipelineLayout);

		pbrConfig.pipelineLayout = pipelineLayout;

		pbrConfig.attributeDescriptions = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 },
			{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10 },
			{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14 },
			{ 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 18 }
		};

		pbrConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
		pbrConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
		Pipes.pbr->Create();
	}

	void GLTFRenderer::GenerateBRDFLUT()
	{
		auto tStart = std::chrono::high_resolution_clock::now();

		const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
		const int32_t dim = 1024;

		// Image
		VkImageCreateInfo imageCI{};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = format;
		imageCI.extent.width = dim;
		imageCI.extent.height = dim;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		vkCreateImage(device->device(), &imageCI, nullptr, &s_SceneInfo.textures.lutBrdf.m_Image);
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device->device(), s_SceneInfo.textures.lutBrdf.m_Image, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(device->device(), &memAllocInfo, nullptr, &s_SceneInfo.textures.lutBrdf.m_DeviceMemory);
		vkBindImageMemory(device->device(), s_SceneInfo.textures.lutBrdf.m_Image, s_SceneInfo.textures.lutBrdf.m_DeviceMemory, 0);

		// View
		VkImageViewCreateInfo viewCI{};
		viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCI.format = format;
		viewCI.subresourceRange = {};
		viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCI.subresourceRange.levelCount = 1;
		viewCI.subresourceRange.layerCount = 1;
		viewCI.image = s_SceneInfo.textures.lutBrdf.m_Image;
		vkCreateImageView(device->device(), &viewCI, nullptr, &s_SceneInfo.textures.lutBrdf.m_View);

		// Sampler
		VkSamplerCreateInfo samplerCI{};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 1.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vkCreateSampler(device->device(), &samplerCI, nullptr, &s_SceneInfo.textures.lutBrdf.m_Sampler);

		// FB, Att, RP, Pipe, etc.
		VkAttachmentDescription attDesc{};
		// Color attachment
		attDesc.format = format;
		attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attDesc;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = 2;
		renderPassCI.pDependencies = dependencies.data();

		VkRenderPass renderpass;
		vkCreateRenderPass(device->device(), &renderPassCI, nullptr, &renderpass);

		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderpass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &s_SceneInfo.textures.lutBrdf.m_View;
		framebufferCI.width = dim;
		framebufferCI.height = dim;
		framebufferCI.layers = 1;

		VkFramebuffer framebuffer;
		vkCreateFramebuffer(device->device(), &framebufferCI, nullptr, &framebuffer);

		// Desriptors
		VkDescriptorSetLayout descriptorsetlayout;
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vkCreateDescriptorSetLayout(device->device(), &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);

		// Pipeline layout
		VkPipelineLayout pipelinelayout;
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
		vkCreatePipelineLayout(device->device(), &pipelineLayoutCI, nullptr, &pipelinelayout);

		// Pipeline
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
		inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
		rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCI.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
		colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
		depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCI.depthTestEnable = VK_FALSE;
		depthStencilStateCI.depthWriteEnable = VK_FALSE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilStateCI.front = depthStencilStateCI.back;
		depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportStateCI{};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
		multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
		dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		VkPipelineVertexInputStateCreateInfo emptyInputStateCI{};
		emptyInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.layout = pipelinelayout;
		pipelineCI.renderPass = renderpass;
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pVertexInputState = &emptyInputStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderStages.data();

		// Look-up-table (from BRDF) pipeline		
		shaderStages = {
			loadShader(device->device(), "../shaders/pbr/genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			loadShader(device->device(), "../shaders/pbr/genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		VkPipeline pipeline;
		vkCreateGraphicsPipelines(device->device(), pipelineCache, 1, &pipelineCI, nullptr, &pipeline);
		for (auto shaderStage : shaderStages) {
			vkDestroyShaderModule(device->device(), shaderStage.module, nullptr);
		}

		// Render
		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderpass;
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = framebuffer;

		VkCommandBuffer cmdBuf = device->beginSingleTimeCommands();
		vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = (float)dim;
		viewport.height = (float)dim;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.extent.width = dim;
		scissor.extent.height = dim;

		vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(cmdBuf, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmdBuf);
		device->endSingleTimeCommands(cmdBuf);

		vkDestroyPipeline(device->device(), pipeline, nullptr);
		vkDestroyPipelineLayout(device->device(), pipelinelayout, nullptr);
		vkDestroyRenderPass(device->device(), renderpass, nullptr);
		vkDestroyFramebuffer(device->device(), framebuffer, nullptr);
		vkDestroyDescriptorSetLayout(device->device(), descriptorsetlayout, nullptr);

		s_SceneInfo.textures.lutBrdf.m_Descriptor.imageView = s_SceneInfo.textures.lutBrdf.m_View;
		s_SceneInfo.textures.lutBrdf.m_Descriptor.sampler = s_SceneInfo.textures.lutBrdf.m_Sampler;
		s_SceneInfo.textures.lutBrdf.m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		LOG_INFO("[Renderer] Generating BRDF LUT took {} ms", tDiff);
	}

	void GLTFRenderer::GenerateCubemaps()
	{
		enum Target { IRRADIANCE = 0, PREFILTEREDENV = 1 };

		for (uint32_t target = 0; target < PREFILTEREDENV + 1; target++) {

			TextureCubeMap cubemap{};

			auto tStart = std::chrono::high_resolution_clock::now();

			VkFormat format;
			int32_t dim;

			switch (target) {
			case IRRADIANCE:
				format = VK_FORMAT_R32G32B32A32_SFLOAT;
				dim = 64;
				break;
			case PREFILTEREDENV:
				format = VK_FORMAT_R16G16B16A16_SFLOAT;
				dim = 1024;
				break;
			};

			const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

			// Create target cubemap
			{
				// Image
				VkImageCreateInfo imageCI{};
				imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCI.imageType = VK_IMAGE_TYPE_2D;
				imageCI.format = format;
				imageCI.extent.width = dim;
				imageCI.extent.height = dim;
				imageCI.extent.depth = 1;
				imageCI.mipLevels = numMips;
				imageCI.arrayLayers = 6;
				imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
				imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
				NYXIS_ASSERT(vkCreateImage(device->device(), &imageCI, nullptr, &cubemap.m_Image) == VK_SUCCESS, "Failed to create cubemap m_Image!");

				VkMemoryRequirements memReqs;
				vkGetImageMemoryRequirements(device->device(), cubemap.m_Image, &memReqs);
				VkMemoryAllocateInfo memAllocInfo{};
				memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memAllocInfo.allocationSize = memReqs.size;
				memAllocInfo.memoryTypeIndex = device->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				vkAllocateMemory(device->device(), &memAllocInfo, nullptr, &cubemap.m_DeviceMemory);
				vkBindImageMemory(device->device(), cubemap.m_Image, cubemap.m_DeviceMemory, 0);

				// View
				VkImageViewCreateInfo viewCI{};
				viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				viewCI.format = format;
				viewCI.subresourceRange = {};
				viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewCI.subresourceRange.levelCount = numMips;
				viewCI.subresourceRange.layerCount = 6;
				viewCI.image = cubemap.m_Image;
				vkCreateImageView(device->device(), &viewCI, nullptr, &cubemap.m_View);

				// Sampler
				VkSamplerCreateInfo samplerCI{};
				samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerCI.magFilter = VK_FILTER_LINEAR;
				samplerCI.minFilter = VK_FILTER_LINEAR;
				samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerCI.minLod = 0.0f;
				samplerCI.maxLod = static_cast<float>(numMips);
				samplerCI.maxAnisotropy = 1.0f;
				samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				vkCreateSampler(device->device(), &samplerCI, nullptr, &cubemap.m_Sampler);
			}

			// FB, Att, RP, Pipe, etc.
			VkAttachmentDescription attDesc{};
			// Color attachment
			attDesc.format = format;
			attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			VkSubpassDescription subpassDescription{};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;

			// Use subpass dependencies for layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// Renderpass
			VkRenderPassCreateInfo renderPassCI{};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = 1;
			renderPassCI.pAttachments = &attDesc;
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDescription;
			renderPassCI.dependencyCount = 2;
			renderPassCI.pDependencies = dependencies.data();
			VkRenderPass renderpass;
			vkCreateRenderPass(device->device(), &renderPassCI, nullptr, &renderpass);

			struct Offscreen {
				VkImage image;
				VkImageView view;
				VkDeviceMemory memory;
				VkFramebuffer framebuffer;
			} offscreen;

			// Create offscreen framebuffer
			{
				// Image
				VkImageCreateInfo imageCI{};
				imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCI.imageType = VK_IMAGE_TYPE_2D;
				imageCI.format = format;
				imageCI.extent.width = dim;
				imageCI.extent.height = dim;
				imageCI.extent.depth = 1;
				imageCI.mipLevels = 1;
				imageCI.arrayLayers = 1;
				imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
				imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				vkCreateImage(device->device(), &imageCI, nullptr, &offscreen.image);
				VkMemoryRequirements memReqs;
				vkGetImageMemoryRequirements(device->device(), offscreen.image, &memReqs);
				VkMemoryAllocateInfo memAllocInfo{};
				memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memAllocInfo.allocationSize = memReqs.size;
				memAllocInfo.memoryTypeIndex = device->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				vkAllocateMemory(device->device(), &memAllocInfo, nullptr, &offscreen.memory);
				vkBindImageMemory(device->device(), offscreen.image, offscreen.memory, 0);

				// View
				VkImageViewCreateInfo viewCI{};
				viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewCI.format = format;
				viewCI.flags = 0;
				viewCI.subresourceRange = {};
				viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewCI.subresourceRange.baseMipLevel = 0;
				viewCI.subresourceRange.levelCount = 1;
				viewCI.subresourceRange.baseArrayLayer = 0;
				viewCI.subresourceRange.layerCount = 1;
				viewCI.image = offscreen.image;
				vkCreateImageView(device->device(), &viewCI, nullptr, &offscreen.view);

				// Framebuffer
				VkFramebufferCreateInfo framebufferCI{};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = renderpass;
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = &offscreen.view;
				framebufferCI.width = dim;
				framebufferCI.height = dim;
				framebufferCI.layers = 1;
				vkCreateFramebuffer(device->device(), &framebufferCI, nullptr, &offscreen.framebuffer);

				VkCommandBuffer layoutCmd = device->beginSingleTimeCommands();
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.image = offscreen.image;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
				vkCmdPipelineBarrier(layoutCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
				device->endSingleTimeCommands(layoutCmd);
			}

			// Descriptors
			VkDescriptorSetLayout descriptorsetlayout;
			VkDescriptorSetLayoutBinding setLayoutBinding = { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = &setLayoutBinding;
			descriptorSetLayoutCI.bindingCount = 1;
			vkCreateDescriptorSetLayout(device->device(), &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);

			// Descriptor Pool
			VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
			VkDescriptorPoolCreateInfo descriptorPoolCI{};
			descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCI.poolSizeCount = 1;
			descriptorPoolCI.pPoolSizes = &poolSize;
			descriptorPoolCI.maxSets = 2;
			VkDescriptorPool descriptorpool;
			vkCreateDescriptorPool(device->device(), &descriptorPoolCI, nullptr, &descriptorpool);

			// Descriptor sets
			VkDescriptorSet descriptorset;
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorpool;
			descriptorSetAllocInfo.pSetLayouts = &descriptorsetlayout;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			vkAllocateDescriptorSets(device->device(), &descriptorSetAllocInfo, &descriptorset);
			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.dstSet = descriptorset;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.pImageInfo = &s_SceneInfo.textures.environmentCube.m_Descriptor;
			vkUpdateDescriptorSets(device->device(), 1, &writeDescriptorSet, 0, nullptr);

			struct PushBlockIrradiance {
				glm::mat4 mvp;
				float deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
				float deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
			} pushBlockIrradiance;

			struct PushBlockPrefilterEnv {
				glm::mat4 mvp;
				float roughness;
				uint32_t numSamples = 32u;
			} pushBlockPrefilterEnv;

			// Pipeline layout
			VkPipelineLayout pipelinelayout;
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			switch (target) {
			case IRRADIANCE:
				pushConstantRange.size = sizeof(PushBlockIrradiance);
				break;
			case PREFILTEREDENV:
				pushConstantRange.size = sizeof(PushBlockPrefilterEnv);
				break;
			};

			VkPipelineLayoutCreateInfo pipelineLayoutCI{};
			pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCI.setLayoutCount = 1;
			pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
			pipelineLayoutCI.pushConstantRangeCount = 1;
			pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
			vkCreatePipelineLayout(device->device(), &pipelineLayoutCI, nullptr, &pipelinelayout);

			// Pipeline
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
			inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
			rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
			rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationStateCI.lineWidth = 1.0f;

			VkPipelineColorBlendAttachmentState blendAttachmentState{};
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachmentState.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
			colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCI.attachmentCount = 1;
			colorBlendStateCI.pAttachments = &blendAttachmentState;

			VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
			depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilStateCI.depthTestEnable = VK_FALSE;
			depthStencilStateCI.depthWriteEnable = VK_FALSE;
			depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilStateCI.front = depthStencilStateCI.back;
			depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

			VkPipelineViewportStateCreateInfo viewportStateCI{};
			viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportStateCI.viewportCount = 1;
			viewportStateCI.scissorCount = 1;

			VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
			multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicStateCI{};
			dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
			dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

			// Vertex input state
			VkVertexInputBindingDescription vertexInputBinding = { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };
			VkVertexInputAttributeDescription vertexInputAttribute = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 };

			VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
			vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
			vertexInputStateCI.vertexAttributeDescriptionCount = 1;
			vertexInputStateCI.pVertexAttributeDescriptions = &vertexInputAttribute;

			std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

			VkGraphicsPipelineCreateInfo pipelineCI{};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.layout = pipelinelayout;
			pipelineCI.renderPass = renderpass;
			pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
			pipelineCI.pVertexInputState = &vertexInputStateCI;
			pipelineCI.pRasterizationState = &rasterizationStateCI;
			pipelineCI.pColorBlendState = &colorBlendStateCI;
			pipelineCI.pMultisampleState = &multisampleStateCI;
			pipelineCI.pViewportState = &viewportStateCI;
			pipelineCI.pDepthStencilState = &depthStencilStateCI;
			pipelineCI.pDynamicState = &dynamicStateCI;
			pipelineCI.stageCount = 2;
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.renderPass = renderpass;

			shaderStages[0] = loadShader(device->device(), "../shaders/pbr/filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			switch (target) {
			case IRRADIANCE:
				shaderStages[1] = loadShader(device->device(), "../shaders/pbr/irradiancecube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
				break;
			case PREFILTEREDENV:
				shaderStages[1] = loadShader(device->device(), "../shaders/pbr/prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
				break;
			};
			VkPipeline pipeline;
			vkCreateGraphicsPipelines(device->device(), pipelineCache, 1, &pipelineCI, nullptr, &pipeline);
			for (auto shaderStage : shaderStages) {
				vkDestroyShaderModule(device->device(), shaderStage.module, nullptr);
			}

			// Render cubemap
			VkClearValue clearValues[1];
			clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderpass;
			renderPassBeginInfo.framebuffer = offscreen.framebuffer;
			renderPassBeginInfo.renderArea.extent.width = dim;
			renderPassBeginInfo.renderArea.extent.height = dim;
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = clearValues;

			std::vector<glm::mat4> matrices = {
				glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			};

			VkCommandBuffer cmdBuf = device->beginSingleTimeCommands();

			VkViewport viewport{};
			viewport.width = (float)dim;
			viewport.height = (float)dim;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.extent.width = dim;
			scissor.extent.height = dim;

			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = numMips;
			subresourceRange.layerCount = 6;

			// Change image layout for all cubemap faces to transfer destination
			{
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.image = cubemap.m_Image;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.subresourceRange = subresourceRange;
				vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
			}

			device->endSingleTimeCommands(cmdBuf);

			for (uint32_t m = 0; m < numMips; m++) {
				for (uint32_t f = 0; f < 6; f++) {

					auto cmdBuf = device->beginSingleTimeCommands();

					viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
					viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
					vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
					vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

					// Render scene from cube face's point of view
					vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

					// Pass parameters for current pass using a push constant block
					switch (target) {
					case IRRADIANCE:
						pushBlockIrradiance.mvp = glm::perspective((float)(3.14159265358979323846 / 2.0), 1.0f, 0.1f, 1024.0f) * matrices[f];
						vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockIrradiance), &pushBlockIrradiance);
						break;
					case PREFILTEREDENV:
						pushBlockPrefilterEnv.mvp = glm::perspective((float)(3.14159265358979323846 / 2.0), 1.0f, 0.1f, 1024.0f) * matrices[f];
						pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);
						vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockPrefilterEnv), &pushBlockPrefilterEnv);
						break;
					}

					vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
					vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);

					VkDeviceSize offsets[1] = { 0 };

					skybox->draw(cmdBuf);

					vkCmdEndRenderPass(cmdBuf);

					VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
					subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					subresourceRange.baseMipLevel = 0;
					subresourceRange.levelCount = numMips;
					subresourceRange.layerCount = 6;

					{
						VkImageMemoryBarrier imageMemoryBarrier{};
						imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						imageMemoryBarrier.image = offscreen.image;
						imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
						vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
					}

					// Copy region for transfer from framebuffer to cube face
					VkImageCopy copyRegion{};

					copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copyRegion.srcSubresource.baseArrayLayer = 0;
					copyRegion.srcSubresource.mipLevel = 0;
					copyRegion.srcSubresource.layerCount = 1;
					copyRegion.srcOffset = { 0, 0, 0 };

					copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copyRegion.dstSubresource.baseArrayLayer = f;
					copyRegion.dstSubresource.mipLevel = m;
					copyRegion.dstSubresource.layerCount = 1;
					copyRegion.dstOffset = { 0, 0, 0 };

					copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
					copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
					copyRegion.extent.depth = 1;

					vkCmdCopyImage(
						cmdBuf,
						offscreen.image,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						cubemap.m_Image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1,
						&copyRegion);

					{
						VkImageMemoryBarrier imageMemoryBarrier{};
						imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						imageMemoryBarrier.image = offscreen.image;
						imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
						vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
					}

					device->endSingleTimeCommands(cmdBuf);
				}
			}

			{
				cmdBuf = device->beginSingleTimeCommands();
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.image = cubemap.m_Image;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.subresourceRange = subresourceRange;
				vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
				device->endSingleTimeCommands(cmdBuf);
			}


			vkDestroyRenderPass(device->device(), renderpass, nullptr);
			vkDestroyFramebuffer(device->device(), offscreen.framebuffer, nullptr);
			vkFreeMemory(device->device(), offscreen.memory, nullptr);
			vkDestroyImageView(device->device(), offscreen.view, nullptr);
			vkDestroyImage(device->device(), offscreen.image, nullptr);
			vkDestroyDescriptorPool(device->device(), descriptorpool, nullptr);
			vkDestroyDescriptorSetLayout(device->device(), descriptorsetlayout, nullptr);
			vkDestroyPipeline(device->device(), pipeline, nullptr);
			vkDestroyPipelineLayout(device->device(), pipelinelayout, nullptr);

			cubemap.m_Descriptor.imageView = cubemap.m_View;
			cubemap.m_Descriptor.sampler = cubemap.m_Sampler;
			cubemap.m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			switch (target) {
			case IRRADIANCE:
				s_SceneInfo.textures.irradianceCube = cubemap;
				break;
			case PREFILTEREDENV:
				s_SceneInfo.textures.prefilteredCube = cubemap;
				s_SceneInfo.shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(numMips);
				break;
			}

			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
			LOG_INFO("[Renderer] Generating cube map with {} mip levels took {} ms", numMips, tDiff);
		}
	}


}
