#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/io/line_based/serial.h>
#include <goby/zeromq/application/multi_thread.h>
#include <goby/middleware/group.h>

#include "config.pb.h"
#include "messages/groups.h"


using goby::glog;

GOBY_DEFINE_GROUP(groups, serial_in)
GOBY_DEFINE_GROUP(groups, serial_out)

using AppBase = goby::zeromq::MultiThreadApplication<SerialExampleConfig>;
using ThreadBase = goby::middleware::SimpleThread<SerialExampleConfig>;

class SerialDataHandleThread : public ThreadBase
{
  public:
    SerialDataHandleThread(const SerialExampleConfig& cfg) : ThreadBase(cfg)
    {
        interthread().subscribe<groups::serial_in>([this](const goby::middleware::protobuf::IOData& data) {
            glog.is_verbose() && glog << data.DebugString() << std::endl;
        });
    }

    ~SerialDataHandleThread() {}
};

class SerialExample : public AppBase
{
  public:
    SerialExample() : AppBase()
    {
        using SerialThread = goby::middleware::io::SerialThreadLineBased<groups::serial_in, groups::serial_out>;

        launch_thread<SerialThread>(cfg().serial());
        launch_thread<SerialDataHandleThread>();
    }
};

int main(int argc, char* argv[]) { return goby::run<SerialExample>(argc, argv); }
