/* 2017-2020 WEBGEARS SERVICES LIMITED (c) All Rights Reserved. 
 * Proprietary and confidential.
 * The Software and any accompanying documentation are copyrighted and protected 
 * by copyright laws and international copyright treaties, as well as other 
 * intellectual property laws and treaties.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 */

#pragma once
#include <string>

// 

/// @brief Place here system-independent special directory paths (temp, desktop etc)
namespace helpers {

    /// @brief: User home directory path
    /// Includes terminating slash
    std::wstring get_home_wpath();

    /// @brief: User temp directory path
    /// Includes terminating slash
    std::wstring get_tempdir_wpath();

    /// @brief: System-wide temp directory path
    /// Includes terminating slash
    std::wstring get_system_temp_wpath();

    /// @brief: User desktop path
    /// Includes terminating slash
    std::wstring get_desktop_wpath();

    /// @brief: The file system directory that contains application data for all users.
    /// Includes terminating slash
    std::wstring get_common_appdata_wpath();

} // namespace helpers
