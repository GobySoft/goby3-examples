#include <goby/middleware/marshalling/dccl.h>
#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/frontseat/groups.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h"
#include "trail.h"

using goby::glog;
using Base = goby::zeromq::SingleThreadApplication<goby3_examples::trail::USVManagerConfig>;

// Sends the USV's position to the AUVs and topside, and relays the AUVs' positions topside
class USVManager : public Base
{
  public:
    USVManager()
    {
        interprocess().subscribe<goby::middleware::frontseat::groups::node_status>(
            [this](const trail::NodeStatus& status)
            {
                intervehicle().publish<trail::groups::usv_nav>(
                    trail::to_dccl(status, cfg().vehicle_id(), geodesy()), trail::nav_publisher());
            });

        for (int auv_modem_id : cfg().auv_modem_id())
        {
            goby::middleware::intervehicle::protobuf::TransporterConfig intervehicle_cfg;
            intervehicle_cfg.add_publisher_id(auv_modem_id);
            auto& buffer = *intervehicle_cfg.mutable_buffer();
            buffer.set_ack_required(false);
            buffer.set_max_queue(1);
            buffer.set_newest_first(true);

            intervehicle().subscribe<trail::groups::auv_nav, trail::NavigationReport>(
                [this](const trail::NavigationReport& nav)
                {
                    glog.is_verbose() && glog << "Relaying " << nav.ShortDebugString() << std::endl;
                    intervehicle().publish<trail::groups::auv_nav>(nav, trail::nav_publisher());
                },
                trail::nav_subscriber(intervehicle_cfg));
        }
    }
};

int main(int argc, char* argv[]) { return goby::run<USVManager>(argc, argv); }
