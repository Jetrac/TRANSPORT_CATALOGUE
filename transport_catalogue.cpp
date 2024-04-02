#include "transport_catalogue.h"

#include <set>
#include <deque>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace transport_catalogue {
    //------------------------------------------------------------------------------------------------------------------
    void TransportCatalogue::AddStop(const Stop &stop) {
        stops_.push_back(stop);
        stop_name_to_stop_.insert({stops_.back().stop_name_, &stops_.back()});
    }

    void TransportCatalogue::AddBus(const Bus &bus) {
        buses_.push_back(bus);
        bus_name_to_bus_.insert({buses_.back().bus_name_, &buses_.back()});
    }

    //------------------------------------------------------------------------------------------------------------------
    Stop *TransportCatalogue::FindStop(string_view stop_name) const {
        return stop_name_to_stop_.at(stop_name);
    }

    Bus *TransportCatalogue::FindBus(string_view bus_name) const {
        return bus_name_to_bus_.at(bus_name);
    }

    //------------------------------------------------------------------------------------------------------------------
    int TransportCatalogue::GetDistance(Stop *first_stop, Stop *second_stop) const {
        return stops_to_distance_.at({first_stop, second_stop});
    }

    int TransportCatalogue::GetUniqueStopsCount(string_view bus_name) const {
        unordered_set<string_view> unique_stops;

        Bus *bus_ = FindBus(bus_name);
        for (const auto &stop: bus_->bus_route_) {
            unique_stops.insert(stop->stop_name_);
        }

        return static_cast<int>(unique_stops.size());
    }

    set<string> TransportCatalogue::GetStopInfo(string_view stop_name) const {
        static const set<string> empty_set{"stop does not exist"s};

        if (stop_name_to_stop_.find(stop_name) == stop_name_to_stop_.end()) {
            return empty_set;
        }

        set<string> buses;
        for (const auto &[bus_name, bus]: bus_name_to_bus_) {
            for (const auto &stop: bus->bus_route_) {
                if (stop->stop_name_ == stop_name) {
                    buses.insert(string(bus_name));
                    continue;
                }
            }
        }

        return buses;
    }

    BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) const {
        if (bus_name_to_bus_.find(bus_name) == bus_name_to_bus_.end()) {
            return {};
        }

        Bus *bus = FindBus(bus_name);

        int route_distance = 0;
        double geographical_distance = 0;
        int stops_amount = static_cast<int>(bus->bus_route_.size());
        if (bus->is_roundtrip_) {
            for (int i = 1; i < stops_amount; ++i) {
                route_distance += GetDistance(bus->bus_route_[i - 1], bus->bus_route_[i]);
                geographical_distance += ComputeDistance(
                        bus->bus_route_[i - 1]->coordinates_,
                        bus->bus_route_[i]->coordinates_
                );
            }
        } else {
            for (int i = 1; i < stops_amount; ++i) {
                route_distance += GetDistance(bus->bus_route_[i - 1], bus->bus_route_[i]);
                route_distance += GetDistance(bus->bus_route_[i], bus->bus_route_[i - 1]);
                geographical_distance += ComputeDistance(
                        bus->bus_route_[i - 1]->coordinates_,
                        bus->bus_route_[i]->coordinates_
                ) * 2;
            }
            stops_amount = (stops_amount * 2) - 1;
        }

        int unique_stops_count = GetUniqueStopsCount(bus_name);
        double curvature = route_distance / geographical_distance;

        return {stops_amount, unique_stops_count, route_distance, curvature};
    }

    //------------------------------------------------------------------------------------------------------------------
    void TransportCatalogue::CreateStopsToDistance(const vector <StopsDistance> &info) {
        for (const auto &[stop1, stop2, length]: info) {
            stops_to_distance_.insert({{FindStop(stop1), FindStop(stop2)}, length});
        }
        for (const auto &[stop1, stop2, length]: info) {
            if (stops_to_distance_.find({FindStop(stop2), FindStop(stop1)}) == stops_to_distance_.end()) {
                stops_to_distance_.insert({{FindStop(stop2), FindStop(stop1)}, length});
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    map<string, Stop *> TransportCatalogue::GetSortedStops() const {
        map<string, Stop *> sorted_stop_name_to_stop_;
        for (const auto &stop: stop_name_to_stop_) {
            sorted_stop_name_to_stop_.insert(stop);
        }
        return sorted_stop_name_to_stop_;
    }

    map<string, Bus *> TransportCatalogue::GetSortedBuses() const {
        map<string, Bus *> sorted_bus_name_to_bus_;

        for (const auto &bus: bus_name_to_bus_) {
            sorted_bus_name_to_bus_.insert(bus);
        }

        return sorted_bus_name_to_bus_;
    }

    //------------------------------------------------------------------------------------------------------------------
    std::unordered_map<std::pair<Stop *, Stop *>, int, TransportCatalogue::KeyHasher> TransportCatalogue::GetStopDistances() const {
        return stops_to_distance_;
    }
    //------------------------------------------------------------------------------------------------------------------
} // namespace transport_catalogue
