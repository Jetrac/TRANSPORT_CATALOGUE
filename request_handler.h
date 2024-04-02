#pragma once

#include <string_view>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include "json.h"
#include "json_builder.h"

namespace request_handler {
    using namespace json;
    using namespace domain;

    class RequestHandler {
    private:
        transport_catalogue::TransportCatalogue &transport_catalogue_;
        renderer::MapRenderer &renderer_;
        transport_router::TransportRouter &router_;
    public:
        RequestHandler(
                transport_catalogue::TransportCatalogue &transport_catalogue,
                renderer::MapRenderer &renderer,
                transport_router::TransportRouter &router
        ) :
                transport_catalogue_(transport_catalogue),
                renderer_(renderer),
                router_(router) {}

        Node MapRequestHandler(int request_id);
        Node StopRequestHandler(int request_id, std::string_view request_name);
        Node BusRequestHandler(int request_id, std::string_view request_name);
        Node RouterHandler(int request_id, std::string_view start_stop, std::string_view final_stop);
    };

} // namespace request_handler
