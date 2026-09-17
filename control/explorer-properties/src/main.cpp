#include <objbase.h>
#include <shellapi.h>
#include <windows.h>

#include <string>
#include <vector>

#include "shellverb.h"
#include "shelluiworker.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    const HRESULT comResult = CoInitializeEx(
        nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(comResult))
        return shellverb::kShellFailureExitCode;

    int argc        = 0;
    const auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        CoUninitialize();
        return shellverb::kArgumentErrorExitCode;
    }

    std::vector<std::wstring> arguments;
    arguments.reserve(static_cast<size_t>(argc));
    for (int index = 0; index < argc; ++index)
        arguments.emplace_back(argv[index]);

    const auto result = ShellUiWorker::run(arguments,
        [](const auto &args) { return shellverb::parseArguments(args).has_value(); },
        [](const auto &args, const auto &ready) {
            return shellverb::invokeProperties(shellverb::parseArguments(args)->inputPath, ready);
        });
    LocalFree(argv);
    CoUninitialize();
    return result;
}
