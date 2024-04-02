#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json_reader {
    using namespace json;
    using namespace transport_catalogue;

    class JsonReader {
    private:
        Document document_;
    public:
        JsonReader(std::istream &in) : document_(Load(in)) {};

        void StopsHandler(const Array &base_requests, TransportCatalogue &transport_catalogue);
        void BusesHandler(const Array &base_requests, TransportCatalogue &transport_catalogue);
        void BaseRequestsHandler(TransportCatalogue &transport_catalogue);

        void PrintJsonResponse(std::ostream &out,request_handler::RequestHandler& request_handler);

        std::string GetSerializationSettings() const;
        renderer::MapRenderer::RenderSettings GetRenderSettings() const;
        transport_router::TransportRouter::RoutingSettings GetRoutingSettings() const;
    private:
        static svg::Color ColorDeterminant(const Node &color);
    };
}
