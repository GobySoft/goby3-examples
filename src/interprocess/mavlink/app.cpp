#include <mavlink/v2.0/common/common.hpp>

#include <goby/middleware/marshalling/mavlink.h>
#include <goby/middleware/marshalling/protobuf.h>
// this space intentionally left blank
#include <goby/middleware/io/groups.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h"

using goby::glog;
namespace si = boost::units::si;
using ApplicationBase =
    goby::zeromq::SingleThreadApplication<goby3_examples::config::MavlinkExample>;

namespace goby3_examples
{
namespace apps
{
class MavlinkExample : public ApplicationBase
{
  public:
    MavlinkExample();

  private:
    void handle_system_status(const mavlink::common::msg::SYS_STATUS& status);

  private:
    void loop() override;
};
} // namespace apps
} // namespace goby3_examples

int main(int argc, char* argv[])
{
    return goby::run<goby3_examples::apps::MavlinkExample>(argc, argv);
}

goby3_examples::apps::MavlinkExample::MavlinkExample()
    : ApplicationBase(1.0 * boost::units::si::hertz)
{
    glog.add_group("in", goby::util::Colors::lt_green);
    glog.add_group("out", goby::util::Colors::lt_blue);

    interprocess()
        .subscribe<goby::middleware::io::groups::mavlink_raw_in,
                   std::tuple<int, int, mavlink::common::msg::HEARTBEAT>,
                   goby::middleware::MarshallingScheme::MAVLINK>(
            [](const std::tuple<int, int, mavlink::common::msg::HEARTBEAT>& hb_with_metadata) {
                int sysid, compid;
                mavlink::common::msg::HEARTBEAT hb;
                std::tie(sysid, compid, hb) = hb_with_metadata;
                goby::glog.is_verbose() &&
                    goby::glog << group("in") << "Received heartbeat [sysid: " << sysid
                               << ", compid: " << compid << "]: " << hb.to_yaml() << std::endl;
            });

    interprocess()
        .subscribe<goby::middleware::io::groups::mavlink_raw_in, mavlink::common::msg::HEARTBEAT>(
            [](const mavlink::common::msg::HEARTBEAT& hb) {
                goby::glog.is_verbose() && goby::glog << "Received heartbeat (w/o metadata): "
                                                      << hb.to_yaml() << std::endl;
            });

    interprocess().subscribe<goby::middleware::io::groups::mavlink_raw_in>(
        [this](const mavlink::common::msg::SYS_STATUS& status) { handle_system_status(status); });

    interprocess().subscribe_type_regex<mavlink::mavlink_message_t>(
        [](std::shared_ptr<const mavlink::mavlink_message_t> mavlink, const std::string& type) {
            goby::glog.is_verbose() &&
                goby::glog << group("in")
                           << "Received raw mavlink_message_t of type (msgid): " << type
                           << " with length: " << static_cast<int>(mavlink->len) << std::endl;
        },
        goby::middleware::io::groups::mavlink_raw_in, "^0|1$");
}

void goby3_examples::apps::MavlinkExample::handle_system_status(
    const mavlink::common::msg::SYS_STATUS& status)
{
    goby::glog.is_verbose() && goby::glog << group("in") << "Received sys status (w/o metadata): "
                                          << status.to_yaml() << std::endl;
}

void goby3_examples::apps::MavlinkExample::loop()
{
    mavlink::common::msg::SYS_STATUS status{};
    status.voltage_battery = 24;

    std::tuple<int, int, mavlink::common::msg::SYS_STATUS> status_tuple{
        cfg().mavlink_sys_id(), cfg().mavlink_component_id(), status};

    glog.is_verbose() && glog << group("out") << "Sending status to Ardupilot: " << status.to_yaml()
                              << std::endl;
    interprocess().publish<goby::middleware::io::groups::mavlink_raw_out>(status_tuple);
}
