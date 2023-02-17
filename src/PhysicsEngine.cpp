#include "PhysicsEngine.hpp"
#include "components.hpp"

namespace ve
{
	void PhysicsEngine::OnUpdate(Scene& scene, float deltaTime)
	{
		auto view = scene.m_Registry.view<Collider>();
		
		for (auto& entity_1 : view)
		{
            auto& collider_1 = scene.m_Registry.get<Collider>(entity_1);
			auto& rigidBody_1 = scene.m_Registry.get<RigidBody>(entity_1);

            // if Gravity component exists, apply gravity
            if (scene.m_Registry.all_of<Gravity>(entity_1))
            {
                auto& gravity = scene.m_Registry.get<Gravity>(entity_1);
                rigidBody_1.velocity.y += gravity.gravity * deltaTime;
            }

			for (auto& entity_2 : view)
			{
				if (entity_1 == entity_2)
					continue;
				auto& collider_2 = scene.m_Registry.get<Collider>(entity_2);
				auto& rigidBody_2 = scene.m_Registry.get<RigidBody>(entity_2);

			}

			if (rigidBody_1.translation.y > edges.y || rigidBody_1.translation.y < -edges.y)
			{
				rigidBody_1.velocity.y = 0.0f;
                if (rigidBody_1.translation.y > edges.y)
                    rigidBody_1.translation.y = edges.y;
                else
                    rigidBody_1.translation.y = -edges.y;
			}

			if (rigidBody_1.translation.x > edges.x || rigidBody_1.translation.x < -edges.x)
			{
				rigidBody_1.velocity.x = 0.0f;
				if (rigidBody_1.translation.x > edges.x)
					rigidBody_1.translation.x = edges.x;
				else
					rigidBody_1.translation.x = -edges.x;
			}

			rigidBody_1.translation += rigidBody_1.velocity * deltaTime;
		}
	}
}
