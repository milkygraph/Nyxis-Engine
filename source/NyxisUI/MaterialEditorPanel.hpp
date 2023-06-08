#pragma once
#include <imgui-node-editor/imgui_node_editor.h>

#include "Graphics/GLTFModel.hpp"

namespace ed = ax::NodeEditor;

namespace Nyxis
{
	class MaterialEditorPanel
	{
	public:
		static void Init();
		static void OnUpdate(Material* material);
		static ed::EditorContext* GetEditorContext() { return m_EditorContext; }

	private:
		inline static ed::EditorContext* m_EditorContext = nullptr;
	};
}