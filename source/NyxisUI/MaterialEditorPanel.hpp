#pragma once
#include <imgui-node-editor/imgui_node_editor.h>

#include "Core/Layer.hpp"
#include "Graphics/GLTFModel.hpp"

namespace ed = ax::NodeEditor;

namespace Nyxis
{
	class MaterialEditorPanel final : public Layer
	{
	public:
		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;
		void OnUpdate() override;
		static ed::EditorContext* GetEditorContext() { return m_EditorContext; }

		inline static std::unordered_map<ModelTexture*, VkDescriptorSet> materialDescriptorSets{};
	private:
		inline static ed::EditorContext* m_EditorContext = nullptr;
	};
}