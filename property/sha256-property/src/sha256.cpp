#include "sha256.h"

#include <windows.h>
#include <bcrypt.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr DWORD kReadBufferSize = 1024 * 1024;

std::wstring win32Error(const wchar_t* operation, DWORD code = GetLastError())
{
    wchar_t* message = nullptr;
    const auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                       | FORMAT_MESSAGE_IGNORE_INSERTS;
    const auto length = FormatMessageW(flags, nullptr, code, 0,
                                       reinterpret_cast<wchar_t*>(&message), 0, nullptr);
    std::wstring result = operation;
    result += L" failed";
    if (length != 0 && message != nullptr) {
        result += L": ";
        result.append(message, length);
        LocalFree(message);
    }
    return result;
}

bool sameFileState(HANDLE file, const LARGE_INTEGER& sizeBefore, const FILETIME& timeBefore)
{
    LARGE_INTEGER sizeAfter{};
    FILETIME creation{}, access{}, write{};
    return GetFileSizeEx(file, &sizeAfter) && GetFileTime(file, &creation, &access, &write)
           && sizeAfter.QuadPart == sizeBefore.QuadPart
           && CompareFileTime(&write, &timeBefore) == 0;
}

bool writeAll(HANDLE file, const char* data, DWORD length)
{
    while (length != 0) {
        DWORD written = 0;
        if (!WriteFile(file, data, length, &written, nullptr) || written == 0)
            return false;
        data += written;
        length -= written;
    }
    return true;
}

std::wstring outputPathFor(const std::wstring& base)
{
    constexpr wchar_t suffix[] = L".json";
    if (base.size() >= 5 && _wcsicmp(base.c_str() + base.size() - 5, suffix) == 0)
        return base;
    return base + suffix;
}

}

bool parseArguments(const std::vector<std::wstring>& arguments, std::wstring& input,
                    std::wstring& output, OutputCase& outputCase)
{
    outputCase = OutputCase::Lower;
    if (arguments.size() < 5 || arguments.size() % 2 == 0)
        return false;
    bool inputSeen = false;
    bool outputSeen = false;
    bool caseSeen = false;
    for (size_t index = 1; index + 1 < arguments.size(); index += 2) {
        const auto& option = arguments[index];
        const auto& value = arguments[index + 1];
        if (value.empty())
            return false;
        if (option == L"--input" && !inputSeen) {
            input = value;
            inputSeen = true;
        } else if (option == L"--output" && !outputSeen) {
            output = value;
            outputSeen = true;
        } else if (option == L"--case" && !caseSeen) {
            if (value == L"lower") {
                outputCase = OutputCase::Lower;
            } else if (value == L"upper") {
                outputCase = OutputCase::Upper;
            } else {
                return false;
            }
            caseSeen = true;
        } else {
            return false;
        }
    }
    return inputSeen && outputSeen;
}

Sha256Result hashFile(const std::wstring& inputPath)
{
    Sha256Result result;
    const auto file = CreateFileW(inputPath.c_str(), GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        result.error = win32Error(L"CreateFileW");
        return result;
    }

    LARGE_INTEGER sizeBefore{};
    FILETIME creation{}, access{}, timeBefore{};
    if (!GetFileSizeEx(file, &sizeBefore) || !GetFileTime(file, &creation, &access, &timeBefore)) {
        result.error = win32Error(L"Read file metadata");
        CloseHandle(file);
        return result;
    }

    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objectLength = 0;
    DWORD bytesReturned = 0;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (status == 0)
        status = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
                                   reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength),
                                   &bytesReturned, 0);
    std::vector<UCHAR> object(objectLength);
    if (status == 0)
        status = BCryptCreateHash(algorithm, &hash, object.data(), objectLength, nullptr, 0, 0);
    if (status != 0) {
        result.error = L"Windows CNG initialization failed";
        if (hash != nullptr)
            BCryptDestroyHash(hash);
        if (algorithm != nullptr)
            BCryptCloseAlgorithmProvider(algorithm, 0);
        CloseHandle(file);
        return result;
    }

    std::vector<char> buffer(kReadBufferSize);
    bool readOk = true;
    for (;;) {
        DWORD read = 0;
        if (!ReadFile(file, buffer.data(), kReadBufferSize, &read, nullptr)) {
            readOk = false;
            result.error = win32Error(L"ReadFile");
            break;
        }
        if (read == 0)
            break;
        status = BCryptHashData(hash, reinterpret_cast<PUCHAR>(buffer.data()), read, 0);
        if (status != 0) {
            readOk = false;
            result.error = L"Windows CNG hashing failed";
            break;
        }
        result.bytes += read;
    }

    if (readOk && !sameFileState(file, sizeBefore, timeBefore)) {
        readOk = false;
        result.error = L"Input file changed while hashing";
    }
    if (readOk) {
        status = BCryptFinishHash(hash, result.digest.data(),
                                  static_cast<ULONG>(result.digest.size()), 0);
        if (status != 0) {
            readOk = false;
            result.error = L"Windows CNG finalization failed";
        }
    }

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    CloseHandle(file);
    result.ok = readOk;
    if (!result.ok)
        result.digest.fill(0);
    return result;
}

int run(const std::vector<std::wstring>& arguments)
{
    std::wstring input;
    std::wstring outputBase;
    OutputCase outputCase = OutputCase::Lower;
    if (!parseArguments(arguments, input, outputBase, outputCase))
        return 2;

    const auto result = hashFile(input);
    if (!result.ok) {
        std::wcerr << result.error << L'\n';
        return 3;
    }

    const char* digits = (outputCase == OutputCase::Upper) ? "0123456789ABCDEF"
                                                           : "0123456789abcdef";
    std::string digest;
    digest.reserve(result.digest.size() * 2);
    for (const auto byte : result.digest) {
        digest.push_back(digits[byte >> 4]);
        digest.push_back(digits[byte & 0x0f]);
    }
    const std::string json = std::string("{\"SHA-256\":\"") + digest + "\"}\n";
    const auto outputPath = outputPathFor(outputBase);
    const auto parent = std::filesystem::path(outputPath).parent_path();
    const auto directory = parent.empty() ? std::filesystem::path(L".") : parent;
    wchar_t temporary[MAX_PATH]{};
    if (GetTempFileNameW(directory.c_str(), L"sha", 0, temporary) == 0) {
        std::wcerr << L"failed to create temporary output file\n";
        return 4;
    }
    const auto temporaryPath = std::wstring(temporary);
    const auto file = CreateFileW(temporaryPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        std::wcerr << win32Error(L"CreateFileW for output") << L'\n';
        DeleteFileW(temporaryPath.c_str());
        return 4;
    }
    const bool written = writeAll(file, json.data(), static_cast<DWORD>(json.size()))
                         && FlushFileBuffers(file);
    CloseHandle(file);
    if (!written || !MoveFileExW(temporaryPath.c_str(), outputPath.c_str(),
                                 MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        std::wcerr << win32Error(L"Publish output") << L'\n';
        DeleteFileW(temporaryPath.c_str());
        return 4;
    }
    return 0;
}
