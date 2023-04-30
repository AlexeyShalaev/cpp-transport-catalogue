#include "json_reader.h"
#include "request_handler.h"

using namespace std::literals;

JsonReader::JsonReader() = default;

void JsonReader::Load(std::istream &input) {
    auto document = json::Load(input);

    if (document.GetRoot().IsNull()) return;

    auto &it = document.GetRoot().AsDict();

    if (it.count("base_requests"s)) {
        ParseBase(it.at("base_requests"s));
    }
    if (it.count("stat_requests"s) && it.at("stat_requests"s).IsArray()) {
        ParseStats(it.at("stat_requests"s));
    }
    if (it.count("render_settings"s)) {
        ParseRenderSettings(it.at("render_settings"s));
    }
    if (it.count("routing_settings"s)) {
        ParseRoutingSettings(it.at("routing_settings"s));
    }
}

std::string JsonReader::ConvertStatRequests(const transport::catalogue::TransportCatalogue &db,
                                            const std::deque<transport::domains::StatRequest> &stats,
                                            std::optional<renderer::MapRenderer::RenderSettings> renderSettings,
                                            std::optional<transport::router::TransportRouter::RoutingSettings> routingSettings) const {
    std::ostringstream result;
    json::Builder builder;
    builder.StartArray();
    for (auto stat: stats) {
        if (stat.type == "Stop") {
            auto info = db.GetBusesByStop(std::get<std::string>(stat.extra));
            if (!info) {
                builder.StartDict()
                        .Key("request_id"s).Value(stat.id)
                        .Key("error_message"s).Value("not found"s)
                        .EndDict();
            } else {
                builder.StartDict()
                        .Key("buses"s).StartArray();
                for (const auto &bus: *info) {
                    builder.Value(bus->name);
                }
                builder.EndArray();
                builder.Key("request_id"s).Value(stat.id).EndDict();
            }
        } else if (stat.type == "Bus") {
            try {
                auto info = db.GetBusInfo(std::get<std::string>(stat.extra));
                builder.StartDict()
                        .Key("curvature"s).Value(info.curvature)
                        .Key("request_id"s).Value(stat.id)
                        .Key("route_length"s).Value(info.length)
                        .Key("stop_count"s).Value(info.route_stops)
                        .Key("unique_stop_count"s).Value(info.unique_stops)
                        .EndDict();
            } catch (std::invalid_argument &) {
                builder.StartDict()
                        .Key("request_id"s).Value(stat.id)
                        .Key("error_message"s).Value("not found"s)
                        .EndDict();
            }
        } else if (stat.type == "Map" && renderSettings.has_value()) {
            builder.StartDict()
                    .Key("request_id"s).Value(stat.id)
                    .Key("map"s).Value(transport::handlers::RequestHandler::RenderMap(db, renderSettings.value()))
                    .EndDict();
        } else if (stat.type == "Route"s && routingSettings.has_value()) {
            const auto two_stops = std::get<transport::domains::StatRequest::TwoStops>(stat.extra);
            auto get_find_route = transport::handlers::RequestHandler::FindRoute(db, routingSettings.value(),
                                                                                 two_stops.first, two_stops.second);
            if (get_find_route == std::nullopt) {
                builder.StartDict()
                        .Key("request_id"s).Value(stat.id)
                        .Key("error_message"s).Value("not found"s)
                        .EndDict();
            } else {
                builder.StartDict()
                        .Key("request_id"s).Value(stat.id)
                        .Key("total_time"s).Value(get_find_route->weight_)
                        .Key("items"s).StartArray();

                for (const auto &get_f_r: get_find_route->busnum) {
                    builder.StartDict()
                            .Key("stop_name"s).Value(get_f_r.stop_name_)
                            .Key("time"s).Value(routingSettings->bus_wait_time_)
                            .Key("type"s).Value("Wait"s)
                            .EndDict();

                    builder.StartDict()
                            .Key("bus"s).Value(get_f_r.bus_num_)
                            .Key("span_count"s).Value(get_f_r.span_count_)
                            .Key("time"s).Value(get_f_r.time_ - routingSettings->bus_wait_time_)
                            .Key("type"s).Value(get_f_r.type_)
                            .EndDict();
                }

                builder.EndArray().EndDict();
            }
        }
    }
    builder.EndArray();

    json::Document doc(builder.Build());
    json::Print(doc, result);
    return result.str();
}


