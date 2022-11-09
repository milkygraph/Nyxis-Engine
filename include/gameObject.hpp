#pragma once

#include "model.hpp"

#include <memory>


namespace ve
{
    struct Transform2dComponent
    {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;
        glm::mat2 mat2()
        {
            auto cos = glm::cos(rotation);
            auto sin = glm::sin(rotation);

            glm::mat2 rotMat{{cos, sin}, {-sin, cos}};
            glm::mat2 scaleMat{{scale.x, .0f}, {.0f ,scale.y}};
            return rotMat * scaleMat; 
        }
    };

    class veGameObject
    {
    public:
        using id_t = unsigned int;

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
        Transform2dComponent transform2d;

    private:
        veGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}