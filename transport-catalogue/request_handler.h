#pragma once

#include "domain.h"
#include "transport_catalogue.h"

#include <map>

namespace transport::interfaces {
    class IReader {
    public:
        virtual void Load(std::istream &input) = 0;

        [[nodiscard]] virtual std::string
        ConvertStatRequests(const transport::catalogue::TransportCatalogue &db,
                            const std::deque<transport::domains::StatRequest> &stats,
                            renderer::MapRenderer::RenderSettings renderSettings) const = 0;

        // getters
        [[nodiscard]] std::deque<transport::domains::StopQuery> GetStopQueries() const { return stopQueries; }

        [[nodiscard]] std::deque<transport::domains::BusQuery> GetBusQueries() const { return busQueries; }

        [[nodiscard]] std::deque<transport::domains::StatRequest> GetStatRequests() const { return statRequests; }

        [[nodiscard]] renderer::MapRenderer::RenderSettings GetRenderSettings() const { return renderSettings; }

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
    };
}

namespace transport::handlers {
    class RequestHandler {
    public:

        explicit RequestHandler(transport::catalogue::TransportCatalogue &db);

        // adding, editing, loading data

        void ExecuteQueries(const std::deque<transport::domains::StopQuery> &stopQueries,
                            const std::deque<transport::domains::BusQuery> &busQueries);

        void ExecuteQueriesAsync(const std::deque<transport::domains::StopQuery> &stopQueries,
                                 const std::deque<transport::domains::BusQuery> &busQueries);

        std::string
        ExecuteStats(const std::deque<transport::domains::StatRequest> &statRequests,
                     const interfaces::IReader &reader);

        std::string
        ExecuteStatsAsync(const std::deque<transport::domains::StatRequest> &statRequests,
                          const interfaces::IReader &reader);

        struct GetCoordinateStops {
            std::vector<geo::Coordinates> coordinate;
            std::vector<std::string> name;
            std::map<std::string, geo::Coordinates> name_coord;

            std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> polyline;
            std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> name_route_inform;
        };

        static GetCoordinateStops GetStopsWithRoute(const transport::catalogue::TransportCatalogue &catalogue);

        static std::string RenderMap(const transport::catalogue::TransportCatalogue &catalogue,
                                     renderer::MapRenderer::RenderSettings renderSettings = {});


    private:
        transport::catalogue::TransportCatalogue &db_;
    };
}





