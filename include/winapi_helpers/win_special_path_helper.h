#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <string>

namespace helpers {

/// @brief Place here Windows-related special directory paths (temp, desktop etc)
/// All paths include terminating slash!
class NativePathHelpers
{
public:

    /// @brief: User home directory path
    /// Includes terminating slash
    /// @return: home path or "" if unable to detect
    static std::wstring get_home_path();

    /// @brief: User AppData/Local/Temp directory path
    /// Includes terminating slash
    /// @return: TEMP path or "" if unable to detect
    static std::wstring get_tempdir_path();

    /// @brief: Common (not user!) desktop path
    /// Includes terminating slash
    static std::wstring get_desktop_path();

    /// @brief: User AppData/Local directory path
    /// Includes terminating slash
    /// @return: AppData/Local path or "" if unable to detect
    static std::wstring get_local_appdata_path();

    /// @brief: User AppData/Roaming directory path
    /// Includes terminating slash
    /// @return: AppData/Roaming path or "" if unable to detect
    static std::wstring get_roaming_appdata_path();

    /// @brief: Common AppData directory path
    /// Includes terminating slash
    /// @return: Common AppData path or "" if unable to detect
    static std::wstring get_common_appdata_path();

    /// @brief: Windows root directory path
    /// Includes terminating slash
    static std::wstring get_windows_path();

    /// @brief: Directory, where service can write log file
    /// Includes terminating slash
    static std::wstring get_service_log_path();

    /// @brief: Windows temporary directory path
    /// Includes terminating slash
    static std::wstring get_system_temp_path();

    //////////////////////////////////////////////////////////////////////////
    // "Append to" methods

    /// @brief Return string (path) which is added to C:/...AppData/Local prefix 
    /// or empty string if path does not exist
    static std::wstring append_to_local(const wchar_t* path);

    /// @brief Return string (path) which is added to C:/...AppData/Roamong prefix
    /// or empty string if path does not exist
    static std::wstring append_to_roaming(const wchar_t* path);

    /// @brief Return string (path) which is added to C:/.../$USERNAME prefix
    /// or empty string if path does not exist
    static std::wstring append_to_home(const wchar_t* path);

private:

    /// @brief Get special directory path
    /// @param special_path_type: macros type see in file "shlobj.h", e.g. CSIDL_APPDATA
	/// @param ignoreSystemUserCheck: true to skip check for running under system user account(e.g. to get common app data path)
    /// return empty string if path does not exist
    static std::wstring get_appdata_path(int special_path_type, bool ignoreSystemUserCheck = false);

    /// @brief Create Temp directory on the root disk
    static std::wstring create_root_tempdir();

};

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)