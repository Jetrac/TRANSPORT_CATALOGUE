#include "transport_router.h"

#include <string_view>
#include <utility>

using namespace std;

namespace transport_router {

    const DirectedWeightedGraph<double> &TransportRouter::BuildGraph(const TransportCatalogue &transport_catalogue) {
        auto stops = transport_catalogue.GetSortedStops();
        auto buses = transport_catalogue.GetSortedBuses();

        DirectedWeightedGraph<double> temp_graph(stops.size() * 2);

        VertexId vertex_id = 0;

        for (const auto &[stop_name, stop]: stops) {
            stop_ids_[stop_name] = vertex_id;

            temp_graph.AddEdge(
                    {
                            stop_name,
                            0,
                            vertex_id,
                            ++vertex_id,
                            static_cast<double>(routing_settings_.bus_wait_time_)
                    }
            );

            ++vertex_id;
        }

        for (const auto &[bus_name, bus]: buses) {
            int stops_count = static_cast<int>(bus->bus_route_.size());
            for (int i = 0; i < stops_count; ++i) {
                for (int j = i + 1; j < stops_count; ++j) {
                    const domain::Stop *final_stop = bus->bus_route_[i];
                    const domain::Stop *start_stop = bus->bus_route_[j];

                    int route_distance = 0;
                    int back_route_distance = 0;
                    for (int k = i + 1; k <= j; ++k) {
                        route_distance += transport_catalogue.GetDistance(bus->bus_route_[k - 1], bus->bus_route_[k]);
                        back_route_distance += transport_catalogue.GetDistance(bus->bus_route_[k],
                                                                               bus->bus_route_[k - 1]);
                    }
                    temp_graph.AddEdge(
                            {
                                    bus->bus_name_,
                                    static_cast<size_t>(j - i),
                                    stop_ids_.at(final_stop->stop_name_) + 1,
                                    stop_ids_.at(start_stop->stop_name_),
                                    route_distance / (routing_settings_.bus_velocity_ * km_to_min_in_hour)
                            }
                    );

                    if (!bus->is_roundtrip_) {
                        temp_graph.AddEdge(
                                {
                                        bus->bus_name_,
                                        static_cast<size_t>(j - i),
                                        stop_ids_.at(start_stop->stop_name_) + 1,
                                        stop_ids_.at(final_stop->stop_name_),
                                        back_route_distance / (routing_settings_.bus_velocity_ * km_to_min_in_hour)
                                }
                        );
                    }
                }
            }
        }
        graph_ = temp_graph;
        router_ = std::make_unique<graph::Router<double>>(graph_);

        return graph_;
    }

    optional <Router<double>::RouteInfo> TransportRouter::FindRoute(
            string_view start_stop,
            string_view final_stop
    ) const {
        return router_->BuildRoute(stop_ids_.at(std::string(start_stop)), stop_ids_.at(std::string(final_stop)));
    }

    const Edge<double> &TransportRouter::GetGraphEdge(const EdgeId &edge_id) const {
        return graph_.GetEdge(edge_id);
    }

    TransportRouter::RoutingSettings TransportRouter::GetRoutingSettings() const {
        return routing_settings_;
    }

    std::map<std::string, graph::VertexId> TransportRouter::GetStopIds() const {
        return stop_ids_;
    }

    graph::DirectedWeightedGraph<double> TransportRouter::GetGraph() const {
        return graph_;
    }

    void TransportRouter::FillRouter(
            const graph::DirectedWeightedGraph<double> graph,
            const std::map<std::string, graph::VertexId> stop_ids
    ) {
        graph_ = graph;
        stop_ids_ = stop_ids;
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }
}
