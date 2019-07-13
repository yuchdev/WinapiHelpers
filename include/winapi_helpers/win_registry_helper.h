#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <memory>
#include <string>
#include <vector>

// Place here WinRegistry-related functions and classes

namespace helpers {


struct HKEY_holder;

/// @brief Class wraps Windows Registry key (incomplete, could be expanded)
class RegistryKey {
public:

    using multi_sz = std::vector<std::string>;

    /// @brief Open Windows registry key using RegOpenKeyEx() call, with all access
    /// @throw: runtime_error or system_error if unable to open the registry key
    explicit RegistryKey(const std::string& key_name = std::string(), bool read_only = false);

    /// @brief Closes Windows registry key using RegCloseKey() call
    ~RegistryKey();

    /// @brief Remove the registry key recursively
    static bool delete_tree(const std::string& key);

    /// @brief Just check whether the registry key exists
    static bool is_key_exist(const std::string& key_name);

    /// @brief Just check whether the registry value in the key exists
    bool is_value_exist(const std::string& value_name) const;

    /// @brief Set DWORD Windows registry value to the key
    bool set_dword_value(const std::string& key_name, unsigned long value);

    /// @brief Set string Windows registry value to the key
    bool set_string_value(const std::string& key_name, const std::string& value);

    /// @brief Set multi-string (REG_MULTI_SZ) Windows registry value to the key
    bool set_multi_string_value(const std::string& key_name, const std::vector<std::string>& values);

    /// @brief Set binary Windows registry value to the key
    bool set_binary_value(const std::string& key_name, const std::string& value);

    /// @brief Saves the specified key and all of its subkeys and values to a new file, in the standard format.
    /// @return part first - status (true/false) second - error string
    bool save_to_file(const std::string& file_path, const std::string& key_name) const;

    /// @brief Saves the specified key and all of its subkeys and values to a new file, in the standard format.
    /// @return part first - status (true/false) second - error string
    bool load_from_file(const std::string& file_path, const std::string& key_name) const;

    /// @brief create specified subkey in context key using RegCreateKeyEx
    bool create_key(const std::string& key_name) const;

    /// @brief delete specified subkey in context key using RegCreateKeyEx
    /// @throw runtime_error
    bool delete_key(const std::string& key_name) const;

    /// @brief delete_value of selected key
    bool delete_value(const std::string& value_name);

    /// @brief Get ULONG Windows registry value
    std::pair<unsigned long, bool> get_dword_value(const std::string& key_name) const;

    /// @brief Get string Windows registry value
    std::pair<std::string, bool> get_string_value(const std::string& key_name) const;

    /// @brief Get binary array Windows registry value
    std::pair<std::string, bool> get_binary_value(const std::string& key_name) const;

    /// @brief Get wide string Windows registry value
    std::pair<std::wstring, bool> get_wstring_value(const std::string& key_name) const;

    /// @brief Get multi-string (REG_MULTI_SZ) Windows registry value
    std::pair<multi_sz, bool> get_multi_string_value(const std::string& key_name) const;

    /// @brief Enumerate current registry key sub-key names
    std::vector<std::string> enumerate_subkeys() const;

    /// @brief Enumerate current registry key value names
    std::vector<std::string> enumerate_values() const;

    /// @brief Enumerate current registry key value names
    /// @return: 1st - number of sub-keys, 2nd - number of sub-values
    std::pair<unsigned long, unsigned long> count_subvalues() const;

private:

    /// https://docs.microsoft.com/en-us/windows/desktop/secauthz/privilege-constants
    /// @brief get privileges for save reg key to file
    bool enable_backup_privilege() const;

    /// https://docs.microsoft.com/en-us/windows/desktop/secauthz/privilege-constants
    /// @brief get privileges for restore reg key from file
    bool enable_restore_privilege() const;

    static constexpr int MAX_KEY_LENGTH = 1024;
    static constexpr int MAX_VALUE_NAME = 16383;

    // forward declaration is used not to include Windows.h
    std::unique_ptr<helpers::HKEY_holder> hkey_;
};

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)
