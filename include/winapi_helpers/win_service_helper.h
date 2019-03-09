#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <string>
#include <sstream>

// Place here Windows Service-related functions and classes

namespace helpers {


/// @brief Set of Windows Service management options
class NativeServiceHelper{
public:

    /// @brief Check whether Windows Service Administrator access granted
    static bool is_admin_access();

    /// @brief Find out if a process is running as the SYSTEM user.
    static bool is_system_user();

    /// @brief Check whether Windows Service is registered
    /// @param service_name: Service name in SCM (not description!)
    static bool is_service_registered(const std::string& service_name);

    /// @brief Check whether Windows Service is running
    /// @param service_name: Service name in SCM (not description!)
    static bool is_service_running(const std::string& service_name);

    /// @brief Run Windows Service
    /// @param service_name: Service name in SCM (not description!)
    static void run_service(const std::string& service_name);

    /// @brief Stop Windows Service
    /// @param service_name: Service name in SCM (not description!)
    static void stop_service(const std::string& service_name);

    /// @brief Register Windows Service in Service Control Manager
    /// @param service_name: Service name in SCM (not description!)
    /// @param display_name: Display name in Services MMC
    /// @param account_name: The name of the account under which the service should run
    /// If the service type is SERVICE_WIN32_OWN_PROCESS, use an account name in the form "DomainName\UserName"
    /// Empty string equals "NT AUTHORITY\LocalService". For network access use "NT AUTHORITY\NetworkService"
    static void register_service(const std::string& service_name, const std::string& display_name, const std::string& account_name);

    /// @brief Unregister Windows Service in Service Control Manager
    /// @param service_name: Service name in SCM (not description!)
    static VOID WINAPI delete_service(const std::string& service_name);

    /// @brief Add description to Windows Service
    /// @param service_name: Service name in SCM
    static void set_service_description(const std::string& service_name, const std::string& service_description);

    /// @brief Make service restart after every failure
    /// @param service_name: Service name in SCM
    static void set_service_restore_action(const std::string& service_name);
};

} // namespace helpers
#endif // defined(_WIN32) || defined(_WIN64)
