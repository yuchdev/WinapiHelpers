#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <string>

namespace helpers{

/// @brief Hardware ID related to Windows systems
class NativeHardwareHelper
{
public:

    /// @brief HDD volume ID (string of numbers)
    static std::string get_volume_id();

    /// @brief CPU ID (string of numbers)
    static std::string get_cpu_id();
};

} // namespace helpers
#endif
