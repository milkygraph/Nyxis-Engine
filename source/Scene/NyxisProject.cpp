#include "Scene/NyxisProject.hpp"

#include "Core/Application.hpp"
#include "Core/Log.hpp"
#include "json/json.hpp"

namespace Nyxis
{
	using json = nlohmann::json;

	NyxisProject::NyxisProject(const std::string& name, const std::string& path)
		: m_Path(path), m_Name(name) {}

	
	/**
	 * \brief Load a project from path file
	 */
	void NyxisProject::Load()
	{
		// TODO: Load project from path file
	}

	/**
	 * \brief Save the project to path file
	 */
	void NyxisProject::Save() const
	{
		auto scene = Application::GetScene();

		std::ifstream stream(m_Path);
		std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

		json j = json::parse(file);
		j["name"] = m_Name;
		j["assetPath"] = m_AssetsPath;
		j["scenesPath"] = m_ScenesPath;
		
		for(auto& scene : m_Scenes)
		{
			json sceneJson;
			sceneJson["Entities"] = json::array();
			for(auto&& [entity, model, transform] : scene->m_Registry.view<Model, TransformComponent>().each())
			{
				json entityJson;
				json transformJson = {
						{"Position", {transform.translation.x, transform.translation.y, transform.translation.z}},
						{"Rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z}},
						{"Scale", {transform.scale.x, transform.scale.y, transform.scale.z}}
				};

				entityJson["Tag"] = scene->GetComponent<TagComponent>(entity).Tag;
				entityJson["Transform"] = transformJson;
				entityJson["Model"] = model.path;
				sceneJson["Entities"].push_back(entityJson);
			}
			j["Scenes"][scene->GetSceneName().c_str()] = sceneJson;
		}

		std::ofstream output_stream(m_Path);
		output_stream << std::setw(4) << j << std::endl;
		output_stream.close();
		LOG_INFO("Saved project to {}", m_Path);
	}

	const std::string& NyxisProject::GetAssetPath() const
	{
		return m_AssetsPath;
	}

	/**
	 * \brief Create a new project
	 */
	void NyxisProject::Create()
	{
		m_AssetsPath = "../assets";
		m_ScenesPath = "../scenes";

		json j;
		j["name"] = m_Name;
		j["assetsPath"] = "../assets";
		j["scenesPath"] = "../scenes";
		std::ofstream o(m_Path);
		o << std::setw(4) << j << std::endl;
		o.close();
	}
}
