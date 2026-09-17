#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct JsonValue {
    enum class Kind { String, Array, Object, Number, Boolean, Null };

    Kind kind;
    std::string stringValue;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;
};

class JsonParser {
public:
    explicit JsonParser(std::string input) : m_input(std::move(input)) {}

    JsonValue parse()
    {
        const auto result = parseValue();
        skipWhitespace();
        if (m_position != m_input.size())
            throw std::runtime_error("unexpected trailing JSON input");
        return result;
    }

private:
    JsonValue parseValue()
    {
        skipWhitespace();
        if (m_position == m_input.size())
            throw std::runtime_error("unexpected end of JSON input");

        switch (m_input[m_position]) {
        case '"':
            return {JsonValue::Kind::String, parseString(), {}, {}};
        case '[':
            return parseArray();
        case '{':
            return parseObject();
        case 't':
            expectLiteral("true");
            return {JsonValue::Kind::Boolean, {}, {}, {}};
        case 'f':
            expectLiteral("false");
            return {JsonValue::Kind::Boolean, {}, {}, {}};
        case 'n':
            expectLiteral("null");
            return {JsonValue::Kind::Null, {}, {}, {}};
        default:
            return parseNumber();
        }
    }

    JsonValue parseArray()
    {
        expect('[');
        JsonValue result{JsonValue::Kind::Array, {}, {}, {}};
        skipWhitespace();
        if (consume(']'))
            return result;

        while (true) {
            result.arrayValue.push_back(parseValue());
            skipWhitespace();
            if (consume(']'))
                return result;
            expect(',');
        }
    }

    JsonValue parseObject()
    {
        expect('{');
        JsonValue result{JsonValue::Kind::Object, {}, {}, {}};
        skipWhitespace();
        if (consume('}'))
            return result;

        while (true) {
            skipWhitespace();
            if (m_position == m_input.size() || m_input[m_position] != '"')
                throw std::runtime_error("expected object key");
            const auto key = parseString();
            expect(':');
            result.objectValue.emplace(key, parseValue());
            skipWhitespace();
            if (consume('}'))
                return result;
            expect(',');
        }
    }

    JsonValue parseNumber()
    {
        const auto begin = m_position;
        while (m_position < m_input.size()
               && (std::isdigit(static_cast<unsigned char>(m_input[m_position]))
                   || m_input[m_position] == '-' || m_input[m_position] == '+'
                   || m_input[m_position] == '.' || m_input[m_position] == 'e'
                   || m_input[m_position] == 'E')) {
            ++m_position;
        }
        if (begin == m_position)
            throw std::runtime_error("invalid JSON value");
        return {JsonValue::Kind::Number, {}, {}, {}};
    }

    std::string parseString()
    {
        expect('"');
        std::string result;
        while (m_position < m_input.size()) {
            const auto character = m_input[m_position++];
            if (character == '"')
                return result;
            if (character == '\\') {
                if (m_position == m_input.size())
                    throw std::runtime_error("incomplete JSON escape");
                const auto escaped = m_input[m_position++];
                switch (escaped) {
                case '"':
                    result += '"';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case '/':
                    result += '/';
                    break;
                case 'b':
                    result += '\b';
                    break;
                case 'f':
                    result += '\f';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 't':
                    result += '\t';
                    break;
                default:
                    throw std::runtime_error("unsupported JSON escape");
                }
            }
            else {
                result += character;
            }
        }
        throw std::runtime_error("unterminated JSON string");
    }

    void expect(const char expected)
    {
        skipWhitespace();
        if (m_position == m_input.size() || m_input[m_position] != expected)
            throw std::runtime_error("unexpected JSON character");
        ++m_position;
    }

    void expectLiteral(const char *expected)
    {
        while (*expected != '\0') {
            if (m_position == m_input.size()
                || m_input[m_position] != *expected)
                throw std::runtime_error("unexpected JSON literal");
            ++m_position;
            ++expected;
        }
    }

    bool consume(const char expected)
    {
        if (m_position < m_input.size() && m_input[m_position] == expected) {
            ++m_position;
            return true;
        }
        return false;
    }

    void skipWhitespace()
    {
        while (
            m_position < m_input.size()
            && std::isspace(static_cast<unsigned char>(m_input[m_position]))) {
            ++m_position;
        }
    }

    std::string m_input;
    size_t m_position = 0;
};

int failures = 0;

void check(const bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

const JsonValue *member(const JsonValue &object, const char *name)
{
    if (object.kind != JsonValue::Kind::Object)
        return nullptr;
    const auto found = object.objectValue.find(name);
    return found == object.objectValue.end() ? nullptr : &found->second;
}

bool isExactStringArray(const JsonValue *value, const char *expected)
{
    return value && value->kind == JsonValue::Kind::Array
           && value->arrayValue.size() == 1
           && value->arrayValue.front().kind == JsonValue::Kind::String
           && value->arrayValue.front().stringValue == expected;
}

bool isWithin(const std::filesystem::path &root,
              const std::filesystem::path &path)
{
    auto rootPart = root.begin();
    auto pathPart = path.begin();
    while (rootPart != root.end()) {
        if (pathPart == path.end() || *rootPart != *pathPart)
            return false;
        ++rootPart;
        ++pathPart;
    }
    return true;
}
}  // namespace

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "usage: shellverb_manifest_test <package-root>\n";
        return 2;
    }

    try {
        const std::filesystem::path packageRoot
            = std::filesystem::weakly_canonical(argv[1]);
        const auto manifestPath
            = std::filesystem::weakly_canonical(packageRoot / "plugin.json");
        check(isWithin(packageRoot, manifestPath),
              "staged manifest remains contained by the package root");
        if (!isWithin(packageRoot, manifestPath))
            return 1;
        std::ifstream manifestFile(manifestPath, std::ios::binary);
        if (!manifestFile) {
            std::cerr << "FAIL: unable to read staged plugin.json\n";
            return 1;
        }

        const std::string manifest(
            (std::istreambuf_iterator<char>(manifestFile)), {});
        const auto root    = JsonParser(manifest).parse();
        const auto id      = member(root, "id");
        const auto backend = member(root, "backend");
        const auto command = member(root, "command");

        check(id && id->kind == JsonValue::Kind::String
                  && id->stringValue == "io.1218.seer.explorer-properties",
              "manifest selects the fixed Properties package");
        check(backend && backend->kind == JsonValue::Kind::String
                  && backend->stringValue == "process",
              "backend is process");
        check(isExactStringArray(member(root, "capabilities"), "control"),
              "capabilities contains only control");
        check(isExactStringArray(member(root, "extensions"), "${type_file}"),
              "extensions contains the file type token");
        check(command && command->kind == JsonValue::Kind::String
                  && command->stringValue == "shellverb_properties.exe",
              "command is the fixed Properties helper");

        if (command && command->kind == JsonValue::Kind::String) {
            const std::filesystem::path executableName(command->stringValue);
            check(!executableName.is_absolute()
                      && !executableName.has_parent_path(),
                  "command is package-relative");

            const auto executablePath = std::filesystem::weakly_canonical(
                packageRoot / executableName);
            check(std::filesystem::is_regular_file(executablePath),
                  "staged executable exists as a regular file");
            check(isWithin(packageRoot, executablePath),
                  "staged executable remains contained by the package root");
        }
    }
    catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }

    return failures == 0 ? 0 : 1;
}
