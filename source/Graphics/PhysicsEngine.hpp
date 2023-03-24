#pragma once
#include "Scene/Scene.hpp"

namespace Nyxis
{
	class PhysicsEngine
	{
	public:
		PhysicsEngine() = default;
		~PhysicsEngine() = default;

		void OnUpdate(Scene& scene, float deltaTime);

        glm::vec2 edges = glm::vec2(1.f, 0.8f);
        float gravity = 0.981f;
    private:
	};
}
