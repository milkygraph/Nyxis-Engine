#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Descriptors.hpp"
#include "Core/Layer.hpp"
#include "Scene/Scene.hpp"
#include "NyxisUI/Viewport.hpp"
#include "NyxisUI/MenuBar.hpp"
#include "NyxisUI/SceneHierarchy.hpp"
#include "NyxisUI/ComponentViewPanel.hpp"

namespace Nyxis
{
    class EditorLayer : public Layer
    {
    public:
		EditorLayer() = default;
		~EditorLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent() override;
        void SetScene(Ref<Scene> scene);

    	void Init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer);
		void Begin();
        void OnUpdate();
        void End();

        static void AddFunction(const std::function<void()>& function);
        static void SetSelectedEntity(Entity entity);
        static void DeselectEntity();
        static Entity GetSelectedEntity() { return m_SelectedEntity; }

    	VkExtent2D GetViewportExtent() const { return m_Viewport.GetExtent(); }
    private:
        Ref<DescriptorPool> imguiPool{};
        Ref<Scene> m_ActiveScene;

    	Viewport m_Viewport;
        MenuBar m_MenuBar;
        SceneHierarchyPanel m_SceneHierarchy;
        ComponentViewPanel m_ComponentView;

		static inline Entity m_SelectedEntity = entt::null;
        static inline std::vector<std::function<void()>> functions;

    	bool m_ShowEntityComponents = false;
    };
}