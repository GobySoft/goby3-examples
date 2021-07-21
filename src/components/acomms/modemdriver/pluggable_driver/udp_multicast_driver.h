// Copyright 2011-2021:
//   GobySoft, LLC (2013-)
//   Massachusetts Institute of Technology (2007-2014)
//   Community contributors (see AUTHORS file)
// File authors:
//   Toby Schneider <toby@gobysoft.org>
//
//
// This file is part of the Goby Underwater Autonomy Project Libraries
// ("The Goby Libraries").
//
// The Goby Libraries are free software: you can redistribute them and/or modify
// them under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// The Goby Libraries are distributed in the hope that they will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Goby.  If not, see <http://www.gnu.org/licenses/>.

// copy of UDPMulticastDriver used to demonstrate third-party acomms modem driver plugins

#ifndef GOBY3_EXAMPLES_SRC_COMPONENTS_ACOMMS_MODEMDRIVER_PLUGGABLE_DRIVER
#define GOBY3_EXAMPLES_SRC_COMPONENTS_ACOMMS_MODEMDRIVER_PLUGGABLE_DRIVER

#include <array>                 // for array
#include <boost/asio/ip/udp.hpp> // for udp, udp::...
#include <cstddef>               // for size_t
#include <cstdint>               // for uint32_t
#include <map>                   // for map

#include "goby/acomms/modemdriver/driver_base.h" // for ModemDrive...
#include "goby/acomms/protobuf/driver_base.pb.h" // for DriverConfig
#include "goby/util/asio_compat.h"

#include "udp_multicast_driver.pb.h"

extern "C"
{
    const char* goby_driver_name();
    goby::acomms::ModemDriverBase* goby_make_driver();
}

namespace boost
{
namespace system
{
class error_code;
} // namespace system
} // namespace boost

namespace goby
{
namespace acomms
{
namespace protobuf
{
class ModemTransmission;
} // namespace protobuf
} // namespace acomms
} // namespace goby

class PluggableUDPMulticastDriver : public goby::acomms::ModemDriverBase
{
  public:
    PluggableUDPMulticastDriver();
    ~PluggableUDPMulticastDriver() override;

    void startup(const goby::acomms::protobuf::DriverConfig& cfg) override;
    void shutdown() override;
    void do_work() override;
    void handle_initiate_transmission(const goby::acomms::protobuf::ModemTransmission& m) override;

  private:
    void start_send(const goby::acomms::protobuf::ModemTransmission& m);
    void send_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
    void start_receive();
    void receive_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
    void receive_message(const goby::acomms::protobuf::ModemTransmission& m);

    const udp_multicast_plugin::protobuf::Config& multicast_driver_cfg() const
    {
        return driver_cfg_.GetExtension(udp_multicast_plugin::protobuf::config);
    }

  private:
    goby::acomms::protobuf::DriverConfig driver_cfg_;
    boost::asio::io_context io_context_;
    boost::asio::ip::udp::socket socket_{io_context_};
    boost::asio::ip::udp::endpoint receiver_;
    boost::asio::ip::udp::endpoint sender_;

    // (16 bit length = 65535 - 8 byte UDP header - 20 byte IP
    static constexpr size_t UDP_MAX_PACKET_SIZE = 65507;

    std::array<char, UDP_MAX_PACKET_SIZE> receive_buffer_;
    std::uint32_t next_frame_{0};

    std::map<int, int> rate_to_bytes_;
};

#endif
