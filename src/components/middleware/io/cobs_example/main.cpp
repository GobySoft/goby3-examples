#include <cmath>
#include <iostream>
#include <linux/can.h>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/group.h>
#include <goby/middleware/io/cobs/pty.h>
#include <goby/middleware/io/cobs/serial.h>
#include <goby/middleware/io/cobs/tcp_client.h>
#include <goby/middleware/io/cobs/tcp_server.h>
#include <goby/util/linebasedcomms/nmea_sentence.h>
#include <goby/zeromq/application/multi_thread.h>

#include "config.pb.h"
#include "messages/gps.pb.h"
#include "messages/groups.h"

using goby::glog;

constexpr goby::middleware::Group tcp_server_in{"tcp_server_in"};
constexpr goby::middleware::Group tcp_server_out{"tcp_server_out"};

constexpr goby::middleware::Group tcp_client_in{"tcp_client_in"};
constexpr goby::middleware::Group tcp_client_out{"tcp_client_out"};

constexpr goby::middleware::Group pty_in{"pty_in"};
constexpr goby::middleware::Group pty_out{"pty_out"};

constexpr goby::middleware::Group serial_in{"serial_in"};
constexpr goby::middleware::Group serial_out{"serial_out"};

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

        if (!cfg.has_tcp_client_config())
        {
            // set up the second and third tcp thread as a point-to-point with the first
            auto& tcp_client = *cfg.mutable_tcp_client_config();
            // dynamically assign
            tcp_client.set_remote_address(tcp_server_addr);
            tcp_client.set_remote_port(tcp_server_port);
        }

        // connect pty and serial
        if (!cfg.has_pty_config())
        {
            auto& pty = *cfg.mutable_pty_config();
            pty.set_port("/tmp/ttygoby3test");
            pty.set_baud(115200);
        }
        if (!cfg.has_serial_config())
        {
            auto& serial = *cfg.mutable_serial_config();
            serial.set_port("/tmp/ttygoby3test");
            serial.set_baud(115200);
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
        glog.add_group("client", goby::util::Colors::lt_green);
        glog.add_group("pty", goby::util::Colors::yellow);
        glog.add_group("serial", goby::util::Colors::lt_red);

        // subscribe to incoming data from server thread
        interthread().subscribe<tcp_server_in>(
            [this](const goby::middleware::protobuf::IOData& tcp_data_in) {
                glog.is_verbose() && glog << group("server")
                                          << "Got request: " << tcp_data_in.ShortDebugString()
                                          << std::endl;
                // respond to request
                goby::middleware::protobuf::IOData tcp_data_out;
                tcp_data_out.set_data("Response");
                *tcp_data_out.mutable_tcp_dest() = tcp_data_in.tcp_src();
                interthread().publish<tcp_server_out>(tcp_data_out);
            });

        // subscribe to events from server thread
        interthread().subscribe<tcp_server_in>(
            [this](const goby::middleware::protobuf::TCPServerEvent& event) {
                glog.is_verbose() && glog << group("server")
                                          << "Got event: " << event.ShortDebugString() << std::endl;
            });

        // subscribe to incoming data from client thread
        interthread().subscribe<tcp_client_in>(
            [this](const goby::middleware::protobuf::IOData& tcp_data_in) {
                glog.is_verbose() && glog << group("client")
                                          << "Got response: " << tcp_data_in.ShortDebugString()
                                          << std::endl;
            });

        // subscribe to incoming data from serial thread
        interthread().subscribe<serial_in>(
            [this](const goby::middleware::protobuf::IOData& serial_data_in) {
                glog.is_verbose() && glog << group("serial") << "Got message from pty: "
                                          << serial_data_in.ShortDebugString() << std::endl;
            });

        // subscribe to incoming data from pty thread
        interthread().subscribe<pty_in>(
            [this](const goby::middleware::protobuf::IOData& pty_data_in) {
                glog.is_verbose() && glog << group("pty") << "Got message from serial: "
                                          << pty_data_in.ShortDebugString() << std::endl;
            });

        // launch the TCP threads
        using TCPServerThread =
            goby::middleware::io::TCPServerThreadCOBS<tcp_server_in, tcp_server_out>;
        using TCPClientThread =
            goby::middleware::io::TCPClientThreadCOBS<tcp_client_in, tcp_client_out>;
        using PTYThread = goby::middleware::io::PTYThreadCOBS<pty_in, pty_out>;
        using SerialThread = goby::middleware::io::SerialThreadCOBS<serial_in, serial_out>;

        launch_thread<TCPServerThread>(cfg().tcp_server_config());
        launch_thread<TCPClientThread>(cfg().tcp_client_config());
        launch_thread<PTYThread>(cfg().pty_config());
        launch_thread<SerialThread>(cfg().serial_config());
    }

  private:
    void loop() override;
};

int main(int argc, char* argv[]) { return goby::run<TCPExample>(TCPConfigurator(argc, argv)); }

void TCPExample::loop()
{
    // transmit messages from the TCP clients
    goby::middleware::protobuf::IOData tcp_data;
    tcp_data.set_data("Request");
    // no tcp_dest required in IOData message because we're using TCPPointToPointThread with a pre-configured destination for all messages
    interthread().publish<tcp_client_out>(tcp_data);

    goby::middleware::protobuf::IOData serial_data;
    serial_data.set_data("ToPTY");
    interthread().publish<serial_out>(serial_data);

    goby::middleware::protobuf::IOData pty_data;
    pty_data.set_data("ToSerial");
    interthread().publish<pty_out>(pty_data);
}
