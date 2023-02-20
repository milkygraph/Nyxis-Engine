#include "ImguiLayer.hpp"
#include <string>
namespace Nyxis
{
    void ShowExampleAppDockSpace() {
        // this is a copy of the imgui demo code
        bool p_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                            | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        } else {
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
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
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
        style.WindowRounding = 0.0f;
        style.WindowBorderSize = 1.0f;
        style.WindowMinSize = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ChildRounding = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(4.0f, 3.0f);
        style.FrameRounding = 0.0f;
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

        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 0.9399999976158142f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2078431397676468f, 0.2196078449487686f, 0.5400000214576721f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4000000059604645f, 0.4000000059604645f, 0.4000000059604645f, 0.4000000059604645f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 0.6700000166893005f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2862745225429535f, 0.2862745225429535f, 0.2862745225429535f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.9372549057006836f, 0.9372549057006836f, 0.9372549057006836f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.4000000059604645f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4588235318660736f, 0.4666666686534882f, 0.47843137383461f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4196078479290009f, 0.4196078479290009f, 0.4196078479290009f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.6980392336845398f, 0.6980392336845398f, 0.6980392336845398f, 0.3100000023841858f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.6980392336845398f, 0.6980392336845398f, 0.6980392336845398f, 0.800000011920929f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.47843137383461f, 0.4980392158031464f, 0.5176470875740051f, 1.0f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7176470756530762f, 0.7176470756530762f, 0.7176470756530762f, 0.7799999713897705f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.9098039269447327f, 0.9098039269447327f, 0.9098039269447327f, 0.25f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.8078431487083435f, 0.8078431487083435f, 0.8078431487083435f, 0.6700000166893005f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4588235318660736f, 0.4588235318660736f, 0.4588235318660736f, 0.949999988079071f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 0.8619999885559082f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.729411780834198f, 0.6000000238418579f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.8666666746139526f, 0.8666666746139526f, 0.8666666746139526f, 0.3499999940395355f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.6000000238418579f, 0.6000000238418579f, 0.6000000238418579f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
    }
    
    std::vector<std::string> getModelNames() {
        std::vector<std::string> modelNames;
        for (const auto &entry: std::filesystem::directory_iterator(Nyxis::model_path)) {
            modelNames.push_back(entry.path().filename().string());
        }
        return modelNames;
    }

