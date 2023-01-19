#if defined(_WIN32) || defined(_WIN64)

#include <winapi-helpers/co_initializer.h>
#include <system_error>

using namespace helpers;

// "Meyers singleton" is thread-safe
ComInitializer& helpers::initialize_com()
{
	static ComInitializer com;
	return com;
}

ComInitializer::ComInitializer(ThreadingModel model)
{
    HRESULT hres{};

    if (ThreadingModel::com_single_thread == model) {
        hres = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
    }
    else if (ThreadingModel::com_multi_thread == model) {
        hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);
    }

    if (FAILED(hres)) {
        throw std::system_error(hres, std::system_category());
    }
}

ComInitializer::~ComInitializer()
{
    CoUninitialize();
}
#endif // defined(_WIN32) || defined(_WIN64)
