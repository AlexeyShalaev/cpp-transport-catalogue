#pragma once

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_router.h"

#include <map>
#include <string>
#include <vector>

namespace json::reader {
    using namespace transport::domains;
    using namespace transport::render;
    using namespace transport::router;
    using namespace transport::response;

    std::vector<std::shared_ptr<Stop>> ParseStop(const json::Array &base_requests);

    std::vector<std::shared_ptr<Bus>> ParseBus(const json::Array &base_requests);

    svg::Color ParseColor(const json::Node &node);

    transport::render::RenderSettings ParseRenderSetting(const json::Dict &render_settings);

    RoutingSettings ParseRouterSetting(const json::Dict &router_settings);

}
