#include <winapi-helpers/process_helper.h>
#include <winapi-helpers/win_errors.h>
#include <winapi-helpers/handle_ptr.h>
#include <Windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <string>

using namespace std;
using namespace helpers;



bool NativeProcessHelper::execute(const char* executable, bool exec_async /*= false*/)
{
    PROCESS_INFORMATION pi = {};
    STARTUPINFOA si = {};

#ifdef WITH_LOGGING
    LOG_DEBUG << "Executing process (ANSI): " << executable;
#endif

    std::string return_message = WinErrorChecker::last_error_nothrow_boolean(CreateProcessA(
        nullptr,
        const_cast<LPSTR>(executable),
        nullptr,	// lpProcessAttributes
        nullptr,	// lpThreadAttributes
        FALSE,	// bInheritHandles
        NULL,	// dwCreationFlags
        nullptr,	// lpEnvironment
        nullptr,	// lpCurrentDirectory
        &si,	// LPSTARTUPINFO
        &pi));	// LPPROCESS_INFORMATION

    if (!return_message.empty()) {
#ifdef WITH_LOGGING
        LOG_WARN << "ANSI execute returned: " << return_message;
#endif
    }

    if (!exec_async) {
        DWORD timeout = 5000;
#ifdef WITH_LOGGING
        LOG_DEBUG << "Waiting for the process PID = " << pi.hProcess << " to complete";
#endif
        
        DWORD completion_code = WaitForSingleObject(pi.hProcess, timeout);
        if (WAIT_TIMEOUT == completion_code) {
#ifdef WITH_LOGGING
            LOG_WARN << "Exiting by timeout, unable to complete in " << timeout << " msec";
#endif
            return false;
        }
        else if (WAIT_FAILED == completion_code) {
            DWORD e = ::GetLastError();
#ifdef WITH_LOGGING
            LOG_WARN << "Waiting was abandoned with the error code = " << e;
#endif
            return false;
        }
    }
    return true;
}

bool NativeProcessHelper::execute(const wchar_t* executable, bool exec_async /*= false*/)
{
    PROCESS_INFORMATION pi = {};
    STARTUPINFOW si = {};

#ifdef WITH_LOGGING
    LOG_DEBUG << "Executing process (Unicode): " << helpers::wstring_to_string(std::wstring(executable));
#endif

    std::string return_message = WinErrorChecker::last_error_nothrow_boolean(CreateProcessW(
        nullptr,
        const_cast<LPWSTR>(executable),
        nullptr,	// lpProcessAttributes
        nullptr,	// lpThreadAttributes
        FALSE,	// bInheritHandles
        NULL,	// dwCreationFlags
        nullptr,	// lpEnvironment
        nullptr,	// lpCurrentDirectory
        &si,	// LPSTARTUPINFO
        &pi));	// LPPROCESS_INFORMATION

    if (!return_message.empty()) {
#ifdef WITH_LOGGING
        LOG_WARN << "Wide execute returned: " << return_message;
#endif

    }

    if (!exec_async) {
        DWORD timeout = 5000;
#ifdef WITH_LOGGING
        LOG_DEBUG << "Waiting for the process PID = " << pi.hProcess << " to complete";
#endif

        DWORD completion_code = WaitForSingleObject(pi.hProcess, timeout);
        if (WAIT_TIMEOUT == completion_code) {
#ifdef WITH_LOGGING
            LOG_WARN << "Exiting by timeout, unable to complete in " << timeout << " msec";
#endif
            return false;
        }
        else if (WAIT_FAILED == completion_code) {
            DWORD e = ::GetLastError();
#ifdef WITH_LOGGING
            LOG_WARN << "Waiting was abandoned with the error code = " << e;
#endif
            return false;
        }
    }
    return true;
}

void NativeProcessHelper::pkill(const char* process_name)
{
    WinHandlePtr handles_snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL));
    
    PROCESSENTRY32 process_entry{};
    process_entry.dwSize = sizeof(process_entry);
    
    // iterate over processes and kill by name
    BOOL proc_handle = Process32First(handles_snapshot.handle(), &process_entry);
    while (proc_handle) {

        if (string(process_entry.szExeFile) == string(process_name)) {
            
            HANDLE current_process = OpenProcess(PROCESS_TERMINATE, 0, process_entry.th32ProcessID);
            
            if (current_process != nullptr && process_entry.th32ProcessID != GetCurrentProcessId()) {
                TerminateProcess(current_process, 9);
                CloseHandle(current_process);
            }
        }
        proc_handle = Process32Next(handles_snapshot.handle(), &process_entry);
    }
}

bool NativeProcessHelper::is_admin_mode()
{
    BOOL run_as_admin = FALSE;
    PSID administrators_group_sid = nullptr;

    try{

        // Allocate and initialize a SID of the Administrators group
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        WinErrorChecker::last_error_throw_boolean(AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &administrators_group_sid));

        // Determine whether the SID of Administrators group is enabled in 
        // the primary access token of the process
        WinErrorChecker::last_error_throw_boolean(CheckTokenMembership(nullptr, administrators_group_sid, &run_as_admin));

    }
    catch (const std::system_error& e){
        if (administrators_group_sid){
            FreeSid(administrators_group_sid);
            throw;
        }
    }

    return run_as_admin ? true : false;
}

bool NativeProcessHelper::elevate_to_admin_mode()
{
    if (!is_admin_mode()) {

        char exe_module_path[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, exe_module_path, ARRAYSIZE(exe_module_path))) {
            SHELLEXECUTEINFOA elevated_executable_info = {};
            elevated_executable_info.cbSize = sizeof(elevated_executable_info);
            elevated_executable_info.lpVerb = "runas";
            elevated_executable_info.lpFile = exe_module_path;
            elevated_executable_info.hwnd = nullptr;
            elevated_executable_info.nShow = SW_NORMAL;

            if (!ShellExecuteExA(&elevated_executable_info))
            {
                // Elevation is canceled
                if (GetLastError() == ERROR_CANCELLED) {
                    return false;
                }
            }
            else {
                // Process is replaced with elevated, exit
                exit(0);
            }
        }
        else {
            // Elevation system error
            DWORD e = ::GetLastError();
#ifdef WITH_LOGGING
            LOG_WARN << "Elevating was abandoned with the error code = " << e;
#endif
            return false;
        }
    }
    return true;
}
