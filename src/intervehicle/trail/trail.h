#ifndef GOBY3_EXAMPLES_MULTIVEHICLE_TRAIL_TRAIL_H
#define GOBY3_EXAMPLES_MULTIVEHICLE_TRAIL_TRAIL_H

#include <goby/middleware/group.h>
#include <goby/middleware/protobuf/frontseat_data.pb.h>
#include <goby/middleware/transport/publisher.h>
#include <goby/middleware/transport/subscriber.h>
#include <goby/time/convert.h>
#include <goby/time/system_clock.h>
#include <goby/util/geodesy.h>

#include "trail/nav.pb.h"

namespace trail
{
using goby::middleware::frontseat::protobuf::NodeStatus;
using goby3_examples::trail::NavigationReport;

namespace groups
{
// numeric groups: the intervehicle layer sends the number, not the name
constexpr goby::middleware::Group usv_nav{"trail::usv_nav", 1};
constexpr goby::middleware::Group auv_nav{"trail::auv_nav", 2};
} // namespace groups

constexpr int usv_vehicle_id = 1;
constexpr int first_auv_vehicle_id = 2;

// matches each vehicle's interprocess platform, so the topside and the vehicle agree on names
inline std::string vehicle_name(const NavigationReport& nav)
{
    switch (nav.type())
    {
        case NavigationReport::USV: return "usv";
        case NavigationReport::AUV:
            return "auv" + std::to_string(nav.vehicle() - first_auv_vehicle_id);
        default: return "vehicle" + std::to_string(nav.vehicle());
    }
}

// both nav groups carry the same type, so the group travels in the message
inline goby::middleware::Publisher<NavigationReport> nav_publisher()
{
    return goby::middleware::Publisher<NavigationReport>(
        {},
        [](NavigationReport& nav, const goby::middleware::Group& group)
        {
            nav.set_type(group == groups::usv_nav ? NavigationReport::USV : NavigationReport::AUV);
        });
}

inline goby::middleware::Subscriber<NavigationReport>
nav_subscriber(const goby::middleware::intervehicle::protobuf::TransporterConfig& intervehicle_cfg)
{
    goby::middleware::protobuf::TransporterConfig cfg;
    *cfg.mutable_intervehicle() = intervehicle_cfg;
    return goby::middleware::Subscriber<NavigationReport>(
        cfg, [](const NavigationReport& nav) -> goby::middleware::Group
        { return nav.type() == NavigationReport::USV ? groups::usv_nav : groups::auv_nav; });
}

inline NavigationReport to_dccl(const NodeStatus& status, int vehicle,
                                const goby::util::UTMGeodesy& geodesy)
{
    using goby::middleware::frontseat::protobuf::VehicleType;
    NavigationReport nav;
    nav.set_vehicle(vehicle);

    // DCCL encodes time against the real clock, so remove the simulation warp before sending
    nav.set_time_with_units(
        goby::time::convert<goby::time::MicroTime>(goby::time::SystemClock::unwarp(
            goby::time::convert<goby::time::SystemClock::time_point>(status.time_with_units()))));

    switch (status.type())
    {
        case VehicleType::AUV:
        case VehicleType::GLIDER: nav.set_type(NavigationReport::AUV); break;
        case VehicleType::USV:
        case VehicleType::USV_POWERED:
        case VehicleType::USV_SAILING: nav.set_type(NavigationReport::USV); break;
        default: nav.set_type(NavigationReport::OTHER); break;
    }

    auto xy = geodesy.convert(
        {status.global_fix().lat_with_units(), status.global_fix().lon_with_units()});
    nav.set_x_with_units(xy.x);
    nav.set_y_with_units(xy.y);
    nav.set_z_with_units(-status.global_fix().depth_with_units());
    nav.set_speed_over_ground_with_units(status.speed().over_ground_with_units());

    auto heading = status.pose().heading_with_units();
    const auto revolution = 360 * boost::units::degree::degrees;
    while (heading >= revolution) heading -= revolution;
    while (heading < 0 * boost::units::degree::degrees) heading += revolution;
    nav.set_heading_with_units(heading);

    return nav;
}

inline NodeStatus to_node_status(const NavigationReport& nav, const goby::util::UTMGeodesy& geodesy)
{
    using goby::middleware::frontseat::protobuf::VehicleType;
    NodeStatus status;
    status.set_name(vehicle_name(nav));
    status.set_time_with_units(
        goby::time::convert<goby::time::MicroTime>(goby::time::SystemClock::warp(
            goby::time::convert<std::chrono::system_clock::time_point>(nav.time_with_units()))));

    switch (nav.type())
    {
        case NavigationReport::AUV: status.set_type(VehicleType::AUV); break;
        case NavigationReport::USV: status.set_type(VehicleType::USV); break;
        default: status.set_type(VehicleType::OTHER); break;
    }

    auto latlon = geodesy.convert({nav.x_with_units(), nav.y_with_units()});
    status.mutable_global_fix()->set_lat_with_units(latlon.lat);
    status.mutable_global_fix()->set_lon_with_units(latlon.lon);
    status.mutable_global_fix()->set_depth_with_units(-nav.z_with_units());
    status.mutable_local_fix()->set_x_with_units(nav.x_with_units());
    status.mutable_local_fix()->set_y_with_units(nav.y_with_units());
    status.mutable_local_fix()->set_z_with_units(nav.z_with_units());
    status.mutable_pose()->set_heading_with_units(nav.heading_with_units());
    status.mutable_speed()->set_over_ground_with_units(nav.speed_over_ground_with_units());
    return status;
}

} // namespace trail

#endif
