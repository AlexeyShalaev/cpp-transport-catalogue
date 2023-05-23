#pragma once

#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Data = std::variant<std::nullptr_t, Array, Dict, int, double, bool, std::string>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node() = default;

        Node(std::nullptr_t) {}

        Node(bool val);

        Node(int val);

        Node(double val);

        Node(std::string val);

        Node(Array array);

        Node(Dict map);

        [[nodiscard]] bool AsBool() const;

        [[nodiscard]] int AsInt() const;

        [[nodiscard]] double AsDouble() const;

        [[nodiscard]] const std::string &AsString() const;

        [[nodiscard]] const Array &AsArray() const;

        [[nodiscard]] const Dict &AsMap() const;

        [[nodiscard]] bool IsNull() const;

        [[nodiscard]] bool IsBool() const;

        [[nodiscard]] bool IsInt() const;

        [[nodiscard]] bool IsDouble() const;

        [[nodiscard]] bool IsPureDouble() const;

        [[nodiscard]] bool IsString() const;

        [[nodiscard]] bool IsArray() const;

        [[nodiscard]] bool IsMap() const;

        bool operator==(const Node &other) const;

        bool operator!=(const Node &other) const;

        void Print(std::ostream &out) const;

    private:
        Data value_;

        struct Printer {
            std::ostream &output;

            void operator()(std::nullptr_t) const;

            void operator()(const Array &value) const;

            void operator()(const Dict &value) const;

            void operator()(bool value) const;

            void operator()(int value) const;

            void operator()(double value) const;

            void operator()(std::string_view value) const;
        };
    };

    class Document {
    public:
        Document(Node root);

        [[nodiscard]] const Node &GetRoot() const;

        bool operator==(const Document &other) const {
            return root_ == other.root_;
        }

        bool operator!=(const Document &other) const {
            return root_ != other.root_;
        }

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

    void Print(Document &&doc, std::ostream &output);
}