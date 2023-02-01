#include "app.hpp"
#include "components.hpp"
#include "descriptors.hpp"
#include "gameObject.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "swap_chain.hpp"
#include "Camera.hpp"
#include "CameraController.hpp"
#include "frameInfo.hpp"
#include "simpleRenderSystem.hpp"
#include "pointLightSystem.hpp"
#include "path.hpp"
#include "Log.hpp"

#include "vepch.hpp"

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

namespace ve
{
    App *App::pInstance = nullptr;

    App::App() {
        globalPool = veDescriptorPool::Builder()
                .setMaxSets(veSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, veSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();
        // calculate the time it takes for below code to execute
        auto start = std::chrono::high_resolution_clock::now();
        loadGameObjects();
        pScene.loadModels();
        std::cout << "loadGameObjects() took " << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start).count() << "ms" << std::endl;
        pWindow.SetEventCallback(std::bind(&App::OnEvent, this, std::placeholders::_1));
        pInstance = this;
    }

    App::~App() = default;

    void App::OnEvent(Event &e) {
        std::string event_name = e.toString();
#if LOGGING_LEVEL == 0
        LOG_INFO(event_name);
#endif // LOGGING_LEVEL
    }

    void App::run() {
        std::vector<std::unique_ptr<veBuffer>> uboBuffers(veSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<veBuffer>(
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        auto globalSetLayout = veDescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(veSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            veDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem srs{pRenderer.getSwapChainRenderPass(),
                               globalSetLayout->getDescriptorSetLayout()}; // srs - simpleRenderSystem
		PointLightSystem pls{ pRenderer.getSwapChainRenderPass(),
		                      globalSetLayout->getDescriptorSetLayout() };   // pls - pointLightSystem

        Camera Camera({0, -1, -2.5});

        auto currentTime = std::chrono::high_resolution_clock::now();

        bool firstFrame = true;

        pImguiLayer.AddFunction([&]() {
            //
            ImGui::Begin("Statistics");
            ImGui::Text("Entity Count: %d", pScene.getEntityCount());
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::End();
        });

        pImguiLayer.AddFunction([&]() {
            ImGui::Begin("Camera");
            ImGui::DragFloat("Move Speed", &Camera.getCameraController().moveSpeed, 0.1f);
            ImGui::End();
        });

        pImguiLayer.AddEntityLoader(pScene);

        auto commandBuffer = pRenderer.beginFrame();
        pImguiLayer.init(pRenderer.getSwapChainRenderPass(), commandBuffer);
        pRenderer.beginSwapChainRenderPass(commandBuffer);
        pRenderer.endSwapChainRenderPass(commandBuffer);
        pRenderer.endFrame();

        while (!pWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            commandBuffer = pRenderer.beginFrame();


            float aspect = pRenderer.getAspectRatio();

            Camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.01f, 1000.f);

            int frameIndex = pRenderer.getFrameIndex();
            FrameInfo frameInfo
                    {frameIndex, frameTime, commandBuffer, Camera, globalDescriptorSets[frameIndex], gameObjects,
                     pScene, 0};

            // updating Camera
            Camera.OnUpdate(frameInfo.frameTime);

            // updating buffers
            GlobalUbo ubo{};
            ubo.projection = Camera.getProjectionMatrix();
            ubo.view = Camera.getViewMatrix();
            ubo.inverseViewMatrix = Camera.getInverseViewMatrix();
            pls.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // rendering
            pRenderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
            srs.render(frameInfo);
            pls.render(frameInfo);
            pImguiLayer.OnUpdate(frameInfo);
            pRenderer.endSwapChainRenderPass(frameInfo.commandBuffer);
            pRenderer.endFrame();
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
		auto pose = pScene.createEntity("Pose");
		pScene.addComponent<TransformComponent>(pose,
			glm::vec3(.2f, .5f, 0.f),
			glm::vec3(.0f, .0f, 0.0f),
			glm::vec3(1.5f, 1.5f, 1.5f),
			0.0f);
		pScene.addComponent<MeshComponent>(pose, model_path + "Srad 750.obj");

		std::vector<glm::vec3> lightColors{
			{ 1.f, .1f, .1f },
			{ .1f, .1f, 1.f },
			{ .1f, 1.f, .1f },
			{ 1.f, 1.f, .1f },
			{ .1f, 1.f, 1.f },
			{ 1.f, 1.f, 1.f }};

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = veGameObject::makePointLight(0.8f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
    }
} // namespace ve