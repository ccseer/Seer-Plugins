#include <objbase.h>
#include <shellapi.h>

#include "openwith.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    const HRESULT comResult = CoInitializeEx(
        nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(comResult))
        return 1;

    int argc  = 0;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        CoUninitialize();
        return 2;
    }

    std::vector<std::wstring> arguments;
    arguments.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i)
        arguments.emplace_back(argv[i]);

    const int result = run(arguments);
    LocalFree(argv);
    CoUninitialize();
    return result;
}
