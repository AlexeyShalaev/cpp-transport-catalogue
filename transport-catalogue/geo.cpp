#include "geo.h"
#include <cmath>

namespace geo {

    bool operator==(const Coordinates &lhs, const Coordinates &rhs) {
        return lhs.lat == rhs.lat && lhs.lng == lhs.lng;
    }

    bool operator!=(const Coordinates &lhs, const Coordinates &rhs) {
        return !(lhs == rhs);
    }

    bool operator<(const Coordinates &lhs, const Coordinates &rhs) {
        return lhs.lat < rhs.lat || ((lhs.lat - rhs.lat) < EPSILON && (lhs.lng < rhs.lng));
    }

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = M_PI / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
                    + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
               * EARTH_RADIUS;
    }

}