#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Device.hpp"

namespace Nyxis
{
    class SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

        SwapChain(VkExtent2D windowExtent);
        SwapChain(VkExtent2D windowExtent, VkExtent2D worldExtent, std::shared_ptr<SwapChain> previous);

        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        [[nodiscard]] VkFramebuffer GetSwapChainFrameBuffer(int index) const { return m_SwapChainFramebuffers[index]; }
        [[nodiscard]] VkFramebuffer GetWorldFrameBuffer(int index) const { return m_WorldFramebuffers[index]; }
        [[nodiscard]] VkRenderPass GetMainRenderPass() const { return m_MainRenderPass; }
        [[nodiscard]] VkRenderPass GetUIRenderPass() const { return m_UIRenderPass; }
        [[nodiscard]] VkImageView GetWorldImageView(int index) const { return m_WorldImageViews[index]; }
        [[nodiscard]] VkImage GetWorldImage(int index) const { return m_WorldImages[index]; }
        [[nodiscard]] size_t ImageCount() const { return m_WorldImages.size(); }
        [[nodiscard]] VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        [[nodiscard]] VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
        [[nodiscard]] VkExtent2D GetWorldExtent() const { return m_WorldExtent; }
        [[nodiscard]] uint32_t GetSwapChainWidth() const { return m_SwapChainExtent.width; }
        [[nodiscard]] uint32_t GetSwapChainHeight() const { return m_SwapChainExtent.height; }
        [[nodiscard]] float ExtentAspectRatio() const {
            return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
        }
        [[nodiscard]] VkFormat FindDepthFormat() const;

    	void RecreateWorldImages();
		void SetWorldImageExtent(VkExtent2D extent) { m_WorldExtent = extent; }
        void SubmitWorldCommandBuffers(const VkCommandBuffer* commandBuffer, uint32_t* imageIndex);
        VkResult SubmitSwapChainCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        VkResult AcquireNextImage(uint32_t *imageIndex);

    private:
        void Init();
        void CreateSwapChain();
        void CreateWorldImages();
        void CreateSwapChainImageViews();
        void CreateWorldImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateSwapChainFramebuffers();
        void CreateWorldFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

		VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        std::vector<VkFramebuffer> m_WorldFramebuffers;
        VkRenderPass m_MainRenderPass;
        VkRenderPass m_UIRenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemories;
        std::vector<VkImageView> m_DepthImageViews;

		std::vector<VkDeviceMemory> m_WorldImageMemories;
        std::vector<VkImage> m_WorldImages;
		std::vector<VkImage> m_SwapChainImages;

    	std::vector<VkImageView> m_WorldImageViews;
        std::vector<VkImageView> m_SwapChainImageViews;

        Device &device = Device::get();
        VkExtent2D m_WindowExtent;
        VkExtent2D m_WorldExtent;

        VkSwapchainKHR m_SwapChain;
        std::shared_ptr<SwapChain> m_OldSwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;

		std::vector<VkSemaphore> m_WorldImageAvailableSemaphores;
		std::vector<VkSemaphore> m_WorldRenderFinishedSemaphores;
		std::vector<VkFence> m_WorldInFlightFences;
		std::vector<VkFence> m_WorldImagesInFlight;

		VkSemaphore m_WorldImageAvailableSemaphore;

        size_t m_CurrentFrame = 0;
    };
} // namespace lve