#pragma once

#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace ve
{

    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };

    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
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

    private:
        veGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}