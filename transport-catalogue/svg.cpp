#include "svg.h"

#include <utility>

namespace svg {

    using namespace std::literals;

    inline std::ostream &operator<<(std::ostream &out, Rgb color) {
        using namespace std::literals;
        out << "rgb("sv << +color.red << ","sv << +color.green << ","sv << +color.blue << ")"sv;
        return out;
    }

    inline std::ostream &operator<<(std::ostream &out, Rgba color) {
        out << "rgba(" << +color.red << "," << +color.green << "," << +color.blue << "," << +color.opacity << ")";
        return out;
    }

    void OstreamColorPrinter::operator()(std::monostate) const {
        out << "none";
    }

    void OstreamColorPrinter::operator()(const std::string &color) const {
        out << color;
    }

    void OstreamColorPrinter::operator()(Rgb color) const {
        out << color;
    }

    void OstreamColorPrinter::operator()(Rgba color) const {
        out << color;
    }

    inline std::ostream &operator<<(std::ostream &out, Color color) {
        if (std::holds_alternative<std::monostate>(color)) {
            out << "none";
        } else if (std::holds_alternative<std::string>(color)) {
            out << std::get<std::string>(color);
        } else if (std::holds_alternative<Rgb>(color)) {
            out << std::get<Rgb>(color);
        } else if (std::holds_alternative<Rgba>(color)) {
            out << std::get<Rgba>(color);
        }

        return out;
    }

    inline uint8_t Lerp(uint8_t from, uint8_t to, double t) {
        return static_cast<uint8_t>(std::round((to - from) * t + from));
    }

    inline svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t) {
        return {Lerp(from.red, to.red, t), Lerp(from.green, to.green, t), Lerp(from.blue, to.blue, t)};
    }

    inline std::ostream &operator<<(std::ostream &out, const StrokeLineCap cap) {
        using namespace std::literals;
        switch (cap) {
            case StrokeLineCap::BUTT:
                out << "butt"sv;
                break;
            case StrokeLineCap::ROUND:
                out << "round"sv;
                break;
            case StrokeLineCap::SQUARE:
                out << "square"sv;
                break;
            default:
                break;
        }
        return out;
    }

    inline std::ostream &operator<<(std::ostream &out, const StrokeLineJoin join) {
        using namespace std::literals;
        switch (join) {
            case StrokeLineJoin::ARCS:
                out << "arcs"sv;
                break;
            case StrokeLineJoin::BEVEL:
                out << "bevel"sv;
                break;
            case StrokeLineJoin::MITER:
                out << "miter"sv;
                break;
            case StrokeLineJoin::MITER_CLIP:
                out << "miter-clip"sv;
                break;
            case StrokeLineJoin::ROUND:
                out << "round"sv;
                break;
            default:
                break;
        }
        return out;
    }

    RenderContext RenderContext::Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }


    void Object::Render(const RenderContext &context) const {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    Circle &Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )" << R"(r=")" << radius_ << R"(")";
        RenderAttrs(context.out);
        out << "/>"sv;
    }


    Polyline &Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << R"(<polyline points=")";
        bool firstPoint = true;
        for (const auto &p: points_) {
            out << (firstPoint ? ""sv : " "sv) << p.x << ","sv << p.y;
            firstPoint = false;
        }
        out << R"(")";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text &Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        value_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << R"( x=")" << position_.x << R"(" y=")" << position_.y << R"(" )"
            << R"(dx=")" << offset_.x << R"(" dy=")" << offset_.y << R"(" )"
            << R"(font-size=")" << size_ << "\"";

        if (!font_family_.empty()) {
            out << R"( font-family=")" << font_family_ << R"(")";
        }
        if (!font_weight_.empty() && font_weight_ != "normal"sv) {
            out << R"( font-weight=")" << font_weight_ << R"(")";
        }

        out << ">"sv;

        for (char c: value_) {
            switch (c) {
                case '"':
                    out << "&quot;"sv;
                    break;
                case '\'':
                    out << "&apos;"sv;
                    break;
                case '<':
                    out << "&lt;"sv;
                    break;
                case '>':
                    out << "&gt;"sv;
                    break;
                case '&':
                    out << "&amp;"sv;
                    break;
                default:
                    out << c;
            }
        }
        out << "</text>"sv;
    }

    void Document::AddPointer(std::shared_ptr<Object> &&obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        for (const auto &object: objects_) {
            out << "  ";
            object->Render(out);
        }
        out << "</svg>"sv;
    }
}