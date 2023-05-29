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
		// wait to finish all rendering
		vkDeviceWaitIdle(Device::Get().device());

		// TODO: Load project from path file
		std::ifstream stream(m_Path);
		std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

		json j = json::parse(file);
		m_Name = j["name"];
		m_AssetsPath = j["assetsPath"];
		m_ScenesPath = j["scenesPath"];

		for(auto scene : j["Scenes"])
		{
			auto newScene = std::make_shared<Scene>(scene["Name"]);
			for(auto entity : scene["Entities"])
			{
				auto newEntity = newScene->CreateEntity(entity["Tag"]);
				newScene->AddComponent<Model>(newEntity, entity["Model"]);
				auto& transform = newScene->GetComponent<TransformComponent>(newEntity);
				auto& transformJson = entity["Transform"];
				transform.translation.x = transformJson["Position"][0];
				transform.translation.y = transformJson["Position"][1];
				transform.translation.z = transformJson["Position"][2];
				transform.rotation.x = transformJson["Rotation"][0];
				transform.rotation.y = transformJson["Rotation"][1];
				transform.rotation.z = transformJson["Rotation"][2];
				transform.scale.x = transformJson["Scale"][0];
				transform.scale.y = transformJson["Scale"][1];
				transform.scale.z = transformJson["Scale"][2];
			}

			auto camera = newScene->GetCameraEntity();
			auto& transform = newScene->GetComponent<TransformComponent>(camera);
			transform.translation.x = scene["Editor Camera"]["Transform"]["Position"][0];
			transform.translation.y = scene["Editor Camera"]["Transform"]["Position"][1];
			transform.translation.z = scene["Editor Camera"]["Transform"]["Position"][2];
			transform.rotation.x = scene["Editor Camera"]["Transform"]["Rotation"][0];
			transform.rotation.y = scene["Editor Camera"]["Transform"]["Rotation"][1];
			transform.rotation.z = scene["Editor Camera"]["Transform"]["Rotation"][2];
			transform.scale.x = scene["Editor Camera"]["Transform"]["Scale"][0];
			transform.scale.y = scene["Editor Camera"]["Transform"]["Scale"][1];
			transform.scale.z = scene["Editor Camera"]["Transform"]["Scale"][2];

			m_Scenes.push_back(newScene);
		}

		Application::SetScene(m_Scenes.back());
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
		j["Scenes"] = json::array();

		for(auto& scene : m_Scenes)
		{
			json sceneJson;
			sceneJson["Entities"] = json::array();
			for(auto&& [entity, model, transform] : scene->m_Registry.view<Model, TransformComponent>().each())
			{
				json entityJson;
				entityJson["Transform"] = 
				{
						{"Position", {transform.translation.x, transform.translation.y, transform.translation.z}},
						{"Rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z}},
						{"Scale", {transform.scale.x, transform.scale.y, transform.scale.z}}
				};

				entityJson["Tag"] = scene->GetComponent<TagComponent>(entity).Tag;
				entityJson["Model"] = model.path;
				sceneJson["Entities"].push_back(entityJson);
			}

			// Editor Camera
			auto cameraEntity = scene->GetCameraEntity();
			auto& transform = scene->GetComponent<TransformComponent>(cameraEntity);
			sceneJson["Editor Camera"]["Transform"] =
			{
					{"Position", {transform.translation.x, transform.translation.y, transform.translation.z}},
					{"Rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z}},
					{"Scale", {transform.scale.x, transform.scale.y, transform.scale.z}}
			};

			sceneJson["Name"] = scene->GetSceneName();
			j["Scenes"].push_back(sceneJson);
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
