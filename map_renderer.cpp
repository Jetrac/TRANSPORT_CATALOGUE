#include <sstream>
#include "map_renderer.h"

using namespace std;

namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    void MapRenderer::RenderRouteLines(SphereProjector &proj, svg::Document &document) {
        int color = 0;
        int i = static_cast<int>(render_settings_.color_palette.size());
        for (const auto &[bus_name, bus_]: buses_) {
            svg::Polyline route_;

            for (const auto &x: bus_->bus_route_) {
                svg::Point screen_coord = proj(x->coordinates_);
                route_.AddPoint(screen_coord);
            }
            if (!bus_->is_roundtrip_) {
                for (auto it = ++bus_->bus_route_.rbegin(); it != bus_->bus_route_.rend(); ++it) {
                    auto x = *it;
                    const svg::Point screen_coord = proj(x->coordinates_);
                    route_.AddPoint(screen_coord);
                }
            }

            document.Add(
                    route_
                            .SetFillColor("none"s)
                            .SetStrokeColor(render_settings_.color_palette[color % i])
                            .SetStrokeWidth(render_settings_.line_width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            );

            ++color;
        }
    }


    void MapRenderer::RenderRouteNames(SphereProjector &proj, svg::Document &document) {
        int color = 0;
        int i = static_cast<int>(render_settings_.color_palette.size());
        for (const auto &[bus_name, bus_]: buses_) {
            svg::Text route_name_underlayer_;
            svg::Text route_name_;
            svg::Point screen_coord{};

            svg::Text base;
            base.SetOffset(render_settings_.bus_label_offset_)
                    .SetFontSize(render_settings_.bus_label_font_size_)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(bus_name);

            screen_coord = proj(bus_->bus_route_[0]->coordinates_);
            route_name_underlayer_ = base;
            document.Add(
                    route_name_underlayer_
                            .SetPosition(screen_coord)
                            .SetFillColor(render_settings_.underlayer_color_)
                            .SetStrokeColor(render_settings_.underlayer_color_)
                            .SetStrokeWidth(render_settings_.underlayer_width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            );

            route_name_ = base;
            document.Add(route_name_.SetPosition(screen_coord).SetFillColor(render_settings_.color_palette[color % i]));

            if (!bus_->is_roundtrip_ && bus_->bus_route_[0] != bus_->bus_route_[bus_->bus_route_.size() - 1]) {
                screen_coord = proj(bus_->bus_route_[bus_->bus_route_.size() - 1]->coordinates_);

                route_name_underlayer_ = base;
                document.Add(
                        route_name_underlayer_
                                .SetPosition(screen_coord)
                                .SetFillColor(render_settings_.underlayer_color_)
                                .SetStrokeColor(render_settings_.underlayer_color_)
                                .SetStrokeWidth(render_settings_.underlayer_width_)
                                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                );

                route_name_ = base;
                document.Add(
                        route_name_.SetPosition(screen_coord).SetFillColor(render_settings_.color_palette[color % i]));
            }
            ++color;
        }
    }

    void MapRenderer::RenderStopsSymbols(SphereProjector &proj, svg::Document &document) {
        for (const auto &[stop_name, coordinates]: stops_) {
            svg::Circle stop_symbol_;
            const svg::Point screen_coord = proj(coordinates);
            document.Add(
                    stop_symbol_
                            .SetCenter(screen_coord)
                            .SetRadius(render_settings_.stop_radius_)
                            .SetFillColor("white"s)
            );
        }
    }

    void MapRenderer::RenderStopsNames(SphereProjector &proj, svg::Document &document) {
        for (const auto &[stop_name, coordinates]: stops_) {
            const svg::Point screen_coord = proj(coordinates);

            svg::Text base;
            base.SetPosition(screen_coord)
                    .SetOffset(render_settings_.stop_label_offset_)
                    .SetFontSize(render_settings_.stop_label_font_size_)
                    .SetFontFamily("Verdana"s)
                    .SetData(stop_name);


            svg::Text stop_name_underlayer_ = base;
            document.Add(
                    stop_name_underlayer_
                            .SetFillColor(render_settings_.underlayer_color_)
                            .SetStrokeColor(render_settings_.underlayer_color_)
                            .SetStrokeWidth(render_settings_.underlayer_width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            );

            svg::Text stop_name_ = base;
            document.Add(stop_name_.SetFillColor("black"s));
        }
    }

    string MapRenderer::RenderMap() {
        SphereProjector proj{
                geo_coords_.begin(), geo_coords_.end(),
                render_settings_.width_, render_settings_.height_, render_settings_.padding_
        };

        svg::Document document;
        RenderRouteLines(proj, document);
        RenderRouteNames(proj, document);
        RenderStopsSymbols(proj, document);
        RenderStopsNames(proj, document);

        ostringstream strm;
        document.Render(strm);
        return strm.str();
    }

    void MapRenderer::LoadRenderInfo(const map<string, Bus *> &buses) {
        for (const auto &[bus_number, bus]: buses) {
            if (bus->bus_route_.empty()) continue;
            for (const auto &stop: bus->bus_route_) {
                geo_coords_.push_back(stop->coordinates_);
                stops_[stop->stop_name_] = stop->coordinates_;
            }
        }
        buses_ = buses;
    }

    MapRenderer::RenderSettings MapRenderer::GetRenderSettings() const {
        return render_settings_;
    }
}
