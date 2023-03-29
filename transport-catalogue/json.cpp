#include "json.h"

using namespace std;

namespace json {

    namespace detail {

        Node LoadNode(istream &input);

        std::istreambuf_iterator<char> ParseEmptySpace(std::istream &input) {
            char c;
            if (!(input >> c)) {
                throw json::ParsingError("Unexpected end of file"s);
            }
            input.putback(c);
            return {input};
        }

        Node LoadArray(istream &input) {
            Array result;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            bool not_a_first_value = false;

            while (true) {
                if (it == end) {
                    throw json::ParsingError("Array parsing error"s);
                }
                it = ParseEmptySpace(input);

                if (*it == ']') {
                    ++it;
                    break;
                }
                if (not_a_first_value && *it != ',') {
                    throw json::ParsingError(", expected"s);
                } else if (not_a_first_value && *it == ',') {
                    ++it;
                }
                result.push_back(LoadNode(input));
                not_a_first_value = true;
            }

            return {std::move(result)};
        }

        Node LoadNumber(std::istream &input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw json::ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw json::ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return {std::stoi(parsed_num)};
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return {std::stod(parsed_num)};
            }
            catch (...) {
                throw json::ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream &input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw json::ParsingError("String parsing error"s);
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw json::ParsingError("String parsing error"s);
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw json::ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw json::ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return {std::move(s)};
        }

        Node LoadBool(istream &input) {
            char c;
            input >> c;

            if (c == 't') {
                input.putback(c);
                char str[4];
                input.read(str, 4);
                if (string(str, 4) != "true"s) {
                    throw json::ParsingError("Invalid boolean value"s);
                };
                return {true};
            } else if (c == 'f') {
                input.putback(c);
                char str[5];
                input.read(str, 5);
                if (string(str, 5) != "false"s) {
                    throw json::ParsingError("Invalid boolean value"s);
                }
                return {false};
            }

            throw json::ParsingError("Unknown lexeme"s);
        }

        Node LoadNull(istream &input) {
            char str[4];
            input.read(str, 4);
            if (string(str, 4) != "null"s) {
                throw json::ParsingError("Unknown value"s);
            }

            return {nullptr};
        }

        Node LoadDict(istream &input) {
            Dict result;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            bool not_a_first_key = false;

            while (true) {
                if (it == end) {
                    throw json::ParsingError("Dict parsing error"s);
                }
                it = ParseEmptySpace(input);
                if (*it == '}') {
                    ++it;
                    break;
                }
                if (not_a_first_key && *it != ',') {
                    throw json::ParsingError(", expected"s);
                } else if (not_a_first_key && *it == ',') {
                    ++it;
                }
                it = ParseEmptySpace(input);
                if (*it != '\"') {
                    throw json::ParsingError("\" expected"s);
                } else {
                    ++it;
                    string key = LoadString(input).AsString();
                    it = ParseEmptySpace(input);
                    if (*it != ':') {
                        throw json::ParsingError(": expected"s);
                    } else {
                        ++it;
                        result.insert({std::move(key), LoadNode(input)});
                        not_a_first_key = true;
                    }
                }
            }

            return {std::move(result)};
        }

        Node LoadNode(istream &input) {
            char c;

            if (!(input >> c)) {
                throw json::ParsingError("Unexpected end of file"s);
            }

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else if (c == '-' || std::isdigit(c)) {
                input.putback(c);
                return LoadNumber(input);
            } else {
                throw json::ParsingError("Unexpected lexeme"s);
            }
        }

// Перегрузка функции PrintValue для вывода значений null
        void PrintValue(std::nullptr_t, const PrintContext &ctx) {
            auto &out = ctx.out;
            out << "null"sv;
        }

        void PrintValue(bool value, const PrintContext &ctx) {
            auto &out = ctx.out;
            if (value) {
                out << "true"sv;
            } else {
                out << "false"sv;
            }
        }

        void PrintValue(const Array &array, const PrintContext &ctx) {
            auto &out = ctx.out;
            out << "[\n"sv;
            bool IsFirst = true;
            auto inner_ctx = ctx.Indented();
            for (auto it = array.begin(); it != array.end(); ++it) {
                if (IsFirst) {
                    IsFirst = false;
                } else {
                    out << ",\n"sv;
                }
                inner_ctx.PrintIndent();
                PrintNode(*it, ctx.Indented());
            }
            out << "\n"sv;
            ctx.PrintIndent();
            out << "]"sv;
        }

        void PrintValue(const std::string &str, const PrintContext &ctx) {
            auto &out = ctx.out;
            out << "\""sv;
            for (const char c: str) {
                switch (c) {
                    case '\n':
                        out << "\\n"sv;
                        break;
                    case '\r':
                        out << "\\r"sv;
                        break;
                    case '\"':
                        out << "\\\""sv;
                        break;
                    case '\\':
                        out << "\\\\"sv;
                        break;
                    default:
                        out << c;
                }
            }
            out << "\""sv;
        }

        void PrintValue(const Dict &dict, const PrintContext &ctx) {
            auto &out = ctx.out;
            out << "{\n"sv;
            bool IsFirst = true;
            auto inner_ctx = ctx.Indented();
            for (auto it = dict.begin(); it != dict.end(); ++it) {
                const auto &[key, value] = *it;
                if (IsFirst) {
                    IsFirst = false;
                } else {
                    out << ",\n"sv;
                }
                inner_ctx.PrintIndent();
                out << "\""sv << key << "\""sv << ": ";
                PrintNode(value, inner_ctx);
            }
            out << "\n"sv;
            ctx.PrintIndent();
            out << "}"sv;
        }

        void PrintNode(const Node &node, const PrintContext &ctx) {
            std::visit(
                    [&ctx](const auto &value) { PrintValue(value, ctx); },
                    node.GetValue());
        }

    }  // namespace detail

    Node::Node(json::Value value) : json::Value(std::move(value)) {
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsInt() || std::holds_alternative<double>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }

    bool Node::IsMap() const {
        return IsDict();
    }

    const Array &Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Value type is not Array."s);
        }
        return get<Array>(*this);
    }

    const Dict &Node::AsDict() const {
        if (!IsDict()) {
            throw std::logic_error("Value type is not Dict."s);
        }
        return get<Dict>(*this);
    }

    const Dict &Node::AsMap() const {
        return AsDict();
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Value type is not int."s);
        }
        return get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Value type is not bool."s);
        }
        return get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Value type is not double."s);
        }
        return IsPureDouble() ? get<double>(*this) : get<int>(*this);
    }

    const string &Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Value type is not string"s);
        }
        return get<std::string>(*this);
    }

    bool Node::operator==(const Node &other) const {
        return this->GetValue() == other.GetValue();
    }

    bool Node::operator!=(const Node &other) const {
        return !(*this == other);
    }

    const json::Value &Node::GetValue() const {
        return *this;
    }

    json::Value &Node::GetValue() {
        return *this;
    }

    Document::Document(Node root)
            : root_(std::move(root)) {
    }

    const Node &Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const json::Document &other) const {
        return GetRoot() == other.GetRoot();
    }

    bool Document::operator!=(const json::Document &other) const {
        return !(*this == other);
    }

    Document Load(istream &input) {
        return Document{detail::LoadNode(input)};
    }

    void Print(const Document &doc, std::ostream &output) {
        PrintContext ctx{output};
        detail::PrintNode(doc.GetRoot(), ctx);
    }

}  // namespace json