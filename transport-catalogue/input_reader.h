#pragma once

#include "transport_catalogue.h"

#include <algorithm>
#include <execution>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>

using namespace transport_catalogue;

namespace input_reader {

    class Query {
    public:
        std::string name;

        explicit Query(std::string_view name) : name(name) {}
    };

    class BusQuery : public Query {
    public:
        bool is_annular;
        const std::vector<std::string> stops;

        BusQuery(std::string_view name, bool is_annular, std::vector<std::string> stops) : Query(name),
                                                                                           is_annular(is_annular),
                                                                                           stops(std::move(stops)) {}
    };

    class StopQuery : public Query {
    public:
        geo::Coordinates coordinates;
        std::vector<std::pair<std::string, int>> distances;

        StopQuery(std::string_view name, double latitude, double longitude,
                  std::vector<std::pair<std::string, int>> distances) : Query(name),
                                                                        coordinates({latitude, longitude}),
                                                                        distances(std::move(distances)) {}
    };


    std::vector<std::pair<std::string, std::string>> QueriesToDataBase(TransportCatalogue &db);

    bool IsIntNumber(std::string_view symbols);

    std::string_view Trim(std::string_view value);

    std::vector<std::string_view> Split(std::string_view line, char delimiter, bool trimmed = false);

    void Split(std::string_view line, char delimiter, std::vector<std::string> &tokens, bool trimmed = false);

    std::string ReadLine();

    int ReadLineWithNumber();
}