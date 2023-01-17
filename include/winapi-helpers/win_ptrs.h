#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>
// Smart pointers for Windows-specific allocators

namespace helpers {

/// @brief Smart pointer for memory allocated by LoaclAlloc()
template <typename T>
class WinLocalPtr{

public:

    /// @brief Allocate raw Windows memory using LoaclAlloc
    explicit WinLocalPtr(size_t sz) : _local_mem(reinterpret_cast<T*>(LocalAlloc(LPTR, sz)))
    {
        if (_local_mem == nullptr) {
            throw std::system_error(std::error_code(GetLastError(), std::system_category()),
                "Unable to allocate on WindowsHeap with LocalAlloc");
        }
    }

    /// @brief Take ownership under the already allocated with LocalAlloc memory
    explicit WinLocalPtr(HLOCAL mem) : _local_mem(reinterpret_cast<T*>(mem)){}

    /// This kind of smart points does not assumed to be copied
    WinLocalPtr(const WinLocalPtr&) = delete;
    WinLocalPtr& operator=(const WinLocalPtr&) = delete;

    /// @brief Release using LocalFree
    ~WinLocalPtr()
    {
        LocalFree(_local_mem);
    }

    //////////////////////////////////////////////////////////////////////////
    // Pointer operations

    /// @ Reference operation
    T* operator->() 
    {
        return _local_mem;
    };

    /// @ Reference operation, for constant objects
    const T* operator->() const 
    {
        return _local_mem;
    };

    /// @ Dereference operation
    T& operator*() 
    {
        return *_local_mem;
    };

    /// @ Dereference operation, for constant objects
    const T& operator*() const 
    {
        return *_local_mem;
    };

    /// @brief Functional version of ->
    T* get() 
    {
        return _local_mem;
    };

    /// @brief Functional version of ->, for constant objects
    const T* get() const 
    {
        return _local_mem;
    };

    /// @brief Release ownership of allocated heap memory
    T* release() 
    {
        T* ptr = _local_mem;
        _local_mem = nullptr;
        return ptr;
    };

    /// @brief Array semantic
    T& operator[](size_t index)
    {
        return *(_local_mem + sizeof(T)*index);
    }

    /// @brief Array semantic
    const T& operator[](size_t index) const 
    {
        return *(_local_mem + sizeof(T)*index);
    }

private:
    T* _local_mem;
};


/// @brief Smart pointer for memory allocated by HeapAlloc()
template <typename T>
class WinHeapPtr{

public:

    /// @brief Allocate memory for the object of known type using HeapAlloc
    WinHeapPtr() : _raw_mem(reinterpret_cast<T*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(T)))){
        if (_raw_mem == nullptr) 
        {
            throw_formatted("Unable to allocate Windows heap");
        }
    }

    /// @brief Allocate memory for the object of known size using HeapAlloc
    explicit WinHeapPtr(size_t s) : _raw_mem(reinterpret_cast<T*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, s))){
        if (_raw_mem == nullptr) 
        {
            throw std::system_error(std::error_code(GetLastError(), std::system_category()),
                "Unable to allocate Windows heap");
        }
    }

    /// @brief Take ownership under the already allocated with HeapAlloc memory
    explicit WinHeapPtr(LPVOID mem) : _raw_mem(reinterpret_cast<T*>(mem)){}

    /// This kind of smart points does not assumed to be copied
    WinHeapPtr(const WinHeapPtr&) = delete;
    WinHeapPtr& operator=(const WinHeapPtr&) = delete;

    /// @brief Release using HeapFree
    ~WinHeapPtr()
    {
        HeapFree(GetProcessHeap(), 0, reinterpret_cast<LPVOID>(_raw_mem));
    }

    //////////////////////////////////////////////////////////////////////////
    // Pointer operations

    /// @ Reference operation
    T* operator->() 
    {
        return _raw_mem;
    };

    /// @ Reference operation, for constant objects
    const T* operator->() const 
    {
        return _raw_mem;
    };

    /// @ Dereference operation
    T& operator*() 
    {
        return *_raw_mem;
    };

    /// @ Dereference operation, for constant objects
    const T& operator*() const 
    {
        return *_raw_mem;
    };

    /// @brief Functional version of ->
    T* get() 
    {
        return _raw_mem;
    };

    /// @brief Functional version of ->, for constant objects
    const T* get() const 
    {
        return _raw_mem;
    };

    /// @brief Release ownership of allocated heap memory
    T* release() 
    {
        T* ptr = _raw_mem;
        _raw_mem = nullptr;
        return ptr;
    };

    /// @brief Reset the allocated memory with the new content
    /// @param s: size of the new-allocated memory
    void reset(size_t s) 
    {
        HeapFree(GetProcessHeap(), 0, reinterpret_cast<LPVOID>(_raw_mem));
        _raw_mem = reinterpret_cast<T*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, s));
    };

    /// @brief Reset the allocated memory with the new content
    /// @param mem: pre-allocated with HeapAlloc() memory
    void reset(LPVOID mem) 
    {
        HeapFree(GetProcessHeap(), 0, reinterpret_cast<LPVOID>(_raw_mem));
        _raw_mem = reinterpret_cast<T*>(mem);
    };

    /// @brief Array semantic
    T& operator[](size_t index)
    {
        return _raw_mem[index];
    }

    /// @brief Array semantic
    const T& operator[](size_t index) const 
    {
        return _raw_mem[index];
    }

private:
    T* _raw_mem;
};

} // namespace helpers 

#endif
