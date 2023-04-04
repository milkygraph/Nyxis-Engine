#include "NyxisUI/EditorLayer.hpp"
#include "Core/Application.hpp"
#include "Core/SwapChain.hpp"
#include "Events/MouseEvents.hpp"

namespace Nyxis
{
	void ShowExampleAppDockSpace()
	{
		// this is a copy of the imgui demo code
		bool p_open = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, .0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &p_open, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}

	void SetupImGuiStyle()
	{
		// Unreal style by dev0-1 from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6000000238418579f;
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.WindowRounding = 8.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ChildRounding = 8.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 7.0f;
		style.PopupBorderSize = 0.5f;
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.FrameRounding = 7.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.CellPadding = ImVec2(4.0f, 2.0f);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 14.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 4.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
		style.WindowMinSize.x = 200.0f;
		style.WindowTitleAlign = { 0.5f, 0.5f };

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.97f);
		colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.92f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.88f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.73f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.49f, 0.49f, 0.49f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.63f, 0.63f, 0.63f, 0.65f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.04f, 0.04f, 0.04f, 0.97f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
		colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.31f, 0.00f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.04f, 0.04f, 0.04f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.00f, 0.31f, 0.00f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.95f, 0.31f, 0.00f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	std::vector<std::string> getModelNames()
	{
		std::vector<std::string> modelNames;
		for (const auto& entry : std::filesystem::directory_iterator(Nyxis::model_path))
		{
			modelNames.push_back(entry.path().filename().string());
		}
		return modelNames;
	}

	void EditorLayer::OnAttach()
	{
		imguiPool = DescriptorPool::Builder()
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.setMaxSets(1000)
			.build();

		ImGui::CreateContext();
		ImGuiIO* IO = &ImGui::GetIO();
		IO->WantCaptureMouse = true;
		IO->WantCaptureKeyboard = true;
		IO->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable  Docking
		IO->ConfigDockingWithShift = false;

		// setting font
		IO->Fonts->AddFontFromFileTTF("../assets/fonts/OpenSans-Regular.ttf", 18.0f);

		SetupImGuiStyle();

		// create sampler
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(Device::Get().device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	void EditorLayer::OnDetach()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		// TODO: delete all the stuff
		vkDestroySampler(Device::Get().device(), m_Sampler, nullptr);
	}

	void EditorLayer::Init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer)
	{
		ImGui_ImplGlfw_InitForVulkan(Window::GetGLFWwindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		Device::Get().createImGuiInitInfo(init_info);
		init_info.DescriptorPool = imguiPool->getDescriptorPool();
		ImGui_ImplVulkan_Init(&init_info, RenderPass);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		dst.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	}

	void EditorLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ShowExampleAppDockSpace();
		ImGui::ShowDemoWindow();
	}

	void EditorLayer::AddFunction(const std::function<void()>& function)
	{
		functions.push_back(function);
	}

	void EditorLayer::OnUpdate()
	{
		m_Viewport.OnUpdate();
		AddSceneHierarchy();
		AddComponentView();
		AddMenuBar();

		for (auto& function : functions)
			function();

		auto frameInfo = Application::GetFrameInfo();
		this->commandBuffer = frameInfo->commandBuffer;
	}

