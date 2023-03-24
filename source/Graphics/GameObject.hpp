#pragma once
#include "Core/Nyxispch.hpp"
#include "Graphics/OBJModel.hpp"
#include "Scene/Components.hpp"

// Deprecated
namespace Nyxis
{
    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };

    class GameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, GameObject>;

        static GameObject createGameObject()
        {
            static id_t currentId = 0;
            return GameObject{currentId++};
        }

        GameObject(const GameObject &) = delete;
        GameObject &operator=(const GameObject &) = delete;

        GameObject(GameObject &&) = default;
        GameObject &operator=(const GameObject &&) = delete;

        id_t getId() { return id; }
        static GameObject makePointLight(float intensity = 1.0, float radius = 0.2, glm::vec3 color = glm::vec3{1.0});

        glm::vec3 color{1.0f, 1.0f, 1.0f};
        TransformComponent transform;

        std::shared_ptr<OBJModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        // TODO: maybe add a string name to each game object to help identification by imgui?
        //  will leave the unwanted game objects' names empty

    private:
        GameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}