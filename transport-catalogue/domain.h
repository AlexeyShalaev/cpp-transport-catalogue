#pragma once

#include "geo.h"
#include "map_renderer.h"

#include <deque>
#include <string>
#include <utility>
#include <vector>

namespace transport::domains {

    // Models

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::deque<Stop *> stops;
        int route_length;
        bool is_annular_trip;
    };

    // InfoModels

    struct BusStat {
        std::string name;
        int route_stops;
        int unique_stops;
        int length;
        double curvature;
    };

    struct StopStat {
        std::string name;
        bool exists;
        std::vector<Bus *> buses;
    };

    /*
    namespace route::item {
        struct Wait {
            std::string stop_name;
            double time;
        };
        struct Bus {
            std::string bus_name;
            int span_count;
            double time;
        };
    }

    struct RouteStat {
        double total_time;
        std::vector<std::variant<route::item::Wait, route::item::Bus>> items;
    };
    */

    // Utils

    const size_t PRIME_NUMBER = 37;

    struct BusCompare final {
        bool operator()(const transport::domains::Bus *left, const transport::domains::Bus *right) const {
            return left->name < right->name;
        }
    };

    struct TwoStopsHasher {
        size_t operator()(const std::pair<transport::domains::Stop *, transport::domains::Stop *> stops) const {
            return reinterpret_cast<size_t>(stops.first) + reinterpret_cast<size_t>(stops.second) * PRIME_NUMBER;
        }
    };

    // queries

    class Query {
    public:
        std::string name;

        explicit Query(std::string_view name) : name(name) {}
    };

    class BusQuery : public Query {
    public:
        bool is_annular;
        const std::vector<std::string> stops;

        BusQuery(std::string_view name, bool is_annular, std::vector<std::string> stops) : Query(name),
                                                                                           is_annular(is_annular),
                                                                                           stops(std::move(stops)) {}
    };

    class StopQuery : public Query {
    public:
        geo::Coordinates coordinates;
        std::vector<std::pair<std::string, int>> distances;

        StopQuery(std::string_view name, double latitude, double longitude,
                  std::vector<std::pair<std::string, int>> distances) : Query(name),
                                                                        coordinates({latitude, longitude}),
                                                                        distances(std::move(distances)) {}
    };

    // Stats

    class StatRequest {
    public:
        using TwoStops = std::pair<std::string, std::string>;

        int id;
        std::string type;
        std::variant<std::string, TwoStops> extra;

        explicit StatRequest(int id, std::string type, std::string name) :
                id(id),
                type(std::move(type)),
                extra(std::move(name)) {}

        explicit StatRequest(int id, std::string type, const std::string &from, const std::string &to) :
                id(id),
                type(std::move(type)),
                extra(std::move(std::make_pair(from, to))) {}
    };
}
