#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

#include <string>

namespace helpers
{

void set_memory_profiling();
void set_console_ctrl_handler(PHANDLER_ROUTINE ctrl_handler);

std::string wstring_to_utf8(const std::wstring& var);
std::wstring utf8_to_wstring(const std::string& var);
std::string wstring_to_string(const std::wstring& var);
std::wstring string_to_wstring(const std::string& var);

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)
