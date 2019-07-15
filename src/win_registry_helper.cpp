#if defined(_WIN32) || defined(_WIN64)

#include <winapi_helpers/win_registry_helper.h>
#include <winapi_helpers/win_errors.h>
#include <winapi_helpers/win_system_information.h>

#include <map>
#include <string>
#include <locale>
#include <sstream>

// https://msdn.microsoft.com/en-us/library/ms724405(v=vs.85).aspx
// http://stackoverflow.com/questions/7011071/detect-32-bit-or-64-bit-of-windows

namespace {

constexpr bool is_x64_application()
{
#ifdef _M_X64
    return true;
#else
    return false;
#endif
}

std::wstring to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv;
    std::wstring wstr = conv.from_bytes(str);
    return wstr;
}

} // namespace

using namespace std;
using namespace helpers;

struct helpers::HKEY_holder {
    HKEY key = nullptr;

    /// @brief get Windows-specific registry root handle by its string value
    static HKEY get_root_key(const char* root_key)
    {
        auto it = key_values.find(root_key);
        if (key_values.end() != it) {
            return (*it).second;
        }
        else {
            return nullptr;
        }
    }

    static std::map<std::string, HKEY> key_values;
};

std::map<std::string, HKEY> helpers::HKEY_holder::key_values = {
    { "HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT },
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
    std::string key_name(key.substr(root_ends + 1));
    HKEY root = HKEY_holder::get_root_key(root_key.c_str());
    if (root == nullptr) {
        throw std::system_error(std::error_code(::GetLastError(), std::system_category()));
    }
    
    REGSAM samDesired;
    if (is_x32_application_on_x64())
        samDesired = KEY_WOW64_64KEY;
    else
        samDesired = KEY_WOW64_32KEY;

    WinErrorChecker::last_error_throw_retcode(
        RegOpenKeyExA(
            root,
            key_name.c_str(),
            0,
            (read_only ? (KEY_READ | samDesired)
                : (KEY_ALL_ACCESS | samDesired)),
            &hkey_->key));
}

RegistryKey::~RegistryKey()
{
    RegCloseKey(hkey_->key);
}

std::optional<bool> helpers::RegistryKey::is_key_exist(const std::string& key)
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

    REGSAM samDesired;
    if (is_x32_application_on_x64())
        samDesired = KEY_WOW64_64KEY;
    else
        samDesired = KEY_WOW64_32KEY;

    LONG ret_code = RegOpenKeyExA(root, key_name.c_str(), 0, KEY_ALL_ACCESS | samDesired, &holder);
    if (ERROR_SUCCESS == ret_code) {
        RegCloseKey(holder);
        return true;
    }
    if (ERROR_FILE_NOT_FOUND == ret_code) {
        return false;
    }
    return false;
}

std::optional<bool> helpers::RegistryKey::is_value_exist(const std::string& value_name) const
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
    return false;
}


bool RegistryKey::set_dword_value(const std::string& key, unsigned long value)
{
    const DWORD dw_value = static_cast<DWORD>(value);
    return WinErrorChecker::retbool_nothrow_retcode(RegSetValueExA(hkey_->key, // Root key
        key.c_str(), // key name
        0, // reserved, == 0
        REG_DWORD, // value type
        // buffer where value stored
        reinterpret_cast<const BYTE*>(&dw_value),
        // buffer size
        sizeof(DWORD)));
}

bool RegistryKey::set_string_value(const std::string& key, const std::string& value)
{
    const std::string str_value(value);
    return WinErrorChecker::retbool_nothrow_retcode(RegSetValueExA(hkey_->key, // Root key
        key.c_str(),
        0,                                                                     // reserved, == 0
        REG_SZ,                                                                // value type - null-terminated string
        reinterpret_cast<const BYTE*>(str_value.c_str()),                      // buffer where value stored
        static_cast<DWORD>(str_value.size() + 1)));                            // buffer size
}

bool RegistryKey::set_multi_string_value(const std::string& key, const std::vector<std::string>& values)
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

    return WinErrorChecker::retbool_nothrow_retcode(RegSetValueExA(hkey_->key, // Root key
        key.c_str(),
        0,                                             // reserved, == 0
        REG_MULTI_SZ,                                  // value type - multi-string with zero-delimiter
        reinterpret_cast<const BYTE*>(&str_value[0]),  // buffer where value stored
        static_cast<DWORD>(str_value.size())));        // buffer size
}

