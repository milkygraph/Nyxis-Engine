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

			for (auto& entity_2 : view)
			{
				if (entity_1 == entity_2)
					continue;
				auto& collider_2 = scene.m_Registry.get<Collider>(entity_2);
				auto& rigidBody_2 = scene.m_Registry.get<RigidBody>(entity_2);
				
					
			}
			
			if (rigidBody_1.translation.y > 0.8f)
			{
				rigidBody_1.velocity.y = 0.0f;
				rigidBody_1.translation.y = 0.8f;
			}

			if (rigidBody_1.translation.x > 1.f || rigidBody_1.translation.x < -1.f)
			{
				rigidBody_1.velocity.x = 0.0f;
				if (rigidBody_1.translation.x > 1.f)
					rigidBody_1.translation.x = 1.f;
				else
					rigidBody_1.translation.x = -1.f;
			}

			rigidBody_1.translation += rigidBody_1.velocity * deltaTime;
		}
	}
}
