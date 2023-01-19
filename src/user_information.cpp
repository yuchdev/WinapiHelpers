#if defined(_WIN32) || defined(_WIN64)
#include <winapi-helpers/user_information.h>
#include <winapi-helpers/win_errors.h>
#include <winapi-helpers/handle_ptr.h>
#include <winapi-helpers/special_path_helper.h>

#define WIN32_LEAN_AND_MEAN
#define SECURITY_WIN32

#include <Windows.h>
#include <Security.h>
#include <Sddl.h>

using namespace helpers;


// http://technet.microsoft.com/en-us/library/bb726984.aspx
// Multiply to 2 if we start to support Unicode
const size_t NativeUserInformation::logon_name_size = 104*2;

NativeUserInformation::NativeUserInformation() noexcept
{
    user_name_ = get_win_user_info(NameDisplay);
    user_guid_ = get_win_user_info(NameUniqueId);
    user_sid_ = get_current_user_sid();

    home_path_ = NativePathHelpers::get_home_path();
    local_path_ = NativePathHelpers::get_local_appdata_path();
    roaming_path_ = NativePathHelpers::get_roaming_appdata_path();
}

std::string NativeUserInformation::get_user_name() const
{
    return user_name_;
}

std::string NativeUserInformation::get_user_guid() const
{
    return user_guid_;
}

std::string NativeUserInformation::get_user_sid() const
{
    return user_sid_;
}

std::wstring NativeUserInformation::get_home_path() const
{
    return home_path_;
}

std::wstring NativeUserInformation::get_local_path() const
{
    return local_path_;
}

std::wstring NativeUserInformation::get_roaming_path() const
{
    return roaming_path_;
}

std::string NativeUserInformation::get_last_error() const
{
    return last_error_;
}

std::string NativeUserInformation::get_win_user_name()
{
    char user_name_buffer[logon_name_size] = {};
    auto user_name_size = static_cast<ULONG>(logon_name_size);
    std::string ret;
    last_error_ = WinErrorChecker::last_error_nothrow_boolean(
        ::GetUserNameA(user_name_buffer, &user_name_size));

    if (last_error_.empty() && user_name_size) {
        ret = user_name_buffer;
    }
    else {
        retrived_successfully_ = false;
    }
    return ret;
}


std::string NativeUserInformation::get_win_user_info(int extended_name_id)
{
    char user_name_buffer[logon_name_size] = {};
    auto user_name_size = static_cast<ULONG>(logon_name_size);
    auto name_format = static_cast<EXTENDED_NAME_FORMAT>(extended_name_id);
    std::string ret;

    last_error_ = WinErrorChecker::last_error_nothrow_boolean(
        ::GetUserNameExA(name_format, user_name_buffer, &user_name_size));

    if (last_error_.empty() && user_name_size) {
        ret = user_name_buffer;
    }
    else {
        // try simple GetUserName()
        ret = get_win_user_name();
    }
    return ret;
}

std::string NativeUserInformation::get_current_user_sid()
{
    UCHAR token_user_information[sizeof(TOKEN_USER) + 8 + (4 * SID_MAX_SUB_AUTHORITIES)] = {};
    auto user_information_ptr = reinterpret_cast<PTOKEN_USER>(token_user_information);
    std::string sid;

    // open process token
    helpers::WinHandlePtr process_token{};
    last_error_ = WinErrorChecker::last_error_nothrow_boolean(
        ::OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, process_token.dereference_handle()));

    if (!last_error_.empty()) {
        retrived_successfully_ = false;
        return "";
    }

    // retrieve user SID ()
    ULONG token_info_size{};
    last_error_ = WinErrorChecker::last_error_nothrow_boolean(
        ::GetTokenInformation(process_token, TokenUser, user_information_ptr, sizeof(token_user_information), &token_info_size));

    if (!last_error_.empty()) {
        retrived_successfully_ = false;
        return "";
    }

    // convert sid to string
    LPSTR string_sid = nullptr;
    last_error_ = WinErrorChecker::last_error_nothrow_boolean(
        ::ConvertSidToStringSidA(user_information_ptr->User.Sid, &string_sid));

    if (!last_error_.empty()) {
        retrived_successfully_ = false;
        return "";
    }

    sid = string_sid;
    LocalFree(string_sid);

    return sid;
}

#endif
