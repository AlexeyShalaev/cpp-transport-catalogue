#pragma once

#include "geo.h"
#include "json.h"

#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <ostream>

namespace transport::domains {

    struct Stop {
        Stop() = default;

        std::string name;
        geo::Coordinates coordinates{};
        std::map<std::string, int> road_distance;
        std::set<std::string> buses;

        void Parse(const json::Dict &request);
    };

    struct Bus {
        Bus() = default;

        std::string name;
        bool is_round_trip{};
        unsigned int stop_count = 0;
        unsigned int unique_stop_count = 0;
        double route_length = 0.0;
        double curvature = 0.0;

        std::vector<std::string> stops;

        void Parse(const json::Dict &request);
    };

    int RealLenBetweenStops(const std::shared_ptr<Stop> &from, const std::shared_ptr<Stop> &to);

}