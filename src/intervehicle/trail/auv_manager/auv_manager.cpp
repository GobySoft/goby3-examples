#include <goby/middleware/marshalling/dccl.h>
#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/frontseat/groups.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h"
#include "trail.h"

using goby::glog;
using Base = goby::zeromq::SingleThreadApplication<goby3_examples::trail::AUVManagerConfig>;

// Sends the AUV's position to the USV, and hands the USV's position to the helm (via the MOOS
// gateway plugin) so the AUV can trail it
class AUVManager : public Base
{
  public:
    AUVManager()
    {
        interprocess().subscribe<goby::middleware::frontseat::groups::node_status>(
            [this](const trail::NodeStatus& status)
            {
                intervehicle().publish<trail::groups::auv_nav>(
                    trail::to_dccl(status, cfg().vehicle_id(), geodesy()), trail::nav_publisher());
            });

        goby::middleware::intervehicle::protobuf::TransporterConfig intervehicle_cfg;
        intervehicle_cfg.add_publisher_id(cfg().usv_modem_id());
        // every AUV hears the USV, so one broadcast serves them all
        intervehicle_cfg.set_broadcast(true);
        auto& buffer = *intervehicle_cfg.mutable_buffer();
        buffer.set_ack_required(false);
        buffer.set_max_queue(1);
        buffer.set_newest_first(true);

        intervehicle().subscribe<trail::groups::usv_nav, trail::NavigationReport>(
            [this](const trail::NavigationReport& nav)
            {
                interprocess()
                    .publish<trail::groups::usv_nav, trail::NavigationReport,
                             goby::middleware::MarshallingScheme::PROTOBUF>(nav);
            },
            trail::nav_subscriber(intervehicle_cfg));
    }
};

int main(int argc, char* argv[]) { return goby::run<AUVManager>(argc, argv); }
