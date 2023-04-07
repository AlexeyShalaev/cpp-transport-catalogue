#pragma once

#include <string>
#include <sstream>
#include <optional>

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <deque>

class JsonReader final : public transport::interfaces::IReader {
public:

    explicit JsonReader();

    void Load(std::istream &input) override;

    [[nodiscard]] std::string
    ConvertStatRequests(const transport::catalogue::TransportCatalogue &db,
                        const std::deque<transport::domains::StatRequest> &stats,
                        renderer::MapRenderer::RenderSettings renderSettings) const override;

private:
    // загрузка данных из json
    void ParseBase(const json::Node &node_);

    void ParseStats(const json::Node &node_);

    void ParseSettings(const json::Node &node_);

    static svg::Color GetColor(const json::Node &node);

};