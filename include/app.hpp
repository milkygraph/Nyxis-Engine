#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

#include <memory>
#include <vector>

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


        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

    private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        veWindow pWindow{WIDTH, HEIGHT, "Vulkan"};
        veDevice pDevice{pWindow};
        veSwapChain pSwapChain{pDevice, pWindow.getExtent()}; 
        std::unique_ptr<vePipeline> pPipeline; 
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
    };
} // namespace ve