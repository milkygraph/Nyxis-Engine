#include "scene.hpp"

namespace ve
{

    Scene::~Scene()
    { m_Registry.clear(); }

    Entity Scene::createEntity(const std::string &name)
    {
        auto entity = m_Registry.create();
        m_Registry.emplace<TagComponent>(entity, name);
        m_EntityCount++;
        return entity;
    }

    void Scene::destroyEntity(Entity entity) { m_Registry.destroy(entity); m_EntityCount--; }
} // namespace ve