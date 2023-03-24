#pragma once
#include "Core/Window.hpp"
#include "Core/Device.hpp"
#include "Core/SwapChain.hpp"
#include "Scene/Scene.hpp"

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
        [[nodiscard]] VkExtent2D GetAspectRatio() const { return m_WorldImageSize; }
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
		VkExtent2D m_WorldImageSize;

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

		VkExtent2D m_OldWorldImageSize;
    }; // class Renderer
} // namespace Nyxis
