#pragma once
#include "Core/Nyxis.hpp"
#include "Core/Layer.hpp"

namespace Nyxis
{
	class LogPanel : public Layer
	{
	public:
		LogPanel() = default;
		~LogPanel() = default;

		void OnUpdate() override;
		void OnAttach() override {};
		void OnDetach() override {};
		void OnEvent(Event& event) override {};

	private:
		ImVec4 m_TraceColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		ImVec4 m_InfoColor = ImVec4(0.0f, 0.6f, 0.0f, 1.0f);
		ImVec4 m_WarnColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		ImVec4 m_ErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		ImVec4 m_CriticalColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		ImVec4 m_DefaultColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	};
}