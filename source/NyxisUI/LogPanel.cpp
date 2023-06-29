#include "NyxisUI/LogPanel.hpp"
#include "Core/Log.hpp"

namespace Nyxis
{
	void LogPanel::OnUpdate()
	{
		ImGui::Begin("Log", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
		auto& log_buffer = Log::GetLogBuffer().GetBuffer();

		ImGui::Dummy(ImVec2(0.0f, 2.0f));
		if(ImGui::SmallButton("Clear"))
			Log::GetLogBuffer().Clear();

		ImGui::SameLine();
		static bool scroll_to_bottom = false;
		if(ImGui::SmallButton("Scroll to Bottom"))
			scroll_to_bottom = true;

		ImGui::Dummy(ImVec2(0.0f, 2.0f));
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
		for(auto& log : log_buffer)
		{
			static ImVec4 color{};
			switch(log.level)
			{
				case spdlog::level::trace:
					color = m_TraceColor;
					break;
				case spdlog::level::info:
					color = m_InfoColor;
					break;
				case spdlog::level::warn:
					color = m_WarnColor;
					break;
				case spdlog::level::err:
					color = m_ErrorColor;
					break;
				case spdlog::level::critical:
					color = m_CriticalColor;
					break;
				default:
					color = m_DefaultColor;
					break;
			}
			ImGui::TextColored(color, "%s", log.text.c_str());

		}
		if(scroll_to_bottom)
		{
			ImGui::SetScrollHereY(1.0f);
			scroll_to_bottom = false;
		}
		ImGui::EndChild();


		ImGui::End();
	}
}