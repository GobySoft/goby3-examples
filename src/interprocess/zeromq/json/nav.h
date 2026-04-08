#ifndef JSON_NAV_H
#define JSON_NAV_H

#include <iostream>

#include <goby/middleware/marshalling/json.h>

// plain struct
struct NavigationReport
{
    // required by Goby to define type
    static constexpr auto goby_json_type = "NavigationReport";
    double x{0};
    double y{0};
    double z{0};
};
// required by nlohmann JSON to encode/decode as JSON
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NavigationReport, x, y, z);

// for convenience
std::ostream& operator<<(std::ostream& os, const NavigationReport& nav)
{
    return os << "x: " << nav.x << ", y: " << nav.y << ", z: " << nav.z;
}

#endif
