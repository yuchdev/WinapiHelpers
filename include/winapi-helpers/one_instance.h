#pragma once

#if defined(_WIN32) || defined(_WIN64)

namespace helpers{

class NativeOneInstance
{
public:
/// @brief Use Windows system-specific named object so that determine the only app instance
static bool is_instance_exists(const char* instance_id);
};

} // namespace helpers
#endif
