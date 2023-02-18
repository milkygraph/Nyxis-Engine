#include "ImguiLayer.hpp"
#include <string>
namespace ve
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

    std::vector<std::string> getModelNames() {
        std::vector<std::string> modelNames;
        for (const auto &entry: std::filesystem::directory_iterator(ve::model_path)) {
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
        ImGui::StyleColorsDark();

        // setting font
        IO->Fonts->AddFontFromFileTTF("../assets/fonts/OpenSans-Regular.ttf", 18.0f);

        // setup style
        ImVec4 *colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.02f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.42f, 0.62f, 0.54f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.22f, 0.36f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09f, 0.11f, 0.14f, 0.40f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.09f, 1.00f);

        ImGui::GetStyle().WindowBorderSize = 0.0f;
        ImGui::GetStyle().FrameBorderSize = 0.0f;
        ImGui::GetStyle().PopupBorderSize = 0.0f;
        ImGui::GetStyle().ChildBorderSize = 0.0f;
        ImGui::GetStyle().FrameRounding = 5;
        ImGui::GetStyle().WindowRounding = 5;
        ImGui::GetStyle().PopupRounding = 5;
        ImGui::GetStyle().ChildRounding = 5;
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

        AddEntityLoader();
        AddSceneHierarchy();
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

    void ImguiLayer::AddEntityLoader()
    {
        ImGui::Begin("Object"); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        {
            auto &rigidBody = m_ActiveScene.getComponent<RigidBody>(m_SelectedEntity); // TODO! Fix load scene bug
            ImGui::DragFloat3("Position", &rigidBody.translation.x, 0.1f);
            ImGui::DragFloat3("Rotation", &rigidBody.rotation.x,0.1f, -180.0f, 180.0f);
            ImGui::DragFloat3("Scale", &rigidBody.scale.x, 0.1f);

            ImGui::DragFloat3("Velocity", &rigidBody.velocity.x, 0.1f);
            ImGui::DragFloat("Restitution", &rigidBody.restitution, 0.1f, 0.0f, 1.0f);

            ImGui::DragFloat("Roughness", &rigidBody.roughness, 0.1f);
            // check if entity has a collider component
            if(m_ActiveScene.m_Registry.all_of<Collider>(m_SelectedEntity))
            {
                auto& collider = m_ActiveScene.getComponent<Collider>(m_SelectedEntity);
                ImGui::DragFloat("Collider Radius", &collider.radius, 0.05f);
            }
        }
        ImGui::End();
    }

    void ImguiLayer::AddSceneHierarchy()
    {
            ImGui::Begin("Scene Hierarchy");
            ImGui::Text("Scene Hierarchy");
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
        if(expanded)
        {
            ImGui::TreePop();
        }
    }
}