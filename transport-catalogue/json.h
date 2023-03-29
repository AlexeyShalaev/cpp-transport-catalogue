#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <cassert>
#include <algorithm>

namespace json {

    class Node;
// Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node final : private Value {
    public:
        using variant::variant;

        Node(Value value);

        bool IsInt() const;

        bool IsDouble() const;

        bool IsPureDouble() const;

        bool IsBool() const;

        bool IsString() const;

        bool IsNull() const;

        bool IsArray() const;

        bool IsDict() const;

        bool IsMap() const;

        const Array &AsArray() const;

        const Dict &AsDict() const;

        const Dict &AsMap() const;

        int AsInt() const;

        bool AsBool() const;

        double AsDouble() const;

        const std::string &AsString() const;

        const Value &GetValue() const;

        Value &GetValue();

        bool operator==(const Node &other) const;

        bool operator!=(const Node &other) const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

        bool operator==(const json::Document &other) const;

        bool operator!=(const json::Document &other) const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    struct PrintContext {
        std::ostream &out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented(int offset = 0) const {
            return {out, indent_step, indent_step + indent + offset};
        }
    };

    namespace detail {

// Шаблон, подходящий для вывода double и int
        template<typename T>
        void PrintValue(const T &value, const PrintContext &ctx) {
            auto &out = ctx.out;
            out << value;
        }

        void PrintNode(const Node &node, const PrintContext &ctx);


        void PrintValue(std::nullptr_t, const PrintContext &ctx);

        void PrintValue(bool value, const PrintContext &ctx);

        void PrintValue(const Array &array, const PrintContext &ctx);

        void PrintValue(const std::string &str, const PrintContext &ctx);

        void PrintValue(const Dict &dict, const PrintContext &ctx);

    } // namespace detail

    void Print(const Document &doc, std::ostream &output);

}  // namespace json