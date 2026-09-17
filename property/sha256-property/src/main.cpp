#include "sha256.h"

#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

int wmain()
{
    int argc = 0;
    const auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr)
        return 2;

    std::vector<std::wstring> arguments;
    arguments.reserve(static_cast<size_t>(argc));
    for (int index = 0; index < argc; ++index)
        arguments.emplace_back(argv[index]);

    const auto result = run(arguments);
    LocalFree(argv);
    return result;
}
