#pragma once
#include "Core/Nyxispch.hpp"
#include <ImGuizmo/ImGuizmo.h>
#include "Core/Layer.hpp"

namespace Nyxis
{
	// imgui only viewport class
	class Viewport final : public Layer
	{
	public:
		Viewport();
		~Viewport() override;

		void OnAttach() override {};
		void OnDetach() override {};
		void OnUpdate() override;
		void OnEvent(Event& event) override;
		static bool IsFocused() { return m_IsFocused; }
		static bool IsClicked() { return m_IsClicked; }
		static bool IsHovered() { return m_IsHovered; }
		static bool IsHoveredOverGizmo() { return m_OverGizmo; }
		static VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_WindowSize.x), static_cast<uint32_t>(m_WindowSize.y) }; }
		static glm::vec2 GetWindowPos() { return { m_WindowPos.x, m_WindowPos.y }; }
		static glm::vec2 GetWindowSize() { return { m_WindowSize.x, m_WindowSize.y }; }

	private:
		void UpdateViewport();
		void UpdateGizmo();
		ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::OPERATION m_LastGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::MODE::WORLD;

		static inline ImVec2 m_WindowPos;
		static inline ImVec2 m_WindowSize;

		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		float m_SnapValue = 0.1f;
		bool m_DrawGizmos = true;
		bool m_GizmoSnapping = false;
		static inline bool m_UsingGizmo = false;
		static inline bool m_OverGizmo = false;
		static inline bool m_IsFocused = false;
		static inline bool m_IsHovered = false;
		static inline bool m_IsClicked = false;
	};
}
