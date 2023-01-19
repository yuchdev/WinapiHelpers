#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

namespace helpers{

/// @brief COM RAII wrapper (same as ATL has)
class ComInitializer {
public:
    
    enum ThreadingModel {
        com_single_thread,
        com_multi_thread
    };

    /// @brief Initialize single-thread or multi-threaded COM model
    explicit ComInitializer(ThreadingModel model = com_single_thread);

    /// @brief Deinitialize COM
    ~ComInitializer();
};

ComInitializer& initialize_com();

} // namespace helpers
#endif // defined(_WIN32) || defined(_WIN64)
