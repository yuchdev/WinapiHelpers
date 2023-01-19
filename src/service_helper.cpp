#if defined(_WIN32) || defined(_WIN64)
#include <winapi-helpers/win_errors.h>
#include <winapi-helpers/service_helper.h>
#include <winapi-helpers/handle_ptr.h>

#include <tlhelp32.h>
#include <stdexcept>
#include <system_error>

using namespace std;
using namespace helpers;

// Uncomment if you're debugging, don't forget to comment back
// #define DEBUG_SERVICE

/// @brief Service Control Manager wrapper
class ScmManager{

public:

	/// @brief Service Control Manager initializer
    explicit ScmManager(DWORD desired_access) :_scm_handle(OpenSCManager(nullptr, // local computer
		// servicesActive database 
		nullptr,
		// full access rights 
        desired_access))
    {
	}

    ~ScmManager()
    {
        CloseServiceHandle(_scm_handle);
    }

    ScmManager(const ScmManager&) = delete;
    ScmManager& operator=(const ScmManager&) = delete;

	SC_HANDLE handle() const 
    {
		return _scm_handle;
	}

    bool valid() const
    {
        return _scm_handle != nullptr;
    }

private:
	SC_HANDLE _scm_handle;
};


/// @brief Windows Service wrapper
class ServiceWrapper{
public:

	ServiceWrapper(const SC_HANDLE& scm, const string& service_name, DWORD desired_access) :
        _service_handle(OpenServiceA(scm, service_name.c_str(), desired_access))
    {
	}

    ~ServiceWrapper()
    {
        CloseServiceHandle(_service_handle);
    }
    

	SC_HANDLE handle() const 
    {
		return _service_handle;
	}

    bool valid() const
    {
        return _service_handle != nullptr;
    }

private:
	SC_HANDLE _service_handle;
};


//static 
std::optional<bool> NativeServiceHelper::is_admin_access()
{

    SC_HANDLE service_handle = OpenSCManager(nullptr, // local computer
        // servicesActive database 
        nullptr,
        // full access rights 
        SC_MANAGER_ALL_ACCESS);

    if ((service_handle == nullptr) && (ERROR_ACCESS_DENIED == GetLastError())){
        return false;
    }
    else if (service_handle == nullptr){
        // OpenService failed
        return {};
    }
    CloseServiceHandle(service_handle);
    return true;
}

bool NativeServiceHelper::is_system_user()
{
    UCHAR token_user_information[sizeof(TOKEN_USER) + 8 + (4 * SID_MAX_SUB_AUTHORITIES)] = {};
    PTOKEN_USER user_information_ptr = reinterpret_cast<PTOKEN_USER>(token_user_information);
    SID_IDENTIFIER_AUTHORITY sid_authority = SECURITY_NT_AUTHORITY;
    bool is_system = false;

    {
        // open process token
        helpers::WinHandlePtr process_token{};
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, process_token.dereference_handle())) {
            return false;
        }

        // retrieve user SID ()
        ULONG token_info_size{};
        if (!GetTokenInformation(process_token, TokenUser, user_information_ptr, sizeof(token_user_information), &token_info_size)) {
            return false;
        }
    }

    // allocate LocalSystem well-known SID (has only 1 sub-authority, 7 others possible are set to 0)
    PSID system_user_sid = {};
    if (!AllocateAndInitializeSid(&sid_authority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &system_user_sid)) {
        return false;
    }

    // compare the user SID from the token with the LocalSystem SID
    is_system = EqualSid(user_information_ptr->User.Sid, system_user_sid);
    FreeSid(system_user_sid);

    return is_system;
}


//static 
std::optional<bool> NativeServiceHelper::is_service_registered(const std::string& service_name)
{
	// Get a handle to the SCM database. 
    ScmManager scm(GENERIC_READ | SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm.valid()) {
        return {};
    }

    SC_HANDLE service_handle = OpenServiceA(scm.handle(), service_name.c_str(), SERVICE_QUERY_STATUS);

	if ((service_handle == nullptr) && (ERROR_SERVICE_DOES_NOT_EXIST == GetLastError())){
		return false;
	}
    else if (service_handle == nullptr) {
        // OpenService failed
        return false;
    };
	return true;
}

