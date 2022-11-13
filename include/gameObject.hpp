#pragma once

#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace ve
{
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

        std::shared_ptr<veModel> model{};
        glm::vec3 color{};
        TransformComponent transform;
    
    private:
        veGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}