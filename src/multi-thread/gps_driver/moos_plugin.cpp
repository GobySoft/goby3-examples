#include "goby/moos/middleware/moos_plugin_translator.h"

#include "messages/gps.pb.h"
#include "messages/groups.h"

using protobuf::GPSPosition;
using goby::glog;
using namespace goby::common::logger;

class NavTranslation : public goby::moos::Translator<NavTranslation>
{
public:
    using Base = goby::moos::Translator<NavTranslation>;
    
    NavTranslation(const GobyMOOSGatewayConfig& cfg) :
        Base(cfg)
        {
            Base::goby_comms().subscribe<groups::gps_data, GPSPosition>(
                [this](const GPSPosition& pos) { this->goby_to_moos(pos); }
                );                

            Base::moos_comms().Register("TEST_LATITUDE");
            Base::moos_comms().Register("TEST_LONGITUDE");
            Base::add_moos_trigger("TEST_LATITUDE");
            Base::add_moos_buffer("TEST_LONGITUDE");
            
        }

private:
    
    void moos_to_goby(const CMOOSMsg& moos_msg) override
        {
            const auto& key = moos_msg.GetKey();

            if(key == "TEST_LATITUDE")
            {
                try
                {
                    GPSPosition pos;
                    pos.set_time(moos_msg.GetTime());
                    pos.set_latitude(moos_msg.GetDouble());
                    pos.set_longitude(Base::moos_buffer().at("TEST_LONGITUDE").GetDouble());
                    Base::goby_comms().publish<groups::gps_data>(pos);
                }
                catch(std::exception& e)
                {
                    glog.is(DEBUG1) && glog << "Failed to create GPSPosition from TEST_LATITUDE and TEST_LONGITUDE. Make sure both are published?" << std::endl;    
                }
            }
        }

    void goby_to_moos(const GPSPosition& pos)
        {
            Base::moos_comms().Notify("GPS_LATITUDE", pos.latitude());
            Base::moos_comms().Notify("GPS_LONGITUDE", pos.longitude());
        }
};

extern "C"
{
    void goby3_moos_gateway_load(goby::MultiThreadApplication<GobyMOOSGatewayConfig>* handler)
    {
        handler->launch_thread<NavTranslation>();
    }
    
    void goby3_moos_gateway_unload(goby::MultiThreadApplication<GobyMOOSGatewayConfig>* handler)
    {
        handler->join_thread<NavTranslation>();
    }
}

