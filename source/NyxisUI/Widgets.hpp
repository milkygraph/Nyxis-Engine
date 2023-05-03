#pragma once
#include "Core/Nyxispch.hpp"

namespace NyxisWidgets
{
	float LineHeight();
	void TableFloat(const std::vector<const char*> labels, std::vector<float*> vecs, float step = 0.1);
	void TableVec3(const std::vector<const char*> labels, std::vector<glm::vec3*> vecs, float step = 0.1);
}
