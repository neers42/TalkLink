#include "EnvHelper.hpp"
#include <cstdlib>
#include <stdexcept>

std::string get_env(const char* name)
{
    const char* value = std::getenv(name);

    if (value == nullptr)
    {
        throw std::runtime_error(
            std::string("Environment variable is not set: ") + name);
    }

    return value;
}
