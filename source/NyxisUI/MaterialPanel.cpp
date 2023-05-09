#include "NyxisUI/MaterialPanel.hpp"
#include "NyxisUI/EditorLayer.hpp"
#include "imgui-node-editor/imgui_node_editor.h"
#include "imgui-node-editor/examples/application/include/application.h"

namespace Nyxis
{
    namespace ed = ax::NodeEditor;

	void MaterialPanel::OnUpdate()
	{
		auto material = EditorLayer::GetSelectedMaterial();

		ImGui::Begin("Material");
		ImGui::ColorEdit4("Base Color", &material->baseColorFactor.x);
		ImGui::ColorEdit4("Emissive Color", &material->emissiveFactor.x);
		ImGui::DragFloat("Specular Color", &material->metallicFactor, 0.1);
		ImGui::DragFloat("Roughness Factor", &material->roughnessFactor, 0.1);
		ImGui::End();
	}
}