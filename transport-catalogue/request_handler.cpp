#include <sstream>
#include <utility>
#include "request_handler.h"

using namespace transport::handlers;

RequestHandler::RequestHandler(transport::catalogue::TransportCatalogue &db) : db_(db) {}

// adding, editing, loading data

void RequestHandler::ExecuteQueries(const std::deque<transport::domains::StopQuery> &stopQueries,
                                    const std::deque<transport::domains::BusQuery> &busQueries) {
    for_each(stopQueries.begin(), stopQueries.end(),
             [&](const auto &stop_query) {
                 db_.AddStop(stop_query.name, stop_query.coordinates);
             });

    for_each(stopQueries.begin(), stopQueries.end(),
             [&](const auto &stop_query) {
                 db_.AddStopDistances(stop_query.name, std::move(stop_query.distances));
             });

    for_each(busQueries.begin(), busQueries.end(),
             [&](const auto &bus_query) {
                 db_.AddBus(bus_query.name, bus_query.is_annular, std::move(bus_query.stops));
             });
}

std::future<void> RequestHandler::ExecuteQueriesAsync(const std::deque<transport::domains::StopQuery> &stopQueries,
                                                      const std::deque<transport::domains::BusQuery> &busQueries) {
    return std::async([&] {
        ExecuteQueries(stopQueries, busQueries);
    });
}

std::string RequestHandler::ExecuteStats(const std::deque<transport::domains::StatRequest> &statRequests,
                                         const transport::interfaces::IReader &reader) {
    return reader.ConvertStatRequests(db_, statRequests, reader.GetRenderSettings(), reader.GetRoutingSettings());
}

std::future<std::string>
RequestHandler::ExecuteStatsAsync(const std::deque<transport::domains::StatRequest> &statRequests,
                                  const transport::interfaces::IReader &reader) {
    return std::async([&] {
        return reader.ConvertStatRequests(db_, statRequests, reader.GetRenderSettings(), reader.GetRoutingSettings());
    });
}

renderer::GetCoordinateStops
RequestHandler::GetStopsWithRoute(const transport::catalogue::TransportCatalogue &catalogue) {
    renderer::GetCoordinateStops getCoordinateStops;

    std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> polyline;
    std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> name_route_inform;
    std::deque<const transport::domains::Stop *> sort_bus_name;

    const auto &buses = catalogue.GetBuses();

    for (const auto &[number, bus]: buses) {
        const auto string_number = std::string(number);
        const auto first_stop = bus->stops.front();
        name_route_inform[string_number].push_back({first_stop->name, first_stop->coordinates});

        if (!bus->is_annular_trip) {
            if (bus->stops.back() != bus->stops[bus->stops.size() / 2]) {
                auto last_stop = bus->stops[bus->stops.size() / 2];
                name_route_inform[string_number].push_back({last_stop->name, last_stop->coordinates});
            }
        }

        for (const auto stop: bus->stops) {
            sort_bus_name.push_back(stop);
            polyline[string_number].push_back({stop->name, stop->coordinates});
        }
    }

    std::map<std::string, geo::Coordinates> res;
    std::vector<geo::Coordinates> coord;
    coord.reserve(buses.size());

    for (const auto stop: sort_bus_name) {
        res.insert({stop->name, stop->coordinates});
        coord.push_back(stop->coordinates);
    }

    getCoordinateStops.polyline = polyline;
    getCoordinateStops.name_route_inform = name_route_inform;
    getCoordinateStops.name_coord = res;
    getCoordinateStops.coordinate = coord;

    return getCoordinateStops;
}

std::string RequestHandler::RenderMap(const transport::catalogue::TransportCatalogue &catalogue,
                                      renderer::MapRenderer::RenderSettings renderSettings) {
    renderer::MapRenderer mapRenderer(std::move(renderSettings));
    std::ostringstream out;
    mapRenderer.Render(RequestHandler::GetStopsWithRoute(catalogue), out);
    return out.str();
}

std::optional<transport::router::TransportRouter::ResponseFindRoute>
RequestHandler::FindRoute(const transport::catalogue::TransportCatalogue &catalogue,
                          router::TransportRouter::RoutingSettings routingSettings,
                          std::string_view stop_from, std::string_view stop_to) {
    router::TransportRouter transportRouter(catalogue, routingSettings);
    graph::DirectedWeightedGraph<double> directedWeightedGraph_(catalogue.GetStops().size());
    transportRouter.CreateGraph(directedWeightedGraph_);
    auto router = std::make_shared<graph::Router<double>>(directedWeightedGraph_);
    transportRouter.SetRouter(router);
    return transportRouter.FindRoute(stop_from, stop_to);
}



