#include "map_renderer.h"

#include <utility>


namespace renderer {

    MapRenderer::MapRenderer(RenderSettings renderSettings)
            : renderSettings_(std::move(renderSettings)) {}

    bool detail::IsZero(double value) {
        return std::abs(value) < detail::EPSILON;
    }

    void MapRenderer::Render(const GetCoordinateStops& get_inform, std::ostream &output) {
        using namespace std::literals;

        svg::Document doc;
        auto first = get_inform.coordinate.begin();
        auto end = get_inform.coordinate.end();
        renderer::SphereProjector sphereProjector(first, end,
                                                  renderSettings_.width,
                                                  renderSettings_.height,
                                                  renderSettings_.padding);
        size_t count_color = 0;

        for (const auto &render_iterator: get_inform.polyline) {
            svg::Polyline polyline;
            for (const auto &stop_geo: render_iterator.second) {
                svg::Point points = sphereProjector.operator()(stop_geo.second);
                polyline.AddPoint(points);
            }
            polyline.SetStrokeColor(renderSettings_.color_palette[count_color % renderSettings_.color_palette.size()]);
            polyline.SetFillColor("none"s);
            polyline.SetStrokeWidth(renderSettings_.line_width);
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            doc.Add(polyline);
            ++count_color;
        }

        auto route_inform = get_inform.name_route_inform;
        count_color = 0;

        for (const auto &render_iterator: route_inform) {
            for (const auto &stop_geo: render_iterator.second) {

                svg::Point points = sphereProjector.operator()(stop_geo.second);
                svg::Text text_substrate;


                text_substrate.SetPosition(points);
                svg::Point point{renderSettings_.bus_label_offset.first, renderSettings_.bus_label_offset.second};
                text_substrate.SetOffset(point);
                text_substrate.SetFontSize(renderSettings_.bus_label_font_size);
                text_substrate.SetFontFamily("Verdana"s);
                text_substrate.SetFontWeight("bold"s);
                text_substrate.SetData(render_iterator.first);

                text_substrate.SetFillColor(renderSettings_.underlayer_color);
                text_substrate.SetStrokeColor(renderSettings_.underlayer_color);
                text_substrate.SetStrokeWidth(renderSettings_.underlayer_width);
                text_substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                text_substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                doc.Add(text_substrate);

                svg::Text text;
                text.SetPosition(points);
                text.SetOffset(point);
                text.SetFontSize(renderSettings_.bus_label_font_size);
                text.SetFontFamily("Verdana"s);
                text.SetFontWeight("bold"s);
                text.SetData(render_iterator.first);
                text.SetFillColor(renderSettings_.color_palette[count_color % renderSettings_.color_palette.size()]);
                doc.Add(text);
            }
            ++count_color;
        }

        for (const auto &g_i: get_inform.name_coord) {
            svg::Point points = sphereProjector.operator()(g_i.second);
            svg::Circle circle;
            circle.SetCenter(points);
            circle.SetRadius(renderSettings_.stop_radius);
            circle.SetFillColor("white"s);
            doc.Add(circle);
        }

        for (const auto &g_i: get_inform.name_coord) {
            svg::Point points = sphereProjector.operator()(g_i.second);
            svg::Text text_substrate_stop;
            text_substrate_stop.SetPosition(points);
            svg::Point point_stop{renderSettings_.stop_label_offset.first, renderSettings_.stop_label_offset.second};
            text_substrate_stop.SetOffset(point_stop);
            text_substrate_stop.SetFontSize(renderSettings_.stop_label_font_size);
            text_substrate_stop.SetFontFamily("Verdana"s);
            text_substrate_stop.SetData(g_i.first);
            text_substrate_stop.SetFillColor(renderSettings_.underlayer_color);
            text_substrate_stop.SetStrokeColor(renderSettings_.underlayer_color);
            text_substrate_stop.SetStrokeWidth(renderSettings_.underlayer_width);
            text_substrate_stop.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_substrate_stop.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(text_substrate_stop);

            svg::Text text_stop;
            text_stop.SetPosition(points);
            text_stop.SetOffset(point_stop);
            text_stop.SetFontSize(renderSettings_.stop_label_font_size);
            text_stop.SetFontFamily("Verdana"s);
            text_stop.SetData(g_i.first);
            text_stop.SetFillColor("black"s);
            doc.Add(text_stop);
        }

        doc.Render(output);
    }
} //namespace map_renderer