#include "NyxisUI/ComponentViewPanel.hpp"
#include "EditorLayer.hpp"
#include "Core/Application.hpp"
#include "Core/GLTFRenderer.hpp"
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
			ImGui::PushStyleColor(ImGuiCol_Separator, { 124, 124, 124, 1 });

			DrawComponentNode<TransformComponent>("Transform", selectedEntity, [&](TransformComponent& transform)
				{
					ImGui::DragFloat3("Translation", &transform.translation.x, 0.1f, 0, 0, "%.1f");
					ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f, 0, 0, "%.1f");
					ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f, 0, 0, "%.1f");
					ImGui::DragFloat3("Velocity", &transform.velocity.x, 0.1, 0, 0, "%.1f");
					ImGui::DragFloat3("Acceleration", &transform.acceleration.x, 0.1, 0, 0, "%.1f");
				});

			DrawComponentNode<RigidBody>("RigidBody", selectedEntity, [&](RigidBody& rigidBody)
				{
					ImGui::DragFloat("Mass", &rigidBody.mass, 0.1f, 0, 0, "%.1f");
					ImGui::DragFloat("Restitution", &rigidBody.restitution, 0.1f, 0, 0, "%.1f");
					ImGui::DragFloat("Friction", &rigidBody.friction, 0.1f, 0, 0, "%.1f");
					ImGui::Checkbox("Kinematic", &rigidBody.isKinematic);
					ImGui::SameLine();
					ImGui::Checkbox("Static", &rigidBody.isStatic);
				});

			DrawComponentNode<Model>("Model", selectedEntity, [&](Model& model)
				{
					// list of all models in ../models directory
					const std::string ext = ".gltf";
					const std::string path = "../models/";
					std::vector<std::string> models = { model.path };
					for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
					{
						if (entry.path().extension() == ext)
						{
							models.push_back(entry.path().relative_path().string());
						}
					}
					if (ImGui::BeginCombo("Models", models[0].data()))
					{
						static int current_item = 0;
						for (int n = 0; n < models.size(); n++)
						{
							const bool is_selected = (current_item == n);
							if (ImGui::Selectable(models[n].data(), is_selected))
								current_item = n;
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
						if (current_item != 0)
						{
							scene->LoadModel(selectedEntity, models[current_item]);
							current_item = 0;
						}
					}
				});

			DrawComponentNode<Collider>("Collider", selectedEntity, [&](Collider& collider)
				{
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
				});
			ImGui::PopStyleColor();
		}
		ImGui::End();
	}
	template <typename T>
	void ComponentViewPanel::DrawComponentNode(const char* name, Entity entity, std::function<void(T& component)> func)
	{
		constexpr ImGuiBackendFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		auto scene = Application::GetScene();
		if (scene->m_Registry.all_of<T>(entity))
		{
			auto& component = scene->GetComponent<T>(entity);
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), flags, name))
			{
				func(component);
				ImGui::TreePop();
			}
		}
	}
}
