#include <string>
#include <algorithm>
#include <winapi_helpers/win_special_path_helper.h>
#include <winapi_helpers/win_ptrs.h>
#include <winapi_helpers/win_errors.h>
#include <winapi_helpers/win_registry_helper.h>
#include <winapi_helpers/win_service_helper.h>
#include <winapi_helpers/win_system_information.h>
#include <winapi_helpers/win_user_information.h>
#include <winapi_helpers/win_partition_information.h>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>

using namespace helpers;
using namespace boost::unit_test;

// Functional tests
/*
*/

#pragma region GeneralHelpersFunctionalTests

BOOST_AUTO_TEST_SUITE(GeneralHelpersFunctionalTests);

///////////////////////////////////
// Helper functions and classes


///////////////////////////////////
// Test cases

#if 0 // test just for debugging
BOOST_AUTO_TEST_CASE(Windows_Registry_Utils)
{
    helpers::suppress_network_location();
    exit(0);
}
#endif


#if defined(_WIN32) || defined(_WIN64)

// Local Heap smart pointer test
BOOST_AUTO_TEST_CASE(TestLocalAllocPtr)
{
    setlocale(0, "");
    std::string ip("10.10.10.10");
    std::string vpn(RASDT_Vpn);
    size_t sz = sizeof(RASENTRY);
    WinLocalPtr<RASENTRY> local_rasentry(sz);
    local_rasentry->dwSize = sz;
    std::copy(vpn.begin(), vpn.end(), local_rasentry->szDeviceType);
    std::copy(ip.begin(), ip.end(), local_rasentry->szLocalPhoneNumber);

    size_t sz1 = local_rasentry->dwSize;
    std::string s1(local_rasentry->szDeviceType);
    std::string s2(local_rasentry->szLocalPhoneNumber);

    BOOST_CHECK_EQUAL(sz1, sz);
    BOOST_CHECK_EQUAL(s1, std::string(RASDT_Vpn));
    BOOST_CHECK_EQUAL(s2, ip);
}

// Windows Heap smart pointer test
BOOST_AUTO_TEST_CASE(TestHeapAllocPtr)
{
    setlocale(0, "");
    std::string ip("192.168.1.1");
    std::string vpn(RASDT_Vpn);
    size_t sz = sizeof(RASENTRY);
    WinHeapPtr<RASENTRY> heap_rasentry(sz);
    heap_rasentry->dwSize = sz;
    std::copy(vpn.begin(), vpn.end(), heap_rasentry->szDeviceType);
    std::copy(ip.begin(), ip.end(), heap_rasentry->szLocalPhoneNumber);

    size_t sz1 = heap_rasentry->dwSize;
    std::string s1(heap_rasentry->szDeviceType);
    std::string s2(heap_rasentry->szLocalPhoneNumber);

    BOOST_CHECK_EQUAL(sz1, sz);
    BOOST_CHECK_EQUAL(s1, std::string(RASDT_Vpn));
    BOOST_CHECK_EQUAL(s2, ip);
}

// Get last error to string test
BOOST_AUTO_TEST_CASE(TestWindowsErrorStringNothrow)
{
    DWORD errorMessageID = ERROR_ACCESS_DENIED;
    std::string error_description{ "Access is denied." };
    bool ret = WinErrorChecker::retbool_nothrow_retcode(errorMessageID);
    std::string msg = WinErrorChecker::last_error_nothrow_retcode(errorMessageID);
    BOOST_CHECK_EQUAL(ret, false);
    BOOST_CHECK_EQUAL(msg, error_description);
}

// Get last error to string test with exception being thrown
BOOST_AUTO_TEST_CASE(TestWindowsErrorStringThrow)
{
    std::string error_description{ "Access is denied." };
    std::string last_error;
    DWORD errorMessageID = ERROR_ACCESS_DENIED;
    try{
        WinErrorChecker::last_error_throw_retcode(errorMessageID);
    }
    catch (const std::runtime_error& e){
        last_error = e.what();
    }
    BOOST_CHECK_EQUAL(last_error, error_description);
}

