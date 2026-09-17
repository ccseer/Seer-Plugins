#include "openwith.h"

#include <windows.h>

#include <iostream>

namespace {
int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::wstring tempFile()
{
    wchar_t temp[MAX_PATH]{};
    GetTempPathW(MAX_PATH, temp);
    std::wstring path = std::wstring(temp) + L"Seer Open With 测试 file.txt";
    HANDLE handle = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);
    return path;
}
}

int main()
{
    const auto path = tempFile();
    const std::vector<std::wstring> valid{L"shellopenwith.exe", L"--input", path};
    auto parsed = parseArguments(valid);
    check(parsed.valid && parsed.path == path, "valid regular file");
    check(!parseArguments({L"shellopenwith.exe", L"--input"}).valid, "missing value");
    check(!parseArguments({L"shellopenwith.exe", L"--input", path, L"--input", path}).valid, "duplicate input");
    check(!parseArguments({L"shellopenwith.exe", L"--input", path, L"extra"}).valid, "extra arguments");
    check(!parseArguments({L"shellopenwith.exe", L"--input", std::wstring(path).append(L"-missing")}).valid, "missing path");

    wchar_t temp[MAX_PATH]{};
    GetTempPathW(MAX_PATH, temp);
    check(!parseArguments({L"shellopenwith.exe", L"--input", temp}).valid, "directory");

    int calls = 0;
    OpenWithInvoker success = [&](const std::wstring& received, HWND owner) {
        ++calls;
        check(received == path && owner == nullptr, "invoker arguments");
        return S_OK;
    };
    check(run(valid, success) == 0 && calls == 1, "successful invoker returns zero");

    calls = 0;
    OpenWithInvoker failure = [&](const std::wstring&, HWND) {
        ++calls;
        return E_FAIL;
    };
    check(run(valid, failure) != 0 && calls == 1, "failed HRESULT returns nonzero");
    calls = 0;
    check(run({L"shellopenwith.exe", L"--input"}, success) != 0 && calls == 0,
          "invalid input never invokes");

    calls = 0;
    check(run({L"shellopenwith.exe", L"--input", temp}, success) != 0 && calls == 0,
          "directory input never invokes");

    const auto missingPath = path + L"-missing";
    calls = 0;
    check(run({L"shellopenwith.exe", L"--input", missingPath}, success) != 0 && calls == 0,
          "missing file input never invokes");

    DeleteFileW(path.c_str());
    return failures == 0 ? 0 : 1;
}
