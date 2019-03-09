#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include <memory>
#include <string>
#include <vector>

// Place here WinRegistry-related functions and classes

namespace helpers {


struct HKEY_holder;

/// @brief Class wraps Windows Registry key (incomplete, could be expanded)
class RegistryKey{
public:

    /// @brief Open Windows registry key using RegOpenKeyEx() call, with all access
    /// @throw: runtime_error or system_error if unable to open the registry key
    explicit RegistryKey(const std::string& key_name, bool read_only = false);

    /// @brief Closes Windows registry key using RegCloseKey() call
    ~RegistryKey();

    /// @brief Remove the registry key recursively
    static bool delete_tree(const std::string& key);

    /// @brief Just check whether the registry key exists
    static bool is_key_exist(const std::string& key_name);

    /// @brief Just check whether the registry value in the key exists
    bool is_value_exists(const std::string& value_name) const;

    /// @brief Set DWORD Windows registry value to the key
    void set_dword_value(const std::string& key_name, unsigned long value);

    /// @brief Get ULONG Windows registry value
    /// @throw: runtime_error is value does not exists (use is_value_exists() to check)
    unsigned long get_dword_value(const std::string& key_name) const;

    /// @brief Set string Windows registry value to the key
    void set_string_value(const std::string& key_name, const std::string& value);

    /// @brief Get string Windows registry value
    /// @throw: runtime_error is value does not exists (use is_value_exists() to check)
    std::string get_string_value(const std::string& key_name) const;

    /// @brief Get wide string Windows registry value
    /// @throw: runtime_error is value does not exists (use is_value_exists() to check)
    std::wstring get_wstring_value(const std::string& key_name) const;

    /// @brief Set multi-string (REG_MULTI_SZ) Windows registry value to the key
    void set_multi_string_value(const std::string& key_name, const std::vector<std::string>& values);

    /// @brief Get multi-string (REG_MULTI_SZ) Windows registry value
    /// @throw: runtime_error is value does not exists (use is_value_exists() to check)
    std::vector<std::string> get_multi_string_value(const std::string& key_name) const;

    /// @brief Enumerate current registry key sub-key names
    std::vector<std::string> enumerate_subkeys() const;

    /// @brief Enumerate current registry key value names
    std::vector<std::string> enumerate_values() const;

    /// @brief Enumerate current registry key value names
    /// @return: 1st - number of sub-keys, 2nd - number of sub-values
    std::pair<unsigned long, unsigned long> count_subvalues() const;

private:

    std::wstring to_wstring(const std::string& str) const;

    static constexpr int MAX_KEY_LENGTH = 1024;
    static constexpr int MAX_VALUE_NAME = 16383;

    // forward declaration is used not to include Windows.h
    std::unique_ptr<helpers::HKEY_holder> hkey_;
};

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)
