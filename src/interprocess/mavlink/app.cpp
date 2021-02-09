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
    // void loop() override;
};
} // namespace apps
} // namespace goby3_examples

int main(int argc, char* argv[])
{
    return goby::run<goby3_examples::apps::MavlinkExample>(argc, argv);
}

goby3_examples::apps::MavlinkExample::MavlinkExample()
{
    interprocess()
        .subscribe<goby::middleware::io::groups::mavlink_raw_in,
                   std::tuple<int, int, mavlink::common::msg::HEARTBEAT>>(
            [](const std::tuple<int, int, mavlink::common::msg::HEARTBEAT>& hb_with_metadata) {
                int sysid, compid;
                mavlink::common::msg::HEARTBEAT hb;
                std::tie(sysid, compid, hb) = hb_with_metadata;
                goby::glog.is_verbose() && goby::glog << "Received heartbeat [sysid: " << sysid
                                                      << ", compid: " << compid
                                                      << "]: " << hb.to_yaml() << std::endl;
            });

    interprocess()
        .subscribe<goby::middleware::io::groups::mavlink_raw_in, mavlink::common::msg::HEARTBEAT>(
            [](const mavlink::common::msg::HEARTBEAT& hb) {
                goby::glog.is_verbose() && goby::glog << "Received heartbeat (w/o metadata): "
                                                      << hb.to_yaml() << std::endl;
            });

    interprocess()
        .subscribe<goby::middleware::io::groups::mavlink_raw_in, mavlink::common::msg::SYS_STATUS>(
            [](const mavlink::common::msg::SYS_STATUS& status) {
                goby::glog.is_verbose() && goby::glog << "Received sys status (w/o metadata): "
                                                      << status.to_yaml() << std::endl;
            });

    

    interprocess().subscribe_type_regex<mavlink::mavlink_message_t>(
        [](std::shared_ptr<const mavlink::mavlink_message_t> mavlink, const std::string& type) {
            goby::glog.is_verbose() &&
                goby::glog << "Received raw mavlink_message_t of type (msgid): " << type
                           << " with length: " << static_cast<int>(mavlink->len) << std::endl;
        },
        goby::middleware::io::groups::mavlink_raw_in, "^0|1$");
}
