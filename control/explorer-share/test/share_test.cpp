#include "share.h"
#include "sharefile.h"
#include <crtdbg.h>
#include <cstdlib>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
int failures = 0;

void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class FakeSharePlatform : public ISharePlatform {
public:
    HRESULT resultToReturn = S_OK;
    int callCount          = 0;
    std::wstring lastPath;
    bool triggerCallback = false;

    HRESULT show(const std::wstring &path,
                 std::function<void()> onDataRequested) override
    {
        ++callCount;
        lastPath = path;
        if (triggerCallback && onDataRequested) {
            onDataRequested();
        }
        return resultToReturn;
    }
};

class FakeWinRtPlatform : public ISharePlatform {
public:
    bool handlerAttached  = false;
    bool handlerRemoved   = false;
    bool simulateTimeout  = false;
    HRESULT showResult    = S_OK;
    int storageItemsCount = 0;
    std::wstring titleSet;
    bool callbackCompleted = false;

    HRESULT show(const std::wstring &path,
                 std::function<void()> onDataRequested) override
    {
        handlerAttached = true;
        struct TokenGuard {
            bool &removed;
            ~TokenGuard()
            {
                removed = true;
            }
        } guard{handlerRemoved};

        if (FAILED(showResult)) {
            return showResult;
        }

        if (simulateTimeout) {
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        }

        // Simulate DataRequested event
        storageItemsCount = 1;  // exactly one StorageFile
        titleSet          = std::filesystem::path(path).filename().wstring();
        callbackCompleted = true;
        if (onDataRequested) {
            onDataRequested();
        }

        return S_OK;
    }
};

std::filesystem::path tempDirectory()
{
    wchar_t buffer[MAX_PATH]{};
    GetTempPathW(MAX_PATH, buffer);
    const auto path = std::filesystem::path(buffer) / L"Seer Share 测试 目录";
    std::filesystem::create_directories(path);
    return path;
}

std::filesystem::path createFile(const std::filesystem::path &path)
{
    std::ofstream out(path, std::ios::binary);
    out << "hello share";
    return path;
}
}  // namespace

int main()
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#ifdef _DEBUG
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL,
        [](int type, wchar_t *, int *result) {
            if (type != _CRT_ASSERT)
                return FALSE;
            ++failures;
            *result = 0;
            return TRUE;
        });
#endif
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    const auto tempDir     = tempDirectory();
    const auto validFile   = createFile(tempDir / L"sample.txt");
    const auto unicodeFile = createFile(tempDir / L"测试 文件 with spaces.txt");
    const auto missingFile = tempDir / L"missing.txt";

    {
        const auto file = loadShareFile(unicodeFile.wstring());
        check(std::wstring(file.Path()) == unicodeFile.wstring(),
              "real WinRT file loads on STA without a blocking get");
        bool rejected = false;
        try {
            loadShareFile(missingFile.wstring());
        }
        catch (const winrt::hresult_error &) {
            rejected = true;
        }
        check(rejected, "real WinRT file loading reports missing files");
    }

    // 1. Argument parsing tests
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", validFile.wstring()}, &error);
        check(parsed.valid && parsed.path == validFile.wstring(),
              "valid arguments parse successfully");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", unicodeFile.wstring()}, &error);
        check(parsed.valid && parsed.path == unicodeFile.wstring(),
              "Unicode and spaces path parses successfully");
    }
    {
        auto forwardSlashPath = validFile.wstring();
        std::replace(forwardSlashPath.begin(), forwardSlashPath.end(), L'\\',
                     L'/');
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", forwardSlashPath}, &error);
        check(parsed.valid && parsed.path == validFile.wstring(),
              "forward-slash path is normalized for WinRT");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments({L"seer_share.exe"}, &error);
        check(!parsed.valid && !error.empty(), "missing arguments rejected");
    }
    {
        std::wstring error;
        const auto parsed
            = parseArguments({L"seer_share.exe", L"--input"}, &error);
        check(!parsed.valid && !error.empty(), "missing input value rejected");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", validFile.wstring(), L"--extra"},
            &error);
        check(!parsed.valid && !error.empty(), "extra arguments rejected");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", validFile.wstring(), L"--input",
             validFile.wstring()},
            &error);
        check(!parsed.valid && !error.empty(), "duplicate arguments rejected");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--other", validFile.wstring()}, &error);
        check(!parsed.valid && !error.empty(), "unrecognized flag rejected");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", missingFile.wstring()}, &error);
        check(!parsed.valid && !error.empty(),
              "non-existent file path rejected");
    }
    {
        std::wstring error;
        const auto parsed = parseArguments(
            {L"seer_share.exe", L"--input", tempDir.wstring()}, &error);
        check(!parsed.valid && !error.empty(), "directory path rejected");
    }

    {
        FakeWinRtPlatform platform;
        bool ready = false;
        check(run({L"seer_share.exe", L"--input", validFile.wstring()}, platform,
                  [&] { ready = true; }) == 0 && ready,
              "launch readiness is reported when share data is populated");
        ready = false;
        platform.simulateTimeout = true;
        check(run({L"seer_share.exe", L"--input", validFile.wstring()}, platform,
                  [&] { ready = true; }) != 0 && !ready,
              "failed share launch never reports readiness");
    }

    // 2. WinRT Platform simulation tests
    {
        FakeWinRtPlatform platform;
        const int rc = run({L"seer_share.exe", L"--input", validFile.wstring()},
                           platform);
        check(rc == 0, "run returns 0 on successful DataRequested share flow");
        check(platform.handlerAttached, "handler was attached during show");
        check(platform.handlerRemoved, "handler was removed on success exit");
        check(platform.storageItemsCount == 1,
              "DataRequested contained exactly one StorageFile");
        check(platform.titleSet == L"sample.txt",
              "DataPackage title matches file name");
        check(platform.callbackCompleted, "callback was completed");
    }
    {
        FakeWinRtPlatform platform;
        platform.simulateTimeout = true;
        const int rc = run({L"seer_share.exe", L"--input", validFile.wstring()},
                           platform);
        check(rc != 0, "run returns nonzero on 30-second timeout");
        check(platform.handlerAttached, "handler was attached before timeout");
        check(platform.handlerRemoved,
              "handler was removed on timeout exit path");
    }
    {
        FakeWinRtPlatform platform;
        platform.showResult = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        const int rc = run({L"seer_share.exe", L"--input", validFile.wstring()},
                           platform);
        check(rc == 3, "unsupported platform returns distinct exit code 3");
        check(platform.handlerRemoved,
              "handler was removed on unsupported platform exit path");
    }
    {
        FakeWinRtPlatform platform;
        platform.showResult = E_NOINTERFACE;
        const int rc = run({L"seer_share.exe", L"--input", validFile.wstring()},
                           platform);
        check(rc == 3,
              "missing Share interop returns distinct unsupported code");
        check(platform.handlerRemoved,
              "handler was removed when Share interop is unavailable");
    }
    {
        FakeWinRtPlatform platform;
        platform.showResult = E_FAIL;
        const int rc = run({L"seer_share.exe", L"--input", validFile.wstring()},
                           platform);
        check(rc == 1, "generic failure returns 1");
        check(platform.handlerRemoved,
              "handler was removed on generic failure exit path");
    }

    std::filesystem::remove_all(tempDir);
    winrt::uninit_apartment();
    return failures == 0 ? 0 : 1;
}
