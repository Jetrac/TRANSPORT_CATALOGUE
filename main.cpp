#include <fstream>
#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;

int main(int argc, char *argv[]) {
    const std::string mode(argv[1]);

    json_reader::JsonReader json_reader_(std::cin);
    std::string serialization_settings = json_reader_.GetSerializationSettings();

    if (mode == "make_base"s) {
        transport_catalogue::TransportCatalogue transport_catalogue_;
        json_reader_.BaseRequestsHandler(transport_catalogue_);

        renderer::MapRenderer map_renderer_(
                json_reader_.GetRenderSettings(),
                transport_catalogue_.GetSortedBuses()
        );

        transport_router::TransportRouter transport_router_(json_reader_.GetRoutingSettings());
        transport_router_.BuildGraph(transport_catalogue_);

        std::ofstream fout(serialization_settings, std::ios::binary);
        if (fout.is_open()) {
            serialization::Serialize(
                    transport_catalogue_,
                    map_renderer_,
                    transport_router_,
                    fout
            );
        }
    } else if (mode == "process_requests"s) {
        std::ifstream db_file(serialization_settings, std::ios::binary);
        if (db_file) {
            auto [transport_catalogue, map_renderer, transport_router, graph, stop_ids] = serialization::Deserialize(db_file);

            transport_router.FillRouter(graph, stop_ids);

            request_handler::RequestHandler request_handler_(
                    transport_catalogue,
                    map_renderer,
                    transport_router
            );

            json_reader_.PrintJsonResponse(std::cout, request_handler_);
        }
    } else {
        return 1;
    }
}
