#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <string>
#include <sstream>
#include <optional>

// Place here Windows Service-related functions and classes

namespace helpers {


/// @brief Set of Windows Service management options
/// Class deliberately made exception-free to safely run in low-level code
class NativeServiceHelper{
public:

    /// @brief Check whether Windows Service Administrator access granted
    static std::optional<bool> is_admin_access();

    /// @brief Find out if a process is running as the SYSTEM user.
    static bool is_system_user();

    /// @brief Check whether Windows Service is registered
    /// @param service_name: Service name in SCM (not description!)
    /// @return: optional{true} if service registered, optional{false} otherwise optional{} on error
    static std::optional<bool> is_service_registered(const std::string& service_name);

    /// @brief Check whether Windows Service is running
    /// @param service_name: Service name in SCM (not description!)
    /// @return: optional{true} if service running, optional{false} otherwise optional{} on error
    static std::optional<bool> is_service_running(const std::string& service_name);

    /// @brief Run Windows Service
    /// @param service_name: Service name in SCM (not description!)
    /// @return: true if start successful, false otherwise
    static bool start_service(const std::string& service_name);

    /// @brief Stop Windows Service
    /// @param service_name: Service name in SCM (not description!)
    /// @return: true if stop successful, false otherwise
    static bool stop_service(const std::string& service_name);

    /// @brief Register Windows Service in Service Control Manager
    /// @param service_name: Service name in SCM (not description!)
    /// @param display_name: Display name in Services MMC
    /// @param account_name: The name of the account under which the service should run
    /// If the service type is SERVICE_WIN32_OWN_PROCESS, use an account name in the form "DomainName\UserName"
    /// Empty string equals "NT AUTHORITY\LocalService". For network access use "NT AUTHORITY\NetworkService"
    /// @return: true if register successful, false otherwise
    static bool register_service(const std::string& service_name, const std::string& display_name, const std::string& account_name);

    /// @brief Unregister Windows Service in Service Control Manager
    /// @param service_name: Service name in SCM (not description!)
    /// @return: true if delete successful, false otherwise
    static bool delete_service(const std::string& service_name);

    /// @brief Add description to Windows Service
    /// @param service_name: Service name in SCM
    /// @return: true if setting description successful, false otherwise
    static bool set_service_description(const std::string& service_name, const std::string& service_description);

    /// @brief Make service restart after every failure
    /// @param service_name: Service name in SCM
    /// @return: true if setting action successful, false otherwise
    static bool set_service_restore_action(const std::string& service_name);
};

} // namespace helpers
#endif // defined(_WIN32) || defined(_WIN64)
