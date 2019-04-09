#include <winapi_helpers/win_process_helper.h>
#include <winapi_helpers/win_errors.h>
#include <winapi_helpers/win_handle_ptr.h>
#include <winapi_helpers/win_handle_ptr.h>
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

    BOOL rc = CreateProcessA(
        NULL,
        const_cast<LPSTR>(executable),
        NULL,	// lpProcessAttributes
        NULL,	// lpThreadAttributes
        FALSE,	// bInheritHandles
        NULL,	// dwCreationFlags
        NULL,	// lpEnvironment
        NULL,	// lpCurrentDirectory
        &si,	// LPSTARTUPINFO
        &pi);	// LPPROCESS_INFORMATION

    if (!rc) {
        return false;
    }

    if (!exec_async) {
        DWORD timeout = 5000;
        DWORD completion_code = WaitForSingleObject(pi.hProcess, timeout);
        if (WAIT_TIMEOUT == completion_code) {
            // Exiting by timeout, unable to complete
            return false;
        }
        else if (WAIT_FAILED == completion_code) {
            // Waiting was abandoned with the error
            return false;
        }
    }
    return true;
}

bool NativeProcessHelper::execute(const wchar_t* executable, bool exec_async /*= false*/)
{
    PROCESS_INFORMATION pi = {};
    STARTUPINFOW si = {};

    BOOL rc = CreateProcessW(
        NULL,
        const_cast<LPWSTR>(executable),
        NULL,	// lpProcessAttributes
        NULL,	// lpThreadAttributes
        FALSE,	// bInheritHandles
        NULL,	// dwCreationFlags
        NULL,	// lpEnvironment
        NULL,	// lpCurrentDirectory
        &si,	// LPSTARTUPINFO
        &pi);	// LPPROCESS_INFORMATION

    if (!rc) {
        return false;
    }

    if (!exec_async) {
        DWORD timeout = 5000;
        DWORD completion_code = WaitForSingleObject(pi.hProcess, timeout);
        if (WAIT_TIMEOUT == completion_code) {
            // Exiting by timeout, unable to complete
            return false;
        }
        else if (WAIT_FAILED == completion_code) {
            // Waiting was abandoned with the error
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
            
            if (current_process != NULL && process_entry.th32ProcessID != GetCurrentProcessId()) {
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
    PSID administrators_group_sid = NULL;

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
        WinErrorChecker::last_error_throw_boolean(CheckTokenMembership(NULL, administrators_group_sid, &run_as_admin));

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
        if (GetModuleFileNameA(NULL, exe_module_path, ARRAYSIZE(exe_module_path))) {
            SHELLEXECUTEINFOA elevated_executable_info = {};
            elevated_executable_info.cbSize = sizeof(elevated_executable_info);
            elevated_executable_info.lpVerb = "runas";
            elevated_executable_info.lpFile = exe_module_path;
            elevated_executable_info.hwnd = NULL;
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
            return false;
        }
    }
    return true;
}
