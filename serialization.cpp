#include "serialization.h"

#include <string>
#include <fstream>

using namespace std;

namespace serialization {
    void Serialize(
            const transport_catalogue::TransportCatalogue &catalogue,
            const renderer::MapRenderer &renderer,
            const transport_router::TransportRouter &router,
            std::ostream &out
    ) {
        proto_transport::Catalogue proto_catalogue;

        SerializeStops(catalogue, proto_catalogue);
        SerializeStopDistances(catalogue, proto_catalogue);
        SerializeBuses(catalogue, proto_catalogue);
        SerializeRenderSettings(renderer, proto_catalogue);
        SerializeRouter(router, proto_catalogue);

        proto_catalogue.SerializeToOstream(&out);
    }

    void SerializeStops(
            const transport_catalogue::TransportCatalogue &catalogue,
            proto_transport::Catalogue &proto_catalogue
    ) {
        const auto all_stops = catalogue.GetSortedStops();
        for (const auto &[stop_name, stop]: all_stops) {
            proto_transport::Stop proto_stop;
            proto_stop.set_stop_name(stop->stop_name_);
            proto_stop.mutable_coordinates()->set_lat(stop->coordinates_.lat);
            proto_stop.mutable_coordinates()->set_lng(stop->coordinates_.lng);

            *proto_catalogue.add_stops() = std::move(proto_stop);
        }
    }

    void SerializeBuses(
            const transport_catalogue::TransportCatalogue &catalogue,
            proto_transport::Catalogue &proto_catalogue
    ) {
        const auto all_buses = catalogue.GetSortedBuses();
        for (const auto &[bus_name, bus]: all_buses) {
            proto_transport::Bus proto_bus;

            proto_bus.set_bus_name(bus->bus_name_);
            for (const auto *stop: bus->bus_route_) {
                *proto_bus.mutable_bus_route()->Add() = stop->stop_name_;
            }
            proto_bus.set_is_roundtrip(bus->is_roundtrip_);

            *proto_catalogue.add_buses() = std::move(proto_bus);
        }
    }

    void SerializeStopDistances(
            const transport_catalogue::TransportCatalogue &catalogue,
            proto_transport::Catalogue &proto_catalogue
    ) {
        for (const auto &[stop_pair, distance]: catalogue.GetStopDistances()) {
            proto_transport::StopsDistance proto_stop_distances;

            proto_stop_distances.set_first_stop(stop_pair.first->stop_name_);
            proto_stop_distances.set_second_stop(stop_pair.second->stop_name_);
            proto_stop_distances.set_distance(distance);

            *proto_catalogue.add_stops_to_distance() = std::move(proto_stop_distances);
        }
    }

    void SerializeRenderSettings(
            const renderer::MapRenderer &renderer,
            proto_transport::Catalogue &proto_catalogue
    ) {
        renderer::MapRenderer::RenderSettings render_settings = renderer.GetRenderSettings();
        proto_map::RenderSettings proto_rs;
        proto_rs.set_width(render_settings.width_);
        proto_rs.set_height(render_settings.height_);
        proto_rs.set_padding(render_settings.padding_);
        proto_rs.set_stop_radius(render_settings.stop_radius_);
        proto_rs.set_line_width(render_settings.line_width_);
        proto_rs.set_bus_label_font_size(render_settings.bus_label_font_size_);
        *proto_rs.mutable_bus_label_offset() = SerializePoint(render_settings.bus_label_offset_);
        proto_rs.set_stop_label_font_size(render_settings.stop_label_font_size_);
        *proto_rs.mutable_stop_label_offset() = SerializePoint(render_settings.stop_label_offset_);
        *proto_rs.mutable_underlayer_color() = SerializeColor(render_settings.underlayer_color_);
        proto_rs.set_underlayer_width(render_settings.underlayer_width_);
        for (const auto &color: render_settings.color_palette) {
            *proto_rs.add_color_palette() = SerializeColor(color);
        }
        *proto_catalogue.mutable_render_settings() = std::move(proto_rs);
    }

    proto_map::Point SerializePoint(const svg::Point &point) {
        proto_map::Point proto_point;
        proto_point.set_x(point.x);
        proto_point.set_y(point.y);

        return proto_point;
    }

    proto_map::Color SerializeColor(const svg::Color &color) {
        proto_map::Color proto_color;
        switch (color.index()) {
            case 1: {
                proto_color.set_name(std::get<1>(color));
                break;
            }
            case 2: {
                *proto_color.mutable_rgb() = SerializeRgb(std::get<2>(color));
                break;
            }
            case 3: {
                *proto_color.mutable_rgba() = SerializeRgba(std::get<3>(color));
                break;
            }
        }
        return proto_color;
    }

    proto_map::Rgb SerializeRgb(const svg::Rgb &rgb) {
        proto_map::Rgb proto_rgb;
        proto_rgb.set_red(rgb.red);
        proto_rgb.set_blue(rgb.blue);
        proto_rgb.set_green(rgb.green);

        return proto_rgb;
    }