	void EditorLayer::End()
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}

	void EditorLayer::AddMenuBar()
	{
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
			}
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				m_ActiveScene->LoadSceneFlag = true;
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				m_ActiveScene->SaveSceneFlag = true;
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
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	void EditorLayer::AddComponentView()
	{
		ImGui::Begin("Component");
		// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		{
			if (m_ShowEntityComponents)
			{
				if (m_ActiveScene->m_Registry.valid(m_SelectedEntity))
				{
					if (m_ActiveScene->m_Registry.all_of<TagComponent>(m_SelectedEntity))
					{
						auto& tag = m_ActiveScene->GetComponent<TagComponent>(m_SelectedEntity);
						char buffer[256];
						memset(buffer, 0, sizeof(buffer));
						strcpy(buffer, tag.Tag.c_str());
						if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
						{
							tag.Tag = std::string(buffer);
						}
					}

					if (m_ActiveScene->m_Registry.all_of<RigidBody>(m_SelectedEntity))
					{
						auto& rigidBody = m_ActiveScene->GetComponent<RigidBody>(
							m_SelectedEntity); // TODO! Fix load scene bug
						ImGui::Text("Rigid Body");
						ImGui::DragFloat3("Position", &rigidBody.translation.x, 0.1f, 0, 0, "%.1f");
						glm::vec3 rotationDeg = glm::degrees(rigidBody.rotation);
						ImGui::DragFloat3("Rotation", &rotationDeg.x, 0.1f, 0, 0, "%.1f");
						rigidBody.rotation = glm::radians(rotationDeg);
						ImGui::DragFloat3("Scale", &rigidBody.scale.x, 0.1f, 0, 0, "%.2f");

						ImGui::DragFloat3("Velocity", &rigidBody.velocity.x, 0.1f);
						ImGui::DragFloat("Restitution", &rigidBody.restitution, 0.1f, 0.0f, 1.0f);

						ImGui::DragFloat("Roughness", &rigidBody.roughness, 0.1f);
						// check if entity has a collider component
						if (m_ActiveScene->m_Registry.all_of<Collider>(m_SelectedEntity))
						{
							auto& collider = m_ActiveScene->GetComponent<Collider>(m_SelectedEntity);

							ImGui::Text("Collider");
							auto preview = Nyxis::collider_name[collider.type];

							if (ImGui::BeginCombo("Collider Type", preview.c_str(), ImGuiComboFlags_NoArrowButton))
							{
								if (ImGui::Selectable("Box"))
									collider.type = ColliderType::Box;

								if (ImGui::Selectable("Sphere"))
									collider.type = ColliderType::Sphere;

								ImGui::EndCombo();
							}

							if (collider.type == ColliderType::Box)
								ImGui::DragFloat3("Collider Size", &collider.size.x, 0.05f);

							else if (collider.type == ColliderType::Sphere)
								ImGui::DragFloat("Collider Radius", &collider.radius, 0.05f);
						}
					}
				}
			}
		}
		ImGui::End();
	}

	void EditorLayer::AddSceneHierarchy()
	{
		ImGui::Begin("Scene Hierarchy");
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_SelectedEntity = m_ActiveScene->createEntity("Empty Entity");

			if (ImGui::MenuItem("Clear Scene"))
				m_ActiveScene->ClearScene();

			ImGui::EndPopup();
		}

		m_ActiveScene->m_Registry.each([&](auto entityID)
		{
			DrawEntityNode(entityID);
		});

		ImGui::End();
	}

	void EditorLayer::DrawEntityNode(Entity entity)
	{
		auto& tag = m_ActiveScene->GetComponent<TagComponent>(entity).Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
			ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());

		if (m_SelectedEntity == entity) {
			ImGui::SetNextItemOpen(true);
		}

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		
		// right click context menu
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				m_ActiveScene->destroyEntity(entity);
			}
			// add component tree with all components that can be added to entity
			if (ImGui::BeginMenu("Add Component"))
			{
				if (ImGui::MenuItem("RigidBody"))
					if (!m_ActiveScene->m_Registry.all_of<RigidBody>(entity))
						m_ActiveScene->addComponent<RigidBody>(entity);

				if (ImGui::MenuItem("Mesh"))
					if (!m_ActiveScene->m_Registry.all_of<MeshComponent>(entity))
						m_ActiveScene->addComponent<MeshComponent>(entity, "../models/sphere.obj");

				if (ImGui::MenuItem("Collider"))
					if (!m_ActiveScene->m_Registry.all_of<Collider>(entity))
						m_ActiveScene->addComponent<Collider>(entity, ColliderType::Sphere, glm::vec3{0.2, 0.2, 0.2},
						                                     0.05);

				if (ImGui::MenuItem("Gravity"))
					if (!m_ActiveScene->m_Registry.all_of<Gravity>(entity))
						m_ActiveScene->addComponent<Gravity>(entity);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Remove Component"))
			{
				if (m_ActiveScene->m_Registry.all_of<RigidBody>(entity))
					if (ImGui::MenuItem("RigidBody"))
						m_ActiveScene->m_Registry.remove<RigidBody>(entity);

				if (m_ActiveScene->m_Registry.all_of<MeshComponent>(entity))
					if (ImGui::MenuItem("Mesh"))
						m_ActiveScene->m_Registry.remove<MeshComponent>(entity);

				if (m_ActiveScene->m_Registry.all_of<Collider>(entity))
					if (ImGui::MenuItem("Collider"))
						m_ActiveScene->m_Registry.remove<Collider>(entity);

				if (m_ActiveScene->m_Registry.all_of<Gravity>(entity))
					if (ImGui::MenuItem("Gravity"))
						m_ActiveScene->m_Registry.remove<Gravity>(entity);

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (expanded)
		{
			ImGui::TreePop();
			m_ShowEntityComponents = true;
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		
	}

	void EditorLayer::OnEvent()
	{

	}

	void EditorLayer::SetScene(Ref<Scene> scene)
	{
		m_ActiveScene = scene;
	}

	void EditorLayer::SetSelectedEntity(Entity entity)
	{
		m_SelectedEntity = entity;
	}
}
