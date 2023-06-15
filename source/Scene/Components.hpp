#pragma once
#include "Core/Input.hpp"
#include "Core/Nyxispch.hpp"
#include "Graphics/OBJModel.hpp"
#include "Events/KeyEvents.hpp"

#include <json/json.hpp>

namespace Nyxis
{
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
        {}
    };

    struct TransformComponent
    {
        glm::vec3 translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};

        glm::vec3 velocity{ 0.0f };
        glm::vec3 acceleration{ 0.0f };

        TransformComponent() = default;
		TransformComponent(glm::vec3 translation, glm::vec3 rotation = glm::vec3{0.0f}, glm::vec3 scale = glm::vec3{1.0f}, float roughness = 0.0f)
            : translation(translation), rotation(rotation), scale(scale) {}

        TransformComponent(const TransformComponent &) = default;
        TransformComponent &operator=(const TransformComponent &) = default;
        TransformComponent(TransformComponent &&) = default;
        TransformComponent &operator=(TransformComponent &&) = default;
        ~TransformComponent() = default;

        glm::mat4 mat4(bool flip = false);
        glm::mat3 normalMatrix();
    };

    struct RigidBody
    {
        float roughness{ 0.0f };
        float mass{ 1.0f };
        float friction{ 0.5f };
        float restitution{ 0.8f };
        bool isStatic{ false };
        bool isKinematic{ false };
        bool isTrigger{ false };
    };

	struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{};    // w is intensity
    };

    class MeshComponent
	{
	public:
        MeshComponent() = default;
		MeshComponent(const std::string& filepath)
		{
            model = OBJModel::CreateModel(filepath);
            this->filepath = filepath;
        }
		void draw(VkCommandBuffer commandBuffer)
		{
			model->draw(commandBuffer);
		}
		std::shared_ptr<OBJModel> model;
        std::string filepath;
    };

    struct Player
    {
        float moveSpeed{ 1.f };

        void OnUpdate(float dt, TransformComponent& transform)
        {
            // 2d movement
            glm::vec3 moveDir{ 0.f };
            if (Input::IsKeyPressed(KeyCodes::W))
                moveDir += glm::vec3{ 0.f, -1.f, 0.f };
            if (Input::IsKeyPressed(KeyCodes::S))
                moveDir -= glm::vec3{ 0.f, -1.f, 0.f };
            if (Input::IsKeyPressed(KeyCodes::D))
                moveDir += glm::vec3{ 1.f, 0.f, 0.f };
            if (Input::IsKeyPressed(KeyCodes::A))
                moveDir -= glm::vec3{ 1.f, 0.f, 0.f };

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
            {
                transform.velocity += moveSpeed * dt * glm::normalize(moveDir);
            }
        }
    };

    enum class ColliderType
    {
		Sphere,
		Box,
		Capsule,
		Mesh
    };

    struct Collider
    {
        Collider() = default;
        Collider(ColliderType type, glm::vec3 size, float radius) 
            : type(type), size(size), radius(radius) {}
        ColliderType type = ColliderType::Sphere;
        
		glm::vec3 size{ 1.0f };
        float radius;
    };

    static std::unordered_map<ColliderType, std::string> collider_name{{ ColliderType::Sphere, "Sphere" },
                     { ColliderType::Box, "Box" },
                     { ColliderType::Capsule, "Capsule" },
                     { ColliderType::Mesh, "Mesh" }};

    struct Gravity
    {
        Gravity() = default;
        bool flag = true;
    };

    struct Particle
    {
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 position{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
		glm::vec2 size = { 0.1f, 0.1f };
    };
}