#include <sstream>
#include "request_handler.h"

using namespace transport::handlers;

RequestHandler::RequestHandler(transport::catalogue::TransportCatalogue &db) : db_(db) {}

// adding, editing, loading data

void RequestHandler::ExecuteQueries(const std::deque<transport::domains::StopQuery> &stopQueries,
                                    const std::deque<transport::domains::BusQuery> &busQueries) {
    for_each(stopQueries.begin(), stopQueries.end(),
             [&](const auto &stop_query) {
                 db_.AddStop(stop_query.name, stop_query.coordinates);
             });

    for_each(stopQueries.begin(), stopQueries.end(),
             [&](const auto &stop_query) {
                 db_.AddStopDistances(stop_query.name, std::move(stop_query.distances));
             });

    for_each(busQueries.begin(), busQueries.end(),
             [&](const auto &bus_query) {
                 db_.AddBus(bus_query.name, bus_query.is_annular, std::move(bus_query.stops));
             });
}

void RequestHandler::ExecuteQueriesAsync(const std::deque<transport::domains::StopQuery> &stopQueries,
                                         const std::deque<transport::domains::BusQuery> &busQueries) {
    // todo async
    ExecuteQueries(stopQueries, busQueries);
}

std::string RequestHandler::ExecuteStats(const std::deque<transport::domains::StatRequest> &statRequests,
                                         const transport::interfaces::IReader &reader) {
    return reader.ConvertStatRequests(db_, statRequests, reader.GetRenderSettings());
}

std::string RequestHandler::ExecuteStatsAsync(const std::deque<transport::domains::StatRequest> &statRequests,
                                              const transport::interfaces::IReader &reader) {
    return reader.ConvertStatRequests(db_, statRequests, reader.GetRenderSettings()); // todo async
}

RequestHandler::GetCoordinateStops
RequestHandler::GetStopsWithRoute(const transport::catalogue::TransportCatalogue &catalogue) {
    RequestHandler::GetCoordinateStops getCoordinateStops;

    std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> polyline;
    std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> name_route_inform;
    std::deque<const transport::domains::Stop *> sort_bus_name;

    const auto &buses = catalogue.GetBuses();

    for (const auto &[number, bus]: buses) {
        const auto string_number = std::string(number);
        const auto first_stop = bus->stops.front();
        name_route_inform[string_number].push_back({first_stop->name, first_stop->coordinates});

        if (!bus->is_annular_trip) {
            if (bus->stops.back() != bus->stops[bus->stops.size() / 2]) {
                auto last_stop = bus->stops[bus->stops.size() / 2];
                name_route_inform[string_number].push_back({last_stop->name, last_stop->coordinates});
            }
        }

        for (const auto stop: bus->stops) {
            sort_bus_name.push_back(stop);
            polyline[string_number].push_back({stop->name, stop->coordinates});
        }
    }

    std::map<std::string, geo::Coordinates> res;
    std::vector<geo::Coordinates> coord;
    coord.reserve(buses.size());

    for (const auto stop: sort_bus_name) {
        res.insert({stop->name, stop->coordinates});
        coord.push_back(stop->coordinates);
    }

    getCoordinateStops.polyline = polyline;
    getCoordinateStops.name_route_inform = name_route_inform;
    getCoordinateStops.name_coord = res;
    getCoordinateStops.coordinate = coord;

    return getCoordinateStops;
}

std::string RequestHandler::RenderMap(const transport::catalogue::TransportCatalogue &catalogue,
                                      renderer::MapRenderer::RenderSettings renderSettings) {
    using namespace std::literals;

    svg::Document doc;
    renderer::MapRenderer mapRenderer(renderSettings);
    const RequestHandler::GetCoordinateStops get_inform = RequestHandler::GetStopsWithRoute(catalogue);

    auto first = get_inform.coordinate.begin();
    auto end = get_inform.coordinate.end();
    renderer::SphereProjector sphereProjector(first, end,
                                              renderSettings.width,
                                              renderSettings.height,
                                              renderSettings.padding);
    size_t count_color = 0;

    for (const auto &render_iterator: get_inform.polyline) {
        svg::Polyline polyline;
        for (const auto &stop_geo: render_iterator.second) {
            svg::Point points = sphereProjector.operator()(stop_geo.second);
            polyline.AddPoint(points);
        }
        polyline.SetStrokeColor(renderSettings.color_palette[count_color % renderSettings.color_palette.size()]);
        polyline.SetFillColor("none"s);
        polyline.SetStrokeWidth(renderSettings.line_width);
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
            svg::Point point{renderSettings.bus_label_offset.first, renderSettings.bus_label_offset.second};
            text_substrate.SetOffset(point);
            text_substrate.SetFontSize(renderSettings.bus_label_font_size);
            text_substrate.SetFontFamily("Verdana"s);
            text_substrate.SetFontWeight("bold"s);
            text_substrate.SetData(render_iterator.first);

            text_substrate.SetFillColor(renderSettings.underlayer_color);
            text_substrate.SetStrokeColor(renderSettings.underlayer_color);
            text_substrate.SetStrokeWidth(renderSettings.underlayer_width);
            text_substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(text_substrate);

            svg::Text text;
            text.SetPosition(points);
            text.SetOffset(point);
            text.SetFontSize(renderSettings.bus_label_font_size);
            text.SetFontFamily("Verdana"s);
            text.SetFontWeight("bold"s);
            text.SetData(render_iterator.first);
            text.SetFillColor(renderSettings.color_palette[count_color % renderSettings.color_palette.size()]);
            doc.Add(text);
        }
        ++count_color;
    }

    for (const auto &g_i: get_inform.name_coord) {
        svg::Point points = sphereProjector.operator()(g_i.second);
        svg::Circle circle;
        circle.SetCenter(points);
        circle.SetRadius(renderSettings.stop_radius);
        circle.SetFillColor("white"s);
        doc.Add(circle);
    }

    for (const auto &g_i: get_inform.name_coord) {
        svg::Point points = sphereProjector.operator()(g_i.second);
        svg::Text text_substrate_stop;
        text_substrate_stop.SetPosition(points);
        svg::Point point_stop{renderSettings.stop_label_offset.first, renderSettings.stop_label_offset.second};
        text_substrate_stop.SetOffset(point_stop);
        text_substrate_stop.SetFontSize(renderSettings.stop_label_font_size);
        text_substrate_stop.SetFontFamily("Verdana"s);
        text_substrate_stop.SetData(g_i.first);
        text_substrate_stop.SetFillColor(renderSettings.underlayer_color);
        text_substrate_stop.SetStrokeColor(renderSettings.underlayer_color);
        text_substrate_stop.SetStrokeWidth(renderSettings.underlayer_width);
        text_substrate_stop.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text_substrate_stop.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        doc.Add(text_substrate_stop);

        svg::Text text_stop;
        text_stop.SetPosition(points);
        text_stop.SetOffset(point_stop);
        text_stop.SetFontSize(renderSettings.stop_label_font_size);
        text_stop.SetFontFamily("Verdana"s);
        text_stop.SetData(g_i.first);
        text_stop.SetFillColor("black"s);
        doc.Add(text_stop);
    }

    std::ostringstream out;
    mapRenderer.Render(doc, out);

    return out.str();
}



