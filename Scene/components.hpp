#pragma once

#include "ve.hpp"
#include <glm/glm.hpp>
#include "model.hpp"
#include "path.hpp"

namespace ve
{
    // tag compoent
    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent &) = default;
        TagComponent &operator=(const TagComponent &) = default;
        TagComponent(TagComponent &&) = default;
        TagComponent &operator=(TagComponent &&) = default;
        ~TagComponent() = default;

        TagComponent(const std::string &tag)
            : Tag(tag)
        {
        }
    };

    // transform component
    struct TransformComponent
    {
        glm::vec3 translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        float roughness = 0.0f;

        TransformComponent() = default;
        TransformComponent(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale, float roughness)
            : translation(translation), rotation(rotation), scale(scale), roughness(roughness) {}

        TransformComponent(const TransformComponent &) = default;
        TransformComponent &operator=(const TransformComponent &) = default;
        TransformComponent(TransformComponent &&) = default;
        TransformComponent &operator=(TransformComponent &&) = default;
        ~TransformComponent() = default;

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };


    static std::unordered_map<std::string, std::shared_ptr<veModel>> meshstuff();
    struct MeshComponent
    {
        MeshComponent(std::string &modelName)
        {
            model = veModel::createModelFromFile(currentPath() + "/../models/" + modelName);
        }

        MeshComponent(std::string modelName)
        {
            model = veModel::createModelFromFile(currentPath() + "/../models/" + modelName);
        }

        std::shared_ptr<veModel> model;
    };

    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{};    // w is intensity
    };
}