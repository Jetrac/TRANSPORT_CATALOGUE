#include "request_handler.h"

using namespace std;

namespace request_handler {
    Node RequestHandler::MapRequestHandler(int request_id) {
        Node node;
        node = Builder{}
                .StartDict()
                .Key("map").Value(renderer_.RenderMap())
                .Key("request_id").Value(request_id)
                .EndDict()
                .Build();
        return node;
    }

    Node RequestHandler::StopRequestHandler(int request_id, std::string_view request_name) {
        Node node;

        set<string> buses_through_stop = transport_catalogue_.GetStopInfo(request_name);

        if (*buses_through_stop.begin() == "stop does not exist"s) {
            node = Builder{}
                    .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
        } else {
            Array buses;
            for (const string &bus_name: buses_through_stop) {
                buses.emplace_back(bus_name);
            }
            node = Builder{}
                    .StartDict()
                    .Key("buses"s).Value(buses)
                    .Key("request_id"s).Value(request_id)
                    .EndDict()
                    .Build();
        }

        return node;
    }

    Node RequestHandler::BusRequestHandler(int request_id, std::string_view request_name) {
        Node node;

        domain::BusInfo bus_request_info = transport_catalogue_.GetBusInfo(request_name);

        if (bus_request_info.stop_count_ == 0) {
            node = Builder{}
                    .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
        } else {
            node = Builder{}
                    .StartDict()
                    .Key("curvature"s).Value(bus_request_info.curvature_)
                    .Key("request_id"s).Value(request_id)
                    .Key("route_length"s).Value(bus_request_info.route_length_)
                    .Key("stop_count"s).Value(bus_request_info.stop_count_)
                    .Key("unique_stop_count"s).Value(bus_request_info.unique_stop_count_)
                    .EndDict()
                    .Build();
        }
        return node;
    }

    Node RequestHandler::RouterHandler(int request_id, string_view start_stop, string_view final_stop) {
        Node node;

        const auto &route = router_.FindRoute(start_stop, final_stop);

        if (!route) {
            node = Builder{}
                    .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
        } else {
            Array items;
            items.reserve(route.value().edges.size());

            double total_time = 0;

            for (auto &edge_id: route.value().edges) {
                const graph::Edge<double> edge = router_.GetGraphEdge(edge_id);

                Node item;
                if (edge.quality == 0) {
                    item = Builder{}
                            .StartDict()
                            .Key("stop_name"s).Value(edge.name)
                            .Key("time"s).Value(edge.weight)
                            .Key("type"s).Value("Wait"s)
                            .EndDict()
                            .Build();
                    items.emplace_back(item);
                } else {
                    item = Builder{}
                            .StartDict()
                            .Key("bus"s).Value(edge.name)
                            .Key("span_count"s).Value(static_cast<int>(edge.quality))
                            .Key("time"s).Value(edge.weight)
                            .Key("type"s).Value("Bus"s)
                            .EndDict()
                            .Build();
                    items.emplace_back(item);
                }
                total_time += edge.weight;
            }

            node = Builder{}
                    .StartDict()
                    .Key("request_id"s).Value(request_id)
                    .Key("total_time"s).Value(total_time)
                    .Key("items"s).Value(items)
                    .EndDict()
                    .Build();
        }

        return node;
    }
} // namespace request_handler
