#include <cmath>
#include <iostream>
#include <linux/can.h>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/group.h>
#include <goby/middleware/io/line_based/tcp_client.h>
#include <goby/middleware/io/line_based/tcp_server.h>
#include <goby/util/linebasedcomms/nmea_sentence.h>
#include <goby/zeromq/application/multi_thread.h>

#include "config.pb.h"
#include "messages/gps.pb.h"
#include "messages/groups.h"

using goby::glog;

constexpr goby::middleware::Group tcp_server_in{"tcp_server_in"};
constexpr goby::middleware::Group tcp_server_out{"tcp_server_out"};

constexpr goby::middleware::Group tcp_client1_in{"tcp_client1_in"};
constexpr goby::middleware::Group tcp_client1_out{"tcp_client1_out"};

constexpr goby::middleware::Group tcp_client2_in{"tcp_client2_in"};
constexpr goby::middleware::Group tcp_client2_out{"tcp_client2_out"};

// Define a Configurator to provide reasonable configuration defaults so this example can be run without manual configuration
class TCPConfigurator : public goby::middleware::ProtobufConfigurator<TCPExampleConfig>
{
  public:
    TCPConfigurator(int argc, char* argv[])
        : goby::middleware::ProtobufConfigurator<TCPExampleConfig>(argc, argv)
    {
        TCPExampleConfig& cfg = mutable_cfg();

        const int tcp_server_port = 53221;
        const char* tcp_server_addr = "127.0.0.1";
        // don't override user reconfiguration at runtime
        if (!cfg.has_tcp_server_config())
        {
            // set up the first tcp thread as a one->many
            auto& tcp_server = *cfg.mutable_tcp_server_config();
            tcp_server.set_bind_port(tcp_server_port);
        }

        if (!cfg.has_tcp_client1_config())
        {
            // set up the second and third tcp thread as a point-to-point with the first
            auto& tcp_client1 = *cfg.mutable_tcp_client1_config();
            // dynamically assign
            tcp_client1.set_remote_address(tcp_server_addr);
            tcp_client1.set_remote_port(tcp_server_port);
        }

        if (!cfg.has_tcp_client2_config())
        {
            // same config as client1
            *cfg.mutable_tcp_client2_config() = cfg.tcp_client1_config();
        }
    }
};

using AppBase = goby::zeromq::MultiThreadApplication<TCPExampleConfig>;
using ThreadBase = goby::middleware::SimpleThread<TCPExampleConfig>;

class TCPExample : public AppBase
{
  public:
    TCPExample() : AppBase(1.0 * boost::units::si::hertz)
    {
        glog.add_group("server", goby::util::Colors::lt_magenta);
        glog.add_group("client1", goby::util::Colors::lt_green);
        glog.add_group("client2", goby::util::Colors::lt_blue);

        // subscribe to incoming data from server thread
        interthread().subscribe<tcp_server_in>(
            [this](const goby::middleware::protobuf::IOData& tcp_data_in) {
                glog.is_verbose() && glog << group("server")
                                          << "Got request: " << tcp_data_in.ShortDebugString()
                                          << std::endl;
                // respond to request
                goby::middleware::protobuf::IOData tcp_data_out;
                tcp_data_out.set_data("Response\n");
                *tcp_data_out.mutable_tcp_dest() = tcp_data_in.tcp_src();
                interthread().publish<tcp_server_out>(tcp_data_out);
            });

        // subscribe to events from server thread
        interthread().subscribe<tcp_server_in>(
            [this](const goby::middleware::protobuf::TCPServerEvent& event) {
                glog.is_verbose() && glog << group("server")
                                          << "Got event: " << event.ShortDebugString() << std::endl;
            });

        // subscribe to incoming data from client thread 1
        interthread().subscribe<tcp_client1_in>(
            [this](const goby::middleware::protobuf::IOData& tcp_data_in) {
                glog.is_verbose() && glog << group("client1")
                                          << "Got response: " << tcp_data_in.ShortDebugString()
                                          << std::endl;
            });

        // subscribe to incoming data from client thread 2
        interthread().subscribe<tcp_client2_in>(
            [this](const goby::middleware::protobuf::IOData& tcp_data_in) {
                glog.is_verbose() && glog << group("client2")
                                          << "Got response: " << tcp_data_in.ShortDebugString()
                                          << std::endl;
            });

        // launch the TCP threads
        using TCPServerThread =
            goby::middleware::io::TCPServerThreadLineBased<tcp_server_in, tcp_server_out>;
        using TCPClient1Thread =
            goby::middleware::io::TCPClientThreadLineBased<tcp_client1_in, tcp_client1_out>;
        using TCPClient2Thread =
            goby::middleware::io::TCPClientThreadLineBased<tcp_client2_in, tcp_client2_out>;

        launch_thread<TCPServerThread>(cfg().tcp_server_config());
        launch_thread<TCPClient1Thread>(cfg().tcp_client1_config());
        launch_thread<TCPClient2Thread>(cfg().tcp_client2_config());
    }

  private:
    void loop() override;
};

int main(int argc, char* argv[]) { return goby::run<TCPExample>(TCPConfigurator(argc, argv)); }

void TCPExample::loop()
{
    // transmit messages from both TCP clients
    goby::middleware::protobuf::IOData tcp_data;
    tcp_data.set_data("Request\n");
    // no tcp_dest required in IOData message because we're using TCPPointToPointThread with a pre-configured destination for all messages

    interthread().publish<tcp_client1_out>(tcp_data);
    interthread().publish<tcp_client2_out>(tcp_data);
}
