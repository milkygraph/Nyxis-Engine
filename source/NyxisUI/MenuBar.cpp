#include "NyxisUI/MenuBar.hpp"
#include "NyxisUI/EditorLayer.hpp"

namespace Nyxis
{
	void MenuBar::OnUpdate()
	{
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
			}
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
			}
			ImGui::Separator();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{
			}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
			{
			} // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X"))
			{
			}
			if (ImGui::MenuItem("Copy", "CTRL+C"))
			{
			}
			if (ImGui::MenuItem("Paste", "CTRL+V"))
			{
			}
			if (ImGui::MenuItem("Deselect", "CTRL+D"))
			{
				EditorLayer::DeselectEntity();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