    proto_map::Rgba SerializeRgba(const svg::Rgba &rgba) {
        proto_map::Rgba proto_rgba;
        proto_rgba.set_red(rgba.red);
        proto_rgba.set_blue(rgba.blue);
        proto_rgba.set_green(rgba.green);
        proto_rgba.set_opacity(rgba.opacity);

        return proto_rgba;
    }

    void SerializeRouter(
            const transport_router::TransportRouter &router,
            proto_transport::Catalogue &proto_catalogue
    ) {
        proto_transport::Router proto_router;
        *proto_router.mutable_routing_settings() = SerializeRoutingSettings(router);
        *proto_router.mutable_graph() = SerializeGraph(router.GetGraph());
        for (const auto &[name, id]: router.GetStopIds()) {
            proto_transport::StopId proto_stop_id;
            proto_stop_id.set_name(name);
            proto_stop_id.set_id(id);

            *proto_router.add_stop_ids() = proto_stop_id;
        }
        *proto_catalogue.mutable_router() = std::move(proto_router);
    }

    proto_transport::RoutingSettings SerializeRoutingSettings(const transport_router::TransportRouter &router) {
        proto_transport::RoutingSettings proto_router_settings;

        transport_router::TransportRouter::RoutingSettings routing_settings = router.GetRoutingSettings();
        proto_router_settings.set_bus_wait_time(routing_settings.bus_wait_time_);
        proto_router_settings.set_bus_velocity(routing_settings.bus_velocity_);

        return proto_router_settings;
    }

    proto_graph::Graph SerializeGraph(const graph::DirectedWeightedGraph<double> &graph) {
        proto_graph::Graph protobuf_graph;
        for (int i = 0; i < graph.GetEdgeCount(); ++i) {
            graph::Edge edge = graph.GetEdge(i);
            proto_graph::Edge proto_edge;
            proto_edge.set_name(edge.name);
            proto_edge.set_quality(edge.quality);
            proto_edge.set_from(edge.from);
            proto_edge.set_to(edge.to);
            proto_edge.set_weight(edge.weight);

            *protobuf_graph.add_edge() = proto_edge;
        }

        for (int i = 0; i < graph.GetVertexCount(); ++i) {
            proto_graph::Vertex proto_vertex;
            for (const auto &edge_id: graph.GetIncidentEdges(i)) {
                proto_vertex.add_edge_id(edge_id);
            }
            *protobuf_graph.add_vertex() = proto_vertex;
        }
        return protobuf_graph;
    }

    //------------------------------------------------------------------------------------------------------------------
    std::tuple<
            transport_catalogue::TransportCatalogue,
            renderer::MapRenderer,
            transport_router::TransportRouter,
            graph::DirectedWeightedGraph<double>,
            std::map<std::string, graph::VertexId>
    > Deserialize(std::istream &input) {
        proto_transport::Catalogue proto_catalogue;
        proto_catalogue.ParseFromIstream(&input);

        transport_catalogue::TransportCatalogue catalogue;

        DeserializeStops(catalogue, proto_catalogue);
        DeserializeBuses(catalogue, proto_catalogue);
        DeserializeStopDistances(catalogue, proto_catalogue);

        renderer::MapRenderer renderer{
                DeserializeRenderSettings(proto_catalogue),
                catalogue.GetSortedBuses()
        };

        transport_router::TransportRouter router(DeserializeRoutingSettings(proto_catalogue));
        router.FillRouter(
                DeserializeGraph(proto_catalogue),
                DeserializeStopIds(proto_catalogue)
        );

        return {
                std::move(catalogue),
                std::move(renderer),
                std::move(router)
        };
    }

    void DeserializeStops(
            transport_catalogue::TransportCatalogue &catalogue,
            const proto_transport::Catalogue &proto_catalogue
    ) {
        for (int i = 0; i < proto_catalogue.stops_size(); ++i) {
            proto_transport::Stop proto_stop = proto_catalogue.stops(i);
            catalogue.AddStop(
                    {
                            proto_stop.stop_name(),
                            {
                                    proto_stop.coordinates().lat(),
                                    proto_stop.coordinates().lng()
                            }
                    }
            );
        }
    }

    void DeserializeBuses(
            transport_catalogue::TransportCatalogue &catalogue,
            const proto_transport::Catalogue &proto_catalogue
    ) {
        for (int i = 0; i < proto_catalogue.buses_size(); ++i) {
            proto_transport::Bus proto_bus = proto_catalogue.buses(i);

            std::vector<domain::Stop *> bus_route;
            for (int j = 0; j < proto_bus.bus_route_size(); ++j) {
                bus_route.push_back(catalogue.FindStop(proto_bus.bus_route(j)));
            }
            catalogue.AddBus({proto_bus.bus_name(), bus_route, proto_bus.is_roundtrip()});
        }
    }


