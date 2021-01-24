#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <memory>
#include <map>
#include <string>


namespace helpers{

/// @brief x64 application on x64 system
constexpr bool is_x64_application()
{
#ifdef _M_X64
    return true;
#else
    return false;
#endif
}


/// @brief x32 application on x64 system
bool is_x32_application_on_x64();

/// @brief Collect system-dependent OS and architecture information 
/// from WinAPI calls GetSystemInfo() and GetVersionEx()
class NativeSystemInformation{
public:

    /// @brief Collect available system information
    NativeSystemInformation();

    /// @brief logical number of CPU (cores)
    int number_of_cpu() const
    {
        return number_of_cpu_;
    }

    /// @brief Windows build number
    int build_number() const
    {
        return os_build_number_;
    }

    /// @brief Windows major version
    int major_version() const
    {
        return os_major_version_;
    }

    /// @brief Windows minor version
    int minor_version() const
    {
        return os_minor_version_;
    }

    /// @brief String representation of CPU architecture
    /// "Unknown" if impossible to detect
    const std::string& cpu_architecture() const
    {
        return cpu_architecture_;
    }

	/// @brief String representation of OS family
	const std::string os_family() const
	{
		return "windows";
	}

    /// @brief String representation of OS name and major version
    /// "Unknown" if impossible to detect
    const std::string& os_name() const
    {
        return os_name_;
    }

    /// @brief String representation of OS. Composed from major, minor and build number
    /// "Unknown" if impossible to detect
    const std::string& os_version() const
    {
        return os_version_;
    }

    /// @brief Return true if OS information is retrieved successfully
    /// Just for caller 
    bool info_retrived_successfully() const 
    {
        return retrived_successfully_;
    }

    /// @brief In case of unsuccessful retrieval GetLastError 
    /// in a string format, for the notifying holding class
    std::string get_last_error() const;

private:

    /// @brief Convert major.minor OS version to string
    void get_system_version();

    /// @brief Converts Windows Vista versions to string
    std::string windows_vista_type(unsigned long product_type);

    /// @brief Converts Windows 7 versions to string
    std::string windows_7_type(unsigned long product_type);

    /// @brief Converts Windows 8 versions to string
    std::string windows_8_type(unsigned long product_type);

    /// @brief Converts Windows 8.1 versions to string
    std::string windows_81_type(unsigned long product_type);

    /// @brief Converts Windows 10 versions to string
    std::string windows_10_type(unsigned long product_type);

private:

    /// Extracted from SYSTEM_INFO
    unsigned long number_of_cpu_ = 0;

    /// Extracted from SYSTEM_INFO
    std::string cpu_architecture_;

    /// Extracted from OSVERSIONINFOEX 
    std::string os_name_;

    /// String representation major.minor.nuild_number
    std::string os_version_;

    /// Extracted from OSVERSIONINFOEX 
    unsigned long os_build_number_;

    /// Extracted from OSVERSIONINFOEX 
    unsigned long os_major_version_;

    /// Extracted from OSVERSIONINFOEX 
    unsigned long os_minor_version_;

    /// GetLastError in string format, for the notifying holding class
    std::string last_error_;

    /// Class state flag
    /// NativeSystemInformation exists even if not all information has been retrieved
    bool retrived_successfully_ = true;

    /// Map major.minor OS version to string
    static std::map<unsigned short, std::string> cpu_architectures_;
};

} // namespace helpers
#endif
