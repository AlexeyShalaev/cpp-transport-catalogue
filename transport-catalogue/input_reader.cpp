#include "input_reader.h"

using namespace std;
using namespace string_view_literals;

using namespace transport::catalogue;
using namespace transport::domains;

namespace input_reader {
    const string_view WHITESPACE = " \f\n\r\t\v"sv;

    vector <pair<string, string>> QueriesToDataBase(TransportCatalogue &db, istream& input) {
        auto queries_length = ReadLineWithNumber(input);
        string query;
        deque<StopQuery> stop_queries;
        deque<BusQuery> bus_queries;
        for (int i = 0; i < queries_length; ++i) {
            query = Trim(ReadLine(input));
            if (query.empty()) continue;
            if (query.find(':') != std::string::npos) {
                auto tokens = Split(query, ':');
                if (tokens.size() != 2) continue;

                auto pos = tokens[0].find(' ');
                if (pos == tokens[0].npos) continue;

                auto key = tokens[0].substr(0, pos);
                auto name = Trim(tokens[0].substr(pos));

                if (key == "Stop") {
                    auto values = Split(tokens[1], ',');
                    auto value_size = values.size();
                    if (value_size < 2) continue;
                    double lat = std::atof(std::string(values[0]).data());
                    double lng = std::atof(std::string(values[1]).data());
                    std::vector<std::pair<std::string, int>> distances;
                    for (int j = 2; j < value_size; ++j) {
                        auto pos_to = values[j].find("to"sv);
                        auto stop_name = std::string(Trim(values[j].substr(pos_to + 2)));
                        auto distance = std::atoi(std::string(values[j].substr(0, pos_to)).data());
                        distances.emplace_back(stop_name, distance);
                    }
                    stop_queries.emplace_back(name, lat, lng, distances);
                } else if (key == "Bus") {
                    auto round_trip = (tokens[1].find('-') != tokens[1].npos);
                    auto annular_trip = (tokens[1].find('>') != tokens[1].npos);
                    if (!round_trip && !annular_trip) continue;
                    auto delimiter = annular_trip ? '>' : '-';
                    vector<std::string> bus_query_tokens;
                    Split(tokens[1], delimiter, bus_query_tokens, true);
                    bus_queries.emplace_back(name, annular_trip, std::move(bus_query_tokens));
                }
            }
        }

        vector<pair<string, string>> info_queries;
        queries_length = ReadLineWithNumber(input);
        for (int i = 0; i < queries_length; ++i) {
            query = ReadLine(input);
            auto pos = query.find(' ');
            if (pos == std::string::npos) {
                info_queries.emplace_back(query, "");
                continue;
            }
            info_queries.emplace_back(query.substr(0, pos), Trim(query.substr(pos)));
        }

        for_each(execution::par,
                 stop_queries.begin(), stop_queries.end(),
                 [&db](const auto &stop_query) {
                     db.AddStop(stop_query.name, stop_query.coordinates);
                 });

        for_each(execution::par,
                 stop_queries.begin(), stop_queries.end(),
                 [&db](const auto &stop_query) {
                     db.AddStopDistances(stop_query.name, std::move(stop_query.distances));
                 });

        for_each(execution::par,
                 bus_queries.begin(), bus_queries.end(),
                 [&db](const auto &bus_query) {
                     db.AddBus(bus_query.name, bus_query.is_annular, std::move(bus_query.stops));
                 });

        return info_queries;
    }

    bool IsIntNumber(string_view symbols) {
        if (symbols.empty()) return false;
        auto start = symbols.cbegin();
        if (symbols[0] == '+' || symbols[0] == '-') ++start;
        return std::all_of(start, symbols.cend(),
                           [](const char &c) {
                               return std::isdigit(c);
                           });
    }

    string_view Trim(string_view value) {
        if (value.empty()) return value;
        auto pos = value.find_first_not_of(WHITESPACE);
        if (pos == std::string_view::npos || pos > value.size()) return {};
        value.remove_prefix(pos);
        pos = value.find_last_not_of(WHITESPACE);
        if (pos < value.size() + 1) value.remove_suffix(value.size() - pos - 1);
        return value;
    }

    vector <string_view> Split(string_view line, char delimiter, bool trimmed) {
        vector<string_view> tokens;
        size_t start;
        size_t end = 0;
        while ((start = line.find_first_not_of(delimiter, end)) != std::string_view::npos) {
            end = line.find(delimiter, start);
            auto token = trimmed ?
                         Trim(line.substr(start, end - start)) :
                         line.substr(start, end - start);
            tokens.push_back(token);
        }
        return tokens;
    }

    void Split(string_view line, char delimiter, vector <std::string> &tokens, bool trimmed) {
        size_t start;
        size_t end = 0;
        while ((start = line.find_first_not_of(delimiter, end)) != std::string_view::npos) {
            end = line.find(delimiter, start);
            auto token = trimmed ?
                         string(Trim(line.substr(start, end - start))) :
                         string(line.substr(start, end - start));
            tokens.push_back(std::move(token));
        }
    }

    string ReadLine(istream& input) {
        string s;
        getline(input, s);
        return s;
    }

    int ReadLineWithNumber(istream& input) {
        int result;
        input >> result;
        ReadLine(input);
        return result;
    }
}