#pragma once

#include "window.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "scene.hpp"


#include <memory>
#include <vector>
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Nyxispch.hpp"

namespace Nyxis
{
    class Renderer
    {
    public:
        Renderer(Scene &scene);
        ~Renderer();

        // copy constructor and destructors

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return pSwapChain->getRenderPass(); }
        float getAspectRatio() const { return pSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer()
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentImageIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentImageIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void SetScene(Scene& scene);
        void RenderScene();

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        Window &window = Window::get();
        Device &device = Device::get();
        Scene *scene = nullptr;
        std::unique_ptr<veSwapChain> pSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};

    }; // class Renderer
} // namespace Nyxis
