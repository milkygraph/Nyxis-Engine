#include "NyxisUI/SceneHierarchy.hpp"
#include "Core/Application.hpp"

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
			if (ImGui::MenuItem("Create Empty Entity"))
				EditorLayer::SetSelectedEntity(scene->createEntity("Empty Entity"));
			if (ImGui::MenuItem("Clear Scene"))
				scene->ClearScene();

			ImGui::EndPopup();
		}

		scene->m_Registry.each([&](auto entityID){ DrawEntityNode(entityID); });

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity) const
	{
		const auto scene = Application::GetScene();
		const auto& tag = scene->GetComponent<TagComponent>(entity).Tag;
		const auto selectedEntity = EditorLayer::GetSelectedEntity();

		ImGuiTreeNodeFlags flags = ((selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
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
				scene->destroyEntity(entity);
			}
			// add component tree with all components that can be added to entity
			if (ImGui::BeginMenu("Add Component"))
			{
				if (ImGui::MenuItem("RigidBody"))
					if (!scene->m_Registry.all_of<RigidBody>(entity))
						scene->addComponent<RigidBody>(entity);

				if (ImGui::MenuItem("Mesh"))
					if (!scene->m_Registry.all_of<MeshComponent>(entity))
						scene->addComponent<MeshComponent>(entity, "../models/sphere.obj");

				if (ImGui::MenuItem("Collider"))
					if (!scene->m_Registry.all_of<Collider>(entity))
						scene->addComponent<Collider>(entity, ColliderType::Sphere, glm::vec3{ 0.2, 0.2, 0.2 },
							0.05);

				if (ImGui::MenuItem("Gravity"))
					if (!scene->m_Registry.all_of<Gravity>(entity))
						scene->addComponent<Gravity>(entity);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Remove Component"))
			{
				if (scene->m_Registry.all_of<RigidBody>(entity))
					if (ImGui::MenuItem("RigidBody"))
						scene->m_Registry.remove<RigidBody>(entity);

				if (scene->m_Registry.all_of<MeshComponent>(entity))
					if (ImGui::MenuItem("Mesh"))
						scene->m_Registry.remove<MeshComponent>(entity);

				if (scene->m_Registry.all_of<Collider>(entity))
					if (ImGui::MenuItem("Collider"))
						scene->m_Registry.remove<Collider>(entity);

				if (scene->m_Registry.all_of<Gravity>(entity))
					if (ImGui::MenuItem("Gravity"))
						scene->m_Registry.remove<Gravity>(entity);

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (expanded)
		{
			ImGui::TreePop();
		}
	}
}
