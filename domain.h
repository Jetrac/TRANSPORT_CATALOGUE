#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {
    struct Stop {
        std::string stop_name_;
        geo::Coordinates coordinates_;
    };

    struct Bus {
        std::string bus_name_;
        std::vector<Stop *> bus_route_;
        bool is_roundtrip_;
    };

    struct StopsDistance {
        std::string first_stop_;
        std::string second_stop_;
        int distance_;
    };

    struct BusInfo {
        int stop_count_;
        int unique_stop_count_;
        int route_length_;
        double curvature_;
    };
}
