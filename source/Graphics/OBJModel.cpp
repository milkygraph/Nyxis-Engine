#include "Graphics/OBJModel.hpp"
#include "Core/Log.hpp"
#include "Core/Nyxis.hpp"
#include "Utils/Utils.hpp"
#include "Core/Nyxispch.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace std
{
    template <>
    struct hash<Nyxis::OBJModel::Vertex>
    {
        size_t operator()(Nyxis::OBJModel::Vertex const &vertex) const
        {
            size_t seed = 0;
            Nyxis::hashCombine(seed, vertex.position, vertex.color, vertex.normal);
            return seed;
        }
    };
}

namespace Nyxis
{
	std::unordered_map<std::string, std::shared_ptr<OBJModel>> OBJModel::models;
    OBJModel::OBJModel(const std::string& filepath)
    {
//        createVertexBuffers(builder.vertices);
//        createIndexBuffers(builder.indices);
		builder = new Builder(filepath);
	}

    OBJModel::~OBJModel() {}

//    std::shared_ptr<OBJModel> OBJModel::createModelFromFile(const std::string &filepath)
//    {
//        Builder builder{};
//        builder.loadModel(filepath);
//        #ifdef LOGGING
//		LOG_INFO("Model {} loaded, {} vertices", filepath, builder.vertices.size());
//        #endif // LOGGING
//
//        return std::make_shared<OBJModel>(builder);
//    }

    void OBJModel::createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex size must be at least 3!");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        
        
        // add these buffer to fist copy to a temporary buffer and then a more efficient
        // gpu buffer for optimal performance
        uint32_t vertexSize = sizeof(vertices[0]);
        Buffer stagingBuffer{vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) vertices.data());

        vertexBuffer = std::make_unique<Buffer>(vertexSize, vertexCount,
                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
        device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void OBJModel::createIndexBuffers(const std::vector<uint32_t> &indices)
    {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if (!hasIndexBuffer)
            return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{indexSize, indexCount,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT};
        
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) indices.data());

        indexBuffer = std::make_unique<Buffer>(indexSize, indexCount,
                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

	void OBJModel::loadModel()
	{
        if (!loaded)
		{
            if (builder == nullptr)
                throw std::runtime_error("Builder is not initialized yet!");

            builder->loadModel();
            createVertexBuffers(builder->vertices);
            createIndexBuffers(builder->indices);
            loaded = true;
            delete builder;
            builder = nullptr;
        }
	}

    void OBJModel::draw(VkCommandBuffer commandBuffer) const
    {
		if (hasIndexBuffer)
			vkCmdDrawIndexed (commandBuffer, indexCount, 1, 0, 0, 0);
		else
			vkCmdDraw (commandBuffer, vertexCount, 1, 0, 0);
    }

    void OBJModel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer)
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    std::vector<VkVertexInputBindingDescription> OBJModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescription(1);
        bindingDescription[0].binding = 0;
        bindingDescription[0].stride = sizeof(Vertex);
        bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> OBJModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions({});

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    void OBJModel::Builder::loadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

//        for (const auto &shape : shapes
	    std::for_each(std::execution::par, shapes.begin(), shapes.end(), [&](const auto &shape)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        });
		#ifdef LOGGING
	    LOG_INFO("Model {} loaded, {} vertices", filepath, vertices.size());
		#endif // LOGGING
    }
}