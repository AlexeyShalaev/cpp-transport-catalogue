#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"

#include <algorithm>
#include <execution>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>

using namespace transport::catalogue;

namespace input_reader {

    std::vector<std::pair<std::string, std::string>>
    QueriesToDataBase(TransportCatalogue &db, std::istream &input_stream = std::cin);

    bool IsIntNumber(std::string_view symbols);

    std::string_view Trim(std::string_view value);

    std::vector<std::string_view> Split(std::string_view line, char delimiter, bool trimmed = false);

    void Split(std::string_view line, char delimiter, std::vector<std::string> &tokens, bool trimmed = false);

    std::string ReadLine(std::istream &input_stream = std::cin);

    int ReadLineWithNumber(std::istream &input_stream = std::cin);
}