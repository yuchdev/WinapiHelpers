#if defined(_WIN32) || defined(_WIN64)

#include <winapi-helpers/win_special_path_helper.h>
#include <winapi-helpers/service_helper.h>
#include <boost/filesystem.hpp>

#include <Windows.h>
#include <shlobj.h>
#include <cassert>


using namespace std;
using namespace helpers;

std::wstring NativePathHelpers::get_tempdir_wpath()
{
    std::wstring tmp_path;
    static wchar_t system_temp_path[MAX_PATH + 1] = {};
    if (::GetTempPathW(MAX_PATH, system_temp_path)) {
        tmp_path = system_temp_path;
    }
    else {
        tmp_path = create_root_tempdir();
    }

    if ((tmp_path.back() != L'/') && (tmp_path.back() != L'\\')) {
        tmp_path += L'/';
    }
        
    return tmp_path;
}

std::wstring NativePathHelpers::get_home_wpath()
{
    // we do not interested about SYSTEM home
    if (NativeServiceHelper::is_system_user()) {
        return std::wstring{};
    }

    std::wstring home_path;
    static wchar_t user_home_path[MAX_PATH + 1] = {};
    if (::SHGetSpecialFolderPathW(NULL, user_home_path, CSIDL_PROFILE, FALSE)) {
        home_path = user_home_path;
    }

    if ((home_path.back() != L'/') && (home_path.back() != L'\\')) {
        home_path += L'\\';
    }

    return home_path;
}

std::wstring NativePathHelpers::get_appdata_path(int special_path_type, bool ignore_system_user_check /*= false*/)
{
    // we do not interested about SYSTEM AppData
    if (NativeServiceHelper::is_system_user() && !ignore_system_user_check) {
        return std::wstring{};
    }

    std::wstring appdata_path;
    static wchar_t user_appdata_path[MAX_PATH + 1] = {};
    if (::SHGetSpecialFolderPathW(NULL, user_appdata_path, special_path_type, FALSE)) {
        appdata_path = user_appdata_path;
    }

    if ((appdata_path.back() != '/') && (appdata_path.back() != '\\')) {
        appdata_path += '\\';
    }

    return appdata_path;
}

std::wstring NativePathHelpers::get_local_appdata_wpath()
{
    return get_appdata_path(CSIDL_LOCAL_APPDATA);
}

std::wstring NativePathHelpers::get_roaming_appdata_wpath()
{
    return get_appdata_path(CSIDL_APPDATA);
}

std::wstring NativePathHelpers::get_common_appdata_wpath()
{
    return get_appdata_path(CSIDL_COMMON_APPDATA, true);
}

std::wstring NativePathHelpers::create_root_tempdir()
{
    std::wstring default_temp(L"C:/Temp");
    boost::system::error_code error;
    if (!boost::filesystem::is_directory(default_temp)) {
        boost::filesystem::create_directory(default_temp, error);
    }
    return default_temp;
}

std::wstring NativePathHelpers::get_desktop_wpath()
{
    std::wstring desktop_path;
    static wchar_t system_desktop_path[MAX_PATH + 1] = {};
    if (::SHGetSpecialFolderPathW(HWND_DESKTOP, system_desktop_path, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE)) {
        desktop_path = system_desktop_path;
    }
    else {
        desktop_path = L"C:\\Users\\Public\\Desktop\\";
    }
        
    if ((desktop_path.back() != L'/') && (desktop_path.back() != L'\\')) {
        desktop_path += L'\\';
    }

    return desktop_path;
}

std::wstring NativePathHelpers::append_to_local(const wchar_t* path)
{
    std::wstring ret(get_local_appdata_wpath());
    if (!ret.empty()) {
        return std::move(ret.append(path));
    }
    else {
        return std::wstring{};
    }    
}

std::wstring NativePathHelpers::append_to_roaming(const wchar_t* path)
{
    std::wstring ret(get_roaming_appdata_wpath());
    if (!ret.empty()) {
        return std::move(ret.append(path));
    }
    else {
        return std::wstring{};
    }
}

std::wstring NativePathHelpers::append_to_home(const wchar_t* path)
{
    std::wstring ret(get_home_wpath());
    if (!ret.empty()) {
        return std::move(ret.append(path));
    }
    else {
        return std::wstring{};
    }
}

std::wstring NativePathHelpers::get_windows_wpath()
{
    std::wstring path;
    static wchar_t buf[MAX_PATH + 1] = {};
    if (::SHGetSpecialFolderPathW(HWND_DESKTOP, buf, CSIDL_WINDOWS, FALSE)) {
        path = buf;
    }
    else {
        path = L"C:\\Windows\\";
    }

    if ((path.back() != L'/') && (path.back() != L'\\')) {
        path += L'\\';
    }

    return path;
}

std::wstring NativePathHelpers::get_system_temp_wpath()
{
    std::wstring win{ get_windows_wpath() };
    win += L"Temp\\";
    return std::move(win);
}

//

#endif // defined(_WIN32) || defined(_WIN64)
