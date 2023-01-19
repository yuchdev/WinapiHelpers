#include <winapi-helpers/utilities.h>
#include <stdexcept>
#include <locale>
#include <codecvt>
using namespace helpers;


#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

void helpers::set_memory_profiling()
{
#ifdef _DEBUG

    // CRT memory leak detector
    // map the malloc and free functions to their Debug versions, _malloc_dbg and _free_dbg
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void helpers::set_console_ctrl_handler(PHANDLER_ROUTINE ctrl_handler)
{
    SetConsoleCtrlHandler(ctrl_handler, TRUE);
}


std::string helpers::wstring_to_utf8(const std::wstring &var)
{
    static std::locale loc("");
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
        return cv.to_bytes(var);
    }
    catch (const std::system_error& e) {
        return std::string{};
    }
    catch (const std::exception& e) {
        return std::string{};
    }
    return std::string{};
}

std::wstring helpers::utf8_to_wstring(const std::string &var)
{
    static std::locale loc("");
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
        return cv.from_bytes(var);
    }
    catch (const std::system_error& e) {
        return std::wstring{};
    }
    catch (const std::exception& e) {
        return std::wstring{};
    }
    return std::wstring{};
}

std::string helpers::wstring_to_string(const std::wstring &var)
{
    static std::locale loc("");
    auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
    try {
        return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
    }
    catch (const std::system_error& e) {
        return std::string{};
    }
    catch (const std::exception& e) {
        return std::string{};
    }
    return std::string{};
}

std::wstring helpers::string_to_wstring(const std::string &var)
{
    static std::locale loc("");
    auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
    try {
        return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
    }
    catch (const std::system_error& e) {
        return std::wstring{};
    }
    catch (const std::exception& e) {
        return std::wstring{};
    }
    return std::wstring{};
}


#elif defined (__unix__) || (defined (__unix) && defined (__APPLE__))

void helpers::set_memory_profiling()
{
#ifdef _DEBUG
    assert(false);
    throw std::runtime_error("Not implemented");
#endif
}

void helpers::set_console_ctrl_handler(posix_ctrl_handler_t ctrl_handler)
{
    assert(false);
    throw std::runtime_error("Not implemented");
}

#endif
