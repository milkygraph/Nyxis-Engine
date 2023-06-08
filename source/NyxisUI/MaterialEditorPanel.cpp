#include "NyxisUI/MaterialEditorPanel.hpp"

#include "EditorLayer.hpp"

namespace Nyxis
{
	void MaterialEditorPanel::Init()
	{
		ed::Config config;
		config.SettingsFile = "MaterialEditorPanel.json";
		m_EditorContext = ed::CreateEditor(&config);
	}

	void MaterialEditorPanel::OnUpdate(Material* material)
	{
		static std::unordered_map<ModelTexture*, VkDescriptorSet> materialDescriptorSets{};

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
		EditorLayer::DisplayUIImage(materialDescriptorSets, material->baseColorTexture);
		ImGui::ColorEdit4("Color", &material->baseColorFactor.x);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Normal Map");
		EditorLayer::DisplayUIImage(materialDescriptorSets, material->normalTexture);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Metallic Roughness");
		EditorLayer::DisplayUIImage(materialDescriptorSets, material->metallicRoughnessTexture);
		ImGui::DragFloat("Metallic", &material->metallicFactor, 0.1f);
		ImGui::DragFloat("Roughness", &material->roughnessFactor, 0.1f);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Occlusion");
		EditorLayer::DisplayUIImage(materialDescriptorSets, material->occlusionTexture);
		ed::EndNode();

		ed::BeginNode(uniqueId++);
		ImGui::Text("Emissive");
		EditorLayer::DisplayUIImage(materialDescriptorSets, material->emissiveTexture);
		ImGui::ColorEdit4("Color", &material->emissiveFactor.x);
		ed::EndNode();

		ed::End();
		ImGui::End();
	}
}
