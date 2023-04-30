#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <future>
#include <map>

namespace transport::interfaces {
    class IReader {
    public:
        virtual void Load(std::istream &input) = 0;

        [[nodiscard]] virtual std::string
        ConvertStatRequests(const transport::catalogue::TransportCatalogue &db,
                            const std::deque<transport::domains::StatRequest> &stats,
                            std::optional<renderer::MapRenderer::RenderSettings> renderSettings,
                            std::optional<transport::router::TransportRouter::RoutingSettings> routingSettings) const = 0;

        // getters
        [[nodiscard]] std::deque<transport::domains::StopQuery> GetStopQueries() const { return stopQueries; }

        [[nodiscard]] std::deque<transport::domains::BusQuery> GetBusQueries() const { return busQueries; }

        [[nodiscard]] std::deque<transport::domains::StatRequest> GetStatRequests() const { return statRequests; }

        [[nodiscard]] renderer::MapRenderer::RenderSettings GetRenderSettings() const { return renderSettings; }

        [[nodiscard]] transport::router::TransportRouter::RoutingSettings
        GetRoutingSettings() const { return routingSettings; }

        // des

        virtual ~IReader() = default;

    protected:
        // loading
        std::deque<transport::domains::StopQuery> stopQueries;
        std::deque<transport::domains::BusQuery> busQueries;

        // stat
        std::deque<transport::domains::StatRequest> statRequests;

        // rendering
        renderer::MapRenderer::RenderSettings renderSettings;

        //routing
        transport::router::TransportRouter::RoutingSettings routingSettings;

    };
}

namespace transport::handlers {
    class RequestHandler {
    public:

        explicit RequestHandler(transport::catalogue::TransportCatalogue &db);

        // adding, editing, loading data

        void ExecuteQueries(const std::deque<transport::domains::StopQuery> &stopQueries,
                            const std::deque<transport::domains::BusQuery> &busQueries);

        std::future<void> ExecuteQueriesAsync(const std::deque<transport::domains::StopQuery> &stopQueries,
                                              const std::deque<transport::domains::BusQuery> &busQueries);

        std::string
        ExecuteStats(const std::deque<transport::domains::StatRequest> &statRequests,
                     const interfaces::IReader &reader);


        std::future<std::string>
        ExecuteStatsAsync(const std::deque<transport::domains::StatRequest> &statRequests,
                          const interfaces::IReader &reader);

        static renderer::GetCoordinateStops
        GetStopsWithRoute(const transport::catalogue::TransportCatalogue &catalogue);

        static std::string RenderMap(const transport::catalogue::TransportCatalogue &catalogue,
                                     renderer::MapRenderer::RenderSettings renderSettings);

        static std::optional<router::TransportRouter::ResponseFindRoute>
        FindRoute(const transport::catalogue::TransportCatalogue &catalogue,
                  transport::router::TransportRouter::RoutingSettings routingSettings,
                  std::string_view stop_from, std::string_view stop_to);

    private:
        transport::catalogue::TransportCatalogue &db_;
    };
}
