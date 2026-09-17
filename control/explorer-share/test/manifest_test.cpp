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
        return {JsonValue::Kind::Number,
                m_input.substr(begin, m_position - begin),
                {},
                {}};
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

    void expect(char expected)
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

    bool consume(char expected)
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

void check(bool condition, const char *message)
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

bool exactStringArray(const JsonValue *value,
                      const std::vector<std::string> &expected)
{
    if (!value || value->kind != JsonValue::Kind::Array
        || value->arrayValue.size() != expected.size())
        return false;
    for (size_t index = 0; index < expected.size(); ++index) {
        if (value->arrayValue[index].kind != JsonValue::Kind::String
            || value->arrayValue[index].stringValue != expected[index])
            return false;
    }
    return true;
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
    std::filesystem::path stagingRoot;
    if (argc >= 2) {
        stagingRoot = std::filesystem::weakly_canonical(argv[1]);
    }
    else {
        if (std::filesystem::exists("plugin.json")) {
            stagingRoot = std::filesystem::weakly_canonical(".");
        }
        else if (std::filesystem::exists("manifest-stage/plugin.json")) {
            stagingRoot = std::filesystem::weakly_canonical("manifest-stage");
        }
        else {
            std::cerr << "usage: share_manifest_test <staging-root>\n";
            return 2;
        }
    }

    try {
        const auto manifestPath = stagingRoot / "plugin.json";
        std::ifstream manifestFile(manifestPath, std::ios::binary);
        if (!manifestFile) {
            std::cerr << "FAIL: unable to read staged plugin.json at "
                      << manifestPath.string() << '\n';
            return 1;
        }

        const std::string manifest(
            (std::istreambuf_iterator<char>(manifestFile)), {});
        const auto root = JsonParser(manifest).parse();

        const auto schemaVersion = member(root, "schema_version");
        const auto id            = member(root, "id");
        const auto name          = member(root, "name");
        const auto backend       = member(root, "backend");
        const auto command       = member(root, "command");

        check(schemaVersion && schemaVersion->kind == JsonValue::Kind::Number
                  && schemaVersion->stringValue == "1",
              "schema_version is 1");
        check(id && id->kind == JsonValue::Kind::String
                  && id->stringValue == "io.1218.seer.explorer-share",
              "manifest selects io.1218.seer.explorer-share");
        check(name && name->kind == JsonValue::Kind::String
                  && name->stringValue == "Share",
              "manifest name is Share");
        check(backend && backend->kind == JsonValue::Kind::String
                  && backend->stringValue == "process",
              "backend is process");
        check(isExactStringArray(member(root, "capabilities"), "control"),
              "capabilities contains only control");
        check(isExactStringArray(member(root, "extensions"), "${type_file}"),
              "extensions contains the file type token ${type_file}");
        check(command && command->kind == JsonValue::Kind::String
                  && command->stringValue == "seer_share.exe",
              "command is seer_share.exe");
        check(exactStringArray(member(root, "arguments"),
                               {"--input", "${input_file}"}),
              "arguments are --input ${input_file}");

        if (command && command->kind == JsonValue::Kind::String) {
            const std::filesystem::path executableName(command->stringValue);
            check(!executableName.is_absolute()
                      && !executableName.has_parent_path(),
                  "command is package-relative");

            const auto executablePath = std::filesystem::weakly_canonical(
                stagingRoot / executableName);
            check(std::filesystem::is_regular_file(executablePath),
                  "staged executable exists as a regular file");
            check(std::filesystem::is_regular_file(executablePath)
                      && std::filesystem::file_size(executablePath) > 0,
                  "staged executable is non-empty");
            check(isWithin(stagingRoot, executablePath),
                  "staged executable remains contained by the staging root");
        }
    }
    catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }

    return failures == 0 ? 0 : 1;
}
