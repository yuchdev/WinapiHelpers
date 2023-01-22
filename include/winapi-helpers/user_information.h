#pragma once
#include <string>
#include <memory>

// The header contains system-independent information about current (logged in) user

namespace helpers {

class NativeUserInformation;

/// @brief Storage for all available current (logged-in) user information in system-independent format
/// Some paths are system-specific, Like Windows Local or Roaming profile
/// They exist like methods but return "" under platforms where they don't make sense
class UserInformation {
public:

    /// @brief Construct on the first call, return "Meyers singleton" afterwards
    static UserInformation& instance();

    /// @brief Make compiler happy
    ~UserInformation() = default;

    /// @brief Get user name in "pretty readable" format
    std::string get_user_name() const;

    /// @brief Get user name GUID format
    /// @return: GUID or user name or empty string
    /// NOTE! Provide simple display name if the GUID is unavailable, it happen often
    std::string get_user_guid() const;

    /// @brief Get user Windows SID
    /// @return: SID or empty string
    std::string get_user_sid() const;

    /// @brief Get user home directory
    std::wstring get_home_path() const;

    /// @brief User Local profile
    /// @return: Windows Local profile path or empty string
    std::wstring get_local_path() const;

    /// @brief User Roaming profile
    /// @return: Windows Roaming profile path or empty string
    std::wstring get_roaming_path() const;

    /// @brief In case of unsuccessful retrieval last error 
    /// in a string format, for notifying the caller
    std::string get_last_error() const;

    /// @brief Return true if all user information is retrieved successfully
    bool info_retrived_successfully() const;

protected:

    /// Polling all native and cross-platform sources for user-related information 
    /// Never create directly, never copy
    UserInformation();
    UserInformation(const UserInformation&) = delete;
    UserInformation& operator=(const UserInformation&) = delete;

private:

    // System-dependent information about OS name, version, build etc
    std::unique_ptr<NativeUserInformation> native_user_info_;
};

} // namespace panic
