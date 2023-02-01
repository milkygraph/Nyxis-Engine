#pragma once

#include "model.hpp"
#include "components.hpp"

#include "vepch.hpp"

namespace ve
{
    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };

    class veGameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, veGameObject>;

        static veGameObject createGameObject()
        {
            static id_t currentId = 0;
            return veGameObject{currentId++};
        }

        veGameObject(const veGameObject &) = delete;
        veGameObject &operator=(const veGameObject &) = delete;

        veGameObject(veGameObject &&) = default;
        veGameObject &operator=(const veGameObject &&) = delete;

        id_t getId() { return id; }
        static veGameObject makePointLight(float intensity = 1.0, float radius = 0.2, glm::vec3 color = glm::vec3{1.0});

        glm::vec3 color{1.0f, 1.0f, 1.0f};
        TransformComponent transform;

        std::shared_ptr<veModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        // TODO: maybe add a string name to each game object to help identification by imgui?
        //  will leave the unwanted game objects' names empty

    private:
        veGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}