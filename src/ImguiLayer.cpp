#include "ImguiLayer.hpp"

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

    ImguiLayer::ImguiLayer() {
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
        ImGui_ImplGlfw_InitForVulkan(veWindow::getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        veDevice::get().createImGuiInitInfo(init_info);
        init_info.DescriptorPool = imguiPool->getDescriptorPool();
        ImGui_ImplVulkan_Init(&init_info, RenderPass);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    }

    void ImguiLayer::AddFunction(std::function<void()> function) {
        functions.push_back(function);
    }

    void ImguiLayer::OnUpdate(FrameInfo &frameInfo) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ShowExampleAppDockSpace();
        for (auto &function : functions) {
            function();
        }
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
    }

    void ImguiLayer::AddEntityLoader(Scene &scene)
    {
        functions.push_back([&scene]() {
            ImGui::Begin(
                    "Object"); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            static std::unordered_map<std::string, Entity> entity_names;

            static bool firstFrame = true;

            auto view = scene.getComponentView<TagComponent>();
            static std::string selectedEntity;
            if (firstFrame) {
                for (auto entity: view) {
                    auto tag = view.get<TagComponent>(entity);
                    entity_names[tag.Tag] = entity;
                }
                if (!entity_names.empty())
                    selectedEntity = entity_names.begin()->first;
                firstFrame = false;
            }

// select entity from list of entities, when there are no entities in the list, create a new one
            if (entity_names.empty()) {
            } else {
                if (ImGui::BeginCombo("Objects",
                                      selectedEntity.c_str())) { // The second parameter is the label previewed before opening the combo.
                    for (auto &kv: entity_names) {
                        bool is_selected = (selectedEntity
                                            ==
                                            kv.first); // You can store your selection however you want, outside or inside your objects
                        if (ImGui::Selectable(kv.first.c_str(), is_selected))
                            selectedEntity = kv.first;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                    }
                    ImGui::EndCombo();
                }

                auto &transform = scene.getComponent<TransformComponent>(entity_names[selectedEntity]);
                ImGui::DragFloat3("Position", &transform.translation.x, 0.1f);
                ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
// rotation in degrees
                ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f, -180.0f, 180.0f);
                ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
                ImGui::DragFloat("Roughness", &transform.roughness, 0.1f);
            }
            static auto modelNames = getModelNames();

            static auto &selectedModel = modelNames[0];
            if (ImGui::BeginCombo("Models",
                                  selectedModel.c_str())) { // The second parameter is the label previewed before opening the combo.
                for (auto &model: modelNames) {
                    bool is_selected = (selectedModel
                                        ==
                                        model); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(model.c_str(), is_selected))
                        selectedModel = model;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Add Object")) {
                auto [name, entity] = scene.addEntity(selectedModel);
                if (entity_names.find(name) != entity_names.end())
                    name = name + std::to_string(scene.getEntityCount());
                entity_names[name] = entity;
                selectedEntity = name;
            }

            ImGui::End();
        });
    }
}