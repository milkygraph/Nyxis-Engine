#include "AssimpModel.hpp"
#include "Log.hpp"
#include "ve.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
namespace ve
{
	AssimpModel::AssimpModel(std::vector<Mesh> meshes)  : meshes(meshes) {}

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
		meshes.reserve(scene->mNumMeshes);
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

		return VE_MAKE_REF(AssimpModel, meshes);
	}
}