#endif

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region ServiceManagerFunctionalTests

BOOST_AUTO_TEST_SUITE(ServiceManagerFunctionalTests);

BOOST_AUTO_TEST_CASE(IsServiceRegisteredTest)
{
    std::optional<bool> registered = NativeServiceHelper::is_service_registered("Netlogon");
    BOOST_CHECK_EQUAL(registered.has_value(), true);
    if (!registered.has_value()) {
        return;
    }
    BOOST_CHECK_EQUAL(registered.value(), true);
    

    std::optional<bool> not_registered = NativeServiceHelper::is_service_registered("NoSuchService");
    BOOST_CHECK_EQUAL(not_registered.has_value(), true);
    if (!not_registered.has_value()) {
        return;
    }

    BOOST_CHECK_EQUAL(not_registered.value(), false);
}

BOOST_AUTO_TEST_CASE(IsServiceRunningTest)
{
    // most probably Windows service is running
    std::optional<bool> running = NativeServiceHelper::is_service_running("Schedule");
    BOOST_CHECK_EQUAL(running.has_value(), true);
    if (!running.has_value()) {
        return;
    }

    BOOST_CHECK_EQUAL(running.value(), true);
}

BOOST_AUTO_TEST_CASE(RunServiceTest)
{
    // Available only under Administrator
    if (!NativeServiceHelper::is_admin_access()) {
        return;
    }

    // Most useless service on a server, could be used for testing
    const char* service_name = "Audiosrv";

    std::optional<bool> service_registered = NativeServiceHelper::is_service_registered(service_name);
    BOOST_CHECK_EQUAL(service_registered.has_value(), true);

    std::optional<bool> test_service_run = NativeServiceHelper::is_service_running(service_name);
    BOOST_CHECK_EQUAL(test_service_run.has_value(), true);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region RegistryHelperFunctionalTests

BOOST_AUTO_TEST_SUITE(RegistryHelperFunctionalTests);

BOOST_AUTO_TEST_CASE(OpenRegistryKeyTest)
{
    // Available only under Administrator
    if (!NativeServiceHelper::is_admin_access()) {
        return;
    }

    {
        helpers::RegistryKey key("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management");
        std::optional<unsigned long> ret = key.get_dword_value("ClearPageFileAtShutdown");;        
        BOOST_CHECK_EQUAL(ret.has_value(), true);
        if (!ret.has_value()) {
            return;
        }
        BOOST_CHECK_LE(ret.value(), 1);
    }

    {
        helpers::RegistryKey key("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control");
        auto ret = key.get_string_value("CurrentUser");
        BOOST_CHECK_EQUAL(ret.has_value(), true);
        if (!ret.has_value()) {
            return;
        }
        BOOST_CHECK_EQUAL(ret.value().empty(), false);
    }

    {
        helpers::RegistryKey key("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control");
        auto ret = key.get_wstring_value("CurrentUser");
        BOOST_CHECK_EQUAL(ret.has_value(), true);
        if (!ret.has_value()) {
            return;
        }
        BOOST_CHECK_EQUAL(ret.value().empty(), false);
    }
}

BOOST_AUTO_TEST_CASE(EnumerateIEHistory)
{
    // Available only under Administrator
    if (!NativeServiceHelper::is_admin_access()) {
        return;
    }

    // SYSTEM does not have HKEY_CURRENT_USER hive
    if (NativeServiceHelper::is_system_user()) {
        return;
    }

    try {
        // we have at least one browser?
        helpers::RegistryKey key("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\TypedURLs");
        auto ret = key.enumerate_values();
        BOOST_CHECK_EQUAL(ret.has_value(), true);
        std::vector<std::string> subvalues = ret.value();
        
        // return: 1st - number of subkeys, 2nd - number of subvalues
        std::pair<DWORD, DWORD> enumerator = key.count_subvalues();
        BOOST_CHECK_NE(subvalues.size(), 0);
        BOOST_CHECK_EQUAL(subvalues.size(), enumerator.second);
    }
    catch (const std::exception& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()), "");
    }
}

