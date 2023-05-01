#pragma once
#include "Scene/Scene.hpp"

namespace Nyxis
{
	struct Node;

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() = default;
		void OnUpdate();
		void DrawEntityNode(Entity entity);
		void DrawNode(Node* node);

		template<typename T>
		void AddComponent(const char* name, Entity entity);

		template<typename T>
		void RemoveComponent(const char* name, Entity entity);
	private:
		Node* m_SelectedNode = nullptr;
	};

}
