#include "share.h"
#include "sharefile.h"
#include "shelluiworker.h"

#include <ShObjIdl_core.h>
#include <roapi.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/base.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

namespace {
std::optional<std::wstring> normalizeInputPath(const std::wstring &path)
{
    std::wstring nativePath = path;
    std::replace(nativePath.begin(), nativePath.end(), L'/', L'\\');

    DWORD bufferSize = MAX_PATH;
    for (;;) {
        std::vector<wchar_t> buffer(bufferSize);
        const DWORD length = GetFullPathNameW(nativePath.c_str(), bufferSize,
                                              buffer.data(), nullptr);
        if (length == 0) {
            return std::nullopt;
        }
        if (length < bufferSize) {
            return std::wstring(buffer.data(), length);
        }
        bufferSize = length + 1;
    }
}

bool isUnsupportedShareError(const HRESULT hr)
{
    return hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
           || hr == HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED)
           || hr == E_NOTIMPL || hr == E_NOINTERFACE
           || hr == REGDB_E_CLASSNOTREG;
}
}  // namespace

ParsedInput parseArguments(const std::vector<std::wstring> &arguments,
                           std::wstring *error)
{
    auto fail = [&](const wchar_t *message) {
        if (error) {
            *error = message;
        }
        return ParsedInput{};
    };

    if (arguments.size() != 3) {
        return fail(L"expected exactly --input <path>");
    }

    if (arguments[1] != L"--input") {
        return fail(L"expected --input flag");
    }

    const auto &path = arguments[2];
    if (path.empty()) {
        return fail(L"input path cannot be empty");
    }

    const auto normalizedPath = normalizeInputPath(path);
    if (!normalizedPath) {
        return fail(L"input path cannot be normalized");
    }

    const DWORD attributes = GetFileAttributesW(normalizedPath->c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return fail(L"input file does not exist or is inaccessible");
    }

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return fail(L"input must be a regular file, not a directory");
    }

    return ParsedInput{true, *normalizedPath};
}

int run(const std::vector<std::wstring> &arguments, ISharePlatform &platform,
        std::function<void()> onReady)
{
    std::wstring error;
    const auto parsed = parseArguments(arguments, &error);
    if (!parsed.valid) {
        return 2;
    }

    const HRESULT hr = platform.show(parsed.path, std::move(onReady));
    if (FAILED(hr)) {
        if (isUnsupportedShareError(hr)) {
            return 3;
        }
        return 1;
    }

    return 0;
}

int run(const std::vector<std::wstring> &arguments)
{
    WindowsSharePlatform platform;
    return run(arguments, platform);
}

HRESULT WindowsSharePlatform::show(const std::wstring &path,
                                   std::function<void()> onDataRequested)
{
    const HRESULT initHr = RoInitialize(RO_INIT_SINGLETHREADED);
    struct ApartmentGuard {
        HRESULT hr;
        ~ApartmentGuard()
        {
            if (SUCCEEDED(hr)) {
                RoUninitialize();
            }
        }
    } aptGuard{initHr};

    if (FAILED(initHr) && initHr != RPC_E_CHANGED_MODE) {
        return initHr;
    }

    const HWND hwnd
        = CreateWindowExW(WS_EX_TOOLWINDOW, L"STATIC",
                          L"SeerShareOwnerWindow", WS_POPUP, 0, 0, 1, 1,
                          nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!hwnd) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    struct WindowGuard {
        HWND w;
        ~WindowGuard()
        {
            if (w) {
                DestroyWindow(w);
            }
        }
    } wndGuard{hwnd};
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);

    winrt::com_ptr<IDataTransferManagerInterop> interop;
    try {
        interop = winrt::get_activation_factory<
            winrt::Windows::ApplicationModel::DataTransfer::DataTransferManager,
            IDataTransferManagerInterop>();
    }
    catch (const winrt::hresult_error &) {
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    if (!interop) {
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }

    winrt::Windows::ApplicationModel::DataTransfer::DataTransferManager dtm{
        nullptr};
    HRESULT hr = interop->GetForWindow(
        hwnd,
        winrt::guid_of<winrt::Windows::ApplicationModel::DataTransfer::
                           DataTransferManager>(),
        winrt::put_abi(dtm));
    if (FAILED(hr) || !dtm) {
        return hr;
    }

    winrt::Windows::Storage::StorageFile file{nullptr};
    try {
        file = loadShareFile(path);
    }
    catch (const winrt::hresult_error &err) {
        return err.code();
    }
    if (!file) {
        return E_FAIL;
    }

    std::atomic<bool> dataPopulated = false;
    auto finished = std::make_shared<std::atomic<bool>>(false);
    winrt::Windows::ApplicationModel::DataTransfer::DataPackage sharedData{nullptr};
    winrt::Windows::ApplicationModel::DataTransfer::DataPackage::ShareCompleted_revoker completed;
    winrt::Windows::ApplicationModel::DataTransfer::DataPackage::ShareCanceled_revoker cancelled;
    winrt::event_token token{};
    try {
        token = dtm.DataRequested(
            [&](const auto &, const winrt::Windows::ApplicationModel::
                                  DataTransfer::DataRequestedEventArgs &args) {
                try {
                    auto request = args.Request();
                    auto data    = request.Data();
                    const std::filesystem::path p(path);
                    data.Properties().Title(p.filename().wstring());

                    std::vector<winrt::Windows::Storage::IStorageItem> items;
                    items.push_back(file);
                    data.SetStorageItems(items);
                    sharedData = data;
                    completed = data.ShareCompleted(winrt::auto_revoke,
                        [finished](const auto &, const auto &) { *finished = true; });
                    cancelled = data.ShareCanceled(winrt::auto_revoke,
                        [finished](const auto &, const auto &) { *finished = true; });

                    dataPopulated = true;
                    if (onDataRequested) {
                        onDataRequested();
                    }
                }
                catch (...) {
                }
            });
    }
    catch (const winrt::hresult_error &err) {
        return err.code();
    }

    struct TokenGuard {
        winrt::Windows::ApplicationModel::DataTransfer::DataTransferManager
            &manager;
        winrt::event_token tok;
        ~TokenGuard()
        {
            try {
                manager.DataRequested(tok);
            }
            catch (...) {
            }
        }
    } tokenGuard{dtm, token};

    hr = interop->ShowShareUIForWindow(hwnd);
    if (FAILED(hr)) {
        return hr;
    }

    const auto startTime    = std::chrono::steady_clock::now();
    constexpr auto kTimeout = std::chrono::seconds(30);

    while (!dataPopulated) {
        if (std::chrono::steady_clock::now() - startTime > kTimeout) {
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        }

        MsgWaitForMultipleObjects(0, nullptr, FALSE, 50, QS_ALLINPUT);
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    while (!*finished) {
        if (!IsWindow(hwnd) || !ShellUiWorker::pumpMessages())
            break;
    }
    return S_OK;
}