//static 
std::optional<bool> NativeServiceHelper::is_service_running(const std::string& service_name)
{

	// Get a handle to the SCM database
    ScmManager scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm.valid()) {
        return {};
    }

	// Get a handle to the service.
    ServiceWrapper serv(scm.handle(), service_name, SERVICE_QUERY_STATUS);
    if (!serv.valid()) {
        return {};
    }

	// Check the status in case the service is not stopped. 
	DWORD size_for_buffer = 0;
	SERVICE_STATUS_PROCESS service_status = {};
	if (!QueryServiceStatusEx(serv.handle(), // handle to service 

		// information level
		SC_STATUS_PROCESS_INFO,

		// address of structure
		(LPBYTE)&service_status,

		// size of structure
		sizeof(SERVICE_STATUS_PROCESS),

		// size needed if buffer is too small
		&size_for_buffer)){

        return {};
	}

	// Check if the service is already running
	if (service_status.dwCurrentState != SERVICE_STOPPED && service_status.dwCurrentState != SERVICE_STOP_PENDING) {
		return true;
	}
	return false;
}


//static 
bool NativeServiceHelper::start_service(const std::string& service_name)
{
	// Get a handle to the SCM database
    ScmManager scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm.valid()) {
        return false;
    }

	// Get a handle to the service.
    ServiceWrapper serv(scm.handle(), service_name, SERVICE_QUERY_STATUS | SERVICE_START);
    if (!serv.valid()) {
        return false;
    }

	DWORD size_for_buffer = 0;
	SERVICE_STATUS_PROCESS service_status = {};
	// Check the status in case the service is not stopped. 
	if (!QueryServiceStatusEx(serv.handle(), // handle to service 

		// information level
		SC_STATUS_PROCESS_INFO,

		// address of structure
		(LPBYTE)&service_status,

		// size of structure
		sizeof(SERVICE_STATUS_PROCESS),

		// size needed if buffer is too small
		&size_for_buffer)){

        // QueryServiceStatusEx failed
        return false;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (service_status.dwCurrentState != SERVICE_STOPPED && service_status.dwCurrentState != SERVICE_STOP_PENDING) {
		// run_service: Cannot start the service because it is already running
		return true;
	}

	// Save the tick count and initial checkpoint.

	DWORD start_tick_count = GetTickCount();
	DWORD old_check_point = service_status.dwCheckPoint;
	DWORD wait_time = 0;
	// Wait for the service to stop before attempting to start it.

	while (service_status.dwCurrentState == SERVICE_STOP_PENDING) {
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		wait_time = service_status.dwWaitHint / 10;

		if (wait_time < 1000)
			wait_time = 1000;
		else if (wait_time > 10000)
			wait_time = 10000;

		Sleep(wait_time);

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx(
			serv.handle(),                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&service_status,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&size_for_buffer)){              // size needed if buffer is too small

            return false;
		}

		if (service_status.dwCheckPoint > old_check_point){
			// Continue to wait and check.

			start_tick_count = GetTickCount();
			old_check_point = service_status.dwCheckPoint;
		}
		else{

			if (GetTickCount() - start_tick_count > service_status.dwWaitHint){
				// run_service: Timeout waiting for service to stop
				return false;
			}
		}
	}

	// Attempt to start the service.
	if (!StartService(serv.handle(),  // handle to service 

		// number of arguments 
		0,

		// no arguments 
		nullptr)){

        // StartService failed
		return false;
	}

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		serv.handle(),                  // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&service_status,              // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&size_for_buffer)){               // if buffer too small

        // QueryServiceStatusEx
		return false;
	}

	// Save the tick count and initial checkpoint.

	start_tick_count = GetTickCount();
	old_check_point = service_status.dwCheckPoint;

	while (service_status.dwCurrentState == SERVICE_START_PENDING) {

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		wait_time = service_status.dwWaitHint / 10;

		if (wait_time < 1000)
			wait_time = 1000;
		else if (wait_time > 10000)
			wait_time = 10000;

		Sleep(wait_time);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			serv.handle(),             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&service_status,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&size_for_buffer)){              // if buffer too small

            // QueryServiceStatusEx
			return false;
		}

		if (service_status.dwCheckPoint > old_check_point) {
			// Continue to wait and check.

			start_tick_count = GetTickCount();
			old_check_point = service_status.dwCheckPoint;
		}
		else {

			if (GetTickCount() - start_tick_count > service_status.dwWaitHint) {
				// No progress made within the wait hint.
				break;
			}
		}
	}

	// Determine whether the service is running.

	if (service_status.dwCurrentState == SERVICE_RUNNING) {
		return true;
	}
	else {
		stringstream ss;
#if 0
		ss << "Service not started. \n"
			<< "  Current State: " << service_status.dwCurrentState << '\n'
			<< "  Exit Code: " << service_status.dwWin32ExitCode << '\n'
			<< "  Check Point: " << service_status.dwCheckPoint << '\n'
			<< "  Wait Hint: " << service_status.dwWaitHint;
#endif
        return false;
	}
    return true;
}


