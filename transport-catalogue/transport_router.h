#pragma once

#include "domain.h"
#include "router.h"

#include "transport_router.pb.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <unordered_map>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <utility>
#include <vector>

namespace transport::router {

    using namespace transport::domains;

    using minutes = double;
    using seconds = double;

    using km_ch = double;
    using m_c = double;


    struct RoutingSettings {
        RoutingSettings() = default;

        RoutingSettings(minutes wait_time, km_ch velocity)
                : bus_wait_time(wait_time * 60), bus_velocity(velocity / 3.6) {}

        seconds bus_wait_time;
        m_c bus_velocity;
    };


    struct RouteItem {
        RouteItem() = default;

        RouteItem(std::shared_ptr<Stop> start_stop,
                  std::shared_ptr<Stop> finish_stop,
                  std::shared_ptr<Bus> new_bus,
                  int new_trip_stop_count,
                  seconds new_trip_time,
                  seconds new_wait_time
        ) :
                start_stop_idx(std::move(start_stop)),
                finish_stop_idx(std::move(finish_stop)),
                bus(std::move(new_bus)),
                stop_count(new_trip_stop_count),
                trip_time(new_trip_time),
                wait_time(new_wait_time) {


        }

        std::shared_ptr<Stop> start_stop_idx = nullptr;
        std::shared_ptr<Stop> finish_stop_idx = nullptr;
        std::shared_ptr<Bus> bus = nullptr;

        unsigned int stop_count = 0;
        seconds trip_time = 0.0;
        seconds wait_time = 0.0;
    };


    class TransportRouter {
    public:

        TransportRouter(
                std::map<std::string_view, std::shared_ptr<Stop>> &stops,
                std::map<std::string_view, std::shared_ptr<Bus>> &buses,
                RoutingSettings settings);

        void BuildGraph();

        const RoutingSettings &GetSettings() const;

        std::shared_ptr<std::vector<RouteItem>> findRoute(std::string_view from, std::string_view to);


        void SerializeSettings(TCProto::RoutingSettings &proto) const;

        static RoutingSettings DeserializeSettings(const TCProto::RoutingSettings &proto);


        void SerializeData(TCProto::TransportRouter &proto) const;

        void DeserializeData(const TCProto::TransportRouter &proto);

    private:
        const std::map<std::string_view, std::shared_ptr<Stop>> &stops_;
        const std::map<std::string_view, std::shared_ptr<Bus>> &buses_;
        const RoutingSettings settings_;

        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> search_in_graph_ = nullptr;

        std::vector<RouteItem> graph_edges_;
        std::unordered_map<std::shared_ptr<Stop>, size_t> graph_vertexes_;

        void FillVertexes();

        void FillEdges();
    };

}