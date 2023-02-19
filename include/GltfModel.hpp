//#include "device.hpp"
//#include "buffer.hpp"
//#include "tinygltf/tiny_gltf.h"
//
//#include "ve.hpp"
//#include "vepch.hpp"
//
//namespace Nyxis
//{
//	struct Material
//	{
//		glm::vec4 baseColorFactor;
//		glm::vec3 emissiveFactor;
//		float metallicFactor;
//		float roughnessFactor;
//		float alphaCutoff;
//		bool doubleSided;
//	};
//	
//	class GLTFModel
//	{
//	public:
//		GLTFModel(const std::string& path);
//		~GLTFModel();
//
//		static VE_REF(GLTFModel) LoadFromFile(const std::string& path);
//		void CreateBuffers();
//
//		std::vector<VE_REF(Buffer)> vertexBuffers;
//		std::vector<VE_REF(Buffer)> indexBuffers;
//		std::vector<VE_REF(Buffer)> materialBuffers;
//		
//	private:
//		Device& device = Device::get();
//		std::string directory;
//		tinygltf::Model model;
//	};
//}
