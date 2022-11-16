#include <filesystem>

namespace ve
{
    inline std::string currentPath()
    {
        return std::filesystem::current_path().std::filesystem::path::string();
    }
}