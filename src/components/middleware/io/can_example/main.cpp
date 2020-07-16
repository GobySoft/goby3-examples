#include <cmath>
#include <iostream>
#include <linux/can.h>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "goby/middleware/marshalling/dccl.h"
#include "goby/middleware/marshalling/protobuf.h"

#include "goby/middleware/io/can.h"
#include "goby/util/linebasedcomms/nmea_sentence.h"
#include "goby/zeromq/application/multi_thread.h"

#include "config.pb.h"
#include "messages/gps.pb.h"
#include "messages/groups.h"

#include "goby/middleware/group.h"

using goby::glog;

constexpr goby::middleware::Group can_in{"can_in"};
constexpr goby::middleware::Group can_out{"can_out"};

using AppBase = goby::zeromq::MultiThreadApplication<CanExampleConfig>;
using ThreadBase = goby::middleware::SimpleThread<CanExampleConfig>;

class CanDataHandleThread : public ThreadBase
{
  public:
    CanDataHandleThread(const CanExampleConfig& cfg) : ThreadBase(cfg)
    {
        interthread().subscribe<can_in, can_frame>([this](const can_frame& rec_frame) {
            if (glog.is_verbose())
            {
                glog << "Data_rec: " << std::hex << rec_frame.can_id << "  ";
                for (int i = 0; i < rec_frame.can_dlc; i++)
                { glog << std::hex << int(rec_frame.data[i]) << " "; }
                glog << std::dec << std::endl;
            }
        });
    }

    ~CanDataHandleThread() {}
};

class CanExample : public AppBase
{
  public:
    CanExample() : AppBase()
    {
        using CanThread = goby::middleware::io::CanThread<can_in, can_out>;

        launch_thread<CanThread>(cfg().can_config());
        launch_thread<CanDataHandleThread>();
    }
};

class CanConfigurator : public goby::middleware::ProtobufConfigurator<CanExampleConfig>
{
  public:
    CanConfigurator(int argc, char* argv[])
        : goby::middleware::ProtobufConfigurator<CanExampleConfig>(argc, argv)
    {
        CanExampleConfig& cfg = mutable_cfg();

        auto* can_config = cfg.mutable_can_config();
        if(!cfg.can_config().has_interface())
            can_config->set_interface("vcan0");

        auto* filter = can_config->add_filter();
        filter->set_can_id(0x1b4);
    }
};

int main(int argc, char* argv[]) { return goby::run<CanExample>(CanConfigurator(argc, argv)); }
