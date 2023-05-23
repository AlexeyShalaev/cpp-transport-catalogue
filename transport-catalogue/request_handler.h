#pragma once

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <string>
#include <variant>
#include <vector>

namespace transport::response {

    using namespace transport::catalogue;
    using namespace transport::router;
    using namespace std::literals;

    enum class RequestType {
        STOP, BUS, MAP, ROUTER
    };

    struct Request {
        Request() = default;

        int id = 0;
        std::string name = ""s;
        std::string from = ""s;
        std::string to = ""s;
        RequestType type;
    };


    class RequestHelper {
    public:
        RequestHelper(TransportCatalogue &tc, const json::Array &stat_requests);

        void GetResponses();

        void PrintResponse(std::ostream &out);

    private:
        TransportCatalogue &catalogue_;
        std::vector<Request> requests_;
        json::Array responses_;

        static json::Node CreateJsonResponseError(int request_id);

        static json::Node CreateJsonResponseStop(int request_id, const domains::Stop &data);

        static json::Node CreateJsonResponseBus(int request_id, const domains::Bus &data);

        static json::Node CreateJsonResponseMap(int request_id, const std::string &map_render_data);

        static json::Node CreateJsonResponseRoute(int request_id, const std::shared_ptr<std::vector<RouteItem>> &route);
    };

}