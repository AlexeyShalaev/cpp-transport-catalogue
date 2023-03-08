#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>


namespace transport_catalogue {

    const size_t PRIME_NUMBER = 37;

    // Models

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::deque<Stop *> stops;
    };


    // InfoModels

    struct BusInfo {
        std::string name;
        int route_stops;
        int unique_stops;
        int length;
        double curvature;
    };

    struct StopInfo {
        std::string name;
        bool exists;
        std::vector<Bus *> buses;
    };

    // Utils

    struct BusCompare final {
        bool operator()(const Bus *left, const Bus *right) const {
            return left->name < right->name;
        }
    };

    using BusSet = std::set<Bus *, BusCompare>;

    struct TwoStopsHasher {
        size_t operator()(const std::pair<Stop *, Stop *> stops) const {
            return reinterpret_cast<size_t>(stops.first) + reinterpret_cast<size_t>(stops.second) * PRIME_NUMBER;
        }
    };


    class TransportCatalogue {
    public:

        // Add data

        void AddStop(std::string name, geo::Coordinates coordinates);

        void AddBus(std::string name, bool annular, const std::vector<std::string> &stops);

        void AddStopDistances(std::string_view name, const std::vector<std::pair<std::string, int>> &distances);

        // Find data

        const Stop *FindStop(std::string_view name) const;

        const Bus *FindBus(std::string_view name) const;

        // Calculating

        int DistanceBetweenStops(std::string_view stop1, std::string_view stop2) const;

        int DistanceBetweenStops(Stop *stop1, Stop *stop2) const;

        double RouteGeoLength(std::string_view name) const;

        int RouteLength(std::string_view name) const;

        // Getters

        BusInfo GetBusInfo(std::string_view name) const;

        StopInfo GetStopInfo(std::string_view name) const;

        const std::unordered_map<std::string_view, Stop *> &GetStops() const;

        const std::unordered_map<std::string_view, Bus *> &GetBuses() const;

        const std::unordered_map<Stop *, BusSet> &GetStopBuses() const;

        const std::unordered_map<std::pair<Stop *, Stop *>, int, TwoStopsHasher> &GetDistances() const;

        // Extra

        void Clear();

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop *> stops_map_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus *> buses_map_;

        std::unordered_map<Stop *, BusSet> stop_buses_map_;
        std::unordered_map<std::pair<Stop *, Stop *>, int, TwoStopsHasher> distances_;

        void InsertBusesToStop(Bus *bus);

    };
}