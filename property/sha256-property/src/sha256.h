#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct Sha256Result {
    bool ok = false;
    std::array<std::uint8_t, 32> digest{};
    std::uint64_t bytes = 0;
    std::wstring error;
};

enum class OutputCase {
    Lower,
    Upper
};

Sha256Result hashFile(const std::wstring& inputPath);
bool parseArguments(const std::vector<std::wstring>& arguments, std::wstring& input,
                    std::wstring& output, OutputCase& outputCase);
int run(const std::vector<std::wstring>& arguments);
