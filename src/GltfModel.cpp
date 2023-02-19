//#include "GltfModel.hpp"
//#include "Log.hpp"
//
//namespace Nyxis
//{
//	static std::string GetFilePathExtension(const std::string& FileName) {
//		if (FileName.find_last_of(".") != std::string::npos)
//			return FileName.substr(FileName.find_last_of(".") + 1);
//		return "";
//	}
//
//	GLTFModel::GLTFModel(const std::string& filepath)
//	{
//		tinygltf::TinyGLTF loader;
//		std::string err;
//		std::string warn;
//
//		std::string ext = GetFilePathExtension(filepath);
//		
//		bool ret = false;
//
//		if (ext.compare("glb") == 0) {
//			// assume binary glTF.
//			ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath.c_str());
//		}
//		else {
//			// assume ascii glTF.
//			ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath.c_str());
//		}
//
//		if (!warn.empty())
//		{
//			LOG_WARN(warn);
//		}
//
//		if (!err.empty())
//		{
//			LOG_ERROR(err);
//		}
//
//		if (!ret)
//		{
//			LOG_ERROR("Failed to load glTF model");
//			return;
//		}
//		
//		CreateBuffers();
//	}
//
//	GLTFModel::~GLTFModel()
//	{
//	}
//
//	VE_REF(GLTFModel) GLTFModel::LoadFromFile(const std::string& filepath)
//	{
//		return VE_MAKE_REF(GLTFModel, filepath);
//	}
//
//	GLTFModel::CreateBuffers()
//	{
//		for (auto& mesh : model.meshes)
//		{
//			for (auto& primitive : mesh.primitives)
//			{
//				// get vertex attributes
//				auto& positionAccessor = model.accessors[primitive.attributes["POSITION"]];
//				auto& positionBufferView = model.bufferViews[positionAccessor.bufferView];
//				
//			}
//		}
//	}
//}
