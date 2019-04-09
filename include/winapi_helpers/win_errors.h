#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include <stdexcept>
#include <system_error>
#include <string>
#include <algorithm>
#include <Windows.h>
#include <Ras.h>
#include <RasError.h>
#include <winapi_helpers/win_ptrs.h>

// Windows and RAS wrapper objects (exception, error message formatting etc)

namespace helpers {

/// @brief RAS error exception with message and code (see <RasError.h>)
class WindowsRasError : public std::runtime_error{
public:
    /// @brief General Desire VPN exception
    WindowsRasError(const std::string& s, DWORD error_code, DWORD win_error) :
        std::runtime_error(s),
        _ras_error_code(error_code),
        _win_error_code(win_error){}

    /// @brief General Desire VPN exception
    WindowsRasError(const char* s, DWORD error_code, DWORD win_error) :
        std::runtime_error(s),
        _ras_error_code(error_code),
        _win_error_code(win_error){}

    DWORD res_error_code() const 
    { 
        return _ras_error_code; 
    }

    DWORD win_error_code() const 
    { 
        return _win_error_code; 
    }

private:

    /// Error codes described in RasError.h
    DWORD _ras_error_code;

    /// Error code returned by GetLastError()
    DWORD _win_error_code;
};


/// @brief transform Windows error code to string with throwing an exception or without it
/// setlocale(0, "") should be set to display error text in the Windows console 
class WinErrorChecker{
public:


    // @brief Transform Windows error code to bool without throwing an exception
    // NOTE! Suits for WinAPI functions that return success code
    // @return true if success, false otherwise
    static bool retbool_nothrow_retcode(DWORD error_message_ID)
    {
        //Get the error message, if any
        bool ret = (error_message_ID == ERROR_SUCCESS) ? true : false;
        return ret;
    }

    // @brief Transform Windows error code to string without throwing an exception
    // NOTE! Suits for WinAPI functions that return success code
    // @return String description of Windows last error
    static std::string last_error_nothrow_retcode(DWORD error_message_ID) 
    {

        //Get the error message, if any
        if (error_message_ID == ERROR_SUCCESS)
            return std::string{};

        return format_error_message(error_message_ID);
    }


    // @brief Transform Windows error code to string with throwing an exception
    // NOTE! Suits for WinAPI functions that return success code
    // @throw std::runtime_error containing string description of the last Windows error
    static void last_error_throw_retcode(DWORD error_message_ID) 
    {

        if (error_message_ID == ERROR_SUCCESS)
            return;
        else {
#ifdef SYSTEM_ERROR
            throw std::system_error(error_message_ID, std::system_category());
#else
            throw std::runtime_error(format_error_message(error_message_ID));
#endif
        }
    }

    // @brief Transform Windows error code to bool without throwing an exception
    // NOTE! Suits for WinAPI functions that return success code
    // @return true if success, false otherwise
    static bool retbool_nothrow_boolean(BOOL return_code)
    {
        //Get the error message, if any
        bool ret = (return_code) ? true : false;
        return ret;
    }

    // @brief Transform Windows error code to string without throwing an exception
    // NOTE! Suits for WinAPI functions that return BOOL
    // @return String description of Windows last error
    static std::string last_error_nothrow_boolean(BOOL return_code)
    {

        //Get the error message, if any
        if (return_code != FALSE)
            return std::string{};

        return format_error_message(GetLastError());
    }

    // @brief Transform Windows error code to string with throwing an exception
    // NOTE! Suits for WinAPI functions that return BOOL
    // @throw std::runtime_error containing string description of the last Windows error
    static void last_error_throw_boolean(BOOL return_code)
    {

        if (return_code != FALSE)
            return;
        else {
#ifdef SYSTEM_ERROR
            throw std::system_error(error_message_ID, std::system_category());
#else
            throw std::runtime_error(format_error_message(GetLastError()));
#endif
        }
    }



private:
    
    static std::string format_error_message(DWORD error_message_ID)
    {
        // Format Windows error message
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_message_ID, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);

        // take ownership under Local Heap, perform cleanup in the end of close
        WinLocalPtr<CHAR> win_string_raii(messageBuffer);
        std::string error_description(messageBuffer, size);
        error_description.erase(std::remove(error_description.begin(), error_description.end(), '\r'), error_description.end());
        error_description.erase(std::remove(error_description.begin(), error_description.end(), '\n'), error_description.end());
        return error_description;
    }
};


/// @brief transform RAS Service error code to string with throwing an exception or without it
/// setlocale(0, "") should be set to display error text in the Windows console
class RasErrorChecker{
public:

    /// @brief: Create an object that performs Windows error translation to std::sstring
    /// @param correct: Return code that reports successful operation
    RasErrorChecker(DWORD correct) :_correct_error_code(correct), _error_code(ERROR_SUCCESS){}

    /// @brief Check operation result code
    /// @return true if operation is successful, false otherwise
    bool check_nothrow(DWORD error_code)
    {
        if (error_code != _correct_error_code){
			_error_code = error_code;
            char ras_error_buffer[MAX_ERROR_BUFFER] = {};
            DWORD ras_error_code = _correct_error_code;
            if (error_code > RASBASE && error_code < RASBASEEND){
                ras_error_code = RasGetErrorStringA(error_code, ras_error_buffer, MAX_ERROR_BUFFER);
            }
            else{
                // unable to obtain an error message
                _description = WinErrorChecker::last_error_nothrow_retcode(error_code);
                return false;
            }
            
            if (ras_error_code != ERROR_SUCCESS){
                // unable to obtain an error message
                _description = "Unable to retrieve RAS error info";
                _description.append(WinErrorChecker::last_error_nothrow_retcode(ras_error_code));
                return false;
            }

            _description.assign(ras_error_buffer, strlen(ras_error_buffer));
            return false;
        }
        return true;
    }


    /// @brief Check operation result code
    /// @throw runtime_error if operation is not successful
    void check_throw(DWORD error_code)
    {
        if (error_code != _correct_error_code){
            _error_code = error_code;
            DWORD ras_error_code = _correct_error_code;
            char rasErrorBuffer[MAX_ERROR_BUFFER] = {};
            if (error_code > RASBASE && error_code < RASBASEEND){
                ras_error_code = RasGetErrorStringA(error_code, rasErrorBuffer, MAX_ERROR_BUFFER);
            }
            else{
                _description = WinErrorChecker::last_error_nothrow_retcode(error_code);
                throw WindowsRasError(_description.c_str(), ras_error_code, GetLastError());
            }

            if (ras_error_code != ERROR_SUCCESS){
                // unable to obtain an error message
                _description = "Unable to retrieve RAS error info";
                _description.append(WinErrorChecker::last_error_nothrow_retcode(ras_error_code));
                throw WindowsRasError(_description.c_str(), ras_error_code, GetLastError());
            }

            _description.assign(rasErrorBuffer);
            throw WindowsRasError(_description.c_str(), ras_error_code, GetLastError());
        }
    }

    /// @brief Last error description
    std::string error_description() const 
    {
        return _description;
    }

    /// @brief Last error code processed by the error-checker
    DWORD last_error() const 
    {
        return _error_code;
    }

    /// @brief Append information to the existing description
    void append(const std::string& err_str)
    {
        _description.append(err_str);
    }


private:

    // Return code that reports successful operation
    DWORD _correct_error_code;

    // Last error code
    DWORD _error_code;

    // Last error string
    std::string _description;

    // That buffer length should be sufficient for any error message according to MSDN
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa377539(v=vs.85).aspx
    static const size_t MAX_ERROR_BUFFER = 1024;
};

} // namespace helpers 

#endif
