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
        Scene() = default;
        ~Scene();

        Entity createEntity(const std::string &name = std::string());
        void destroyEntity(Entity entity);

        // add component to entity
        template <typename T, typename... Args>
        T &addComponent(Entity entity, Args &&...args)
        {
            return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        // get entites with specific component
        template <typename T>
        auto getEntitiesWithComponent()
        {
            return m_Registry.view<T>();
        }

        std::string getEntityName(Entity entity)
        {
            return m_Registry.get<TagComponent>(entity).Tag;
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


    private:
        Registry m_Registry;
    };
} // namespace ve