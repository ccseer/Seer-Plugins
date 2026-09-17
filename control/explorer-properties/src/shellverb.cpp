#include "shellverb.h"
#include "shelluiworker.h"
#include <algorithm>

#include <windows.h>
#include <shellapi.h>

namespace shellverb {

std::optional<ParsedInput> parseArguments(const std::vector<std::wstring> &argv,
                                          std::wstring *error)
{
    const auto fail
        = [error](const wchar_t *message) -> std::optional<ParsedInput> {
        if (error)
            *error = message;
        return std::nullopt;
    };

    if (argv.size() != 3 || argv[1] != L"--input")
        return fail(L"expected exactly --input <path>");

    auto inputPath = argv[2];
    std::replace(inputPath.begin(), inputPath.end(), L'/', L'\\');
    if (inputPath.empty())
        return fail(L"input path is empty");

    const auto attributes = GetFileAttributesW(inputPath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES
        || (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return fail(L"input must be an existing regular file");

    return ParsedInput{inputPath};
}

int invokeProperties(const std::wstring &inputPath,
                     const std::function<void()> &onReady)
{
    SHELLEXECUTEINFOW info{};
    info.cbSize = sizeof(info);
    info.fMask
        = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_INVOKEIDLIST | SEE_MASK_NOASYNC;
    info.lpVerb = L"properties";
    info.lpFile = inputPath.c_str();
    info.nShow  = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&info))
        return kShellFailureExitCode;

    if (info.hProcess)
        CloseHandle(info.hProcess);
    const auto hasWindow = [] {
        bool found = false;
        EnumWindows([](HWND hwnd, LPARAM data) -> BOOL {
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
            if (pid == GetCurrentProcessId() && IsWindowVisible(hwnd)) {
                *reinterpret_cast<bool *>(data) = true;
                return FALSE;
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&found));
        return found;
    };
    const auto deadline = GetTickCount64() + 3000;
    while (!hasWindow()) {
        if (GetTickCount64() >= deadline || !ShellUiWorker::pumpMessages())
            return kShellFailureExitCode;
    }
    if (onReady)
        onReady();
    while (hasWindow()) {
        if (!ShellUiWorker::pumpMessages())
            break;
    }
    return 0;
}

int run(const std::vector<std::wstring> &argv, const PropertiesInvoker &invoker)
{
    const auto parsed = parseArguments(argv);
    if (!parsed)
        return kArgumentErrorExitCode;

    return invoker(parsed->inputPath);
}

int run(const std::vector<std::wstring> &argv)
{
    return run(argv, [](const std::wstring &path) { return invokeProperties(path); });
}

}  // namespace shellverb
