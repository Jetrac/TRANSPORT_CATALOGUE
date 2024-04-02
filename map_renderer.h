#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <string>

namespace renderer {
    using namespace domain;

    inline const double EPSILON = 1e-6;

    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height,
                        double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    template<typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                                     double max_height, double padding) : padding_(padding) {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }
    //------------------------------------------------------------------------------------------------------------------

    class MapRenderer {
    public:
        struct RenderSettings {
            double width_ = 1;
            double height_ = 1;
            double padding_ = 1;

            double line_width_ = 1;
            double stop_radius_ = 1;

            int bus_label_font_size_ = 1;
            svg::Point bus_label_offset_ = {0, 0};

            int stop_label_font_size_ = 1;
            svg::Point stop_label_offset_ = {0, 0};

            double underlayer_width_ = 1;
            svg::Color underlayer_color_ = svg::NoneColor;

            std::vector<svg::Color> color_palette{};
        };
    private:
        RenderSettings render_settings_;
        std::vector<geo::Coordinates> geo_coords_;
        std::map<std::string, geo::Coordinates> stops_;
        std::map<std::string, Bus *> buses_;
    public:
        MapRenderer(const RenderSettings& render_settings, const std::map<std::string, Bus *>& buses) : render_settings_(render_settings) {
            LoadRenderInfo(buses);
        };

        void LoadRenderInfo(const std::map<std::string, Bus *> &buses);

        std::string RenderMap();

        RenderSettings GetRenderSettings() const;
    private:
        void RenderRouteLines(SphereProjector &proj, svg::Document &document);
        void RenderRouteNames(SphereProjector &proj, svg::Document &document);
        void RenderStopsSymbols(SphereProjector &proj, svg::Document &document);
        void RenderStopsNames(SphereProjector &proj, svg::Document &document);
    };
} // namespace renderer
