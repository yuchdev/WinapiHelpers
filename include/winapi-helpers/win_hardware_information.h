/* 2017-2018 WEBGEARS SERVICES LIMITED (c) All Rights Reserved. 
 * Proprietary and confidential.
 * The Software and any accompanying documentation are copyrighted and protected 
 * by copyright laws and international copyright treaties, as well as other 
 * intellectual property laws and treaties.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 */

#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <string>

namespace helpers{

/// @brief Hardware IDs from WinAPI call NativePartititonInformation::get_volume_information()
/// for the system drive, and system-independent __cpuid()
class NativeHardwareInformation
{
public:

    /// @brief Retrieve Volume IDs of system drive using NativePartititonInformation and CPUID
    NativeHardwareInformation();

    /// @brief Volume name if defined
    std::string get_volume_name();

    /// @brief HDD volume ID (string of numbers)
    std::string get_volume_id();

    /// @brief Filesystem name (Usually uppercase, e.g. "NTFS")
    std::string get_filesystem_name();

    /// @brief CPU ID (string of numbers)
    std::string get_cpu_id();

    /// @brief In case of unsuccessful retrieval last error 
    /// in a string format, for notifying the caller
    std::string get_last_error() const;

    /// @brief Return true if hardware IDs is retrieved successfully
    bool info_retrived_successfully() const;

private:

    /// Volume name if defined
    std::string volume_name_;

    /// HDD volume ID (string of numbers)
    std::string volume_id_;

    /// Filesystem name like "NTFS"
    std::string filesystem_name_;

    /// CPUID (string of numbers)
    std::string cpu_id_;

    /// GetLastError in string format, for the notifying holder class
    std::string last_error_;

    /// Class state flag
    /// NativeSystemInformation exists even if not all information has been retrieved
    bool retrived_successfully_ = true;
};

} // namespace helpers
#endif
