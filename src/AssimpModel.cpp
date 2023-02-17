#include "AssimpModel.hpp"
#include "Log.hpp"
#include "ve.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
namespace ve
{
	AssimpModel::AssimpModel(std::vector<Mesh> meshes, std::vector<Material> materials)  
		: meshes(meshes), materials(materials)
	{
		CreateBuffers();
	}

	AssimpModel::~AssimpModel()
	{
	}

	VE_REF(AssimpModel) AssimpModel::LoadFromFile(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR("AssimpModel::LoadFromFile: Error loading model: {0}", importer.GetErrorString());
			return nullptr;
		}

		// Load all the meshes in the model
		std::vector<Mesh> meshes;
		std::vector<Material> materials;
		meshes.reserve(scene->mNumMeshes);
		materials.reserve(scene->mNumMaterials);
		
		for (unsigned int meshID = 0; meshID < scene->mNumMeshes; meshID++)
		{
			aiMesh* mesh = scene->mMeshes[meshID];

			std::vector<Vertex> vertices;
			vertices.reserve(mesh->mNumVertices);

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiColor4D  specularColor;
			aiColor4D  diffuseColor;
			aiColor4D  ambientColor;
			float shininess;

			aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor);
			aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);

			AssimpModel::Material meshMaterial = {
				glm::vec4(specularColor.r, specularColor.g, specularColor.b, shininess),
				glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a),
				glm::vec4(ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a),
			};

			materials.push_back(meshMaterial);

			for(uint32_t vertID = 0u; vertID < mesh->mNumVertices; vertID++)
			{
				aiVector3D position = mesh->mVertices[vertID];
				aiVector3D normal = mesh->mNormals[vertID];

				AssimpModel::Vertex vertex = {
					glm::vec3(position.x, position.y, position.z),
					glm::vec3(normal.x, normal.y, normal.z)
				};
			}

			// Load the indices
			std::vector<uint32_t> indices;
			indices.reserve(mesh->mNumFaces * 3); // 3 indices per face
			for(uint32_t faceID = 0u; faceID < mesh->mNumFaces; faceID++)
			{
				VE_ASSERT(mesh->mFaces[faceID].mNumIndices == 3, ("FaceID {} of mesh does not have 3 indices!", faceID));

				indices.push_back(mesh->mFaces[faceID].mIndices[0]);
				indices.push_back(mesh->mFaces[faceID].mIndices[1]);
				indices.push_back(mesh->mFaces[faceID].mIndices[2]);
			}
		}
		// Release imported data
		importer.FreeScene();

		return VE_MAKE_REF(AssimpModel, meshes, materials);
	}
	void AssimpModel::CreateBuffers()
	{
		vertexBuffers.reserve(meshes.size());
		indexBuffers.reserve(meshes.size());
		for (uint32_t i = 0; i < meshes.size(); i++) 
		{
			// Create vertex buffer
			{
				VE_ASSERT(meshes[i].vertices.size() > 0, ("Mesh {} has no vertices!", i));
				VkDeviceSize bufferSize = sizeof(meshes[i].vertices[0]) * meshes[i].vertices.size();

				uint32_t vertexSize = sizeof(meshes[i].vertices[0]);
				Buffer stagingBuffer{ vertexSize, static_cast<uint32_t>(meshes[i].vertices.size()), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };

				stagingBuffer.map();
				stagingBuffer.writeToBuffer((void*)meshes[i].vertices.data());

				auto vertexBuffer = VE_MAKE_REF(Buffer, vertexSize, static_cast<uint32_t>(meshes[i].vertices.size()), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

				vertexBuffers.push_back(vertexBuffer);
			}

			// Create index buffer
			{
				VE_ASSERT(meshes[i].indices.size() > 0, ("Mesh {} has no indices!", i));
				VkDeviceSize bufferSize = sizeof(meshes[i].indices[0]) * meshes[i].indices.size();

				uint32_t indexSize = sizeof(meshes[i].indices[0]);
				Buffer stagingBuffer{ indexSize, static_cast<uint32_t>(meshes[i].indices.size()), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };

				stagingBuffer.map();
				stagingBuffer.writeToBuffer((void*)meshes[i].indices.data());

				auto indexBuffer = VE_MAKE_REF(Buffer, indexSize, static_cast<uint32_t>(meshes[i].indices.size()), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

				indexBuffers.push_back(indexBuffer);
			}
		}
	
	}
}