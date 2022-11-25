#pragma once
#include "ve.hpp"
#include "components.hpp"
#include <entt/entt.hpp>

namespace ve
{
    using Entity = entt::entity;
    using Registry = entt::registry;

    class Scene
    {
    public:
        Scene(veDevice& device) {}
        ~Scene();

        Entity createEntity(const std::string &name = std::string());
        void destroyEntity(Entity entity);

        inline uint32_t getEntityCount() { return m_EntityCount; }

        // add component to entity
        template <typename T, typename... Args>
        T &addComponent(Entity entity, Args &&...args)
        {
            return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename...Ts>
        void addComponents(Entity entity, Ts&&...args)
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
        T &getComponent(Entity entity)
        {
            return m_Registry.get<T>(entity);
        }

        template<typename A, typename B>
        auto getComponentView()
        {
            return m_Registry.group<A, B>();
        }
        template<typename A>
        auto getComponentView()
        {
            return m_Registry.view<A>();
        }

        float m_SkyColor[3] = {0.0f, 0.0f, 0.0f};        
        Registry m_Registry;

    private:
        // float array of 3 floats
        uint32_t m_EntityCount = 0;
    };
} // namespace ve