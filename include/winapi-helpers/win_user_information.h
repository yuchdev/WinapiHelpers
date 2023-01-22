#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <string>

namespace helpers{


/// @brief Collect system-dependent information about current (logged in) user
/// from WinAPI calls GetUserName(), GetUserNameEx(), GetTokenInformation(), ConvertSidToStringSid()
/// Re-use NativePathHelpers for user-related directories
class NativeUserInformation {
public:

    /// Collect available current (logged in) user information
    NativeUserInformation() noexcept;

    /// @brief Make compiler happy
    ~NativeUserInformation() = default;

    /// @brief Get user name in "pretty readable" format
    /// Login names can be up to 104 characters!
    std::string get_user_name() const;

    /// @brief A GUID string that the IIDFromString() function returns, for example, {4fa050f0-f561-11cf-bdd9-00aa003a77b6}
    /// NOTE! Provide simple display name if the GUID is unavailable, it happen often
    std::string get_user_guid() const;

    /// @brief User NTSecurity ID
    std::string get_user_sid() const;

    /// @brief User home directory path
    std::wstring get_home_path() const;

    /// @brief User Local profile
    std::wstring get_local_path() const;

    /// @brief User Roaming profile
    std::wstring get_roaming_path() const;

    /// @brief In case of unsuccessful retrieval GetLastError 
    /// in a string format, for the notifying holding class
    std::string get_last_error() const;

    /// @brief Return true if all user information is retrieved successfully
    bool info_retrived_successfully() const 
    {
        return retrived_successfully_; 
    }

private:

    /// Get information abiut user with GetUserNameEx()
    /// // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724435%28v=vs.85%29.aspx
    /// param extended_name_id: cast to EXTENDED_NAME_FORMAT enumeration
    std::string get_win_user_info(int extended_name_id);
    
    /// Get information abiut user with GetUserName()
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724432(v=vs.85).aspx
    std::string get_win_user_name();

    /// Find out current user SID from the user-space process
    std::string get_current_user_sid();

    /// Logged in user name
    std::string user_name_;

    /// Logged in user GUID OR display name if the GUID is unavailable
    std::string user_guid_;

    /// Logged in user SID
    std::string user_sid_;

    /// User home directory path
    std::wstring home_path_;

    /// Local profile
    std::wstring local_path_;

    /// Roaming profile
    std::wstring roaming_path_;

    /// GetLastError in string format, for the notifying holding class
    std::string last_error_;

    /// Class state flag
    // If retrived_successfully_ == false check get_last_error() for last problem.
    /// NativeUserInformation exists even if not all information has been retrieved
    bool retrived_successfully_ = true;

    /// Login names can be up to 104 characters!
    /// http://technet.microsoft.com/en-us/library/bb726984.aspx
    static const size_t logon_name_size;
};

} // namespace helpers
#endif
