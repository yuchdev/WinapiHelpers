/* 2017-2020 WEBGEARS SERVICES LIMITED (c) All Rights Reserved. 
 * Proprietary and confidential.
 * The Software and any accompanying documentation are copyrighted and protected 
 * by copyright laws and international copyright treaties, as well as other 
 * intellectual property laws and treaties.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 */

#include <winapi-helpers/special_path_helper.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winapi-helpers/win_special_path_helper.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <posix_helpers/posix_special_path_helper.h>
#endif


using namespace std;
using namespace helpers;

std::wstring helpers::get_home_wpath()
{
    return NativePathHelpers::get_home_wpath();
}

std::wstring helpers::get_tempdir_wpath()
{
    return NativePathHelpers::get_tempdir_wpath();
}

std::wstring helpers::get_system_temp_wpath()
{
    return NativePathHelpers::get_system_temp_wpath();
}

std::wstring helpers::get_desktop_wpath()
{
    return NativePathHelpers::get_desktop_wpath();
}

std::wstring helpers::get_common_appdata_wpath()
{
	return NativePathHelpers::get_common_appdata_wpath();
}
