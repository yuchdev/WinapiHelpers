#if defined(_WIN32) || defined(_WIN64)
#include <winapi_helpers/win_one_instance.h>
#include <Windows.h>
#include <tchar.h>

using namespace helpers;

bool NativeOneInstance::is_instance_exists(const char* instance_id) 
{
    HANDLE one_instance_mutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, instance_id);

    if (!one_instance_mutex) {
        one_instance_mutex = CreateMutexA(0, 0, instance_id);
        return false;
    }
    else {
        return true;
    }
}

#endif // defined(_WIN32) || defined(_WIN64)
