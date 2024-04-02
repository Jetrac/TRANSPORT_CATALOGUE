#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <utility>

namespace transport_router {
    using namespace graph;
    using namespace transport_catalogue;

    class TransportRouter {
    public:
        struct RoutingSettings {
            int bus_wait_time_ = 0;
            double bus_velocity_ = 0;
        };
    private:
        static constexpr double km_to_min_in_hour = 1000.0 / 60.0;

        RoutingSettings routing_settings_{};
        DirectedWeightedGraph<double> graph_{};
        std::map<std::string, graph::VertexId> stop_ids_{};
        std::unique_ptr<graph::Router<double>> router_ = nullptr;
    public:

        TransportRouter(RoutingSettings routing_settings) : routing_settings_(routing_settings) {}

        const DirectedWeightedGraph<double> &BuildGraph(const TransportCatalogue &transport_catalogue);

        std::optional<Router<double>::RouteInfo> FindRoute(
                std::string_view start_stop,
                std::string_view final_stop
        ) const;

        const Edge<double> &GetGraphEdge(const EdgeId &edge_id) const;

        RoutingSettings GetRoutingSettings() const;

        graph::DirectedWeightedGraph<double> GetGraph() const;

        std::map<std::string, graph::VertexId> GetStopIds() const;

        void FillRouter(
                const graph::DirectedWeightedGraph<double> graph,
                const std::map<std::string, graph::VertexId> stop_ids
        );

    };
}
