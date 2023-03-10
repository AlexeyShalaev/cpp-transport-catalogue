#include <algorithm>
#include <iostream>
#include <utility>
#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue {
    void TransportCatalogue::AddStop(string name, geo::Coordinates coordinates) {
        stops_.push_back({std::move(name), coordinates});
        stops_map_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddStopDistances(string_view name, const vector <pair<string, int>> &distances) {
        if (stops_map_.count(name) == 0) throw std::invalid_argument("Invalid Stop in Distances");
        const auto &from_stop = stops_map_.at(name);
        for (auto &[stop_name, distance]: distances) {
            if (stops_map_.count(stop_name) == 0) throw std::invalid_argument("Invalid Stop in Distances");
            distances_[{from_stop, stops_map_.at(stop_name)}] = distance;
        }
    }

    void TransportCatalogue::AddBus(const string &name, bool annular, const vector <string> &stops) {
        Bus bus;
        for (auto &stop: stops) {
            if (stops_map_.count(stop) == 0) throw std::invalid_argument("Invalid Stop in Bus");
            bus.stops.push_back(stops_map_.at(stop));
        }
        if (!annular) {
            for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
                bus.stops.push_back(stops_map_.at(*it));
            }
        }
        bus.name = name;
        buses_.push_back(bus);
        buses_map_[buses_.back().name] = &buses_.back();
        buses_map_[buses_.back().name]->route_length = RouteLength(name);;
        InsertBusesToStop(&buses_.back());
    }

    const Bus *TransportCatalogue::FindBus(std::string_view name) const {
        return (buses_map_.count(name) > 0) ? buses_map_.at(name) : nullptr;
    }

    const Stop *TransportCatalogue::FindStop(std::string_view name) const {
        return (stops_map_.count(name) > 0) ? stops_map_.at(name) : nullptr;
    }

    int TransportCatalogue::DistanceBetweenStops(Stop *stop1, Stop *stop2) const {
        auto pair_stop = std::pair(stop1, stop2);
        if (distances_.count(pair_stop) > 0) return distances_.at(pair_stop);
        pair_stop = std::pair(stop2, stop1);
        if (distances_.count(pair_stop) > 0) return distances_.at(pair_stop);
        return 0;
    }

    int TransportCatalogue::DistanceBetweenStops(string_view stop1, string_view stop2) const {
        if (stops_map_.count(stop1) == 0 || stops_map_.count(stop2) == 0)
            throw std::invalid_argument("Invalid Stop in Distances");
        return DistanceBetweenStops(stops_map_.at(stop1),
                                    stops_map_.at(stop2));
    }

    int TransportCatalogue::RouteLength(const std::string_view name) const {
        if (buses_map_.count(name) == 0) {
            return 0;
        }
        int result = 0;
        Stop *from_stop;
        bool is_first_stop = true;
        for (auto &stop: buses_map_.at(name)->stops) {
            if (is_first_stop) {
                from_stop = stop;
                is_first_stop = false;
                continue;
            }
            result += DistanceBetweenStops(from_stop, stop);
            from_stop = stop;
        }
        return result;
    }

    double TransportCatalogue::RouteGeoLength(string_view name) const {
        if (buses_map_.count(name) == 0) return 0.0;
        double result = 0.0;
        double prev_latitude;
        double prev_longitude;
        bool is_first_stop = true;
        for (auto &stop: buses_map_.at(name)->stops) {
            if (is_first_stop) {
                prev_latitude = stop->coordinates.lat;
                prev_longitude = stop->coordinates.lng;
                is_first_stop = false;
                continue;
            }
            result += geo::ComputeDistance(
                    {prev_latitude, prev_longitude},
                    {stop->coordinates.lat, stop->coordinates.lng});
            prev_latitude = stop->coordinates.lat;
            prev_longitude = stop->coordinates.lng;
        }
        return result;
    }

    StopInfo TransportCatalogue::GetStopInfo(string_view name) const {
        if (stops_map_.count(name) > 0) {
            Stop *stop = stops_map_.at(name);
            if (stop_buses_map_.count(stop) > 0) {
                auto &buses_at_stop = stop_buses_map_.at(stop);
                return {string(name), true,
                        {buses_at_stop.begin(), buses_at_stop.end()}};
            } else {
                return {string(name), true, {}};
            }
        }
        return {string(name), false, {}};
    }

    BusInfo TransportCatalogue::GetBusInfo(string_view name) const {
        int route_stops = 0;
        int unique_stops = 0;
        int length = 0;
        double curvature = 0.0;
        if (buses_map_.count(name) > 0) {
            const auto &bus = buses_map_.at(name);
            const auto &stops = bus->stops;
            route_stops = stops.size();
            std::unordered_set<Stop *> tmp_stops = {stops.begin(), stops.end()};
            unique_stops = tmp_stops.size();
            length = bus->route_length;
            curvature = length / RouteGeoLength(name);
        }
        return {string(name), route_stops, unique_stops, length, curvature};
    }

    const unordered_map<string_view, Stop *> &TransportCatalogue::GetStops() const { return stops_map_; }

    const unordered_map<string_view, Bus *> &TransportCatalogue::GetBuses() const { return buses_map_; }

    const unordered_map<Stop *, BusSet> &TransportCatalogue::GetStopBuses() const { return stop_buses_map_; }

    const unordered_map<pair<Stop *, Stop *>, int, TwoStopsHasher> &
    TransportCatalogue::GetDistances() const { return distances_; }

    void TransportCatalogue::Clear() {
        for (auto &[_, bus]: stop_buses_map_) bus.clear();
        stop_buses_map_.clear();
        distances_.clear();
        stops_map_.clear();
        stops_.clear();
        for (auto &[_, bus]: buses_map_) bus->stops.clear();
        buses_map_.clear();
        buses_.clear();
    }

    void TransportCatalogue::InsertBusesToStop(Bus *bus) {
        for (const auto &stop: bus->stops) stop_buses_map_[stop].insert(bus);
    }

}