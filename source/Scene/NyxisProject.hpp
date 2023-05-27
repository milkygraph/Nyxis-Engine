#pragma once
#include "Core/Nyxis.hpp"
#include "Core/Nyxispch.hpp"


namespace Nyxis
{
	class Scene;

	/**
	 * \brief A project is a collection of scenes, assets, and other data.
	 */
	class NyxisProject
	{
	public:
		NyxisProject(const std::string& name, const std::string& path);
		~NyxisProject() = default;
		void Create();
		void Load();
		void Save() const;
		void SetAssetsPath(const std::string& path) { m_AssetsPath = path; }
		void SetScenesPath(const std::string& path) { m_ScenesPath = path; }
		const std::string& GetAssetPath() const;

		void AddScene(const Ref<Scene>& scene) { m_Scenes.push_back(scene); }

	private:
		const std::string m_Path;
		std::string m_Name;
		std::string m_AssetsPath;
		std::string m_ScenesPath;

		std::vector<Ref<Scene>> m_Scenes;
	};
}
