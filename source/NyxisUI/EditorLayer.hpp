#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Descriptors.hpp"
#include "Core/Layer.hpp"
#include "Scene/Scene.hpp"
#include "NyxisUI/Viewport.hpp"
#include "NyxisUI/MenuBar.hpp"
#include "NyxisUI/SceneHierarchy.hpp"
#include "NyxisUI/ComponentViewPanel.hpp"
#include "NyxisUI/MaterialPanel.hpp"
#include "Graphics/GLTFModel.hpp"

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
		static void Begin();
        void OnUpdate();
		static void End();

		static void AddFunction(const std::function<void()>& function);

    	VkExtent2D GetViewportExtent() const { return m_Viewport.GetExtent(); }
    	static Entity GetSelectedEntity() { return m_SelectedEntity; }
        static Node* GetSelectedNode() { return m_SelectedNode; }
        static Material* GetSelectedMaterial() { return m_SelectedMaterial; }

        static void SetSelectedEntity(Entity entity);
        static void SetSelectedNode(Node* node) { m_SelectedNode = node; }
        static void SetSelectedMaterial(Material* material) { m_SelectedMaterial = material; }
        static void DeselectEntity();
		static void DeselectMaterial();

		static bool DisplayUIImage(std::unordered_map<ModelTexture*, VkDescriptorSet>& map, ModelTexture* texture);
    private:
		Ref<DescriptorPool> imguiPool{};
        Ref<Scene> m_ActiveScene;

    	Viewport m_Viewport;
        MenuBar m_MenuBar;
        SceneHierarchyPanel m_SceneHierarchy;
        ComponentViewPanel m_ComponentView;
		MaterialPanel m_MaterialView;

		static inline Entity m_SelectedEntity = entt::null;
        static inline Node* m_SelectedNode = nullptr;
    	static inline Material* m_SelectedMaterial = nullptr;
        static inline std::vector<std::function<void()>> functions;
    };
}
