#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>


namespace transport::catalogue {

    using BusSet = std::set<transport::domains::Bus *, transport::domains::BusCompare>;

    class TransportCatalogue {
    public:

        // Add data

        void AddStop(std::string name, geo::Coordinates coordinates);

        void AddBus(const std::string &name, bool annular, const std::vector<std::string> &stops);

        void AddStopDistances(std::string_view name, const std::vector<std::pair<std::string, int>> &distances);

        // Find data

        const transport::domains::Stop *FindStop(std::string_view name) const;

        const transport::domains::Bus *FindBus(std::string_view name) const;

        // Calculating

        int DistanceBetweenStops(std::string_view stop1, std::string_view stop2) const;

        int DistanceBetweenStops(transport::domains::Stop *stop1, transport::domains::Stop *stop2) const;

        double RouteGeoLength(std::string_view name) const;

        int RouteLength(std::string_view name) const;

        // Getters

        transport::domains::BusStat GetBusInfo(std::string_view name) const;

        transport::domains::StopStat GetStopInfo(std::string_view name) const;

        const std::unordered_map<std::string_view, transport::domains::Stop *> &GetStops() const;

        const std::unordered_map<std::string_view, transport::domains::Bus *> &GetBuses() const;

        const std::unordered_map<transport::domains::Stop *, BusSet> &GetStopBuses() const;

        const std::unordered_map<std::pair<transport::domains::Stop *, transport::domains::Stop *>, int, transport::domains::TwoStopsHasher> &
        GetDistances() const;

        // Extra

        void Clear();

        [[nodiscard]] std::optional<transport::catalogue::BusSet> GetBusesByStop(const std::string_view stop_name) const;

    private:
        std::deque<transport::domains::Stop> stops_;
        std::unordered_map<std::string_view, transport::domains::Stop *> stops_map_;

        std::deque<transport::domains::Bus> buses_;
        std::unordered_map<std::string_view, transport::domains::Bus *> buses_map_;

        std::unordered_map<transport::domains::Stop *, BusSet> stop_buses_map_;
        std::unordered_map<std::pair<transport::domains::Stop *, transport::domains::Stop *>, int, transport::domains::TwoStopsHasher> distances_;

        void InsertBusesToStop(transport::domains::Bus *bus);

    };
}