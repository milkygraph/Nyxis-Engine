#include "app.hpp"
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

namespace ve
{

    static char *intToChar(id_t id)
    {
        std::vector<char> vec;
        id_t mask = 1e9;

        while (mask > id && mask > 9)
            mask /= 10;
        // id 1e5, reduce mask to 1e5
        while (mask > 0)
        {
            vec.push_back(id / mask + '0');
            id %= mask;
            mask /= 10;
        }
        vec.push_back('\0');
        auto res = new char[vec.size()];
        strncpy(res, vec.data(), vec.size());
        return res;
    }

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

        // this initializes imgui for SDL
        ImGui_ImplGlfw_InitForVulkan(pWindow.getGLFWwindow(), true);

        // this initializes imgui for Vulkan
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

        // add the destroy the imgui created structures
        // ImGui_ImplVulkan_Shutdown();
    }

    void ShowExampleAppDockSpace(bool *p_open)
    {
        // If you strip some features of, this demo is pretty much equivalent to calling DockSpaceOverViewport()!
        // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
        // In this specific demo, we are not using DockSpaceOverViewport() because:
        // - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
        // - we allow the host window to have padding (when opt_padding == true)
        // - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
        // TL;DR; this demo is more complicated than what you would normally use.
        // If we removed all the options we are showcasing, this demo would become:
        //     void ShowExampleAppDockSpace()
        //     {
        //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        //     }

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
        ImGui::Begin("DockSpace Demo", p_open, window_flags);
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

    // TODO: Fix object adding functionality
    // TODO: Create abstraction layer for ImGui
    void App::render_imgui(FrameInfo &frameInfo)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool dock = false;

        ShowExampleAppDockSpace(&dock);

        static id_t new_id;
        bool show_window = true;
        static float sliderFloat = 0.0f;

        ImGui::Begin("Object"); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

        static std::vector<char *> items;

        static bool firstFrame = true;

        // use this implementaion if you don't care about the order of items in Objects ComboList
        // if (firstFrame)
        // {
        // std::for_each(gameObjects.begin(), gameObjects.end(), [](auto &pair)
        //   { items.insert(items.begin(), (intToChar(pair.first))); });
        // std::string path = "models";
        // for (const auto &entry : std::filesystem::directory_iterator(path))
        //     models.push_back(entry.path().std::filesystem::path::c_str());

        // firstFrame = false;
        // }

        // This is a faster version of if statement above
        if (firstFrame)
        {
            std::for_each(gameObjects.begin(), gameObjects.end(), [](auto &pair)
                          { items.push_back(intToChar(pair.first)); });
            items.resize(gameObjects.size());
            firstFrame = false;
        }

        static int selectedObject = atoi(items[0]);

        ImGui::Combo(" ", &selectedObject, items.data(), gameObjects.size(), 4);
        auto object = &gameObjects.at(selectedObject);

        ImGui::Text("Translation");
        ImGui::InputFloat3(" ", &object->transform.translation.x);
        ImGui::SliderFloat3("  ", &object->transform.translation.x, -10.0f, 10.0f);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Text("Rotation");
        ImGui::InputFloat3("   ", &object->transform.rotation.x);
        ImGui::SliderFloat3("    ", &object->transform.rotation.x, -10.0f, 10.0f);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Text("Scale");
        ImGui::InputFloat3("     ", &object->transform.scale.x);
        ImGui::SliderFloat3("      ", &object->transform.scale.x, -10.0f, 10.0f);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Text("Roughness");
        ImGui::InputFloat("      ", &object->transform.roughness);
        ImGui::SliderFloat("       ", &object->transform.roughness, -1.0f, 1.0f);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

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

    App::App()
    {
        globalPool = veDescriptorPool::Builder(pDevice)
                         .setMaxSets(veSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, veSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();

        loadGameObjects();
    }

    App::~App() {}

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
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

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
        }

        vkDeviceWaitIdle(pDevice.device());
        close_imgui();
    }

    void App::loadGameObjects()
    {

        std::shared_ptr<veModel> model1 = veModel::createModelFromFile(pDevice, currentPath() + "/../models/smooth_vase.obj");

        auto obj1 = veGameObject::createGameObject();
        obj1.model = model1;
        obj1.transform.translation = {-.5f, .5f, 0.f};
        obj1.transform.rotation = {.0f, .0f, 0.0f};
        obj1.transform.scale = {1.5f, 1.5f, 1.5f};
        obj1.transform.roughness = 0.8f;

        std::shared_ptr<veModel> model2 = veModel::createModelFromFile(pDevice, currentPath() + "/../models/pose.obj");

        auto obj2 = veGameObject::createGameObject();
        obj2.model = model2;
        obj2.transform.translation = {.2f, .5f, 0.f};
        obj2.transform.rotation = {.0f, .0f, 0.0f};
        obj2.transform.scale = {1.5f, 1.5f, 1.5f};
        obj2.transform.roughness = 0.0f;

        std::shared_ptr<veModel> floorModel = veModel::createModelFromFile(pDevice, currentPath() + "/../models/floor.obj");
        auto floor = veGameObject::createGameObject();
        floor.model = floorModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.rotation = {0.f, 0.f, 0.f};
        floor.transform.scale = {20.f, 1.f, 20.f};
        floor.transform.roughness = 0.8f;

        std::shared_ptr<veModel> motoModel = veModel::createModelFromFile(pDevice, currentPath() + "/../models/Srad 750.obj");
        auto moto1 = veGameObject::createGameObject();
        moto1.model = motoModel;
        moto1.transform.translation = {0.f, .5f, 0.f};
        moto1.transform.rotation = {0.f, 0.f, 0.f};
        moto1.transform.scale = {1.f, 1.f, 1.f};
        moto1.transform.roughness = 0.0f;

        auto moto2 = veGameObject::createGameObject();
        moto2.model = motoModel;
        moto2.transform.translation = {-1.f, .5f, 0.f};
        moto2.transform.rotation = {0.f, 0.f, 0.f};
        moto2.transform.scale = {1.f, 1.f, 1.f};
        moto2.transform.roughness = 0.0f;

        gameObjects.emplace(obj1.getId(), std::move(obj1));
        gameObjects.emplace(obj2.getId(), std::move(obj2));
        gameObjects.emplace(floor.getId(), std::move(floor));
        gameObjects.emplace(moto1.getId(), std::move(moto1));
        gameObjects.emplace(moto2.getId(), std::move(moto2));

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

    id_t App::addGameObject(std::string &model)
    {
        auto obj = veGameObject::createGameObject();
        auto path = currentPath() + "/../models/" + model;
        obj.model = veModel::createModelFromFile(pDevice, currentPath() + "/../models/" + model);
        obj.transform.translation = {0.f, 0.f, 0.f};
        obj.transform.rotation = {0.f, 0.f, 0.f};
        obj.transform.scale = {1.f, 1.f, 1.f};

        gameObjects.emplace(obj.getId(), std::move(obj));
        newObject = true;

        return obj.getId();
    }
} // namespace ve