#pragma once

#include "buffer.hpp"

#include "Nyxis.hpp"
#include "Nyxispch.hpp"

namespace Nyxis
{
	class AssimpModel {

		using Color = glm::vec4;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texCoord;
		};

		struct Material
		{
			Color specular;
		    Color diffuse;
		    Color ambient;
		};

		struct Texture
		{
			VkSampler sampler;
			VkImageView imageView;
			VkImage image;
			VkDeviceMemory memory;
		};
		
		struct Mesh
		{
            std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
		};

	public:
		AssimpModel(std::vector<Mesh> meshes, std::vector<Material> materials);
		~AssimpModel();

		static REF(AssimpModel) LoadFromFile(const std::string &path);

		void CreateBuffers();
		
	private:

		Device& device = Device::get();
		
		std::vector<Mesh> meshes;
		std::vector<Material> materials;
		std::vector<Texture> textures;

	    std::vector<REF(Buffer)> vertexBuffers;
		std::vector<REF(Buffer)> indexBuffers;
		std::vector<REF(Buffer)> materialBuffers;
	};
}