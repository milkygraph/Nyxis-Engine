#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Descriptors.hpp"
#include "Core/Layer.hpp"
#include "Scene/Scene.hpp"
#include "NyxisUI/Viewport.hpp"

namespace Nyxis
{
    class EditorLayer : public Layer
    {
    public:
		EditorLayer() = default;
		~EditorLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnImGuiRender() override;
        void OnEvent() override;
        void SetScene(Ref<Scene> scene);

    	void Init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer);
		void Begin();
        void OnUpdate();
        void End();
    	void AddFunction(const std::function<void()>& function);
    	void AddComponentView();
        void AddSceneHierarchy();
        void AddMenuBar();

    	VkExtent2D GetViewportExtent() { return m_Viewport.GetExtent(); }

    private:
        void DrawEntityNode(Entity entity);

        Viewport m_Viewport;

    	Ref<Scene> m_ActiveScene;
        Entity m_SelectedEntity;
		bool m_ShowEntityLoader = false;
        std::vector<std::function<void()>> functions;
    	Ref<DescriptorPool> imguiPool{};

        VkCommandBuffer commandBuffer;
    	VkSampler m_Sampler;
		std::vector<VkDescriptorSet> dst;
    };
}