BOOST_AUTO_TEST_CASE(EnumerateRegistryKeyTest)
{
    // Available only under Administrator
    if (!NativeServiceHelper::is_admin_access()) {
        return;
    }

    try {
        // we have at least one browser?
        helpers::RegistryKey key("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\StartMenuInternet");
        auto ret = key.enumerate_subkeys();
        BOOST_CHECK_EQUAL(ret.has_value(), true);
        std::vector<std::string> subkeys = ret.value();

        // return: 1st - number of subkeys, 2nd - number of subvalues
        std::pair<DWORD, DWORD> enumerator = key.count_subvalues();
        BOOST_CHECK_NE(subkeys.size(), 0);
        BOOST_CHECK_EQUAL(subkeys.size(), enumerator.first);
    }
    catch (const std::exception& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()), "");
    }

    try {
        // browser by default
        helpers::RegistryKey key("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\StartMenuInternet");
        auto ret = key.enumerate_values();
        BOOST_CHECK_EQUAL(ret.has_value(), true);
        std::vector<std::string> subkeys = ret.value();
        BOOST_CHECK_NE(subkeys.size(), 0);
    }
    catch (const std::exception& e) {
        BOOST_CHECK_EQUAL(std::string(e.what()), "");
    }
}

BOOST_AUTO_TEST_CASE(ExistentKeyTest)
{
    // Available only under Administrator
    if (!NativeServiceHelper::is_admin_access()) {
        return;
    }

    auto ret = helpers::RegistryKey::is_key_exist("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management");
    BOOST_CHECK_EQUAL(ret.has_value(), true);
    BOOST_CHECK_EQUAL(ret.value(), true);
}

