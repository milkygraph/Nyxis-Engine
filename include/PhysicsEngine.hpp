#pragma once
#include "scene.hpp"

namespace ve
{
	class PhysicsEngine
	{
	public:
		PhysicsEngine() = default;
		~PhysicsEngine() = default;

		void OnUpdate(Scene& scene, float deltaTime);
	};
}