void JsonReader::ParseBase(const json::Node &node_) {
    // clearing data
    stopQueries.clear();
    busQueries.clear();

    auto &nodes = node_.AsArray();
    for (auto &node: nodes) {
        const auto &tag = node.AsDict();
        const auto tagType = tag.at("type"s).AsString();

        if (tagType == "Stop"s) {
            const auto stopName = tag.at("name"s).AsString();
            double lat = tag.at("latitude"s).AsDouble();
            double lng = tag.at("longitude"s).AsDouble();
            std::vector<std::pair<std::string, int>> distances;

            //дистанция
            if (tag.count("road_distances"s)) {
                for (const auto &[name, value]: tag.at("road_distances"s).AsDict()) {
                    distances.emplace_back(name, value.AsInt());
                }
            }

            stopQueries.emplace_back(stopName, lat, lng, distances);
        } else if (tagType == "Bus"s) {
            const auto name = tag.at("name"s).AsString();
            const auto is_annular_trip = tag.at("is_roundtrip"s).AsBool();
            auto &it = tag.at("stops"s).AsArray();

            std::vector<std::string> stops_;
            stops_.reserve(it.size());
            for (const auto &stop: it) {
                if (stop.IsString()) {
                    stops_.push_back(stop.AsString());
                }
            }

            busQueries.emplace_back(name, is_annular_trip, stops_);
        } else {
            throw std::invalid_argument("Unknown type"s);
        }
    }
}

void JsonReader::ParseStats(const json::Node &node_) {
    auto &nodes = node_.AsArray();
    for (auto &node: nodes) {
        const auto &tag = node.AsDict();
        const auto &type = tag.at("type"s).AsString();
        if (type == "Stop"s || type == "Bus"s) {
            statRequests.emplace_back(tag.at("id"s).AsInt(), type, tag.at("name"s).AsString());
        } else if (type == "Map"s) {
            statRequests.emplace_back(tag.at("id"s).AsInt(), type, "");
        } else if (type == "Route"s) {
            statRequests.emplace_back(tag.at("id"s).AsInt(), type, tag.at("from"s).AsString(),
                                      tag.at("to"s).AsString());
        } else {
            throw std::invalid_argument("Unknown type"s);
        }
    }
}

void JsonReader::ParseRenderSettings(const json::Node &node_) {
    auto &settings = node_.AsDict();
    renderer::MapRenderer::RenderSettings render_settings;

    if (settings.count("width"s)) {
        render_settings.width = settings.at("width"s).AsDouble();
    }
    if (settings.count("height"s)) {
        render_settings.height = settings.at("height"s).AsDouble();
    }
    if (settings.count("padding"s)) {
        render_settings.padding = settings.at("padding"s).AsDouble();
    }
    if (settings.count("line_width"s)) {
        render_settings.line_width = settings.at("line_width"s).AsDouble();
    }
    if (settings.count("stop_radius"s)) {
        render_settings.stop_radius = settings.at("stop_radius"s).AsDouble();
    }
    if (settings.count("bus_label_font_size"s)) {
        render_settings.bus_label_font_size = settings.at("bus_label_font_size"s).AsDouble();
    }
    if (settings.count("bus_label_offset"s)) {
        auto it = settings.at("bus_label_offset"s).AsArray();
        render_settings.bus_label_offset = {it[0].AsDouble(), it[1].AsDouble()};
    }
    if (settings.count("stop_label_font_size"s)) {
        render_settings.stop_label_font_size = settings.at("stop_label_font_size"s).AsDouble();
    }
    if (settings.count("stop_label_offset"s)) {
        auto it = settings.at("stop_label_offset"s).AsArray();
        render_settings.stop_label_offset = {it[0].AsDouble(), it[1].AsDouble()};
    }

    if (settings.count("underlayer_color"s)) {
        render_settings.underlayer_color = GetColor(settings.at("underlayer_color"s));
    }
    if (settings.count("underlayer_width"s)) {
        render_settings.underlayer_width = settings.at("underlayer_width"s).AsDouble();
    }
    //массив цветов
    if (settings.count("color_palette"s)) {
        auto &array = settings.at("color_palette"s).AsArray();
        render_settings.color_palette.reserve(array.size());
        for (auto &node: array) {
            render_settings.color_palette.emplace_back();
            render_settings.color_palette.back() = GetColor(node);
        }
    }
    renderSettings = std::move(render_settings);
}

void JsonReader::ParseRoutingSettings(const json::Node &node_) {
    auto key = node_.AsDict();
    auto bus_wait_time = key["bus_wait_time"s].AsInt();
    auto bus_velocity = key["bus_velocity"s].AsDouble();

    if (bus_wait_time < 0) {
        std::cerr << "time in not valid"s << std::endl;
    }

    routingSettings = {bus_wait_time, bus_velocity * (1000.0 / 60.0)};
}

svg::Color JsonReader::GetColor(const json::Node &node) {
    if (node.IsString()) {
        return node.AsString();
    } else if (node.AsArray().size() == 3) {
        return svg::Rgb(node.AsArray()[0].AsInt(),
                        node.AsArray()[1].AsInt(),
                        node.AsArray()[2].AsInt());
    } else if (node.AsArray().size() == 4) {
        return svg::Rgba(node.AsArray()[0].AsInt(),
                         node.AsArray()[1].AsInt(),
                         node.AsArray()[2].AsInt(),
                         node.AsArray()[3].AsDouble());
    }
    throw std::invalid_argument("Couldn't get color");
}
