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
	private:
		Node* m_SelectedNode = nullptr;
	};

}