//static 
bool NativeServiceHelper::stop_service(const std::string& service_name)
{
    // Get a handle to the SCM database
    ScmManager scm(SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm.valid()) {
        return false;
    }

    // Get a handle to the service.
    ServiceWrapper serv(scm.handle(), service_name, SERVICE_QUERY_STATUS | SERVICE_START);
    if (!serv.valid()) {
        return false;
    }

	DWORD start_time = GetTickCount();
	DWORD size_for_buffer = 0;
	// Make sure the service is not already stopped.
	SERVICE_STATUS_PROCESS service_status_process;
	if (!QueryServiceStatusEx(serv.handle(),
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&service_status_process,
		sizeof(SERVICE_STATUS_PROCESS),
		&size_for_buffer)){

        // stop_service: QueryServiceStatusEx
		return false;
	}

	if (service_status_process.dwCurrentState == SERVICE_STOPPED)
	{
		// Service is already stopped
		return true;
	}

	// If a stop is pending, wait for it.
	// 30-second time-out
	DWORD service_timeout = 30000;
	DWORD wait_time = 0;
	while (service_status_process.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		wait_time = service_status_process.dwWaitHint / 10;

		if (wait_time < 1000)
			wait_time = 1000;
		else if (wait_time > 10000)
			wait_time = 10000;

		Sleep(wait_time);

		if (!QueryServiceStatusEx(
			serv.handle(),
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&service_status_process,
			sizeof(SERVICE_STATUS_PROCESS),
			&size_for_buffer)){

            // stop_service: QueryServiceStatusEx failed
			return false;
		}

		if (service_status_process.dwCurrentState == SERVICE_STOPPED) {
			return true;
		}

		if (GetTickCount() - start_time > service_timeout) {
			// Service stop timed out
			return false;
		}
	}

	// If the service is running, dependencies must be stopped first.
	// For example: StopDependentServices();

	// Send a stop code to the service.

	if (!ControlService(
		serv.handle(),
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&service_status_process)){

        // stop_service: ControlService failed
		return false;
	}

	// Wait for the service to stop.

	while (service_status_process.dwCurrentState != SERVICE_STOPPED)
	{
		Sleep(service_status_process.dwWaitHint);
		if (!QueryServiceStatusEx(
			serv.handle(),
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&service_status_process,
			sizeof(SERVICE_STATUS_PROCESS),
			&size_for_buffer)) {

            // stop_service: QueryServiceStatusEx failed
			return false;
		}

		if (service_status_process.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - start_time > service_timeout){
			// Wait timed out
			return false;
		}
	}
    return true;
}


