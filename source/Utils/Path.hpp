#pragma once
#include <filesystem>

namespace Nyxis
{
        const auto current_path = std::filesystem::current_path().std::filesystem::path::string();
		const auto model_path = current_path + "/../models/";
		const auto texture_path = current_path + "/../textures/";
}