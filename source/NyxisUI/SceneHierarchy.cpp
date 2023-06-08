#include "NyxisUI/SceneHierarchy.hpp"
#include "Core/Application.hpp"
#include "Graphics/GLTFModel.hpp"
#include "Scene/NyxisProject.hpp"

namespace Nyxis
{
	void SceneHierarchyPanel::OnUpdate()
	{
		const auto scene = Application::GetScene();

		ImGui::Begin("Scene Hierarchy");
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			EditorLayer::DeselectEntity();
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if(ImGui::BeginMenu("CreateEntity"))
			{
				if(ImGui::MenuItem("Create Empty Entity"))
					EditorLayer::SetSelectedEntity(scene->CreateEntity("Empty Entity"));
				if(ImGui::MenuItem("Sphere"))
				{
					auto assets_path = Application::GetProject()->GetAssetPath();
					auto entity = scene->CreateEntity("Sphere");
					scene->AddComponent<Model>(entity, "/models/basic/sphere.gltf");
				}
				if(ImGui::MenuItem("Cube"))
				{
					auto assets_path = Application::GetProject()->GetAssetPath();
					auto entity = scene->CreateEntity("Cube");
					scene->AddComponent<Model>(entity, "/models/basic/cube.gltf");
				}
				if(ImGui::MenuItem("Plane"))
				{
					auto assets_path = Application::GetProject()->GetAssetPath();
					auto entity = scene->CreateEntity("Plane");
					scene->AddComponent<Model>(entity, "/models/basic/plane.gltf");
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Clear Scene"))
				scene->ClearScene();

			ImGui::EndPopup();
		}

		scene->m_Registry.each([&](auto entityID){ DrawEntityNode(entityID); });

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity) 
	{
		const auto scene = Application::GetScene();
		const auto& tag = scene->GetComponent<TagComponent>(entity).Tag;
		const auto selectedEntity = EditorLayer::GetSelectedEntity();

		ImGuiTreeNodeFlags flags = ((selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | 
			ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool expanded = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, "%s", tag.c_str());

		if (ImGui::IsItemClicked())
		{
			EditorLayer::SetSelectedEntity(entity);
		}

		// right click context menu
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				scene->DestroyEntity(entity);
				EditorLayer::DeselectEntity();
			}
			// add component tree with all components that can be added to entity
			if (ImGui::BeginMenu("Add Component"))
			{
				AddComponent<RigidBody>("RigidBody", selectedEntity);
				AddComponent<Model>("Model", selectedEntity);
				AddComponent<Collider>("Collider", selectedEntity);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Remove Component"))
			{
				RemoveComponent<RigidBody>("RigidBody", selectedEntity);
				RemoveComponent<Model>("Model", selectedEntity);
				RemoveComponent<Collider>("Collider", selectedEntity);

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (expanded)
		{
			if (scene->m_Registry.all_of<Model>(entity))
			{
				auto& model = scene->GetComponent<Model>(entity);
				for (auto& node : model.nodes)
				{
					DrawNode(node);
				}
			}
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DrawNode(Node* node)
	{
		ImGuiTreeNodeFlags flags = ((m_SelectedNode == node) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (ImGui::TreeNodeEx(node->name.c_str(), flags))
		{
			for (auto* child : node->children)
				DrawNode(child);
			m_SelectedNode = node;
			ImGui::TreePop();
		}
	}

	template<typename T>
	void SceneHierarchyPanel::AddComponent(const char* name, Entity entity)
	{
		if(ImGui::MenuItem(name))
		{
			const auto scene = Application::GetScene();
			if (!scene->m_Registry.all_of<T>(entity))
				scene->AddComponent<T>(entity);
			ImGui::CloseCurrentPopup();
		}
	}

	template<typename T>
	void SceneHierarchyPanel::RemoveComponent(const char* name, Entity entity)
	{
		if(ImGui::MenuItem(name))
		{
			const auto scene = Application::GetScene();
			if (scene->m_Registry.all_of<T>(entity))
				scene->RemoveComponent<T>(entity);
			ImGui::CloseCurrentPopup();
		}
	}	
}
