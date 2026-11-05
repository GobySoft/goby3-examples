#include <iomanip>
#include <limits>
#include <sstream>

#include <goby/middleware/marshalling/protobuf.h>

#include <goby/moos/middleware/moos_plugin_translator.h>
#include <goby/zeromq/application/multi_thread.h>

#include "trail.h"

using goby::glog;

namespace trail
{
// Posts the USV's position as a MOOS NODE_REPORT, which BHV_Trail treats as a contact
class USVContactTranslation : public goby::moos::Translator
{
  public:
    USVContactTranslation(const goby::apps::moos::protobuf::GobyMOOSGatewayConfig& cfg)
        : goby::moos::Translator(cfg)
    {
        goby()
            .interprocess()
            .subscribe<groups::usv_nav, NavigationReport,
                       goby::middleware::MarshallingScheme::PROTOBUF>(
                [this](const NavigationReport& nav) { post_node_report(nav); });
    }

  private:
    void post_node_report(const NavigationReport& nav)
    {
        using boost::units::quantity;
        namespace si = boost::units::si;

        // the report crossed the link unwarped; MOOS expects warped UNIX seconds
        double moos_time = goby::time::convert<goby::time::SITime>(
                               goby::time::SystemClock::warp(
                                   goby::time::convert<std::chrono::system_clock::time_point>(
                                       nav.time_with_units())))
                               .value();

        std::stringstream report;
        report << std::setprecision(std::numeric_limits<double>::digits10)
               << "NAME=" << vehicle_name(nav) << ","
               << "TYPE=" << NavigationReport::VehicleClass_Name(nav.type()) << ","
               << "TIME=" << moos_time << ","
               << "X=" << nav.x_with_units<quantity<si::length>>().value() << ","
               << "Y=" << nav.y_with_units<quantity<si::length>>().value() << ","
               << "DEPTH=" << -nav.z_with_units<quantity<si::length>>().value() << ","
               << "HDG="
               << nav.heading_with_units<quantity<boost::units::degree::plane_angle>>().value()
               << ","
               << "SPD=" << nav.speed_over_ground_with_units<quantity<si::velocity>>().value();

        glog.is_verbose() && glog << "NODE_REPORT: " << report.str() << std::endl;
        moos().comms().Notify("NODE_REPORT", report.str());
    }
};
} // namespace trail

extern "C"
{
    void goby3_moos_gateway_load(
        goby::zeromq::MultiThreadApplication<goby::apps::moos::protobuf::GobyMOOSGatewayConfig>*
            handler)
    {
        handler->launch_thread<trail::USVContactTranslation>();
    }

    void goby3_moos_gateway_unload(
        goby::zeromq::MultiThreadApplication<goby::apps::moos::protobuf::GobyMOOSGatewayConfig>*
            handler)
    {
        handler->join_thread<trail::USVContactTranslation>();
    }
}
