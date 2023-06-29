#include "Graphics/PhysicsEngine.hpp"
#include "Core/Application.hpp"
#include "Scene/Components.hpp"

namespace Nyxis
{
	PhysicsEngine::PhysicsEngine()
	{
		LOG_INFO("[Core] Initializing Physics Engine");
	}

	PhysicsEngine::~PhysicsEngine()
	{
		LOG_INFO("[Core] Destroying Physics Engine");
	}

	void PhysicsEngine::OnUpdate(float deltaTime)
	{
        if (!enable)
            return;

        auto scene = Application::GetScene();
		auto view = scene->m_Registry.view<Collider>();
		
		for (auto& entity_1 : view)
		{
            auto& collider_1 = scene->m_Registry.get<Collider>(entity_1);
			auto& transform_1 = scene->m_Registry.get<TransformComponent>(entity_1);
            auto& rigidbody_1 = scene->m_Registry.get<RigidBody>(entity_1);

            // if Gravity component exists, apply gravity
            if (scene->m_Registry.all_of<Gravity>(entity_1))
            {
                transform_1.velocity.y += gravity * deltaTime;
            }

			for (auto& entity_2 : view)
			{
				if (entity_1 == entity_2)
					continue;
				auto& collider_2 = scene->m_Registry.get<Collider>(entity_2);
				auto& transform_2 = scene->m_Registry.get<TransformComponent>(entity_2);
                auto& rigidbody_2 = scene->m_Registry.get<RigidBody>(entity_2);

                auto distance = glm::distance(transform_1.translation, transform_2.translation);
                auto overlap = collider_1.radius + collider_2.radius - distance;

                if(overlap > 0)
                {
                    auto normal = glm::normalize(transform_1.translation - transform_2.translation);
                    auto relative_velocity = transform_1.velocity - transform_2.velocity;
                    auto normal_velocity = glm::dot(relative_velocity, normal);

                    if(normal_velocity < 0)
                    {
                        float mass_sum = rigidbody_1.mass + rigidbody_2.mass;
                        float impulse = (1 + rigidbody_1.restitution + rigidbody_2.restitution) * normal_velocity / mass_sum;

                        transform_1.velocity -= impulse * rigidbody_1.mass * normal;
                        transform_2.velocity += impulse * rigidbody_1.mass * normal;
                    }

                    // apply separation to prevent objects from getting into each other
                    auto separation = overlap * normal;
                    transform_1.translation += separation * (rigidbody_1.mass / (rigidbody_1.mass + rigidbody_2.mass));
                    transform_2.translation -= separation * (rigidbody_2.mass / (rigidbody_1.mass + rigidbody_2.mass));
                }
            }

			if (transform_1.translation.y > edges.y || transform_1.translation.y < -edges.y)
			{
				// when it hits the wall change the velocity component of y if needed
                transform_1.velocity.y = -transform_1.velocity.y * rigidbody_1.restitution;

                if (transform_1.translation.y > edges.y)
                    transform_1.translation.y = edges.y;
                else
                    transform_1.translation.y = -edges.y;
			}

			if (transform_1.translation.x > edges.x || transform_1.translation.x < -edges.x)
			{
                transform_1.velocity.x = -transform_1.velocity.x * rigidbody_1.restitution;
				if (transform_1.translation.x > edges.x)
					transform_1.translation.x = edges.x;
				else
					transform_1.translation.x = -edges.x;
			}

			transform_1.translation += transform_1.velocity * deltaTime;
		}
	}
}
