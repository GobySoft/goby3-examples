#include <cmath>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <goby/middleware/marshalling/dccl.h>
#include <goby/middleware/marshalling/protobuf.h>

#include <goby/middleware/io/line_based/serial.h>
#include <goby/util/linebasedcomms/nmea_sentence.h>
#include <goby/zeromq/application/multi_thread.h>

#include "config.pb.h"
#include "messages/gps.pb.h"
#include "messages/groups.h"

using protobuf::GPSCommand;
using protobuf::GPSPosition;

using AppBase = goby::zeromq::MultiThreadApplication<GPSDriverConfig>;
using ThreadBase = goby::middleware::SimpleThread<GPSDriverConfig>;
namespace si = boost::units::si;

using goby::glog;

class GPSParseThread : public ThreadBase
{
  public:
    GPSParseThread(const GPSDriverConfig& cfg) : ThreadBase(cfg)
    {
        // subscribe to data from the serial port
        interthread().subscribe<groups::gps_raw_in, goby::middleware::protobuf::IOData>(
            [this](std::shared_ptr<const goby::middleware::protobuf::IOData> io_msg) {
                parse_line(io_msg->data());
            });
    }
    ~GPSParseThread() {}

    void parse_line(const std::string& line)
    {
        glog.is_verbose() && glog << "GPSParseThread: " << line << std::endl;

        try
        {
            goby::util::NMEASentence nmea(line);

            if (nmea.sentence_id() == "GGA")
            {
                GPSPosition gps;

                enum
                {
                    TALKER = 0,
                    UTC = 1,
                    LAT = 2,
                    LAT_HEMI = 3,
                    LON = 4,
                    LON_HEMI = 5,
                    FIX_QUALITY = 6
                };

                gps.set_time(nmea_time_to_seconds(nmea.as<double>(UTC)));
                gps.set_latitude(
                    nmea_geo_to_decimal(nmea.as<double>(LAT), nmea.as<char>(LAT_HEMI)));
                gps.set_longitude(
                    nmea_geo_to_decimal(nmea.as<double>(LON), nmea.as<char>(LON_HEMI)));

                interprocess().publish<groups::gps_data>(gps);
            }
        }
        catch (goby::util::bad_nmea_sentence& e)
        {
            glog.is_warn() && glog << "Invalid NMEA sentence: " << e.what() << std::endl;
        }
    }

  private:
    // given a time in "NMEA form", returns the value as seconds since the start of the day
    // NMEA form is HHMMSS.SSSS where "H" is hours, "M" is minutes, "S" is seconds or fractional seconds
    double nmea_time_to_seconds(double nmea_time)
    {
        double hours = std::floor(nmea_time / 1e4);
        nmea_time -= hours * 1e4;
        double minutes = std::floor(nmea_time / 1e2);
        nmea_time -= minutes * 1e2;
        double seconds = nmea_time;

        return hours * 3600 + minutes * 60 + seconds;
    }

    // given a latitude or longitude in "NMEA form" and the hemisphere character ('N', 'S', 'E' or 'W'), returns the value as decimal degrees
    // NMEA form is DDDMM.MMMM or DDMM.MMMM where "D" is degrees, and "M" is minutes
    double nmea_geo_to_decimal(double nmea_geo, char hemi)
    {
        // DDMM.MMMM
        double deg_int = std::floor(nmea_geo / 1e2);
        double deg_frac = (nmea_geo - (deg_int * 1e2)) / 60;

        double sign = 1;
        if (hemi == 'S' || hemi == 'W')
            sign = -1;

        return sign * (deg_int + deg_frac);
    }
};

class GPSAnalyzeThread : public ThreadBase
{
  public:
    GPSAnalyzeThread(const GPSDriverConfig& cfg) : ThreadBase(cfg)
    {
        interthread().subscribe<groups::gps_data, GPSPosition>([](const GPSPosition& pos) {
            glog.is_verbose() && glog << "GPSAnalyzeThread: " << pos.ShortDebugString()
                                      << std::endl;
        });
    }
};

class GPSDriver : public AppBase
{
  public:
    using SerialThread =
        goby::middleware::io::SerialThreadLineBased<groups::gps_raw_in, groups::gps_raw_out>;

    GPSDriver()
    {
        interprocess().subscribe<groups::gps_control, GPSCommand>(
            [this](const GPSCommand& cmd) { this->incoming_command(cmd); });

        launch_thread<SerialThread>(cfg().serial());
        launch_thread<GPSParseThread>();
        launch_thread<GPSAnalyzeThread>();
    }

    void incoming_command(const GPSCommand& cmd)
    {
        glog.is_verbose() && glog << "GPSDriver (main thread): incoming command: "
                                  << cmd.ShortDebugString() << std::endl;
        try
        {
            if (cmd.read_gps())
                launch_thread<SerialThread>(cfg().serial());
            else
                join_thread<SerialThread>();
        }
        catch (goby::Exception& e)
        {
            glog.is_verbose() && glog << "Did not process command. Reason: " << e.what()
                                      << std::endl;
        }
    }
};

// Use a custom configurator to modify the configuration after default protobuf parser to set default serial parameters for a GPS.
class GPSDriverConfigurator
    : public goby::middleware::ProtobufConfigurator<GPSDriverConfig>
{
  public:
    GPSDriverConfigurator(int argc, char* argv[])
        : goby::middleware::ProtobufConfigurator<GPSDriverConfig>(argc, argv)
    {
        GPSDriverConfig& cfg = mutable_cfg();
        goby::middleware::protobuf::SerialConfig& serial_cfg = *cfg.mutable_serial();

        // set default baud
        if (!serial_cfg.has_baud())
            serial_cfg.set_baud(4800);
        // set default end of line
        if (!serial_cfg.has_end_of_line())
            serial_cfg.set_end_of_line("\r\n");
    }
};

int main(int argc, char* argv[]) { return goby::run<GPSDriver>(GPSDriverConfigurator(argc, argv)); }
