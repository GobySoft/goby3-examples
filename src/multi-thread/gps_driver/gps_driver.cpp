#include <vector>
#include <string>
#include <cmath>

#include <boost/asio.hpp>

#include "goby/middleware/multi-thread-application.h"
#include "goby/util/linebasedcomms/nmea_sentence.h"

#include "messages/gps.pb.h"
#include "messages/groups.h"
#include "config.pb.h"

using protobuf::GPSCommand;
using protobuf::GPSPosition;

using AppBase = goby::MultiThreadApplication<GPSDriverConfig>;
using ThreadBase = goby::SimpleThread<GPSDriverConfig>;
namespace si = boost::units::si;

using namespace goby::common::logger;
using goby::glog;

class GPSSerialThread : public ThreadBase
{
public:
    GPSSerialThread(const GPSDriverConfig& config)
        : ThreadBase(config, ThreadBase::loop_max_frequency()), // max loop frequency since we're going to block on serial I/O inside of loop
          serial_port_(io_, cfg().serial_port())
        {
            using boost::asio::serial_port_base;
            serial_port_.set_option(serial_port_base::baud_rate(cfg().serial_baud()));
            // no flow control
            serial_port_.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));

            // 8N1
            serial_port_.set_option(serial_port_base::character_size(8));
            serial_port_.set_option(serial_port_base::parity(serial_port_base::parity::none));
            serial_port_.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        }

    ~GPSSerialThread()
        {
            serial_port_.close();
        }
    
    void loop() override
        {
            boost::asio::read_until(serial_port_, buffer_, '\n');
            std::istream is(&buffer_);
            std::string line;
            std::getline(is, line);            
            glog.is(VERBOSE) && glog << "GPSSerialThread: " << line << std::endl;

            try
            {
                goby::util::NMEASentence nmea(line);

                if(nmea.sentence_id() == "GGA")
                {
                    GPSPosition gps;

                    enum { TALKER = 0,
                           UTC = 1,
                           LAT = 2,
                           LAT_HEMI = 3,
                           LON = 4,
                           LON_HEMI = 5,
                           FIX_QUALITY = 6 };

                    gps.set_time(nmea_time_to_seconds(nmea.as<double>(UTC)));
                    gps.set_latitude(nmea_geo_to_decimal(nmea.as<double>(LAT), nmea.as<char>(LAT_HEMI)));
                    gps.set_longitude(nmea_geo_to_decimal(nmea.as<double>(LON), nmea.as<char>(LON_HEMI)));
                
                    interprocess().publish<groups::gps_data>(gps);
                }
                
            }
            catch(goby::util::bad_nmea_sentence& e)
            {
                glog.is(WARN) && glog << "Invalid NMEA sentence: " << e.what() << std::endl;
            }
        }

    // given a time in "NMEA form", returns the value as seconds since the start of the day
    // NMEA form is HHMMSS.SSSS where "H" is hours, "M" is minutes, "S" is seconds or fractional seconds
    double nmea_time_to_seconds(double nmea_time)
        {
            double hours = std::floor(nmea_time / 1e4);
            nmea_time -= hours*1e4;
            double minutes = std::floor(nmea_time / 1e2);
            nmea_time -= minutes*1e2;
            double seconds = nmea_time;
    
            return hours*3600 + minutes*60 + seconds;    
        }

    // given a latitude or longitude in "NMEA form" and the hemisphere character ('N', 'S', 'E' or 'W'), returns the value as decimal degrees
    // NMEA form is DDDMM.MMMM or DDMM.MMMM where "D" is degrees, and "M" is minutes
    double nmea_geo_to_decimal(double nmea_geo, char hemi)
        {
            // DDMM.MMMM
            double deg_int = std::floor(nmea_geo / 1e2);
            double deg_frac = (nmea_geo - (deg_int * 1e2)) / 60;

            double sign = 1;
            if(hemi == 'S' || hemi == 'W')
                sign = -1;
    
            return sign*(deg_int + deg_frac);
        }


    
private:
    boost::asio::io_service io_;
    boost::asio::serial_port serial_port_;
    boost::asio::streambuf buffer_;

};

class GPSAnalyzeThread : public ThreadBase
{
public:
    GPSAnalyzeThread(const GPSDriverConfig& cfg)
        : ThreadBase(cfg)
        {
            interprocess().subscribe<groups::gps_data, GPSPosition>(
                [](const GPSPosition& pos)
                {
                    glog.is(VERBOSE) && glog << "GPSAnalyzeThread: " << pos.ShortDebugString() << std::endl;
                }
                );
        }    
};
    

class GPSDriver : public AppBase
{
public:
    GPSDriver()
        {
            interprocess().subscribe<groups::gps_control, GPSCommand>(
                [this] (const GPSCommand& cmd) { this->incoming_command(cmd); }
                );

            launch_thread<GPSAnalyzeThread>();
            launch_thread<GPSSerialThread>();
        }

    void incoming_command(const GPSCommand& cmd)
        {
            glog.is(VERBOSE) && glog << "GPSDriver (main thread): incoming command: " << cmd.ShortDebugString() << std::endl;
            try
            {
                if(cmd.read_gps())
                {
                    launch_thread<GPSSerialThread>();
                }
                else
                {
                    join_thread<GPSSerialThread>();
                }
            }
            catch(goby::Exception& e)
            {
                glog.is(VERBOSE) && glog << "Did not process command. Reason: " << e.what() << std::endl;
            }
        }

};



int main(int argc, char* argv[])
{ return goby::run<GPSDriver>(argc, argv); }
