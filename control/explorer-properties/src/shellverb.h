#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace shellverb {

struct ParsedInput {
    std::wstring inputPath;
};

inline constexpr int kArgumentErrorExitCode = 2;
inline constexpr int kShellFailureExitCode = 1;

std::optional<ParsedInput> parseArguments(const std::vector<std::wstring>& argv,
                                          std::wstring* error = nullptr);
int invokeProperties(const std::wstring& inputPath,
                     const std::function<void()> &onReady = {});
using PropertiesInvoker = std::function<int(const std::wstring&)>;
int run(const std::vector<std::wstring>& argv);
int run(const std::vector<std::wstring>& argv, const PropertiesInvoker& invoker);

}
