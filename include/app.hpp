#pragma once
#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "model.hpp"

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>

namespace ve
{
    class App
    {
    public:
        App();
        ~App();

        // copy constructor and destructors

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        static constexpr int HEIGHT = 600;
        static constexpr int WIDTH = 800;

        void run();

    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int ImageIndex);

        veWindow pWindow{WIDTH, HEIGHT, "VulkanApp"};
        veDevice pDevice{pWindow};

        std::unique_ptr<veSwapChain> pSwapChain; 
        std::unique_ptr<vePipeline> pPipeline; 
        std::unique_ptr<veModel> pModel;

        std::vector<VkCommandBuffer> commandBuffers;
        VkPipelineLayout pipelineLayout;
    }; // class App
} // namespace ve