    ImguiLayer::ImguiLayer(Scene& scene) : m_ActiveScene(scene)
    {
        imguiPool = veDescriptorPool::Builder()
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
        ImGuiIO *IO = &ImGui::GetIO();
        IO->WantCaptureMouse = true;
        IO->WantCaptureKeyboard = true;
        IO->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable  Docking
        IO->ConfigDockingWithShift = true;
        //ImGui::StyleColorsDark();

        // setting font
        IO->Fonts->AddFontFromFileTTF("../assets/fonts/OpenSans-Regular.ttf", 18.0f);

         //setup style
        //ImVec4 *colors = ImGui::GetStyle().Colors;
        //colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.02f, 1.00f);
        //colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.42f, 0.62f, 0.54f);
        //colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
        //colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.22f, 0.36f, 1.00f);
        //colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.11f, 0.14f, 0.40f);
        //colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.09f, 1.00f);

        //ImGui::GetStyle().WindowBorderSize = 0.0f;
        //ImGui::GetStyle().FrameBorderSize = 0.0f;
        //ImGui::GetStyle().PopupBorderSize = 0.0f;
        //ImGui::GetStyle().ChildBorderSize = 0.0f;
        //ImGui::GetStyle().FrameRounding = 5;
        //ImGui::GetStyle().WindowRounding = 5;
        //ImGui::GetStyle().PopupRounding = 5;
        //ImGui::GetStyle().ChildRounding = 5;
        
		SetupImGuiStyle();
    }

    
    ImguiLayer::~ImguiLayer() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImguiLayer::init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer) {
        ImGui_ImplGlfw_InitForVulkan(Window::getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        Device::get().createImGuiInitInfo(init_info);
        init_info.DescriptorPool = imguiPool->getDescriptorPool();
        ImGui_ImplVulkan_Init(&init_info, RenderPass);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    }

    void ImguiLayer::AddFunction(const std::function<void()>& function) {
        functions.push_back(function);
    }

    void ImguiLayer::OnUpdate(FrameInfo &frameInfo) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ShowExampleAppDockSpace();
		ImGui::ShowDemoWindow();
        
        AddSceneHierarchy();
        AddComponentView();
        AddMenuBar();

        for (auto &function : functions) {
            function();
        }
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
    }

    void ImguiLayer::AddMenuBar()
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                m_ActiveScene.LoadSceneFlag = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                m_ActiveScene.SaveSceneFlag = true;
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if(ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if(ImGui::MenuItem("Cut", "CTRL+X")) {}
            if(ImGui::MenuItem("Copy", "CTRL+C")) {}
            if(ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    void ImguiLayer::AddComponentView()
    {
        ImGui::Begin("Component"); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        {
            if (m_ShowEntityLoader) {
                if(m_ActiveScene.m_Registry.valid(m_SelectedEntity))
                {
                    if(m_ActiveScene.m_Registry.all_of<TagComponent>(m_SelectedEntity))
                    {
                        auto &tag = m_ActiveScene.getComponent<TagComponent>(m_SelectedEntity);
                        char buffer[256];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, tag.Tag.c_str());
                        if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                            tag.Tag = std::string(buffer);
                        }
                    }

                    if(m_ActiveScene.m_Registry.all_of<RigidBody>(m_SelectedEntity))
                    {
                        auto &rigidBody = m_ActiveScene.getComponent<RigidBody>(
                                m_SelectedEntity); // TODO! Fix load scene bug
                        ImGui::Text("Rigid Body");
                        ImGui::DragFloat3("Position", &rigidBody.translation.x, 0.1f);
                        ImGui::DragFloat3("Rotation", &rigidBody.rotation.x, 0.1f, -180.0f, 180.0f);
                        ImGui::DragFloat3("Scale", &rigidBody.scale.x, 0.1f);

                        ImGui::DragFloat3("Velocity", &rigidBody.velocity.x, 0.1f);
                        ImGui::DragFloat("Restitution", &rigidBody.restitution, 0.1f, 0.0f, 1.0f);

                        ImGui::DragFloat("Roughness", &rigidBody.roughness, 0.1f);
                        // check if entity has a collider component
                        if (m_ActiveScene.m_Registry.all_of<Collider>(m_SelectedEntity)) {
                            auto &collider = m_ActiveScene.getComponent<Collider>(m_SelectedEntity);

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

                            if(collider.type == ColliderType::Box)
                                ImGui::DragFloat3("Collider Size", &collider.size.x, 0.05f);

                            else if(collider.type == ColliderType::Sphere)
                                ImGui::DragFloat("Collider Radius", &collider.radius, 0.05f);

                        }
                    }
                }
            }
        }
        ImGui::End();
    }

    void ImguiLayer::AddSceneHierarchy()
    {
        ImGui::Begin("Scene Hierarchy");
        if(ImGui::BeginPopupContextWindow())
        {
            if(ImGui::MenuItem("Create Empty Entity"))
            {
                auto entity = m_ActiveScene.createEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }

        m_ActiveScene.m_Registry.each([&](auto entityID)
        {
            DrawEntityNode(entityID);
        });

        ImGui::End();
    }

    void ImguiLayer::DrawEntityNode(Entity entity)
    {
        auto& tag = m_ActiveScene.getComponent<TagComponent>(entity).Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());
        if(ImGui::IsItemClicked())
        {
            m_SelectedEntity = entity;
        }
        // right click context menu
        if(ImGui::BeginPopupContextItem())
        {
            if(ImGui::MenuItem("Delete Entity"))
            {
                m_ActiveScene.destroyEntity(entity);
            }
            // add component tree with all components that can be added to entity
            if(ImGui::BeginMenu("Add Component"))
            {
                if(ImGui::MenuItem("RigidBody"))
                    if(!m_ActiveScene.m_Registry.all_of<RigidBody>(entity))
                        m_ActiveScene.addComponent<RigidBody>(entity);

                if(ImGui::MenuItem("Mesh"))
                    if(!m_ActiveScene.m_Registry.all_of<MeshComponent>(entity))
                        m_ActiveScene.addComponent<MeshComponent>(entity, "../models/sphere.obj");

                if(ImGui::MenuItem("Collider"))
                    if(!m_ActiveScene.m_Registry.all_of<Collider>(entity))
                        m_ActiveScene.addComponent<Collider>(entity, ColliderType::Sphere, glm::vec3{ 0.2, 0.2, 0.2 }, 0.05);

                if(ImGui::MenuItem("Gravity"))
                    if(!m_ActiveScene.m_Registry.all_of<Gravity>(entity))
                        m_ActiveScene.addComponent<Gravity>(entity);

                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Remove Component"))
            {
                if(m_ActiveScene.m_Registry.all_of<RigidBody>(entity))
                    if(ImGui::MenuItem("RigidBody"))
                        m_ActiveScene.m_Registry.remove<RigidBody>(entity);

                if(m_ActiveScene.m_Registry.all_of<MeshComponent>(entity))
                    if(ImGui::MenuItem("Mesh"))
                        m_ActiveScene.m_Registry.remove<MeshComponent>(entity);

                if(m_ActiveScene.m_Registry.all_of<Collider>(entity))
                    if(ImGui::MenuItem("Collider"))
                        m_ActiveScene.m_Registry.remove<Collider>(entity);

                if(m_ActiveScene.m_Registry.all_of<Gravity>(entity))
                    if(ImGui::MenuItem("Gravity"))
                        m_ActiveScene.m_Registry.remove<Gravity>(entity);

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        if(expanded)
        {
            ImGui::TreePop();
            m_ShowEntityLoader = true;
        }
    }
}