//static 
bool NativeServiceHelper::register_service(const std::string& service_name, 
    const std::string& display_name, 
    const std::string& account_name)
{
	char service_binary[MAX_PATH] = {};
	if (!GetModuleFileNameA(nullptr, service_binary, MAX_PATH)) {
        // Cannot install service
		return false;
	}

	// Get a handle to the SCM database
	ScmManager scm(SC_MANAGER_ALL_ACCESS);
    if (!scm.valid()) {
        return false;
    }

	// Create the service

	SC_HANDLE schService = CreateServiceA(
		scm.handle(),              // SCM database 
		service_name.c_str(),     // name of service 

		// service name to display 
		display_name.empty() ? service_name.c_str() : display_name.c_str(),

		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_AUTO_START,        // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		service_binary,            // path to service's binary 
		nullptr,                      // no load ordering group 
		nullptr,                      // no tag identifier 
		nullptr,                      // no dependencies 
        // LocalSystem account if nullptr, "DomainName\UserName" otherwise
        account_name.empty() ? nullptr : account_name.c_str(),
		nullptr);                     // no password 

	if (schService == nullptr) {
        // register_service: CreateService failed
		return false;
	}

	CloseServiceHandle(schService);
    return true;
}


//static 
bool NativeServiceHelper::delete_service(const std::string& service_name)
{

	// Get a handle to the SCM database. 
	ScmManager scm(SC_MANAGER_ALL_ACCESS);
    if (!scm.valid()) {
        return false;
    }

	// Get a handle to the service.
	SC_HANDLE service_handle = OpenServiceA(
		scm.handle(),          // SCM database 
		service_name.c_str(),  // name of service 
		DELETE);               // need delete access 

	if (service_handle == nullptr){
        // delete_service: OpenService failed
		return false;
	}

	// Delete the service.
	if (!DeleteService(service_handle)){
        // delete_service: DeleteService failed
        return false;
	}

	CloseServiceHandle(service_handle);
    return true;
}

bool helpers::NativeServiceHelper::set_service_description(const std::string& service_name, const std::string& service_description)
{
    // Get a handle to the SCM database
    ScmManager scm(SC_MANAGER_ALL_ACCESS);
    if (!scm.valid()) {
        return false;
    }

    // Get a handle to the service.
    ServiceWrapper serv(scm.handle(), service_name, SERVICE_CHANGE_CONFIG);

    SERVICE_DESCRIPTIONA system_service_description{};
    system_service_description.lpDescription = const_cast<LPSTR>(service_description.c_str());

    if (!ChangeServiceConfig2A(serv.handle(), SERVICE_CONFIG_DESCRIPTION, &system_service_description)) {
        // set_service_description: ChangeServiceConfig2 failed
        return false;
    }
    return true;
}

bool NativeServiceHelper::set_service_restore_action(const std::string& service_name)
{
    // Get a handle to the SCM database
    ScmManager scm(SC_MANAGER_ALL_ACCESS);
    if (!scm.valid()) {
        return false;
    }

    // Get a handle to the service.
    ServiceWrapper serv(scm.handle(), service_name, SERVICE_ALL_ACCESS);
    if (!serv.valid()) {
        return false;
    }

    // 1st, 2nd, 3rd+ actions
    
    // Debug option, don't forget to turn it off in the begging of file
#ifdef DEBUG_SERVICE
    SC_ACTION sequence_of_actions[] = { { SC_ACTION_NONE, 0 },{ SC_ACTION_NONE, 0 },{ SC_ACTION_NONE, 0 } };
#else
    SC_ACTION sequence_of_actions[] = { { SC_ACTION_RESTART, 1 },{ SC_ACTION_NONE, 0 },{ SC_ACTION_NONE, 0 } };
    
#endif

    // assign actions
    SERVICE_FAILURE_ACTIONS failuse_actions{};
    failuse_actions.dwResetPeriod = 86400; // 1 day
    failuse_actions.cActions = ARRAYSIZE(sequence_of_actions);
    failuse_actions.lpsaActions = sequence_of_actions;

    if (!ChangeServiceConfig2(serv.handle(), SERVICE_CONFIG_FAILURE_ACTIONS, &failuse_actions)) {
        // set_service_restore_action: ChangeServiceConfig2 failed
        return false;
    }
    return true;
}

#endif // defined(_WIN32) || defined(_WIN64)
