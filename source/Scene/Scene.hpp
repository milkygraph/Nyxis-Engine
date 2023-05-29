#pragma once
#include "Core/Camera.hpp"
#include "Core/Nyxispch.hpp"

#define LOAD_STATE_NONE 0
#define LOAD_STATE_STARTED 1
#define LOAD_STATE_FINISHED 2

namespace Nyxis
{
    using Entity = entt::entity;
    using Registry = entt::registry;

    class Scene
    {
    public:
        Scene(const std::string name = "Default Scene");
        ~Scene();

        Entity CreateEntity(const std::string &name);
        std::pair<std::string, Entity> AddEntity(const std::string &filename);
        void DestroyEntity(Entity entity);

    	uint32_t GetEntityCount() { return m_EntityCount; }

        // Add component to entity
        template <typename T, typename... Args>
        T &AddComponent(Entity entity, Args &&...args)
        {
            if(!m_Registry.any_of<T>(entity))
				return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
            LOG_INFO("Component already exists");
            return m_Registry.get<T>(entity);
        }

        // Get component from entity
        template <typename T> auto &GetComponent(Entity entity)
        {
            return m_Registry.get<T>(entity);
        }

        // Get all entities with provided components
    	template <typename... Comps> auto GetComponentView()
        {
			return m_Registry.view<Comps...>();
        }

        template <typename T> void RemoveComponent(Entity entity)
        {
            vkDeviceWaitIdle(device.device());
            m_Registry.remove<T>(entity);
        }

        void ClearScene();
		void LoadModel(Entity entity, const std::string& filename);

        void OnUpdate(float dt, float aspect);

        Entity GetCameraEntity() { return m_CameraEntity; }
		Camera *GetCamera() { return m_Camera; }
		void SetCameraControl(bool control) { m_CameraControl = control; }
        std::string GetSceneName() { return m_SceneName; }

        Registry m_Registry;
        bool SaveSceneFlag = false;
        bool LoadSceneFlag = false;


    private:
        std::string m_SceneName;
        friend class GLTFRenderer;
		Camera* m_Camera = nullptr;
		Entity m_CameraEntity = entt::null;
        uint32_t m_EntityCount = 0;
        std::atomic_int m_loadingEntity = 0;
        Device &device = Device::Get();

        bool m_CameraControl = false;
    	std::queue<Entity> m_EntityDeletionQueue;
    };
} // namespace Nyxis