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

ParsedInput parseArguments(const std::vector<std::wstring>& arguments, std::wstring* error = nullptr);
HRESULT invokeOpenWith(const std::wstring& path, HWND owner);
using OpenWithInvoker = std::function<HRESULT(const std::wstring&, HWND)>;
int run(const std::vector<std::wstring>& arguments);
int run(const std::vector<std::wstring>& arguments, const OpenWithInvoker& invoker);
