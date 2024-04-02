#include "json_reader.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

namespace json_reader {

    void JsonReader::StopsHandler(const Array &base_requests, TransportCatalogue &transport_catalogue) {
        vector<domain::StopsDistance> stops_distance_info;

        for (const Node &node_map: base_requests) {
            if (node_map.AsMap().at("type"s).AsString() == "Stop"s) {
                string stop_name = node_map.AsMap().at("name"s).AsString();
                transport_catalogue.AddStop(
                        {stop_name,
                         {node_map.AsMap().at("latitude"s).AsDouble(), node_map.AsMap().at("longitude"s).AsDouble()}}
                );

                Dict stops_to_length = node_map.AsMap().at("road_distances").AsMap();

                if (!stops_to_length.empty()) {
                    for (const auto &[stop_on_route, length]: stops_to_length) {
                        stops_distance_info.push_back(
                                {
                                        stop_name,
                                        stop_on_route,
                                        length.AsInt()
                                }
                        );
                    }
                }
            }
        }

        transport_catalogue.CreateStopsToDistance(stops_distance_info);
    }

    void JsonReader::BusesHandler(const Array &base_requests, TransportCatalogue &transport_catalogue) {
        for (const Node &node_map: base_requests) {
            if (node_map.AsMap().at("type"s).AsString() == "Bus"s) {
                string bus_name = node_map.AsMap().at("name"s).AsString();

                Array stops = node_map.AsMap().at("stops"s).AsArray();

                vector<domain::Stop *> bus_route;
                for (const Node &stop_name: stops) {
                    bus_route.push_back(transport_catalogue.FindStop(stop_name.AsString()));
                }

                transport_catalogue.AddBus(
                        {
                                bus_name,
                                bus_route,
                                node_map.AsMap().at("is_roundtrip"s).AsBool()
                        }
                );
            }
        }
    }

    void JsonReader::BaseRequestsHandler(transport_catalogue::TransportCatalogue &transport_catalogue) {
        Array base_requests = document_.GetRoot().AsMap().at("base_requests"s).AsArray();

        StopsHandler(base_requests, transport_catalogue);
        BusesHandler(base_requests, transport_catalogue);
    }

    //------------------------------------------------------------------------------------------------------------------
    svg::Color JsonReader::ColorDeterminant(const Node &node) {
        svg::Color color;
        if (node.IsString()) {
            color = node.AsString();
        } else if (node.IsArray()) {
            Array arr = node.AsArray();
            if (arr.size() == 3) {
                svg::Rgb rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
                color = rgb;
            } else if (arr.size() == 4) {
                svg::Rgba rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
                color = rgba;
            }
        }
        return color;
    }

    renderer::MapRenderer::RenderSettings JsonReader::GetRenderSettings() const {
        Dict render_settings_requests = document_.GetRoot().AsMap().at("render_settings"s).AsMap();

        renderer::MapRenderer::RenderSettings render_settings;

        render_settings.width_ = render_settings_requests.at("width"s).AsDouble();
        render_settings.height_ = render_settings_requests.at("height"s).AsDouble();
        render_settings.padding_ = render_settings_requests.at("padding"s).AsDouble();
        render_settings.stop_radius_ = render_settings_requests.at("stop_radius"s).AsDouble();
        render_settings.line_width_ = render_settings_requests.at("line_width"s).AsDouble();

        render_settings.bus_label_font_size_ = render_settings_requests.at("bus_label_font_size"s).AsInt();

        Array bus_offset = render_settings_requests.at("bus_label_offset"s).AsArray();
        render_settings.bus_label_offset_ = {bus_offset[0].AsDouble(), bus_offset[1].AsDouble()};

        render_settings.stop_label_font_size_ = render_settings_requests.at("stop_label_font_size"s).AsInt();

        Array stop_offset = render_settings_requests.at("stop_label_offset"s).AsArray();
        render_settings.stop_label_offset_ = {stop_offset[0].AsDouble(), stop_offset[1].AsDouble()};

        render_settings.underlayer_color_ = ColorDeterminant(render_settings_requests.at("underlayer_color"s));

        render_settings.underlayer_width_ = render_settings_requests.at("underlayer_width"s).AsDouble();

        Array colors = render_settings_requests.at("color_palette"s).AsArray();
        for (const Node &color: colors) {
            render_settings.color_palette.emplace_back(ColorDeterminant(color));
        }

        return render_settings;
    }

    //------------------------------------------------------------------------------------------------------------------
    string JsonReader::GetSerializationSettings() const {
        return document_.GetRoot().AsMap().at("serialization_settings"s).AsMap().at("file"s).AsString();
    }

    //------------------------------------------------------------------------------------------------------------------
    transport_router::TransportRouter::RoutingSettings JsonReader::GetRoutingSettings() const {
        return {
                document_.GetRoot().AsMap().at("routing_settings"s).AsMap().at("bus_wait_time"s).AsInt(),
                document_.GetRoot().AsMap().at("routing_settings"s).AsMap().at("bus_velocity"s).AsDouble()
        };
    }

    //------------------------------------------------------------------------------------------------------------------
    void JsonReader::PrintJsonResponse(std::ostream &out, request_handler::RequestHandler& request_handler) {
        Array stat_requests = document_.GetRoot().AsMap().at("stat_requests"s).AsArray();

        Array response;
        response.reserve(stat_requests.size());

        for (const Node &node_map: stat_requests) {
            string type = node_map.AsMap().at("type"s).AsString();
            if (type == "Stop"s) {
                response.push_back(
                        request_handler.StopRequestHandler(
                                node_map.AsMap().at("id"s).AsInt(),
                                node_map.AsMap().at("name"s).AsString()
                        )
                );
            } else if (type == "Bus"s) {
                response.push_back(
                        request_handler.BusRequestHandler(
                                node_map.AsMap().at("id"s).AsInt(),
                                node_map.AsMap().at("name"s).AsString()
                        )
                );
            } else if (type == "Route"s) {
                response.push_back(
                        request_handler.RouterHandler(
                                node_map.AsMap().at("id"s).AsInt(),
                                node_map.AsMap().at("from"s).AsString(),
                                node_map.AsMap().at("to"s).AsString()
                        )
                );
            } else if (type == "Map"s) {
                response.push_back(request_handler.MapRequestHandler(node_map.AsMap().at("id"s).AsInt()));
            }
        }

        Print(Document{response}, out);
    }
    //------------------------------------------------------------------------------------------------------------------

} // namespace json_reader
