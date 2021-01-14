#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/io/line_based/pty.h>
#include <goby/zeromq/application/multi_thread.h>
#include <goby/middleware/group.h>

#include "config.pb.h"
#include "messages/groups.h"


using goby::glog;

constexpr goby::middleware::Group pty_in{"pty_in"};
constexpr goby::middleware::Group pty_out{"pty_out"};

using AppBase = goby::zeromq::MultiThreadApplication<PTYExampleConfig>;
using ThreadBase = goby::middleware::SimpleThread<PTYExampleConfig>;

class PTYDataHandleThread : public ThreadBase
{
  public:
    PTYDataHandleThread(const PTYExampleConfig& cfg) : ThreadBase(cfg)
    {
        interthread().subscribe<pty_in>([this](const goby::middleware::protobuf::IOData& data) {
            glog.is_verbose() && glog << data.DebugString() << std::endl;
        });
    }

    ~PTYDataHandleThread() {}
};

class PTYExample : public AppBase
{
  public:
    PTYExample() : AppBase()
    {
        using PTYThread = goby::middleware::io::PTYThreadLineBased<pty_in, pty_out>;

        launch_thread<PTYThread>(cfg().pty_config());
        launch_thread<PTYDataHandleThread>();
    }
};

class PTYConfigurator : public goby::middleware::ProtobufConfigurator<PTYExampleConfig>
{
  public:
    PTYConfigurator(int argc, char* argv[])
        : goby::middleware::ProtobufConfigurator<PTYExampleConfig>(argc, argv)
    {
        PTYExampleConfig& cfg = mutable_cfg();

        auto* pty_config = cfg.mutable_pty_config();
        if (!cfg.pty_config().has_port())
            pty_config->set_port("/tmp/ttygoby3-example");
    }
};

int main(int argc, char* argv[]) { return goby::run<PTYExample>(PTYConfigurator(argc, argv)); }
