#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

namespace helpers {

/// @brief RAII wrapper for Windows recourse handle
class WinHandlePtr {
public:

    /// @brief Empty (invalid) handle
    WinHandlePtr() = default;

    /// @brief Wrap handle
    WinHandlePtr(HANDLE value) : value_(value == INVALID_HANDLE_VALUE ? nullptr : value) {}

    /// @brief Set handle
    void set_handle(HANDLE value) {
        close_handle();
        value_ = (value == INVALID_HANDLE_VALUE ? nullptr : value);
    }

    /// @brief Dereference handle
    HANDLE* dereference_handle() {
        return &value_;
    }

    /// @brief Close handle
    ~WinHandlePtr()
    {
        close_handle();
    }

    /// @brief Get raw handle
    HANDLE handle() { return value_; }

    explicit operator bool() const 
    {
        return value_ != nullptr; 
    }

    operator HANDLE() const 
    { 
        return value_; 
    }

    friend bool operator ==(WinHandlePtr l, WinHandlePtr r) { return l.value_ == r.value_; }
    friend bool operator !=(WinHandlePtr l, WinHandlePtr r) { return !(l == r); }

    /// @brief For proper use with smart pointers
    struct Deleter 
    {
        typedef WinHandlePtr pointer;
        void operator()(const WinHandlePtr& handle) const { ::CloseHandle(handle); }
    };
private:

    void close_handle() 
    {
        if (value_) {
            ::CloseHandle(value_);
            value_ = nullptr;
        }
    }

    HANDLE value_ = nullptr;
};

inline bool operator ==(HANDLE l, WinHandlePtr r) { return WinHandlePtr(l) == r; }
inline bool operator !=(HANDLE l, WinHandlePtr r) { return !(l == r); }
inline bool operator ==(WinHandlePtr l, HANDLE r) { return l == WinHandlePtr(r); }
inline bool operator !=(WinHandlePtr l, HANDLE r) { return !(l == r); }

} // namespace helpers

#endif // defined(_WIN32) || defined(_WIN64)