BOOST_AUTO_TEST_CASE(NonExistentKeyTest)
{
    // Available only under Administrator
    if (!NativeServiceHelper::is_admin_access()) {
        return;
    }

    auto ret = helpers::RegistryKey::is_key_exist("HKEY_LOCAL_MACHINE\\SYSTEM\\Whooooaaa");
    BOOST_CHECK_EQUAL(ret.has_value(), true);
    BOOST_CHECK_EQUAL(ret.value(), false);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region NativeSystemInformationFunctionalTests

BOOST_AUTO_TEST_SUITE(NativeSystemInformationFunctionalTests);

BOOST_AUTO_TEST_CASE(NativeSystemInformationTest)
{
    NativeSystemInformation sysinfo;

    BOOST_CHECK_EQUAL(sysinfo.info_retrived_successfully(), true);
    BOOST_CHECK_EQUAL(sysinfo.get_last_error().empty(), true);

    std::string arch = sysinfo.cpu_architecture();
    std::string os_name = sysinfo.os_name();
    BOOST_CHECK_EQUAL(arch.empty(), false);
    BOOST_CHECK_EQUAL(os_name.empty(), false);

    unsigned long number_cpu = sysinfo.number_of_cpu();
    BOOST_CHECK_NE(number_cpu, 0);

    unsigned long build_num = sysinfo.build_number();
    BOOST_CHECK_NE(build_num, 0);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region NativeSystemUserFunctionalTests

BOOST_AUTO_TEST_SUITE(NativeSystemUserFunctionalTests);

BOOST_AUTO_TEST_CASE(NativeSystemUserTest)
{
    // SYSTEM user, local run only
    if (NativeServiceHelper::is_system_user()) {
        return;
    }

    NativeUserInformation userinfo;
    std::string user_name = userinfo.get_user_name();
    std::string user_guid = userinfo.get_user_guid();
    std::string user_sid = userinfo.get_user_sid();
    std::wstring home_path = userinfo.get_home_path();
    std::wstring local_path = userinfo.get_local_path();
    std::wstring roaming_path = userinfo.get_roaming_path();

    std::string last_error = userinfo.get_last_error();
    
    BOOST_CHECK_EQUAL(user_name.empty(), false);
    BOOST_CHECK_EQUAL(user_guid.empty(), false);
    BOOST_CHECK_EQUAL(user_sid.empty(), false);
    BOOST_CHECK_EQUAL(home_path.empty(), false);
    BOOST_CHECK_EQUAL(local_path.empty(), false);
    BOOST_CHECK_EQUAL(roaming_path.empty(), false);
    BOOST_CHECK_EQUAL(last_error.empty(), true);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region WindowsPathFunctionalTests

BOOST_AUTO_TEST_SUITE(WindowsPathFunctionalTests);

BOOST_AUTO_TEST_CASE(TempDirectoryPathTest)
{
    std::wstring temp_path = helpers::NativePathHelpers::get_tempdir_path();
    BOOST_CHECK_EQUAL(temp_path.empty(), false);
}

BOOST_AUTO_TEST_CASE(DesctopDirectoryPathTest)
{
    std::wstring desktop_path = helpers::NativePathHelpers::get_desktop_path();
    BOOST_CHECK_EQUAL(desktop_path.empty(), false);
}

BOOST_AUTO_TEST_CASE(HomedirPathTest)
{
    // SYSTEM user, local run only
    if (NativeServiceHelper::is_system_user()) {
        return;
    }

    std::wstring roaming_path = helpers::NativePathHelpers::get_roaming_appdata_path();
    BOOST_CHECK_EQUAL(roaming_path.empty(), false);
}

BOOST_AUTO_TEST_CASE(UserAppdataPathTest)
{
    // SYSTEM user, local run only
    if (NativeServiceHelper::is_system_user()) {
        return;
    }

    std::wstring appdata_path = helpers::NativePathHelpers::get_local_appdata_path();
    BOOST_CHECK_EQUAL(appdata_path.empty(), false);
}

BOOST_AUTO_TEST_CASE(RoamingAppdataPathTest)
{
    // SYSTEM user, local run only
    if (NativeServiceHelper::is_system_user()) {
        return;
    }

    std::wstring home_path = helpers::NativePathHelpers::get_home_path();
    BOOST_CHECK_EQUAL(home_path.empty(), false);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region WindowsPartitionInformationTests

BOOST_AUTO_TEST_SUITE(WindowsPartitionInformationTests);

BOOST_AUTO_TEST_CASE(FixedDriveExistsTest)
{
    NativePartititonInformation p;
    std::vector<NativePartititonInformation::NativePartititon> partitions = p.enumerate_partititons();
    
    // at least one
    BOOST_CHECK_EQUAL(partitions.empty(), false);
    
    size_t fixed_part = std::count_if(partitions.begin(), partitions.end(), [](const auto& part) {
        return static_cast<NativePartititonInformation::PlacementType>(part.placement_type) 
            == NativePartititonInformation::FixedDisk;
    });

    // at least one drive is fixed
    BOOST_CHECK_GE(fixed_part, 1);
}

BOOST_AUTO_TEST_CASE(ReEnumeratePartitionsTests)
{
    NativePartititonInformation p;
    std::vector<NativePartititonInformation::NativePartititon> partitions = p.enumerate_partititons();

    // at least one
    BOOST_CHECK_EQUAL(partitions.empty(), false);
    size_t size1 = partitions.size();

    // re-collect information
    p.collect_partititon_information();
    
    partitions = p.enumerate_partititons();
    BOOST_CHECK_EQUAL(partitions.empty(), false);
    size_t size2 = partitions.size();

    BOOST_CHECK_EQUAL(size1, size2);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion

#pragma region RegistryManagerFunctionalTests

BOOST_AUTO_TEST_SUITE(RegistryHelperFunctionalTests);

BOOST_AUTO_TEST_SUITE_END()

#pragma endregion
