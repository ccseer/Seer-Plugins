#pragma once

#include <functional>
#include <string>
#include <vector>

#include <windows.h>
#include <winerror.h>

struct ParsedInput {
    bool valid = false;
    std::wstring path;
};

struct ISharePlatform {
    virtual HRESULT show(const std::wstring& path, std::function<void()> onDataRequested = nullptr) = 0;
    virtual ~ISharePlatform() = default;
};

class WindowsSharePlatform : public ISharePlatform {
public:
    HRESULT show(const std::wstring& path, std::function<void()> onDataRequested = nullptr) override;
};

ParsedInput parseArguments(const std::vector<std::wstring>& arguments, std::wstring* error = nullptr);
int run(const std::vector<std::wstring>& arguments, ISharePlatform& platform,
        std::function<void()> onReady = {});
int run(const std::vector<std::wstring>& arguments);
