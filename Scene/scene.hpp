#pragma once
#include "Nyxis.hpp"
#include "components.hpp"
#include "Camera.hpp"
#include "Log.hpp"

#include <entt/entt.hpp>

#include <atomic>
#include <thread>
#include <mutex>

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
        Scene();
        ~Scene();

        Entity createEntity(const std::string &name);
        std::pair<std::string, Entity> addEntity(const std::string &filename);
        void destroyEntity(Entity entity);

        inline uint32_t getEntityCount() { return m_EntityCount; }

        // add component to entity
        template <typename T, typename... Args>
        T &addComponent(Entity entity, Args &&...args)
        {
            return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename... Ts>
        void addComponents(Entity entity, Ts &&...args)
        {
            (addComponent<Ts>(entity, std::forward<Ts>(args)...), ...);
        }

        // get entites with specific component
        template <typename T>
        auto getEntitiesWithComponent()
        {
            return m_Registry.view<T>();
        }

        TagComponent getEntityName(Entity entity)
        {
            return m_Registry.get<TagComponent>(entity);
        }

        // get component from entity
        template <typename T>
        auto &getComponent(Entity entity)
        {
            return m_Registry.get<T>(entity);
        }

        template <typename... Comps>
        auto getComponentView()
        {
			return m_Registry.view<Comps...>();
        }

        template <typename T>
        void RemoveComponent(Entity entity)
        {
            m_Registry.remove<T>(entity);
        }

        void ClearScene();
        void SaveScene(const std::string &filename = "scene.json");
        void LoadScene(const std::string &filename);
        void LoadModels();

		void OnUpdate(float dt, float aspect, bool projection);

		Camera *GetCamera() { return m_Camera; }

        Registry m_Registry;
        bool SaveSceneFlag = false;
        bool LoadSceneFlag = false;

        std::string SceneName = "scene.json";

    private:
		Camera* m_Camera = nullptr;
		Entity m_CameraEntity = entt::null;
        uint32_t m_EntityCount = 0;
        std::atomic_int m_loadingEntity = 0;
        Device &device = Device::get();

        std::queue<Entity> m_DeletionQueue;
    };
} // namespace Nyxis