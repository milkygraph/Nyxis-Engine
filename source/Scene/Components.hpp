#pragma once
#include "Core/Input.hpp"
#include "Core/Nyxispch.hpp"
#include "Graphics/OBJModel.hpp"
#include "Events/KeyEvents.hpp"
#include "Utils/Path.hpp"

#include <json/json.hpp>

namespace Nyxis
{
    struct RigidBody
    {
        glm::vec3 translation{ 0.0f };
        glm::vec3 rotation{ 0.0f };
        glm::vec3 scale{ 1.0f };

        glm::vec3 velocity{ 0.0f };
        glm::vec3 acceleration{ 0.0f };

        float roughness{ 0.0f };
        float mass{ 1.0f };
        float friction{ 0.5f };
        float restitution{ 0.8f };
        bool isStatic{ false };
        bool isKinematic{ false };
        bool isTrigger{ false };

        glm::mat4 mat4(bool flipY = false);
        glm::mat3 normalMatrix();
    };

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

        void ToJson(nlohmann::json& j) const
        {
            j["Tag"] = Tag;
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
		TransformComponent(glm::vec3 translation, glm::vec3 rotation = glm::vec3{0.0f}, glm::vec3 scale = glm::vec3{1.0f}, float roughness = 0.0f)
            : translation(translation), rotation(rotation), scale(scale), roughness(roughness) {}

        TransformComponent(const TransformComponent &) = default;
        TransformComponent &operator=(const TransformComponent &) = default;
        TransformComponent(TransformComponent &&) = default;
        TransformComponent &operator=(TransformComponent &&) = default;
        ~TransformComponent() = default;

        glm::mat4 mat4();
        glm::mat3 normalMatrix();

        void ToJson(nlohmann::json& j) const
        {
            j["Transform"] = {
                    {translation.x, translation.y, translation.z},
                    {rotation.x, rotation.y, rotation.z},
                    {scale.x, scale.y, scale.z},
                    {roughness}
                };
        }

        void FromJson(const nlohmann::json& j)
        {
        }
    };

	struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{};    // w is intensity

        void ToJson(nlohmann::json& j) const
        {
            j["PointLight"] = {
                    {"position", {position.x, position.y, position.z}},
                    {"color", {color.x, color.y, color.z, color.w}},
                };
        }

        void FromJson(const nlohmann::json& j)
        {
            position.x = j["PointLight"]["position"][0];
            position.y = j["PointLight"]["position"][1];
            position.z = j["PointLight"]["position"][2];

            color.x = j["PointLight"]["color"][0];
            color.y = j["PointLight"]["color"][1];
            color.z = j["PointLight"]["color"][2];
            color.w = j["PointLight"]["color"][3];
        }
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

        void ToJson(nlohmann::json& j) const
        {
            j["MeshComponent"] ={ {"filepath", filepath} };
        }

        void FromJson(const nlohmann::json& j)
        {
            filepath = j["MeshComponent"]["filepath"];
            model = OBJModel::CreateModel(model_path + filepath);
        }
    };

    struct Player
    {
        float moveSpeed{ 1.f };

        void OnUpdate(float dt, RigidBody& rigidBody)
        {
            // 2d movement
            glm::vec3 moveDir{ 0.f };
            if (Input::isKeyPressed(KeyCodes::W))
                moveDir += glm::vec3{ 0.f, -1.f, 0.f };
            if (Input::isKeyPressed(KeyCodes::S))
                moveDir -= glm::vec3{ 0.f, -1.f, 0.f };
            if (Input::isKeyPressed(KeyCodes::D))
                moveDir += glm::vec3{ 1.f, 0.f, 0.f };
            if (Input::isKeyPressed(KeyCodes::A))
                moveDir -= glm::vec3{ 1.f, 0.f, 0.f };

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
            {
                rigidBody.velocity += moveSpeed * dt * glm::normalize(moveDir);
            }
        }

        void ToJson(nlohmann::json& j) const
        {
            j["Player"] ={ {"moveSpeed", moveSpeed} };
        }

        void FromJson(const nlohmann::json& j)
        {
            moveSpeed = j["Player"]["moveSpeed"];
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
        Collider(ColliderType type, glm::vec3 size, float radius) 
            : type(type), size(size), radius(radius) {}
        ColliderType type;
        
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