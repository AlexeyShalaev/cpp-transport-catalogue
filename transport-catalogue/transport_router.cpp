#include "transport_router.h"

using namespace transport::router;
using namespace graph;
using namespace std;

set<const transport::domains::Bus *> TransportRouter::FillGraphWithBuses() {
    set<const transport::domains::Bus *> bus_routes;

    for (const auto &[_, bus]: transportCatalogue_.GetBuses()) {
        bus_routes.insert(bus);
    }

    return bus_routes;
}

void TransportRouter::CreateGraph(graph::DirectedWeightedGraph<double> &directedWeightedGraph) {
    FillGraphWithStops();
    auto get_all_bus_route = FillGraphWithBuses();
    for (const transport::domains::Bus *item: get_all_bus_route) {
        size_t size = item->stops.size();
        for (size_t i = 0; i < size - 1; ++i) {
            double distance = 0.0;
            for (size_t j = i + 1; j < size; ++j) {
                auto find_stop_first = item->stops.at(j - 1u);
                auto find_stop_second = item->stops.at(j);

                if (item->stops.at(i) == item->stops.at(j)) {
                    continue;
                }

                distance += transportCatalogue_.DistanceBetweenStops(find_stop_first, find_stop_second);
                AddEdges(directedWeightedGraph,
                         GraphInfoStops{
                                 item->name,
                                 item->stops.at(i)->name,
                                 item->stops.at(j)->name,
                                 distance,
                                 static_cast<int>(j - i)
                         }
                );
            }
        }
    }
}


void TransportRouter::AddEdges(graph::DirectedWeightedGraph<double> &directedWeightedGraph,
                               const GraphInfoStops &graphInfoStops) {

    VertexId stop_from_vertex_id = stop_name_vertex_id_[std::string(graphInfoStops.stop_from)];
    VertexId stop_to_vertex_id = stop_name_vertex_id_[std::string(graphInfoStops.stop_to)];

    Edge<double> edge_{};
    edge_.from = stop_from_vertex_id;
    edge_.to = stop_to_vertex_id;
    edge_.weight = graphInfoStops.width / RoutingSettings_.bus_velocity_ + RoutingSettings_.bus_wait_time_;

    auto edge_id = directedWeightedGraph.AddEdge(edge_);
    data_.push_back({graphInfoStops.bus_num, graphInfoStops.stop_from, graphInfoStops.stop_to, edge_.weight,
                     graphInfoStops.span_count});

    assert(edge_id == data_.size() - 1);
}

void TransportRouter::FillGraphWithStops() {
    size_t vertex_index = 0;
    for (const auto &[stop, _]: transportCatalogue_.GetStops()) {
        stop_name_vertex_id_.insert({std::string(stop), vertex_index++});
    }
}


optional<TransportRouter::ResponseFindRoute>
TransportRouter::FindRoute(string_view stop_from, string_view stop_to) const {
    auto it_vertex_from = stop_name_vertex_id_.find(std::string(stop_from));
    if (it_vertex_from == stop_name_vertex_id_.end()) {
        return nullopt;
    }

    auto it_vertex_to = stop_name_vertex_id_.find(std::string(stop_to));
    if (it_vertex_to == stop_name_vertex_id_.end()) {
        return nullopt;
    }

    VertexId vertex_from = it_vertex_from->second;
    VertexId vertex_to = it_vertex_to->second;
    auto get_build = router_->BuildRoute(vertex_from, vertex_to);
    if (get_build == nullopt) {
        return nullopt;
    }

    ResponseFindRoute result;
    result.weight_ = get_build->weight;

    DataBuild data_build;
    for (const auto &v: get_build->edges) {
        data_build.bus_num_ = data_[v].bus_num;
        data_build.stop_name_ = data_[v].stop_from;
        data_build.time_ = data_[v].width;
        data_build.type_ = GetItemType(ItemType::Bus);
        data_build.span_count_ = data_[v].span_count;

        result.busnum.push_back(data_build);
    }

    return result;
}