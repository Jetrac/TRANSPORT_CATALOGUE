#pragma once

#include "json.h"
#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.pb.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace serialization {
    void Serialize(
            const transport_catalogue::TransportCatalogue &catalogue,
            const renderer::MapRenderer &renderer,
            const transport_router::TransportRouter &router,
            std::ostream &out
    );

    void SerializeStops(
            const transport_catalogue::TransportCatalogue &catalogue,
            proto_transport::Catalogue &proto_catalogue
    );

    void SerializeStopDistances(
            const transport_catalogue::TransportCatalogue &catalogue,
            proto_transport::Catalogue &proto_catalogue
    );

    void SerializeBuses(
            const transport_catalogue::TransportCatalogue &catalogue,
            proto_transport::Catalogue &proto_catalogue
    );

    void SerializeRenderSettings(
            const renderer::MapRenderer &renderer,
            proto_transport::Catalogue &proto_catalogue);

    proto_map::Point SerializePoint(const svg::Point &point);

    proto_map::Color SerializeColor(const svg::Color &color);

    proto_map::Rgb SerializeRgb(const svg::Rgb &rgb);

    proto_map::Rgba SerializeRgba(const svg::Rgba &rgba);

    void SerializeRouter(
            const transport_router::TransportRouter &router,
            proto_transport::Catalogue &proto_catalogue
    );

    proto_transport::RoutingSettings SerializeRoutingSettings(const transport_router::TransportRouter &router);

    proto_graph::Graph SerializeGraph(const graph::DirectedWeightedGraph<double> &graph);

    //------------------------------------------------------------------------------------------------------------------
    std::tuple<
            transport_catalogue::TransportCatalogue,
            renderer::MapRenderer,
            transport_router::TransportRouter,
            graph::DirectedWeightedGraph<double>,
            std::map<std::string, graph::VertexId>
    > Deserialize(std::istream &input);

    void DeserializeStops(
            transport_catalogue::TransportCatalogue &catalogue,
            const proto_transport::Catalogue &proto_catalogue
    );

    void DeserializeStopDistances(
            transport_catalogue::TransportCatalogue &catalogue,
            const proto_transport::Catalogue &proto_catalogue
    );

    void DeserializeBuses(
            transport_catalogue::TransportCatalogue &catalogue,
            const proto_transport::Catalogue &proto_catalogue
    );

    renderer::MapRenderer::RenderSettings DeserializeRenderSettings(const proto_transport::Catalogue &proto_catalogue);

    transport_router::TransportRouter::RoutingSettings DeserializeRoutingSettings(
            const proto_transport::Catalogue &proto_catalogue
            );

    graph::DirectedWeightedGraph<double> DeserializeGraph(const proto_transport::Catalogue &proto_catalogue);

    std::map<std::string, graph::VertexId> DeserializeStopIds(const proto_transport::Catalogue &proto_catalogue);

    svg::Point DeserializePoint(const proto_map::Point &proto_point);

    svg::Color DeserializeColor(const proto_map::Color &proto_color);

} // serialization
