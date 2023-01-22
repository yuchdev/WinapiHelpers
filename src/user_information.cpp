#include <winapi-helpers/user_information.h>
#include <winapi-helpers/special_path_helper.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winapi-helpers/win_user_information.h>
#else
#include <posix_helpers/posix_user_information.h>
#endif

using namespace helpers;

UserInformation& UserInformation::instance()
{
    static UserInformation single_user_information;
    return single_user_information;
}

UserInformation::UserInformation() : native_user_info_(std::make_unique<NativeUserInformation>())
{
}

std::string UserInformation::get_user_name() const
{
    return native_user_info_->get_user_name();
}

std::string UserInformation::get_user_guid() const
{
#if defined(_WIN32) || defined(_WIN64)
    return native_user_info_->get_user_guid();
#else
    return "";
#endif
}

std::string UserInformation::get_user_sid() const
{
#if defined(_WIN32) || defined(_WIN64)
    return native_user_info_->get_user_sid();
#else
    return "";
#endif
}


std::wstring helpers::UserInformation::get_home_path() const
{
    return native_user_info_->get_home_path();
}

std::wstring helpers::UserInformation::get_local_path() const
{
#if defined(_WIN32) || defined(_WIN64)
    return native_user_info_->get_local_path();
#else
    return L"";
#endif
}

std::wstring helpers::UserInformation::get_roaming_path() const
{
#if defined(_WIN32) || defined(_WIN64)
    return native_user_info_->get_roaming_path();
#else
    return L"";
#endif
}

std::string helpers::UserInformation::get_last_error() const
{
    return native_user_info_->get_last_error();
}

bool helpers::UserInformation::info_retrived_successfully() const
{
    return native_user_info_->info_retrived_successfully();
}
