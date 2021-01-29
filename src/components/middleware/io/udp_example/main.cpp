#include <cmath>
#include <iostream>
#include <linux/can.h>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/io/udp_one_to_many.h>
#include <goby/middleware/io/udp_point_to_point.h>
#include <goby/util/linebasedcomms/nmea_sentence.h>
#include <goby/zeromq/application/multi_thread.h>
#include <goby/middleware/group.h>

#include "config.pb.h"
#include "messages/gps.pb.h"
#include "messages/groups.h"


using goby::glog;

constexpr goby::middleware::Group udp_server_in{"udp_server_in"};
constexpr goby::middleware::Group udp_server_out{"udp_server_out"};

constexpr goby::middleware::Group udp_client1_in{"udp_client1_in"};
constexpr goby::middleware::Group udp_client1_out{"udp_client1_out"};

constexpr goby::middleware::Group udp_client2_in{"udp_client2_in"};
constexpr goby::middleware::Group udp_client2_out{"udp_client2_out"};

// Define a Configurator to provide reasonable configuration defaults so this example can be run without manual configuration
class UDPConfigurator : public goby::middleware::ProtobufConfigurator<UDPExampleConfig>
{
  public:
    UDPConfigurator(int argc, char* argv[])
        : goby::middleware::ProtobufConfigurator<UDPExampleConfig>(argc, argv)
    {
        UDPExampleConfig& cfg = mutable_cfg();

        const int udp_server_port = 53221;
        const char* udp_server_addr = "127.0.0.1";
        // don't override user reconfiguration at runtime
        if (!cfg.has_udp_server_config())
        {
            // set up the first udp thread as a one->many
            auto& udp_server = *cfg.mutable_udp_server_config();
            udp_server.set_bind_port(udp_server_port);
        }

        if (!cfg.has_udp_client1_config())
        {
            // set up the second and third udp thread as a point-to-point with the first
            auto& udp_client1 = *cfg.mutable_udp_client1_config();
            // dynamically assign
            udp_client1.set_bind_port(0);
            udp_client1.set_remote_address(udp_server_addr);
            udp_client1.set_remote_port(udp_server_port);
        }

        if (!cfg.has_udp_client2_config())
        {
            // same config as client1
            *cfg.mutable_udp_client2_config() = cfg.udp_client1_config();
        }
    }
};

using AppBase = goby::zeromq::MultiThreadApplication<UDPExampleConfig>;
using ThreadBase = goby::middleware::SimpleThread<UDPExampleConfig>;

class UDPExample : public AppBase
{
  public:
    UDPExample() : AppBase(1.0 * boost::units::si::hertz)
    {
        glog.add_group("server", goby::util::Colors::lt_magenta);
        glog.add_group("client1", goby::util::Colors::lt_green);
        glog.add_group("client2", goby::util::Colors::lt_blue);

        // subscribe to incoming data from server thread
        interthread().subscribe<udp_server_in>(
            [this](const goby::middleware::protobuf::IOData& udp_data_in) {
                glog.is_verbose() && glog << group("server")
                                          << "Got request: " << udp_data_in.ShortDebugString()
                                          << std::endl;
                // respond to request
                goby::middleware::protobuf::IOData udp_data_out;
                udp_data_out.set_data("Response");
                *udp_data_out.mutable_udp_dest() = udp_data_in.udp_src();
                interthread().publish<udp_server_out>(udp_data_out);
            });

        // subscribe to incoming data from client thread 1
        interthread().subscribe<udp_client1_in>(
            [this](const goby::middleware::protobuf::IOData& udp_data_in) {
                glog.is_verbose() && glog << group("client1")
                                          << "Got response: " << udp_data_in.ShortDebugString()
                                          << std::endl;
            });

        // subscribe to incoming data from client thread 2
        interthread().subscribe<udp_client2_in>(
            [this](const goby::middleware::protobuf::IOData& udp_data_in) {
                glog.is_verbose() && glog << group("client2")
                                          << "Got response: " << udp_data_in.ShortDebugString()
                                          << std::endl;
            });


        // launch the UDP threads
        using UDPServerThread =
            goby::middleware::io::UDPOneToManyThread<udp_server_in, udp_server_out>;
        using UDPClient1Thread =
            goby::middleware::io::UDPPointToPointThread<udp_client1_in, udp_client1_out>;
        using UDPClient2Thread =
            goby::middleware::io::UDPPointToPointThread<udp_client2_in, udp_client2_out>;

        launch_thread<UDPServerThread>(cfg().udp_server_config());
        launch_thread<UDPClient1Thread>(cfg().udp_client1_config());
        launch_thread<UDPClient2Thread>(cfg().udp_client2_config());
    }

  private:
    void loop() override;
};

int main(int argc, char* argv[]) { return goby::run<UDPExample>(UDPConfigurator(argc, argv)); }

void UDPExample::loop()
{
    // transmit messages from both UDP clients
    goby::middleware::protobuf::IOData udp_data;
    udp_data.set_data("Request");
    // no udp_dest required in IOData message because we're using UDPPointToPointThread with a pre-configured destination for all messages
    
    interthread().publish<udp_client1_out>(udp_data);
    interthread().publish<udp_client2_out>(udp_data);
}
