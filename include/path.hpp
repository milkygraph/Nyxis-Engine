#pragma once

#include <filesystem>

namespace ve
{
        const auto current_path = std::filesystem::current_path().std::filesystem::path::string();
		const auto model_path = current_path + "/../models/";
}