#include "sha256.h"

#include <windows.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

namespace {
int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path tempDirectory()
{
    wchar_t buffer[MAX_PATH]{};
    GetTempPathW(MAX_PATH, buffer);
    const auto path = std::filesystem::path(buffer) / L"Seer Sha256 测试";
    std::filesystem::create_directories(path);
    return path;
}

std::filesystem::path writeFile(const std::filesystem::path& path, const std::string& data)
{
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(data.data(), static_cast<std::streamsize>(data.size()));
    return path;
}

std::string digestHex(const Sha256Result& result)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string value;
    value.reserve(result.digest.size() * 2);
    for (const auto byte : result.digest) {
        value.push_back(digits[byte >> 4]);
        value.push_back(digits[byte & 0x0f]);
    }
    return value;
}

std::string readText(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
}

int main()
{
    const auto directory = tempDirectory();
    const auto empty = writeFile(directory / L"empty.bin", "");
    const auto abc = writeFile(directory / L"abc.txt", "abc");
    const auto binary = writeFile(directory / L"binary.bin", std::string("\0\x01\x7f\xff", 4));
    const auto unicode = writeFile(directory / L"路径 file.txt", "unicode");

    const auto emptyResult = hashFile(empty.wstring());
    check(emptyResult.ok && emptyResult.bytes == 0, "empty file hashes");
    check(digestHex(emptyResult)
              == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "empty SHA-256 vector");

    const auto abcResult = hashFile(abc.wstring());
    check(abcResult.ok && abcResult.bytes == 3, "abc file hashes");
    check(digestHex(abcResult)
              == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
          "abc SHA-256 vector");

    const auto binaryResult = hashFile(binary.wstring());
    check(binaryResult.ok && binaryResult.bytes == 4, "binary fixture hashes");
    check(digestHex(binaryResult)
              == "9beb9b4fbb3161c1c60d01c253b504f0dd2ea909f764fd3d7c8213fa1580ae94",
          "binary SHA-256 vector");

    const auto unicodeResult = hashFile(unicode.wstring());
    check(unicodeResult.ok && unicodeResult.bytes == 7, "Unicode path hashes");

    const auto large = directory / L"larger-than-buffer.bin";
    {
        std::ofstream output(large, std::ios::binary | std::ios::trunc);
        const std::string block(1024 * 1024, 'x');
        for (int index = 0; index < 3; ++index)
            output.write(block.data(), static_cast<std::streamsize>(block.size()));
    }
    const auto largeResult = hashFile(large.wstring());
    check(largeResult.ok && largeResult.bytes == 3 * 1024 * 1024,
          "file larger than read buffer hashes");

    const auto missingResult = hashFile((directory / L"missing.bin").wstring());
    check(!missingResult.ok && !missingResult.error.empty(), "input-open failure is reported");

    const auto deleteRace = directory / L"delete-race.bin";
    {
        std::ofstream output(deleteRace, std::ios::binary | std::ios::trunc);
        const std::string block(1024 * 1024, 'd');
        for (int index = 0; index < 256; ++index)
            output.write(block.data(), static_cast<std::streamsize>(block.size()));
    }
    std::atomic<bool> deleteRaceFinished = false;
    Sha256Result deleteRaceResult;
    std::thread deleteRaceHasher([&] {
        deleteRaceResult = hashFile(deleteRace.wstring());
        deleteRaceFinished = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const bool deletionBlocked = !deleteRaceFinished && DeleteFileW(deleteRace.c_str()) == FALSE;
    deleteRaceHasher.join();
    check(deletionBlocked && deleteRaceResult.ok,
          "hashFile blocks deletion while reading its input");

    const auto outputBase = directory / L"result";
    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output", outputBase.wstring()})
              == 0,
          "process succeeds for valid input");
    const auto outputPath = outputBase.wstring() + L".json";
    const auto outputText = readText(outputPath);
    check(outputText == "{\"SHA-256\":\"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\"}\n",
          "output JSON has the required flat shape");

    const auto lowerBase = directory / L"result-lower";
    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output", lowerBase.wstring(),
               L"--case", L"lower"})
              == 0,
          "process succeeds with explicit lower case");
    const auto lowerText = readText(lowerBase.wstring() + L".json");
    check(lowerText == outputText, "explicit lower matches default lower output");

    const auto upperBase = directory / L"result-upper";
    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output", upperBase.wstring(),
               L"--case", L"upper"})
              == 0,
          "process succeeds with explicit upper case");
    const auto upperText = readText(upperBase.wstring() + L".json");
    check(upperText
              == "{\"SHA-256\":\"BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD\"}\n",
          "upper output produces exact ASCII uppercase digest");

    const auto upperOrderBase = directory / L"result-upper-order";
    check(run({L"sha256_property.exe", L"--case", L"upper", L"--input", abc.wstring(),
               L"--output", upperOrderBase.wstring()})
              == 0,
          "process succeeds with case option placed first");
    check(readText(upperOrderBase.wstring() + L".json") == upperText,
          "case option ordering independence produces matching output");

    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output", outputBase.wstring(),
               L"--case", L"invalid"})
              == 2,
          "run returns exit code 2 on unknown case value");
    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output", outputBase.wstring(),
               L"--case", L"lower", L"--case", L"upper"})
              == 2,
          "run returns exit code 2 on duplicate case option");

    const auto suffixedBase = directory / L"result-suffixed.json";
    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output",
               suffixedBase.wstring()})
              == 0,
          "process accepts an output base ending in .json");
    check(std::filesystem::exists(suffixedBase)
              && !std::filesystem::exists(suffixedBase.wstring() + L".json"),
          "output suffix is appended exactly once");
    check(run({L"sha256_property.exe", L"--input", abc.wstring(), L"--output",
               (directory / L"missing-dir" / L"result").wstring()})
              != 0,
          "output-create failure is nonzero");
    check(run({L"sha256_property.exe", L"--input", (directory / L"missing.bin").wstring(),
               L"--output", (directory / L"missing-result").wstring()})
              != 0,
          "input-open process failure is nonzero");

    std::wstring parsedInput;
    std::wstring parsedOutput;
    OutputCase parsedCase = OutputCase::Upper;
    check(parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json"},
                         parsedInput, parsedOutput, parsedCase)
              && parsedInput == L"in.bin" && parsedOutput == L"out.json"
              && parsedCase == OutputCase::Lower,
          "parseArguments default case is Lower");

    parsedCase = OutputCase::Upper;
    check(parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                          L"--case", L"lower"},
                         parsedInput, parsedOutput, parsedCase)
              && parsedCase == OutputCase::Lower,
          "parseArguments explicit lower");

    parsedCase = OutputCase::Lower;
    check(parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                          L"--case", L"upper"},
                         parsedInput, parsedOutput, parsedCase)
              && parsedCase == OutputCase::Upper,
          "parseArguments explicit upper");

    check(parseArguments({L"sha256_property.exe", L"--case", L"upper", L"--input", L"in.bin",
                          L"--output", L"out.json"},
                         parsedInput, parsedOutput, parsedCase)
              && parsedCase == OutputCase::Upper,
          "parseArguments case before required options");

    check(parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--case", L"upper",
                          L"--output", L"out.json"},
                         parsedInput, parsedOutput, parsedCase)
              && parsedCase == OutputCase::Upper,
          "parseArguments case between required options");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                           L"--case", L"lower", L"--case", L"upper"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects duplicate --case");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in1.bin", L"--input", L"in2.bin",
                           L"--output", L"out.json"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects duplicate --input");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out1.json",
                           L"--output", L"out2.json"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects duplicate --output");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                           L"--case", L"UPPER"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects case value with uppercase option string");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                           L"--case", L"unknown"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects unknown case value");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                           L"--case"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects missing option value");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                           L"--case", L""},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects empty case value");

    check(!parseArguments({L"sha256_property.exe", L"--input", L"in.bin", L"--output", L"out.json",
                           L"--unknown", L"val"},
                          parsedInput, parsedOutput, parsedCase),
          "parseArguments rejects unknown option");

    const auto changed = directory / L"changed.bin";
    {
        std::ofstream output(changed, std::ios::binary | std::ios::trunc);
        const std::string block(1024 * 1024, 'y');
        for (int index = 0; index < 256; ++index)
            output.write(block.data(), static_cast<std::streamsize>(block.size()));
    }
    std::thread modifier([&changed] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::ofstream output(changed, std::ios::binary | std::ios::app);
        for (int index = 0; index < 32 && output; ++index) {
            output.put('z');
            output.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    const auto changedResult = hashFile(changed.wstring());
    modifier.join();
    check(!changedResult.ok, "changed input is rejected");

    return failures == 0 ? 0 : 1;
}
