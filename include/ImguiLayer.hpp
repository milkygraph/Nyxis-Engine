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
        void OnUpdate(FrameInfo &frameInfo);
        void AddFunction(const std::function<void()>& function);
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
    };
}