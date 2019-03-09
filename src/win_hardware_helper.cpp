#if defined(_WIN32) || defined(_WIN64)
#include <winapi_helpers/win_hardware_helper.h>
#include <cstdlib>
#include <regex>
#include <Windows.h>
#include <intrin.h>

using namespace helpers;

std::string NativeHardwareHelper::get_volume_id()
{
    const size_t path_size = MAX_PATH + 1;
    char volume_name[path_size] = { 0 };
    char file_system_name[path_size] = { 0 };
    DWORD serial_number = 0;
    DWORD max_component_len = 0;
    DWORD file_system_flags = 0;
    std::string system_drive;
    
    static const char* drive_regex_str = R"(^[a-zA-Z]:\\$)";
    std::regex drive_regex(drive_regex_str);

    const char* system_drive_env = std::getenv("SystemDrive");
    if (nullptr == system_drive_env) {
        system_drive = "C:\\";
    }
    else {
        system_drive = system_drive_env;
        system_drive +='\\';
    }

    if (!std::regex_match(system_drive, drive_regex)) {
        system_drive = "C:\\";
    }
    
    if (GetVolumeInformationA(
        system_drive.c_str(),
        volume_name,
        path_size,
        &serial_number,
        &max_component_len,
        &file_system_flags,
        file_system_name,
        path_size))
    {
        return std::to_string(static_cast<unsigned long>(serial_number));
    }
    return std::string{};
}

std::string NativeHardwareHelper::get_cpu_id()
{
    int CPUInfo[4] = { -1 };
    __cpuid(CPUInfo, 0);
    std::string cpu_id_string(std::to_string(CPUInfo[0]));
    cpu_id_string += std::to_string(CPUInfo[1]);
    cpu_id_string += std::to_string(CPUInfo[2]);
    cpu_id_string += std::to_string(CPUInfo[3]);
    return cpu_id_string;
}

#endif // defined(_WIN32) || defined(_WIN64)
