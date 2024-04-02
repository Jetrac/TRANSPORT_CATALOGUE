#pragma once

#include "domain.h"

#include <set>
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace transport_catalogue {
    using namespace domain;

    class TransportCatalogue {
    private:
        struct KeyHasher {
            std::size_t operator()(std::pair<Stop *, Stop *> k) const {
                using std::size_t;
                using std::hash;
                using std::string;
                return (
                        std::hash<std::string>{}(k.first->stop_name_) * 7 +
                        std::hash<std::string>{}(k.second->stop_name_) * 19
                );
            }
        };

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Stop *> stop_name_to_stop_;
        std::unordered_map<std::string_view, Bus *> bus_name_to_bus_;
        std::unordered_map<std::pair<Stop *, Stop *>, int, KeyHasher> stops_to_distance_;
    public:
        void AddStop(const Stop &stop);

        void AddBus(const Bus &bus);

        Stop *FindStop(std::string_view stop_name) const;

        Bus *FindBus(std::string_view bus_name) const;

        std::set<std::string> GetStopInfo(std::string_view stop_name) const;

        BusInfo GetBusInfo(std::string_view bus_name) const;

        int GetDistance(Stop *first_stop, Stop *second_stop) const;

        std::unordered_map<std::pair<Stop *, Stop *>, int, KeyHasher> GetStopDistances() const;

        std::map<std::string, Stop *> GetSortedStops() const;

        std::map<std::string, Bus *> GetSortedBuses() const;

        void CreateStopsToDistance(const std::vector<StopsDistance> &info);
    private:
        int GetUniqueStopsCount(std::string_view bus_name) const;
    };
} // namespace transport_catalogue
