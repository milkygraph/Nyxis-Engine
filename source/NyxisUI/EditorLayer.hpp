#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Descriptors.hpp"
#include "Scene/Scene.hpp"

namespace Nyxis
{
    class EditorLayer
    {
    public:
        EditorLayer(Scene& scene);
        ~EditorLayer();

        void init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer);
		void Begin();
        VkExtent2D OnUpdate(FrameInfo &frameInfo, VkImageView imageView);
        void End();
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
    	Ref<DescriptorPool> imguiPool{};

        VkCommandBuffer commandBuffer;
    	VkSampler m_Sampler;
		std::vector<VkDescriptorSet> dst;
    };
}