#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "goby/middleware/marshalling/protobuf.h"

#include "goby/middleware/io/line_based/serial.h"
#include "goby/zeromq/application/multi_thread.h"

#include "config.pb.h"
#include "messages/groups.h"

#include "goby/middleware/group.h"

using goby::glog;

constexpr goby::middleware::Group serial_in{"serial_in"};
constexpr goby::middleware::Group serial_out{"serial_out"};

using AppBase = goby::zeromq::MultiThreadApplication<SerialExampleConfig>;
using ThreadBase = goby::middleware::SimpleThread<SerialExampleConfig>;

class SerialDataHandleThread : public ThreadBase
{
  public:
    SerialDataHandleThread(const SerialExampleConfig& cfg) : ThreadBase(cfg)
    {
        interthread().subscribe<serial_in>([this](const goby::middleware::protobuf::IOData& data) {
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
        using SerialThread = goby::middleware::io::SerialThreadLineBased<serial_in, serial_out>;

        launch_thread<SerialThread>(cfg().serial());
        launch_thread<SerialDataHandleThread>();
    }
};

int main(int argc, char* argv[]) { return goby::run<SerialExample>(argc, argv); }
