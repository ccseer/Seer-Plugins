#pragma once

#include <windows.h>
#include <chrono>
#include <string>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>

inline winrt::Windows::Storage::StorageFile loadShareFile(const std::wstring &path)
{
    const auto operation = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(path);
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (operation.Status() == winrt::Windows::Foundation::AsyncStatus::Started) {
        if (std::chrono::steady_clock::now() >= deadline) {
            operation.Cancel();
            winrt::throw_hresult(HRESULT_FROM_WIN32(ERROR_TIMEOUT));
        }
        if (MsgWaitForMultipleObjects(0, nullptr, FALSE, 50, QS_ALLINPUT) == WAIT_FAILED) {
            const auto error = HRESULT_FROM_WIN32(GetLastError());
            operation.Cancel();
            winrt::throw_hresult(error);
        }
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                operation.Cancel();
                PostQuitMessage(static_cast<int>(message.wParam));
                winrt::throw_hresult(E_ABORT);
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    return operation.GetResults();
}
