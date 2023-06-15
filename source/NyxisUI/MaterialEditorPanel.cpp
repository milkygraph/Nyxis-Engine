#include "NyxisUI/MaterialEditorPanel.hpp"

#include "EditorLayer.hpp"

namespace Nyxis
{
	void MaterialEditorPanel::OnAttach()
	{
		ed::Config config;
		config.SettingsFile = "MaterialEditorPanel.json";
		m_EditorContext = ed::CreateEditor(&config);
	}

	void MaterialEditorPanel::OnDetach()
	{
		ed::DestroyEditor(m_EditorContext);
	}

	void MaterialEditorPanel::OnEvent(Event& event)
	{
	}

	void MaterialEditorPanel::OnUpdate()
	{
		const auto material = EditorLayer::GetSelectedMaterial();
		if(material == nullptr)
			return;

		ImGui::Begin("Material Editor");
		if(material == nullptr)
		{
			ImGui::End();
			return;
		}

		ed::SetCurrentEditor(m_EditorContext);

		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		int uniqueId = 1;
		// Start drawing nodes.
		ed::BeginNode(uniqueId++);
		ImGui::Text("Node A");
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();

		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin(uniqueId++, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Base Color");
		EditorLayer::Image(materialDescriptorSets, material->baseColorTexture);
		ImGui::ColorEdit4("Color", &material->baseColorFactor.x);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Normal Map");
		EditorLayer::Image(materialDescriptorSets, material->normalTexture);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Metallic Roughness");
		EditorLayer::Image(materialDescriptorSets, material->metallicRoughnessTexture);
		ImGui::DragFloat("Metallic", &material->metallicFactor, 0.1f, 0, 1);
		ImGui::DragFloat("Roughness", &material->roughnessFactor, 0.1f, 0, 1);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Occlusion");
		EditorLayer::Image(materialDescriptorSets, material->occlusionTexture);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Emissive");
		EditorLayer::Image(materialDescriptorSets, material->emissiveTexture);
		ImGui::ColorEdit4("Color", &material->emissiveFactor.x);
		ed::EndNode();

		ed::End();
		ImGui::End();
	}
}
