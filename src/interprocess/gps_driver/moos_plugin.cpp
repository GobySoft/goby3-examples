#include "goby/middleware/marshalling/protobuf.h"
#include "goby/moos/middleware/moos_plugin_translator.h"
#include "goby/zeromq/application/multi_thread.h"

#include "messages/gps.pb.h"
#include "messages/groups.h"

using goby::glog;
using protobuf::GPSPosition;
using namespace goby::util::logger;

class NavTranslation : public goby::moos::Translator
{
  public:
    using Base = goby::moos::Translator;

    NavTranslation(const goby::apps::moos::protobuf::GobyMOOSGatewayConfig& cfg) : Base(cfg)
    {
        goby().interprocess().subscribe<groups::gps_data, GPSPosition>(
            [this](const GPSPosition& pos) { this->goby_to_moos(pos); });
        
        moos().add_buffer("TEST_LONGITUDE");
        moos().add_trigger("TEST_LATITUDE",
                                 [this](const CMOOSMsg& moos_msg) { moos_to_goby(moos_msg); });
    }

  private:
    void moos_to_goby(const CMOOSMsg& moos_msg)
    {
        const auto& key = moos_msg.GetKey();

        if (key == "TEST_LATITUDE")
        {
            try
            {
                GPSPosition pos;
                pos.set_time(moos_msg.GetTime());
                pos.set_latitude(moos_msg.GetDouble());
                pos.set_longitude(moos().buffer().at("TEST_LONGITUDE").GetDouble());
                goby().interprocess().publish<groups::gps_data>(pos);
                glog.is(DEBUG1) && glog << "Goby: gps_data: " << pos.ShortDebugString()
                                        << std::endl;
            }
            catch (std::exception& e)
            {
                glog.is(DEBUG1) && glog << "Failed to create GPSPosition from TEST_LATITUDE and "
                                           "TEST_LONGITUDE. Make sure both are published? "
                                        << e.what() << std::endl;
            }
        }
    }

    void goby_to_moos(const GPSPosition& pos)
    {
        glog.is(DEBUG1) && glog << "MOOS: GPS_LATITUDE: " << pos.latitude() << std::endl;
        moos().comms().Notify("GPS_LATITUDE", pos.latitude());

        glog.is(DEBUG1) && glog << "MOOS: GPS_LONGITUDE: " << pos.longitude() << std::endl;
        moos().comms().Notify("GPS_LONGITUDE", pos.longitude());
    }
};

extern "C"
{
    void goby3_moos_gateway_load(
        goby::zeromq::MultiThreadApplication<goby::apps::moos::protobuf::GobyMOOSGatewayConfig>*
            handler)
    {
        handler->launch_thread<NavTranslation>();
    }

    void goby3_moos_gateway_unload(
        goby::zeromq::MultiThreadApplication<goby::apps::moos::protobuf::GobyMOOSGatewayConfig>*
            handler)
    {
        handler->join_thread<NavTranslation>();
    }
}
