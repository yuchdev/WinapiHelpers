#pragma once
#if defined(_WIN32) || defined(_WIN64)


namespace helpers {

class WinHandlePtr;

/// @brief Windows-specific class for working with processes
class NativeProcessHelper{
    
public:

    /// @brief Execute process (ANSI)
    /// @return: true if success, false otherwise
    static bool execute(const char* executable, bool exec_async = false);

    /// @brief Execute process (Unicode)
    /// @return: true if success, false otherwise
    static bool execute(const wchar_t* executable, bool exec_async = false);

    /// @brief Kill processes by name
    static void pkill(const char* process_name);

    /// @brief Is the current process run under administrator (Administrators group)
    /// @throw: std::system_error
    static bool is_admin_mode();

    /// @brief Raise current process to administrator mode
    /// @throw: std::system_error
    static bool elevate_to_admin_mode();

};

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)
