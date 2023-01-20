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
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

namespace ve
{
    class veRenderer
    {
    public:
        veRenderer(Scene &scene);
        ~veRenderer();

        // copy constructor and destructors

        veRenderer(const veRenderer &) = delete;
        veRenderer &operator=(const veRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const
        {
            return pSwapChain->getRenderPass();
        }
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

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        veWindow &window = veWindow::get();
        veDevice &device = veDevice::get();
        Scene& scene;
        std::unique_ptr<veSwapChain> pSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};

    }; // class veRenderer
} // namespace ve