bool RegistryKey::set_binary_value(const string &key_name, const string &value)
{
    const std::string str_value(value);
    return WinErrorChecker::retbool_nothrow_retcode(RegSetValueExA(hkey_->key,    // Root key
        key_name.c_str(),
        0,                                                                  // reserved, == 0
        REG_BINARY,                                                         // value type - binary array
        reinterpret_cast<const BYTE*>(str_value.c_str()),                   // buffer where value stored
        static_cast<DWORD>(str_value.size() + 1)));                         // buffer size
}


bool RegistryKey::load_from_file(const string &file_path, const std::string& key_name) const
{
    if (!enable_restore_privilege()) {
        return false;
    }

    return WinErrorChecker::retbool_nothrow_boolean(
        RegRestoreKeyA(hkey_->key, (file_path + key_name).data(), NULL));
}

bool RegistryKey::create_key(const string &key_name) const
{
    return WinErrorChecker::retbool_nothrow_retcode(RegCreateKeyExA(hkey_->key,
        key_name.data(), 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &hkey_->key, NULL));
}

bool RegistryKey::delete_tree(const std::string& key)
{
    HKEY root{};
    HKEY holder{};
    size_t root_ends = key.find_first_of('\\');
    std::string root_key(key.substr(0, root_ends));
    std::string key_name(key.substr(root_ends + 1));

    root = HKEY_holder::get_root_key(root_key.c_str());
    if (nullptr == root) {
        return false;
    }

    LONG ret_code = RegOpenKeyExA(root, key_name.c_str(), 0, KEY_ALL_ACCESS, &holder);
    if (ERROR_SUCCESS == ret_code) {
        ret_code = RegDeleteTree(holder, NULL);
        if (ERROR_SUCCESS != ret_code) {
            return false;
        }
        RegCloseKey(holder);
        return true;
    }
    if (ERROR_FILE_NOT_FOUND == ret_code) {
        return false;
    }

    return false;
}

bool RegistryKey::delete_key(const string &key_name) const
{
    return WinErrorChecker::retbool_nothrow_retcode(RegDeleteKeyExA(hkey_->key,
        key_name.data(), KEY_ALL_ACCESS, NULL));
}

bool RegistryKey::delete_value(const string &value_name)
{
    return WinErrorChecker::retbool_nothrow_retcode(RegDeleteValueA(hkey_->key, value_name.data()));
}

bool RegistryKey::save_to_file(const std::string &file_path, const std::string& key_name) const
{
    if (!enable_backup_privilege()) {
        return false;
    }

    std::string filename{ file_path };
    filename += key_name;
    return WinErrorChecker::retbool_nothrow_retcode(RegSaveKeyA(hkey_->key,
        filename.c_str(),
        NULL));
}

std::optional<unsigned long> helpers::RegistryKey::get_dword_value(const std::string& key_name) const
{
    unsigned long result = 0UL;
    DWORD dw_value = 1L;
    DWORD value_type = REG_DWORD;
    DWORD ret_buffer_size = sizeof(DWORD);

    bool ret = WinErrorChecker::retbool_nothrow_retcode(::RegQueryValueExA(hkey_->key, // Root key
        key_name.c_str(),       // key name
        0,              // reserved, == 0
        &value_type,    // receive here type
        reinterpret_cast<LPBYTE>(&dw_value),    // receive here value
        &ret_buffer_size));                             // receive here size

    if ((!ret) || (value_type != REG_DWORD)) {
        return {};
    }

    return result;
}

std::optional<std::string> helpers::RegistryKey::get_string_value(const std::string& key_name) const
{
    std::string{};
    char buffer[MAX_KEY_LENGTH] = {};
    DWORD value_type = REG_SZ;
    DWORD ret_buffer_size = sizeof(buffer);

    bool ret = WinErrorChecker::retbool_nothrow_retcode(::RegQueryValueExA(hkey_->key, // Root key
        key_name.c_str(),                            // key name
        0,                                      // reserved, == 0
        &value_type,                            // receive here type
        reinterpret_cast<LPBYTE>(&buffer),      // receive here value
        &ret_buffer_size));                     // receive here size

    if ((!ret) || (value_type != REG_SZ)) {
        return {};
    }

    return std::string{buffer};
}

