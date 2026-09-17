#include "openwith.h"

#include <shlobj.h>

ParsedInput parseArguments(const std::vector<std::wstring> &arguments,
                           std::wstring *error)
{
    auto fail = [&](const wchar_t *message) {
        if (error)
            *error = message;
        return ParsedInput{};
    };

    if (arguments.size() != 3 || arguments[1] != L"--input")
        return fail(L"expected exactly --input <path>");

    auto path = arguments[2];
    if (path.empty())
        return fail(L"input path is empty");

    for (auto &ch : path) {
        if (ch == L'/') {
            ch = L'\\';
        }
    }

    const auto attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES
        || (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return fail(L"input must be an existing regular file");

    return ParsedInput{true, path};
}

HRESULT invokeOpenWith(const std::wstring &path, HWND owner)
{
    OPENASINFO info{};
    info.pcszFile    = path.c_str();
    info.pcszClass   = nullptr;
    info.oaifInFlags = OAIF_ALLOW_REGISTRATION | OAIF_EXEC;
    return SHOpenWithDialog(owner, &info);
}

int run(const std::vector<std::wstring> &arguments,
        const OpenWithInvoker &invoker)
{
    std::wstring error;
    const auto parsed = parseArguments(arguments, &error);
    if (!parsed.valid)
        return 2;

    const auto result = invoker(parsed.path, nullptr);
    return SUCCEEDED(result) ? 0 : 1;
}

int run(const std::vector<std::wstring> &arguments)
{
    return run(arguments, invokeOpenWith);
}
