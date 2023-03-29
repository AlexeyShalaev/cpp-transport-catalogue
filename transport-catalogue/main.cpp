#include <iostream>
#include <fstream>

#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"


using namespace std;
using namespace transport::catalogue;
using namespace renderer;

int main() {
    istringstream input("{\n"
                        "    \"base_requests\": [\n"
                        "      {\n"
                        "        \"type\": \"Bus\",\n"
                        "        \"name\": \"114\",\n"
                        "        \"stops\": [\"Морской вокзал\", \"Ривьерский мост\"],\n"
                        "        \"is_roundtrip\": false\n"
                        "      },\n"
                        "      {\n"
                        "        \"type\": \"Stop\",\n"
                        "        \"name\": \"Ривьерский мост\",\n"
                        "        \"latitude\": 43.587795,\n"
                        "        \"longitude\": 39.716901,\n"
                        "        \"road_distances\": {\"Морской вокзал\": 850}\n"
                        "      },\n"
                        "      {\n"
                        "        \"type\": \"Stop\",\n"
                        "        \"name\": \"Морской вокзал\",\n"
                        "        \"latitude\": 43.581969,\n"
                        "        \"longitude\": 39.719848,\n"
                        "        \"road_distances\": {\"Ривьерский мост\": 850}\n"
                        "      }\n"
                        "    ],\n"
                        "    \"render_settings\": {\n"
                        "      \"width\": 200,\n"
                        "      \"height\": 200,\n"
                        "      \"padding\": 30,\n"
                        "      \"stop_radius\": 5,\n"
                        "      \"line_width\": 14,\n"
                        "      \"bus_label_font_size\": 20,\n"
                        "      \"bus_label_offset\": [7, 15],\n"
                        "      \"stop_label_font_size\": 20,\n"
                        "      \"stop_label_offset\": [7, -3],\n"
                        "      \"underlayer_color\": [255,255,255,0.85],\n"
                        "      \"underlayer_width\": 3,\n"
                        "      \"color_palette\": [\"green\", [255,160,0],\"red\"]\n"
                        "    },\n"
                        "    \"stat_requests\": [\n"
                        "      { \"id\": 1, \"type\": \"Map\" },\n"
                        "      { \"id\": 2, \"type\": \"Stop\", \"name\": \"Ривьерский мост\" },\n"
                        "      { \"id\": 3, \"type\": \"Bus\", \"name\": \"114\" }\n"
                        "    ]\n"
                        "  }");


    // main instance of TC
    TransportCatalogue transportCatalogue;

    // reading data
    JsonReader jsonReader;
    jsonReader.Load(input);

    // executing with handler
    transport::handlers::RequestHandler requestHandler(transportCatalogue);
    requestHandler.ExecuteQueries(jsonReader.GetStopQueries(), jsonReader.GetBusQueries());
    const auto r = requestHandler.ExecuteStats(jsonReader.GetStatRequests(), jsonReader);
    cout << r;

    return 0;
}