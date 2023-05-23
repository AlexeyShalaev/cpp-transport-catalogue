#pragma once

#include <cmath>

namespace geo {

    const int EARTH_RADIUS = 6'371'000;
    const double EPSILON = 1e-6;

    struct Coordinates {
        double lat;
        double lng;
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}