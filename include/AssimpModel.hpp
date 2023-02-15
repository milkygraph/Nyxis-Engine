#pragma once

#include "buffer.hpp"

#include "ve.hpp"
#include "vepch.hpp"

namespace ve
{
	class AssimpModel {

		using Color = glm::vec4;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
		};

		struct Material
		{
			Color specular;
		    Color diffuse;
		    Color ambient;
		};

		struct Mesh
		{
            std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
		};

	public:
		AssimpModel(std::vector<Mesh> meshes);
		~AssimpModel();

		static VE_REF(AssimpModel) LoadFromFile(const std::string &path);

	private:
		std::vector<Mesh> meshes;
	    VE_REF(Buffer) vertexBuffer;
		VE_REF(Buffer) indexBuffer;
	};
}