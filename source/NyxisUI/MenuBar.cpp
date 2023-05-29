#include "NyxisUI/MenuBar.hpp"

#include "Core/Application.hpp"
#include "NyxisUI/EditorLayer.hpp"
#include "Scene/NyxisProject.hpp"

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
			if (ImGui::MenuItem("Load", "Ctrl+L"))
			{
				Application::GetProject()->Load();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				Application::GetProject()->Save();
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
