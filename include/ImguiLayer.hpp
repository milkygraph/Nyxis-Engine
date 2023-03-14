#pragma once

#include "Nyxispch.hpp"
#include "frameInfo.hpp"
#include "descriptors.hpp"
#include "scene.hpp"

namespace Nyxis
{
    class ImguiLayer
    {
    public:
        ImguiLayer(Scene& scene);
        ~ImguiLayer();

        void init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer);
        VkExtent2D OnUpdate(FrameInfo &frameInfo, VkImageView imageView);
        void AddFunction(const std::function<void()>& function);
        VkExtent2D AddViewport(FrameInfo& frameInfo, VkImageView imageView);
        void AddComponentView();
        void AddSceneHierarchy();
        void AddMenuBar();
    private:
        void DrawEntityNode(Entity entity);

        Scene& m_ActiveScene;
        Entity m_SelectedEntity;
		bool m_ShowEntityLoader = false;
        std::vector<std::function<void()>> functions;
    	std::unique_ptr<veDescriptorPool> imguiPool{};

		VkSampler m_Sampler;
		std::vector<VkDescriptorSet> dst;
    };
}