#if defined(_WIN32) || defined(_WIN64)
#include <winapi_helpers/win_errors.h>
#include <winapi_helpers/win_registry_helper.h>
#include <map>
#include <string>
#include <locale>

// https://msdn.microsoft.com/en-us/library/ms724405(v=vs.85).aspx
// http://stackoverflow.com/questions/7011071/detect-32-bit-or-64-bit-of-windows

constexpr bool is_x64_application()
{
#ifdef _M_X64
    return true;
#else
    return false;
#endif
}

using namespace std;
using namespace helpers;

struct helpers::HKEY_holder {
    HKEY key;

    /// @brief get Windows-specific registry root handle by its string value
    static HKEY get_root_key(const char* root_key) 
    {
        auto it = key_values.find(root_key);
        if (key_values.end() != it) {
            return (*it).second;
        }
        else {
            std::stringstream ss;
            ss << "Wrong Windows registry root param passed: " << root_key;
            throw std::runtime_error(ss.str().c_str());
        }
    }

    static std::map<std::string, HKEY> key_values;
};

std::map<std::string, HKEY> helpers::HKEY_holder::key_values = {
    {"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT },
    { "HKEY_CURRENT_USER", HKEY_CURRENT_USER },
    { "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE },
    { "HKEY_USERS", HKEY_USERS },
    { "HKEY_PERFORMANCE_DATA", HKEY_PERFORMANCE_DATA },
    { "HKEY_PERFORMANCE_TEXT", HKEY_PERFORMANCE_TEXT },
    { "HKEY_PERFORMANCE_NLSTEXT", HKEY_PERFORMANCE_NLSTEXT },
#if (WINVER >= 0x0400)
    { "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
#endif
    { "HKEY_DYN_DATA", HKEY_DYN_DATA },
    { "HKEY_CURRENT_USER_LOCAL_SETTINGS", HKEY_CURRENT_USER_LOCAL_SETTINGS }
};


RegistryKey::RegistryKey(const std::string& key, bool read_only/* = false*/) : hkey_(new HKEY_holder)
{
    size_t root_ends = key.find_first_of('\\');
    std::string root_key(key.substr(0, root_ends));
    std::string key_name(key.substr(root_ends+1));
    HKEY root = HKEY_holder::get_root_key(root_key.c_str());
    WinErrorChecker::last_error_throw_retcode(RegOpenKeyExA(root, key_name.c_str(), 0, (read_only ? KEY_READ : KEY_ALL_ACCESS), &hkey_->key));
}

RegistryKey::~RegistryKey()
{
    RegCloseKey(hkey_->key);
}

bool RegistryKey::delete_tree(const std::string& key)
{
    HKEY root{};
    HKEY holder{};
    size_t root_ends = key.find_first_of('\\');
    std::string root_key(key.substr(0, root_ends));
    std::string key_name(key.substr(root_ends + 1));

    try {
        root = HKEY_holder::get_root_key(root_key.c_str());
    }
    catch (const std::exception&) {
        return false;
    }

    LONG ret_code = RegOpenKeyExA(root, key_name.c_str(), 0, KEY_ALL_ACCESS, &holder);
    if (ERROR_SUCCESS == ret_code) {
        ret_code = RegDeleteTree(holder, NULL);
        if (ERROR_SUCCESS != ret_code) {
            throw_formatted(key, " is unable to remove recursively with error code ", ret_code);
            return false;
        }
        RegCloseKey(holder);
        return true;
    }
    if (ERROR_FILE_NOT_FOUND == ret_code) {
        return false;
    }
    throw_formatted(key, " is unable to open with error code ", ret_code);
    return false;
}

bool RegistryKey::is_key_exist(const std::string& key)
{
    HKEY root{};
    HKEY holder{};
    size_t root_ends = key.find_first_of('\\');
    std::string root_key(key.substr(0, root_ends));
    std::string key_name(key.substr(root_ends + 1));

    try {
        root = HKEY_holder::get_root_key(root_key.c_str());
    }
    catch (const std::exception&) {
        return false;
    }

    LONG ret_code = RegOpenKeyExA(root, key_name.c_str(), 0, KEY_ALL_ACCESS, &holder);
    if (ERROR_SUCCESS == ret_code) {
        RegCloseKey(holder);
        return true;
    }
    if (ERROR_FILE_NOT_FOUND == ret_code) {
        return false;
    }
    throw_formatted(key, " is unable to open with error code ", ret_code);
    return false;
}

bool RegistryKey::is_value_exists(const std::string& value_name) const
{
    LONG ret_code = ::RegQueryValueExA(hkey_->key, // Root key
        value_name.c_str(), // value name
        0,          // reserved, == 0
        NULL,       // receive type - nothing, just check
        NULL,       // receive here value - also nothing, just check
        NULL);      // size is null

    if (ERROR_SUCCESS == ret_code) {
        return true;
    }
    if (ERROR_FILE_NOT_FOUND == ret_code) {
        return false;
    }
    throw_formatted(value_name, " is unable to open with error code ", ret_code);
    return false;
}


void RegistryKey::set_dword_value(const std::string& key, unsigned long value)
{
    const DWORD dw_value = static_cast<DWORD>(value);
    WinErrorChecker::last_error_throw_retcode(RegSetValueExA(hkey_->key, // Root key
        key.c_str(),    // key name
        0,      // reserved, == 0
        REG_DWORD,  // value type
        reinterpret_cast<const BYTE*>(&dw_value),   // buffer where value stored
        sizeof(DWORD)));                            // buffer size
}

unsigned long RegistryKey::get_dword_value(const std::string& key) const
{
    DWORD dw_value = 1L;
    DWORD value_type = REG_DWORD;
    DWORD ret_buffer_size = sizeof(DWORD);

    WinErrorChecker::last_error_throw_retcode(::RegQueryValueExA(hkey_->key, // Root key
        key.c_str(),       // key name
        0,              // reserved, == 0
        &value_type,    // receive here type
        reinterpret_cast<LPBYTE>(&dw_value),    // receive here value
        &ret_buffer_size));                             // receive here size

    if (value_type != REG_DWORD){
        throw std::runtime_error("Value you trying to read is not a DWORD");
    }

    return dw_value;
}

void RegistryKey::set_string_value(const std::string& key, const std::string& value)
{
    const std::string str_value(value);
    WinErrorChecker::last_error_throw_retcode(RegSetValueExA(hkey_->key, // Root key
        key.c_str(),
        0,      // reserved, == 0
        REG_SZ, // value type - null-terminated string
        reinterpret_cast<const BYTE*>(str_value.c_str()),  // buffer where value stored
        static_cast<DWORD>(str_value.size() + 1)));                            // buffer size
}

std::string RegistryKey::get_string_value(const std::string& key) const
{
    char buffer[1024] = {};
    DWORD value_type = REG_SZ;
    DWORD ret_buffer_size = sizeof(buffer);

    WinErrorChecker::last_error_throw_retcode(::RegQueryValueExA(hkey_->key, // Root key
        key.c_str(),       // key name
        0,              // reserved, == 0
        &value_type,    // receive here type
        reinterpret_cast<LPBYTE>(&buffer),    // receive here value
        &ret_buffer_size));                           // receive here size

    if (value_type != REG_SZ) {
        throw std::runtime_error("Value you trying to read is not a null-terminated string");
    }

    return string(buffer);
}

std::wstring RegistryKey::get_wstring_value(const std::string& key) const
{
    return this->to_wstring(get_string_value(key));
}

void RegistryKey::set_multi_string_value(const std::string& key, const std::vector<std::string>& values)
{
    // if REG_MULTI_SZ is empty we should add one /0 value anyway
    size_t buffer_size = values.empty() ? 1 : 0;
    for (auto& value : values) {
        buffer_size += value.size() ? (value.size() + 1) : 0;
    }
    // for terminator
    buffer_size += 1;

    std::vector<char> str_value(buffer_size);
    auto it = str_value.begin();
    for (auto& value : values) {
        std::copy(value.begin(), value.end(), it);
        it += (value.size() + 1);
    }

    WinErrorChecker::last_error_throw_retcode(RegSetValueExA(hkey_->key, // Root key
        key.c_str(),
        0,              // reserved, == 0
        REG_MULTI_SZ,   // value type - multi-string with zero-delimiter
        reinterpret_cast<const BYTE*>(&str_value[0]),  // buffer where value stored
        static_cast<DWORD>(str_value.size())));                            // buffer size
}

std::vector<std::string> RegistryKey::get_multi_string_value(const std::string& key) const
{
    DWORD value_type = REG_MULTI_SZ;
    DWORD ret_buffer_size{};
    std::vector<char> buffer(1024);

    LONG retcode = ::RegQueryValueExA(hkey_->key, // Root key
        key.c_str(),       // key name
        0,              // reserved, == 0
        &value_type,    // receive here type
                        // receive here value
        reinterpret_cast<LPBYTE>(&buffer[0]),
        // receive here size
        &ret_buffer_size);

    if (ERROR_MORE_DATA == retcode) {
        buffer.resize(static_cast<size_t>(ret_buffer_size));

        WinErrorChecker::last_error_throw_retcode(::RegQueryValueExA(hkey_->key, // Root key
            key.c_str(),       // key name
            0,              // reserved, == 0
            &value_type,    // receive here type
                            // receive here value
            reinterpret_cast<LPBYTE>(&buffer[0]),
            // receive here size
            &ret_buffer_size));
    }

    if (value_type != REG_MULTI_SZ) {
        throw std::runtime_error("Value you trying to read is not a null-terminated string");
    }

    // parse REG_MULTI_SZ
    std::vector<std::string> result;
    const char* string_item = &buffer[0];
    do {

        if (nullptr == string_item) {
            // most probably we called parsing from the entry which do not contain strings
            break;
        }
        result.push_back(std::string(reinterpret_cast<const char*>(string_item)));
        string_item += result.back().size() + 1;

    } while (string_item[0] != 0 && string_item[1] != 0);

    return std::move(result);
}

std::pair<DWORD, DWORD> RegistryKey::count_subvalues() const
{
    // All parameters are [IN], so they receive values for the current key
    TCHAR    class_name_in_buffer[MAX_PATH] = {};
    DWORD    class_name_length = MAX_PATH;
    DWORD    number_of_subkeys_{};
    DWORD    number_of_subvalues_{};
    DWORD    longest_subkey_size{};
    DWORD    longest_class_string{};
    DWORD    longest_value_name{};
    DWORD    longest_value_size{};
    DWORD    security_handle{};
    FILETIME last_change_time{};

    // Get the class name, subkeys and values count
    WinErrorChecker::last_error_nothrow_boolean(RegQueryInfoKey(
        hkey_->key,
        class_name_in_buffer,
        &class_name_length,
        NULL, // reserved 
        &number_of_subkeys_,
        &longest_subkey_size,
        &longest_class_string,
        &number_of_subvalues_,
        &longest_value_name,
        &longest_value_size,
        &security_handle,
        &last_change_time));

    return std::make_pair(number_of_subkeys_, number_of_subvalues_);
}

std::vector<std::string> RegistryKey::enumerate_values() const
{
    // return: 1st - number of subkeys, 2nd - number of subvalues
    std::pair<DWORD, DWORD> enumerator = count_subvalues();
    std::vector<std::string> subvalues;
    std::string error_string;

    // enumerator.second is number os subvalues
    for (int i = 0; i < enumerator.second; i++) {

        char subvalue_name_buffer[MAX_KEY_LENGTH] = {};
        DWORD subvalue_name_size = MAX_KEY_LENGTH;
        DWORD value_type{};

        error_string = WinErrorChecker::last_error_nothrow_retcode(RegEnumValueA(hkey_->key, i,
            subvalue_name_buffer,
            &subvalue_name_size,
            NULL,
            NULL,
            NULL,
            &value_type));

        // probably multithreading (very rare) issue
        // because MAX_KEY_LENGTH enough for any key
        if (!error_string.empty()) {
            return std::vector<std::string>{};
        }

        // enumerate subvalue name (empty string means default)
        if (subvalue_name_buffer) {
            subvalues.push_back(subvalue_name_buffer);
        }
    }
    return std::move(subvalues);
}

std::vector<std::string> RegistryKey::enumerate_subkeys() const
{
    // return: 1st - number of subkeys, 2nd - number of subvalues
    std::pair<DWORD, DWORD> enumerator = count_subvalues();
    std::vector<std::string> subkeys;
    FILETIME last_change_time{};
    std::string error_string;

    for (int i = 0; i < enumerator.first; i++) {

        char subkey_name_buffer[MAX_KEY_LENGTH] = {};
        DWORD subkey_name_size = MAX_KEY_LENGTH;

        error_string = WinErrorChecker::last_error_nothrow_retcode(RegEnumKeyExA(hkey_->key, i,
            subkey_name_buffer,
            &subkey_name_size,
            NULL,
            NULL,
            NULL,
            &last_change_time));

        // probably multithreading (very rare) issue
        // because MAX_KEY_LENGTH enough for any key
        if (!error_string.empty()) {
            return std::vector<std::string>{};
        }

        // enumerate subkey name
        if (subkey_name_buffer) {
            subkeys.push_back(subkey_name_buffer);
        }
    }

    return std::move(subkeys);
}

std::wstring RegistryKey::to_wstring(const std::string& str) const
{
    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv;
    std::wstring wstr = conv.from_bytes(str);
    return wstr;
}

#endif // defined(_WIN32) || defined(_WIN64)
