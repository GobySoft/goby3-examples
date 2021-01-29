#include <goby/middleware/marshalling/protobuf.h>

#include <goby/zeromq/application/single_thread.h>

#include "messages/gps.pb.h"
#include "messages/groups.h"

#include "config.pb.h"

class GPSController : public goby::zeromq::SingleThreadApplication<GPSControllerConfig>
{
  public:
    GPSController()
    {
        protobuf::GPSCommand cmd;
        cmd.set_read_gps(cfg().read_gps());
        interprocess().publish<groups::gps_control>(cmd);
        quit();
    }
};

int main(int argc, char* argv[]) { return goby::run<GPSController>(argc, argv); }
