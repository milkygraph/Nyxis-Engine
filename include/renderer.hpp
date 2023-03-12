#pragma once

#include "window.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "scene.hpp"
#include "frameInfo.hpp"

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

        [[nodiscard]] VkImageView GetWorldImageView(int index) const;
    	  [[nodiscard]] VkRenderPass GetSwapChainRenderPass() const { return pSwapChain->GetMainRenderPass(); }
        [[nodiscard]] VkRenderPass GetUIRenderPass() const { return pSwapChain->GetUIRenderPass(); }
        [[nodiscard]] float GetAspectRatio() const { return pSwapChain->ExtentAspectRatio(); }
        [[nodiscard]] bool IsFrameInProgress() const { return m_IsFrameStarted; }

        [[nodiscard]] VkCommandBuffer GetMainCommandBuffer() const
        {
            assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
            return m_MainCommandBuffers[m_CurrentImageIndex];
        }

        [[nodiscard]] VkCommandBuffer GetUICommandBuffer() const
        {
            assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
            return m_UICommandBuffers[m_CurrentImageIndex];
        }

        [[nodiscard]] int GetFrameIndex() const
        {
            // assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
            return m_CurrentImageIndex;
        }

        VkCommandBuffer BeginWorldFrame() ;
        void EndWorldFrame();

        VkCommandBuffer BeginUIFrame() ;
		void EndUIRenderPass(VkCommandBuffer commandBuffer);

    	void BeginMainRenderPass(VkCommandBuffer commandBuffer);
        void EndMainRenderPass(VkCommandBuffer commandBuffer);

        std::unique_ptr<SwapChain> pSwapChain;

    private:
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void RecreateSwapChain();

        Window &window = Window::get();
        Device &device = Device::get();
        Scene *scene = nullptr;

        std::vector<VkCommandBuffer> m_MainCommandBuffers;
        std::vector<VkCommandBuffer> m_UICommandBuffers;

        uint32_t m_CurrentImageIndex;
        int m_CurrentFrameIndex{0};
        bool m_IsFrameStarted{false};

    }; // class Renderer
} // namespace Nyxis
