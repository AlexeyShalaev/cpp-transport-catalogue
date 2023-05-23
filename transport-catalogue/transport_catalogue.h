#pragma once

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport::catalogue {

    using namespace transport::domains;
    using namespace transport::render;
    using namespace transport::router;


    class TransportCatalogue {
    public:

        TransportCatalogue() = default;

        TransportCatalogue(
                std::vector<std::shared_ptr<Stop>> &stops,
                std::vector<std::shared_ptr<Bus>> &buses,
                const RenderSettings& render_settings,
                RoutingSettings router_settings
        );


        ~TransportCatalogue() = default;

        std::optional<Stop> GetStopInfo(std::string_view stop_name);

        std::optional<Bus> GetBusInfo(std::string_view bus_name);

        std::shared_ptr<Stop> StopByName(std::string_view name);

        std::shared_ptr<Bus> BusByName(std::string_view name);

        const std::string &GetMap();

        std::shared_ptr<std::vector<RouteItem>> findRouteInBase(std::string_view from, std::string_view to);


        [[nodiscard]] std::string Serialize() const;

        void Deserialize(const std::string &data);

    private:
        std::map<std::string_view, std::shared_ptr<Stop>> stops_;
        std::map<std::string_view, std::shared_ptr<Bus>> buses_;

        std::unique_ptr<MapRenderer> render_;
        std::unique_ptr<TransportRouter> router_;

        std::string map_;
    };

}