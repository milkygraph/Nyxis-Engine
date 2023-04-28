#include "NyxisUI/ComponentViewPanel.hpp"
#include "EditorLayer.hpp"
#include "Core/Application.hpp"
#include "Core/GLTFRenderer.hpp"
#include "Core/SwapChain.hpp"
#include "Graphics/GLTFModel.hpp"
#include "Scene/Components.hpp"

namespace Nyxis
{
	void ComponentViewPanel::OnUpdate()
	{
		auto scene = Application::GetScene();
		auto selectedEntity = EditorLayer::GetSelectedEntity();

		ImGui::Begin("Component");
		// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		if (scene->m_Registry.valid(selectedEntity))
		{
			if (scene->m_Registry.all_of<TagComponent>(selectedEntity))
			{
				auto& tag = scene->GetComponent<TagComponent>(selectedEntity);
				char buffer[256] = {};
				strcpy_s(buffer, tag.Tag.c_str());
				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					tag.Tag = std::string(buffer);
				}
			}

			if (scene->m_Registry.all_of<RigidBody>(selectedEntity))
			{
				auto& rigidBody = scene->GetComponent<RigidBody>(
					selectedEntity); // TODO! Fix load scene bug
				ImGui::Text("Rigid Body");
				ImGui::DragFloat3("Position", &rigidBody.translation.x, 0.1f, 0, 0, "%.1f");
				glm::vec3 rotationDeg = glm::degrees(rigidBody.rotation);
				ImGui::DragFloat3("Rotation", &rotationDeg.x, 0.1f, 0, 0, "%.1f");
				rigidBody.rotation = glm::radians(rotationDeg);
				ImGui::DragFloat3("Scale", &rigidBody.scale.x, 0.1f, 0, 0, "%.2f");

				ImGui::DragFloat3("Velocity", &rigidBody.velocity.x, 0.1f);
				ImGui::DragFloat("Restitution", &rigidBody.restitution, 0.1f, 0.0f, 1.0f);

				ImGui::DragFloat("Roughness", &rigidBody.roughness, 0.1f);
				// check if entity has a collider component
				if (scene->m_Registry.all_of<Collider>(selectedEntity))
				{
					auto& collider = scene->GetComponent<Collider>(selectedEntity);

					ImGui::Text("Collider");
					auto preview = collider_name[collider.type];

					if (ImGui::BeginCombo("Collider Type", preview.c_str(), ImGuiComboFlags_NoArrowButton))
					{
						if (ImGui::Selectable("Box"))
							collider.type = ColliderType::Box;

						if (ImGui::Selectable("Sphere"))
							collider.type = ColliderType::Sphere;

						ImGui::EndCombo();
					}

					if (collider.type == ColliderType::Box)
						ImGui::DragFloat3("Collider Size", &collider.size.x, 0.05f);

					else if (collider.type == ColliderType::Sphere)
						ImGui::DragFloat("Collider Radius", &collider.radius, 0.05f);
				}
			}
		}
		ImGui::End();
	}
}