std::optional<std::string> helpers::RegistryKey::get_binary_value(const std::string& key_name) const
{
    std::string result{};
    char buffer[4096] = {};
    DWORD value_type = REG_BINARY;
    DWORD ret_buffer_size = sizeof(buffer);
    bool ret = WinErrorChecker::retbool_nothrow_retcode(::RegQueryValueExA(hkey_->key, // Root key
        key_name.data(),                                                          // key name
        0,                                                                   // reserved
        &value_type,                                                         // value type
        reinterpret_cast<LPBYTE>(&buffer),                                   // OUT
        &ret_buffer_size));                                                  // size

    if ((!ret) || (value_type != REG_BINARY)) {
        return {};
    }

    result = std::string(buffer, ret_buffer_size);
    return std::move(result);
}

std::optional<std::wstring> helpers::RegistryKey::get_wstring_value(const std::string& key_name) const
{
    wstring result{};
    optional<string> ret = get_string_value(key_name);
    if (!ret.has_value()) {
        return {};
    }

    return to_wstring(ret.value());
}

std::optional<RegistryKey::multi_sz> RegistryKey::get_multi_string_value(const std::string& key_name) const
{
    multi_sz result{};
    DWORD value_type = REG_MULTI_SZ;
    DWORD ret_buffer_size{};
    std::vector<char> buffer(1024);

    LONG retcode = ::RegQueryValueExA(hkey_->key, // Root key
        key_name.c_str(),// key name
        0,              // reserved, == 0
        &value_type,    // receive here type
                        // receive here value
        reinterpret_cast<LPBYTE>(&buffer[0]),
        // receive here size
        &ret_buffer_size);

    if (ERROR_MORE_DATA == retcode) {
        buffer.resize(static_cast<size_t>(ret_buffer_size));

        bool ret = WinErrorChecker::retbool_nothrow_retcode(::RegQueryValueExA(hkey_->key, // Root key
            key_name.c_str(),       // key name
            0,              // reserved, == 0
            &value_type,    // receive here type
                            // receive here value
            reinterpret_cast<LPBYTE>(&buffer[0]),
            // receive here size
            &ret_buffer_size));

        if ((!ret) || (value_type != REG_MULTI_SZ)) {
            return {};
        }
    }

    // parse REG_MULTI_SZ
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

bool RegistryKey::enable_backup_privilege() const
{
    HANDLE handle_token_ = NULL;
    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &handle_token_)) {
        CloseHandle(handle_token_);
        return false;
    }

    TOKEN_PRIVILEGES privileges_token_;
    if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &privileges_token_.Privileges[0].Luid)) {
        CloseHandle(handle_token_);
        return false;
    }

    privileges_token_.PrivilegeCount = 1;
    privileges_token_.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(handle_token_, FALSE, &privileges_token_, 0, NULL, 0)) {
        CloseHandle(handle_token_);
        return false;
    }

    CloseHandle(handle_token_);
    return TRUE;
}

bool RegistryKey::enable_restore_privilege() const
{
    HANDLE handle_token_ = NULL;
    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &handle_token_)) {
        CloseHandle(handle_token_);
        return false;
    }

    TOKEN_PRIVILEGES privileges_token_;
    if (!LookupPrivilegeValue(
        NULL,
        SE_RESTORE_NAME,
        &privileges_token_.Privileges[0].Luid)) {
        CloseHandle(handle_token_);
        return false;
    }

    privileges_token_.PrivilegeCount = 1;
    privileges_token_.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(handle_token_, FALSE, &privileges_token_, 0, NULL, 0)) {
        CloseHandle(handle_token_);
        return false;
    }

    CloseHandle(handle_token_);
    return true;
}

std::optional<RegistryKey::values_list> RegistryKey::enumerate_subkeys() const
{
    // return: 1st - number of subkeys, 2nd - number of subvalues
    std::pair<DWORD, DWORD> enumerator = count_subvalues();
    std::vector<std::string> subkeys;
    FILETIME last_change_time{};
    std::string error_string;

    for (DWORD i = 0; i < enumerator.first; i++) {

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
        // because MAX_KEY_LENGTH enough for any
        if (!error_string.empty()) {
            return std::vector<std::string>{};
        }

        // enumerate subkey name
        if (subkey_name_buffer) {
            subkeys.push_back(subkey_name_buffer);
        }
    }

    return subkeys;
}

std::optional<RegistryKey::values_list> RegistryKey::enumerate_values() const
{
    // return: 1st - number of subkeys, 2nd - number of subvalues
    std::pair<DWORD, DWORD> enumerator = count_subvalues();
    std::vector<std::string> subvalues;
    std::string error_string;

    // enumerator.second is number of subvalues
    for (DWORD i = 0; i < enumerator.second; i++) {

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
    return subvalues;
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

#endif // defined(_WIN32) || defined(_WIN64)
