#include <goby/middleware/marshalling/dccl.h>
#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/frontseat/groups.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h"
#include "trail.h"

using goby::glog;
using Base = goby::zeromq::SingleThreadApplication<goby3_examples::trail::TopsideManagerConfig>;

// Republishes every vehicle's position received over the satellite link as a NodeStatus on the
// interprocess layer, where topside tools such as goby_liaison's Map tab pick it up
class TopsideManager : public Base
{
  public:
    TopsideManager()
    {
        goby::middleware::intervehicle::protobuf::TransporterConfig intervehicle_cfg;
        intervehicle_cfg.add_publisher_id(cfg().usv_modem_id());

        auto& buffer = *intervehicle_cfg.mutable_buffer();
        buffer.set_ack_required(false);
        buffer.set_max_queue(1);
        buffer.set_newest_first(true);
        intervehicle().subscribe<trail::groups::usv_nav, trail::NavigationReport>(
            [this](const trail::NavigationReport& nav) { republish(nav); },
            trail::nav_subscriber(intervehicle_cfg));

        // relayed AUV reports interleave, so keep them all rather than only the newest
        buffer.set_ack_required(true);
        buffer.set_max_queue(10);
        buffer.set_newest_first(false);
        intervehicle().subscribe<trail::groups::auv_nav, trail::NavigationReport>(
            [this](const trail::NavigationReport& nav) { republish(nav); },
            trail::nav_subscriber(intervehicle_cfg));
    }

  private:
    void republish(const trail::NavigationReport& nav)
    {
        auto status = trail::to_node_status(nav, geodesy());
        glog.is_verbose() && glog << status.ShortDebugString() << std::endl;
        interprocess().publish<goby::middleware::frontseat::groups::node_status>(status);
    }
};

int main(int argc, char* argv[]) { return goby::run<TopsideManager>(argc, argv); }
