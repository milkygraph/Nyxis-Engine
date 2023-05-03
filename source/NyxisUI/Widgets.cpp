#include "NyxisUI/Widgets.hpp"
#include "ImGui/imgui_internal.h"

namespace NyxisWidgets
{
	float LineHeight()
	{
		return GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2;
	}


	void TableFloat(const std::vector<const char*> labels, std::vector<float*> vecs, float step)
	{
		ImGui::BeginTable("##table1", 2);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);  // set column width

		char id[] = "##0";
		for (int row = 0; row < labels.size(); row++)
		{
			ImGui::TableNextRow();
			if (row == 0)
			{
				// Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
				auto padding = ImGui::GetStyle().FramePadding.x;
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(-FLT_MIN - padding); // Right-aligned
			}

			ImGui::TableNextColumn();
			ImGui::Text(labels[row]);

			ImGui::TableNextColumn();
			id[2] = row;
			ImGui::DragFloat(id, vecs[row], step, 0, 0, "%.1f");
		}
		ImGui::EndTable();
	}

	void TableVec3(const std::vector<const char*> labels, std::vector<glm::vec3*> vecs, float step)
	{
		ImGui::BeginTable("##table1", 2, ImGuiTableFlags_None);
		ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("2");

		char id[] = "##0";
		for (int row = 0; row < labels.size(); ++row)
		{
			ImGui::TableNextRow();
			if (row == 0)
			{
				// Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
				auto padding = ImGui::GetStyle().FramePadding.x;
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(-FLT_MIN - padding); // Right-aligned
			}

			ImGui::TableNextColumn();
			ImGui::Text(labels[row]);

			ImGui::TableNextColumn();
			id[2] = row;
			ImGui::DragFloat3(id, &vecs[row]->x, step, 0, 0, "%.1f");
		}
		ImGui::EndTable();
	}
}
