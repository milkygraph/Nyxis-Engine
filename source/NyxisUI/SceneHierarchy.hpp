#pragma once
#include "Scene/Scene.hpp"
#include "Core/Layer.hpp"

namespace Nyxis
{
	struct Node;

	class SceneHierarchyPanel final : public Layer
	{
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() = default;

		void OnAttach() override {}
		void OnDetach() override {}
		void OnUpdate() override;
		void OnEvent(Event& event) override;
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
