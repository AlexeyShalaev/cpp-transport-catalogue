#include "domain.h"

#include <ostream>

namespace transport::domains {
    using namespace std::literals;

    void Stop::Parse(const json::Dict &request) {
        name = request.at("name"s).AsString();
        coordinates = {
                request.at("latitude"s).AsDouble(),
                request.at("longitude"s).AsDouble()
        };

        for (const auto &[stop_name, distance]: request.at("road_distances"s).AsMap()) {
            road_distance[stop_name] = distance.AsInt();
        }
    }

    void Bus::Parse(const json::Dict &request) {
        name = request.at("name"s).AsString();
        is_round_trip = request.at("is_roundtrip"s).AsBool();

        for (const auto &stop_name: request.at("stops"s).AsArray()) {
            stops.push_back(stop_name.AsString());
        }
    }

    int RealLenBetweenStops(const std::shared_ptr<Stop> &from, const std::shared_ptr<Stop> &to) {
        if (from == nullptr || to == nullptr) throw std::invalid_argument("stops no find");

        auto it = from->road_distance.find(to->name);
        if (it != from->road_distance.end()) return it->second;

        it = to->road_distance.find(from->name);
        if (it != to->road_distance.end()) return it->second;

        return 0;
    }

}