#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include "shelluiworker.h"

#include <iostream>

#include "shellverb.h"

namespace {
int failures = 0;

void check(const bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::wstring tempDirectory()
{
    wchar_t path[MAX_PATH]{};
    GetTempPathW(MAX_PATH, path);
    std::wstring directory = path;
    directory += L"Seer Shell Verb ";
    directory += std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(directory.c_str(), nullptr);
    return directory;
}

std::wstring createFile(const std::wstring &directory, const std::wstring &name)
{
    const auto path = directory + L"\\" + name;
    const auto handle
        = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
    return path;
}
}  // namespace

int main()
{
    const auto directory   = tempDirectory();
    const auto spacedPath  = createFile(directory, L"regular file.txt");
    const auto unicodePath = createFile(directory, L"Unicode \u6d4b\u8bd5.txt");
    const std::vector<std::wstring> valid{L"shellverb_properties.exe",
                                          L"--input", spacedPath};

    const auto parsed = shellverb::parseArguments(valid);
    check(parsed && parsed->inputPath == spacedPath,
          "accepts an existing file with spaces");
    const auto unicode = shellverb::parseArguments(
        {L"shellverb_properties.exe", L"--input", unicodePath});
    check(unicode && unicode->inputPath == unicodePath,
          "accepts a Unicode path");
    check(!shellverb::parseArguments({L"shellverb_properties.exe", L"--input"}),
          "rejects a missing value");
    check(!shellverb::parseArguments({L"shellverb_properties.exe", L"--input",
                                      spacedPath, L"--input", unicodePath}),
          "rejects duplicate input");
    check(!shellverb::parseArguments(
              {L"shellverb_properties.exe", L"--input", spacedPath, L"extra"}),
          "rejects extra arguments");
    check(!shellverb::parseArguments(
              {L"shellverb_properties.exe", L"--input", directory}),
          "rejects a directory");
    check(!shellverb::parseArguments({L"shellverb_properties.exe", L"--input",
                                      spacedPath + L"-missing"}),
          "rejects a missing path");

    auto forwardPath = spacedPath;
    std::replace(forwardPath.begin(), forwardPath.end(), L'\\', L'/');
    const auto normalized = shellverb::parseArguments(
        {L"shellverb_properties.exe", L"--input", forwardPath});
    check(normalized && normalized->inputPath == spacedPath,
          "normalizes host paths before invoking the shell");
    for (const auto value : {L"", L"space path", L"C:\\folder\\", L"a\"b"}) {
        const auto command = std::wstring(L"helper ") + ShellUiWorker::quote(value);
        int count = 0;
        const auto args = CommandLineToArgvW(command.c_str(), &count);
        check(args && count == 2 && std::wstring(args[1]) == value,
              "worker argument quoting round trips through Windows parsing");
        if (args)
            LocalFree(args);
    }

    int calls = 0;
    const shellverb::PropertiesInvoker succeeds
        = [&](const std::wstring &inputPath) {
              ++calls;
              check(inputPath == spacedPath,
                    "passes the accepted path to the invoker");
              return 0;
          };
    check(shellverb::run(valid, succeeds) == 0 && calls == 1,
          "returns zero for a successful invocation");

    calls                                    = 0;
    const shellverb::PropertiesInvoker fails = [&](const std::wstring &) {
        ++calls;
        return shellverb::kShellFailureExitCode;
    };
    check(shellverb::run(valid, fails) == shellverb::kShellFailureExitCode
              && calls == 1,
          "returns a distinct shell failure code");
    calls = 0;
    check(shellverb::run({L"shellverb_properties.exe", L"--input"}, succeeds)
                  == shellverb::kArgumentErrorExitCode
              && calls == 0,
          "returns a distinct argument failure code without invoking");

    DeleteFileW(spacedPath.c_str());
    DeleteFileW(unicodePath.c_str());
    RemoveDirectoryW(directory.c_str());
    return failures == 0 ? 0 : 1;
}
