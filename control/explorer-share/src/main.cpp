#include <windows.h>
#include <shellapi.h>

#include "share.h"
#include "shelluiworker.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    int argc        = 0;
    const auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv)
        return 2;

    std::vector<std::wstring> arguments;
    arguments.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        arguments.emplace_back(argv[i]);
    }
    const int result = ShellUiWorker::run(arguments,
        [](const auto &args) { return parseArguments(args).valid; },
        [](const auto &args, const auto &ready) {
            WindowsSharePlatform platform;
            return ::run(args, platform, ready);
        });
    LocalFree(argv);
    return result;
}
