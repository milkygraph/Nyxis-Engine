#pragma once
#include "Core/Nyxispch.hpp"
#include <ImGuizmo/ImGuizmo.h>

namespace Nyxis
{
	// imgui only viewport class
	class Viewport
	{
	public:
		Viewport();
		~Viewport();

		void OnUpdate();
		void OnEvent();
		static bool IsFocused() { return m_IsFocused; }
		static bool IsClicked() { return ImGui::IsMouseClicked(ImGuiMouseButton_Left) && IsHovered(); }
		static bool IsHovered() { return m_IsHovered; }
		VkExtent2D GetExtent() const { return m_Extent; }

	private:
		ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::OPERATION m_LastGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::MODE::WORLD;

		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkExtent2D m_Extent{};
		bool m_DrawGizmos = true;
		bool m_GizmoSnapping = false;
		static inline bool m_IsFocused = false;
		static inline bool m_IsHovered = false;
	};
}