    void DeserializeStopDistances(
            transport_catalogue::TransportCatalogue &catalogue,
            const proto_transport::Catalogue &proto_catalogue
    ) {
        vector<domain::StopsDistance> stops_to_distance;
        for (int i = 0; i < proto_catalogue.stops_to_distance_size(); ++i) {
            proto_transport::StopsDistance proto_stop_distances = proto_catalogue.stops_to_distance(i);
            stops_to_distance.push_back(
                    {
                            proto_stop_distances.first_stop(),
                            proto_stop_distances.second_stop(),
                            proto_stop_distances.distance()
                    }
            );
        }
        catalogue.CreateStopsToDistance(stops_to_distance);
    }


    renderer::MapRenderer::RenderSettings DeserializeRenderSettings(const proto_transport::Catalogue &proto_catalogue) {
        proto_map::RenderSettings proto_render_settings = proto_catalogue.render_settings();

        renderer::MapRenderer::RenderSettings render_settings;
        render_settings.width_ = proto_render_settings.width();
        render_settings.height_ = proto_render_settings.height();
        render_settings.padding_ = proto_render_settings.padding();
        render_settings.stop_radius_ = proto_render_settings.stop_radius();
        render_settings.line_width_ = proto_render_settings.line_width();
        render_settings.bus_label_font_size_ = proto_render_settings.bus_label_font_size();
        render_settings.bus_label_offset_ = DeserializePoint(proto_render_settings.bus_label_offset());
        render_settings.stop_label_font_size_ = proto_render_settings.stop_label_font_size();
        render_settings.stop_label_offset_ = DeserializePoint(proto_render_settings.stop_label_offset());
        render_settings.underlayer_color_ = DeserializeColor(proto_render_settings.underlayer_color());
        render_settings.underlayer_width_ = proto_render_settings.underlayer_width();
        for (int i = 0; i < proto_render_settings.color_palette_size(); ++i) {
            render_settings.color_palette.push_back(DeserializeColor(proto_render_settings.color_palette(i)));
        }
        return render_settings;
    }

    svg::Point DeserializePoint(const proto_map::Point &proto_point) {
        return {proto_point.x(), proto_point.y()};
    }

    svg::Color DeserializeColor(const proto_map::Color &proto_color) {
        if (proto_color.has_rgb())
            return svg::Rgb{static_cast<uint8_t>(proto_color.rgb().red()),
                            static_cast<uint8_t>(proto_color.rgb().green()),
                            static_cast<uint8_t>(proto_color.rgb().blue())};
        else if (proto_color.has_rgba())
            return svg::Rgba{static_cast<uint8_t>(proto_color.rgba().red()),
                             static_cast<uint8_t>(proto_color.rgba().green()),
                             static_cast<uint8_t>(proto_color.rgba().blue()),
                             proto_color.rgba().opacity()};
        else return proto_color.name();

        throw std::runtime_error("Error deserialized color");
    }

    transport_router::TransportRouter::RoutingSettings DeserializeRoutingSettings(
            const proto_transport::Catalogue &proto_catalogue
    ) {
        return {
                proto_catalogue.router().routing_settings().bus_wait_time(),
                proto_catalogue.router().routing_settings().bus_velocity()
        };
    }

    graph::DirectedWeightedGraph<double> DeserializeGraph(const proto_transport::Catalogue &proto_catalogue) {
        const proto_graph::Graph &protobuf_graph = proto_catalogue.router().graph();
        std::vector<graph::Edge<double>> edges(protobuf_graph.edge_size());
        std::vector<std::vector<graph::EdgeId>> incidence_lists(protobuf_graph.vertex_size());
        for (int i = 0; i < protobuf_graph.edge_size(); ++i) {
            proto_graph::Edge proto_edge = protobuf_graph.edge(i);
            edges[i] = {
                    proto_edge.name(),
                    static_cast<size_t>(proto_edge.quality()),
                    static_cast<size_t>(proto_edge.from()),
                    static_cast<size_t>(proto_edge.to()),
                    proto_edge.weight()
            };
        }
        for (size_t i = 0; i < incidence_lists.size(); ++i) {
            proto_graph::Vertex proto_vertex = protobuf_graph.vertex(i);
            incidence_lists[i].reserve(proto_vertex.edge_id_size());
            for (const auto &id: proto_vertex.edge_id()) {
                incidence_lists[i].push_back(id);
            }
        }
        return graph::DirectedWeightedGraph<double>(edges, incidence_lists);
    }

    std::map<std::string, graph::VertexId> DeserializeStopIds(const proto_transport::Catalogue &proto_catalogue) {
        std::map<std::string, graph::VertexId> stop_ids;
        for (const proto_transport::StopId &proto_stop_id: proto_catalogue.router().stop_ids()) {
            stop_ids[proto_stop_id.name()] = proto_stop_id.id();
        }
        return stop_ids;
    }
} // serialization
