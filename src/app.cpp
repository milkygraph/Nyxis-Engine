#include "app.hpp"
#include "components.hpp"
#include "descriptors.hpp"
#include "gameObject.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "swap_chain.hpp"
#include "camera.hpp"
#include "keyboardInput.hpp"
#include "frameInfo.hpp"
#include "simpleRenderSystem.hpp"
#include "pointLightSystem.hpp"
#include "path.hpp"

#include <iostream>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <array>
#include <string>
#include <algorithm>
#include <filesystem>

#include <utility>
#include <vulkan/vulkan_core.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>

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
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}

std::vector<std::string> getModelNames()
{
    std::vector<std::string> modelNames;
    for (const auto &entry : std::filesystem::directory_iterator(ve::currentPath() + "/../models"))
    {
        modelNames.push_back(entry.path().filename().string());
    }
    return modelNames;
}

namespace ve
{
    App::App()
    {
        globalPool = veDescriptorPool::Builder(pDevice)
                         .setMaxSets(veSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, veSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();

        loadGameObjects();
    }

    App::~App() {}

    void App::init_imgui(VkCommandBuffer commandBuffer)
    {
        // 1: create descriptor pool for IMGUI
        //  the size of the pool is very oversize, but it's copied from imgui demo itself.

        imguiPool = veDescriptorPool::Builder(pDevice)
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

        // 2: initialize imgui library

        // this initializes the core structures of imgui
        ImGui::CreateContext();
        ImGuiIO *IO = &ImGui::GetIO();
        IO->WantCaptureMouse = true;
        IO->WantCaptureKeyboard = true;
        IO->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable  Docking
        IO->ConfigDockingWithShift = true;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(pWindow.getGLFWwindow(), true);

        ImGui_ImplVulkan_InitInfo init_info = {};

        pDevice.createImGuiInitInfo(init_info);
        init_info.DescriptorPool = imguiPool->getDescriptorPool();

        ImGui_ImplVulkan_Init(&init_info, pRenderer.getSwapChainRenderPass());

        // execute a gpu command to upload imgui font textures
        // immediate_submit([&](VkCommandBuffer cmd)
        //                  { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        // clear font textures from cpu data

        // vkDestroyDescriptorPool(pDevice.device(), imguiPool->getDescriptorPool(), nullptr);

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

    // TODO: Fix object adding functionality
    // TODO: Create abstraction layer for ImGui
    void App::render_imgui(FrameInfo &frameInfo)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowExampleAppDockSpace();

        ImGui::Begin("Object"); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

        static std::unordered_map<std::string, Entity> entity_names;

        static bool firstFrame = true;

        auto view = pScene.getComponentView<TagComponent>();
        if (firstFrame)
        {
            for (auto entity : view)
            {
                auto tag = view.get<TagComponent>(entity);
                entity_names[tag.Tag] = entity;
            }
            firstFrame = false;
        }

        static std::string current_item = entity_names.begin()->first;

        if (ImGui::BeginCombo("Objects", current_item.c_str()))
        { // The second parameter is the label previewed before opening the combo.
            for (auto &kv : entity_names)
            {
                bool is_selected = (current_item == kv.first); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(kv.first.c_str(), is_selected))
                    current_item = kv.first;
                if (is_selected)
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
            }
            ImGui::EndCombo();
        }

        auto &transform = pScene.getComponent<TransformComponent>(entity_names[current_item]);
        ImGui::DragFloat3("Position", &transform.translation.x, 0.1f);
        ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
        ImGui::DragFloat("Roughness", &transform.roughness, 0.1f);

        static auto modelNames = getModelNames();

        static auto &currentModel = modelNames[0];
        if (ImGui::BeginCombo("Models", currentModel.c_str()))
        { // The second parameter is the label previewed before opening the combo.
            for (auto &model : modelNames)
            {
                bool is_selected = (currentModel == model); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(model.c_str(), is_selected))
                    currentModel = model;
                if (is_selected)
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Add Object"))
        {
            // use addGameObject() to add a new object to the scene
            auto [name, entity] = addGameObject(currentModel);
            entity_names[name] = entity;
        }

        // ImGui::ColorPicker3("Sky Color", pScene.m_SkyColor);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
    }

    void App::close_imgui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void App::run()
    {
        std::vector<std::unique_ptr<veBuffer>> uboBuffers(veSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<veBuffer>(
                pDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = veDescriptorSetLayout::Builder(pDevice)
                                   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(veSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            veDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem srs{pDevice, pRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()}; // srs - simpleRenderSystem
        PointLightSystem pls{pDevice, pRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};   // pls - pointLightSystem
        veCamera camera{};

        auto viewerObject = veGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        bool firstFrame = true;
        double lastTime = glfwGetTime();
        int nbFrames = 0;

        while (!pWindow.shouldClose())
        {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(pWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = pRenderer.getAspectRatio();

            camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.01f, 20.f);

            if (auto commandBuffer = pRenderer.beginFrame())
            {
                if (firstFrame)
                {
                    init_imgui(commandBuffer);
                    firstFrame = false;
                }

                int frameIndex = pRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects, pScene};

                // updating buffers
                GlobalUbo ubo{};
                ubo.projection = camera.getProjectionMatrix();
                ubo.view = camera.getViewMatrix();
                ubo.inverseViewMatrix = camera.getInverseViewMatrix();
                pls.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // rendering
                pRenderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
                srs.renderGameObjects(frameInfo);
                pls.render(frameInfo);
                render_imgui(frameInfo);
                pRenderer.endSwapChainRenderPass(frameInfo.commandBuffer);
                pRenderer.endFrame();
            }

            double time = glfwGetTime();
            nbFrames++;
            if (time - lastTime >= 1.0)
            { // If last prinf() was more than 1 sec ago
                std::cout << "FPS: " << nbFrames << std::endl;
                nbFrames = 0;
                lastTime += 1.0;
            }
        }

        vkDeviceWaitIdle(pDevice.device());
        close_imgui();
    }

    void App::loadGameObjects()
    {

        auto vase = pScene.createEntity("Vase");
        pScene.addComponent<TransformComponent>(vase, glm::vec3(-.5f, .5f, 0.f), glm::vec3(.0f, .0f, 0.0f), glm::vec3(1.5f, 1.5f, 1.5f), 0.0f);
        pScene.addComponent<MeshComponent>(vase, pDevice, "smooth_vase.obj");

        auto pose = pScene.createEntity("Pose");
        pScene.addComponent<TransformComponent>(pose, glm::vec3(.2f, .5f, 0.f), glm::vec3(.0f, .0f, 0.0f), glm::vec3(1.5f, 1.5f, 1.5f), 0.0f);
        pScene.addComponent<MeshComponent>(pose, pDevice, "pose.obj");

        auto floor = pScene.createEntity("Floor");
        pScene.addComponent<TransformComponent>(floor, glm::vec3(0.f, 0.5f, 0.f), glm::vec3(.0f, .0f, 0.0f), glm::vec3(10.f, 10.f, 10.f), 0.0f);
        pScene.addComponent<MeshComponent>(floor, pDevice, "floor.obj");

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}};

        for (int i = 0; i < lightColors.size(); i++)
        {
            auto pointLight = veGameObject::makePointLight(0.8f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

    /**
     * @note - It makes sense to use this function only for imgui interface which will have its own class in the future
     *
     * @param filename - path to the file with .obj extension
     * @return std::pair<std::string, Entity> - pair of the name of the object and the entity
     */
    std::pair<std::string, Entity> App::addGameObject(const std::string &filename)
    {
        auto name = filename.substr(0, filename.find_last_of('.'));
        auto entity = pScene.createEntity(name);
        pScene.addComponent<TransformComponent>(entity, glm::vec3(0.f, 0.f, 0.f), glm::vec3(.0f, .0f, 0.0f), glm::vec3(1.f, 1.f, 1.f), 0.0f);
        pScene.addComponent<MeshComponent>(entity, pDevice, filename);
        return {name, entity};
    }
} // namespace ve