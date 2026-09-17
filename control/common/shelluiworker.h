#pragma once

#include <windows.h>
#include <objbase.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ShellUiWorker {
struct CloseHandleDeleter {
    void operator()(HANDLE handle) const { CloseHandle(handle); }
};
using Handle = std::unique_ptr<void, CloseHandleDeleter>;

inline std::wstring quote(const std::wstring &value)
{
    std::wstring result = L"\"";
    size_t slashes = 0;
    for (const auto ch : value) {
        if (ch == L'\\') {
            ++slashes;
            continue;
        }
        result.append(ch == L'"' ? slashes * 2 + 1 : slashes, L'\\');
        result += ch;
        slashes = 0;
    }
    result.append(slashes * 2, L'\\');
    return result + L'"';
}

inline bool pumpMessages(DWORD waitMs = 25)
{
    MsgWaitForMultipleObjects(0, nullptr, FALSE, waitMs, QS_ALLINPUT);
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT)
            return false;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return true;
}

template<typename Validate, typename Show>
int run(std::vector<std::wstring> arguments, Validate validate, Show show)
{
    constexpr auto workerFlag = L"--ui-ready-event";
    if (arguments.size() == 5 && arguments[3] == workerFlag) {
        Handle ready(OpenEventW(EVENT_MODIFY_STATE, FALSE, arguments[4].c_str()));
        arguments.resize(3);
        if (!ready || !validate(arguments))
            return 2;
        return show(arguments, [&] { SetEvent(ready.get()); });
    }
    if (!validate(arguments))
        return 2;

    GUID guid{};
    wchar_t guidText[40]{};
    if (FAILED(CoCreateGuid(&guid)) || !StringFromGUID2(guid, guidText, 40))
        return 1;
    const auto eventName = std::wstring(L"Local\\SeerPluginUi-") + guidText;
    Handle ready(CreateEventW(nullptr, TRUE, FALSE, eventName.c_str()));
    if (!ready)
        return 1;

    std::vector<wchar_t> executable(32768);
    const auto length = GetModuleFileNameW(nullptr, executable.data(),
                                           static_cast<DWORD>(executable.size()));
    if (!length || length >= executable.size())
        return 1;
    auto command = quote(executable.data());
    for (size_t i = 1; i < arguments.size(); ++i)
        command += L" " + quote(arguments[i]);
    command += L" " + std::wstring(workerFlag) + L" " + quote(eventName);
    STARTUPINFOW startup{sizeof(startup)};
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.data(), command.data(), nullptr, nullptr, FALSE,
                         DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | CREATE_SUSPENDED,
                         nullptr, nullptr, &startup, &process))
        return 1;
    Handle child(process.hProcess);
    Handle thread(process.hThread);
    AllowSetForegroundWindow(process.dwProcessId);
    if (ResumeThread(thread.get()) == static_cast<DWORD>(-1)) {
        TerminateProcess(child.get(), 1);
        return 1;
    }
    // The host timeout covers launch, not the user's time in a system dialog.
    HANDLE waits[]{ready.get(), child.get()};
    const auto result = WaitForMultipleObjects(2, waits, FALSE, 4000);
    if (result == WAIT_OBJECT_0)
        return 0;
    if (result == WAIT_OBJECT_0 + 1) {
        DWORD code = 1;
        GetExitCodeProcess(child.get(), &code);
        return code == 0 ? 1 : static_cast<int>(code);
    }
    TerminateProcess(child.get(), 1);
    WaitForSingleObject(child.get(), 1000);
    return 1;
}
} // namespace ShellUiWorker
