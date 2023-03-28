#include "Graphics/PhysicsEngine.hpp"
#include "Core/Application.hpp"
#include "Scene/Components.hpp"

namespace Nyxis
{
	void PhysicsEngine::OnUpdate(float deltaTime)
	{
        auto scene = Application::GetScene();
		auto view = scene->m_Registry.view<Collider>();
		
		for (auto& entity_1 : view)
		{
            auto& collider_1 = scene->m_Registry.get<Collider>(entity_1);
			auto& rigidBody_1 = scene->m_Registry.get<RigidBody>(entity_1);

            // if Gravity component exists, apply gravity
            if (scene->m_Registry.all_of<Gravity>(entity_1))
            {
                rigidBody_1.velocity.y += gravity * deltaTime;
            }

			for (auto& entity_2 : view)
			{
				if (entity_1 == entity_2)
					continue;
				auto& collider_2 = scene->m_Registry.get<Collider>(entity_2);
				auto& rigidBody_2 = scene->m_Registry.get<RigidBody>(entity_2);

                auto distance = glm::distance(rigidBody_1.translation, rigidBody_2.translation);
                auto overlap = collider_1.radius + collider_2.radius - distance;

                if(overlap > 0)
                {
                    auto normal = glm::normalize(rigidBody_1.translation - rigidBody_2.translation);
                    auto relative_velocity = rigidBody_1.velocity - rigidBody_2.velocity;
                    auto normal_velocity = glm::dot(relative_velocity, normal);

                    if(normal_velocity < 0)
                    {
                        float mass_sum = rigidBody_1.mass + rigidBody_2.mass;
                        float impulse = (1 + rigidBody_1.restitution + rigidBody_2.restitution) * normal_velocity / mass_sum;

                        rigidBody_1.velocity -= impulse * rigidBody_1.mass * normal;
                        rigidBody_2.velocity += impulse * rigidBody_1.mass * normal;
                    }

                    // apply separation to prevent objects from getting into each other
                    auto separation = overlap * normal;
                    rigidBody_1.translation += separation * (rigidBody_1.mass / (rigidBody_1.mass + rigidBody_2.mass));
                    rigidBody_2.translation -= separation * (rigidBody_2.mass / (rigidBody_1.mass + rigidBody_2.mass));
                }
            }

			if (rigidBody_1.translation.y > edges.y || rigidBody_1.translation.y < -edges.y)
			{
				// when it hits the wall change the velocity component of y if needed
                rigidBody_1.velocity.y = -rigidBody_1.velocity.y * rigidBody_1.restitution;

                if (rigidBody_1.translation.y > edges.y)
                    rigidBody_1.translation.y = edges.y;
                else
                    rigidBody_1.translation.y = -edges.y;
			}

			if (rigidBody_1.translation.x > edges.x || rigidBody_1.translation.x < -edges.x)
			{
                rigidBody_1.velocity.x = -rigidBody_1.velocity.x * rigidBody_1.restitution;
				if (rigidBody_1.translation.x > edges.x)
					rigidBody_1.translation.x = edges.x;
				else
					rigidBody_1.translation.x = -edges.x;
			}

			rigidBody_1.translation += rigidBody_1.velocity * deltaTime;
		}
	}
}
