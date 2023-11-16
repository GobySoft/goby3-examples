#include <goby/middleware/marshalling/dccl.h>

#include <goby/middleware/protobuf/intervehicle.pb.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h" // Protobuf, defines SameGroupConfig
#include "messages/groups.h"
#include "messages/nav_dccl.pb.h" // DCCL version of NavigationWithID

// optional "using" declaration (reduces verbiage)
using Base = goby::zeromq::SingleThreadApplication<SameGroupConfig>;
using dccl::NavigationWithID;
using goby::middleware::intervehicle::protobuf::AckData;
using goby::middleware::intervehicle::protobuf::ExpireData;
using goby::middleware::intervehicle::protobuf::Subscription;

class SameGroup : public Base
{
  public:
    SameGroup() : Base(0.1 * boost::units::si::hertz)
    {
        auto on_nav_ack = [](const NavigationWithID& nav, const AckData& ack)
        {
            std::cout << "Received acknowledgment:\n\t" << ack.ShortDebugString()
                      << "\nfor message:\n\t" << nav.ShortDebugString() << std::endl;
        };

        auto on_nav_expired = [](const NavigationWithID& nav, const ExpireData& expire)
        {
            std::cout << "Received expiration notice:\n\t" << expire.ShortDebugString()
                      << "\nfor subscription:\n\t" << nav.ShortDebugString() << std::endl;
        };

        nav_publisher_ = {{}, on_nav_ack, on_nav_expired};

        // callback when data are received
        auto on_nav = [](const NavigationWithID& nav)
        { std::cout << "\n==== Received: " << nav.ShortDebugString() << std::endl; };

        // optional callback to indicate that our subscription was ack'd
        auto on_nav_subscribed = [](const Subscription& sub, const AckData& ack)
        {
            std::cout << "Received acknowledgment:\n\t" << ack.ShortDebugString()
                      << "\nfor subscription:\n\t" << sub.ShortDebugString() << std::endl;
        };

        // optional callback to indicate that our subscription expired before being ack'd
        auto on_nav_subscribe_expired = [](const Subscription& sub, const ExpireData& expire)
        {
            std::cout << "Received expiration notice:\n\t" << expire.ShortDebugString()
                      << "\nfor subscription:\n\t" << sub.ShortDebugString() << std::endl;
        };

        // optional callback to modify the data message using parts of the link header if desired
        auto on_nav_header = [](NavigationWithID& nav,
                                const goby::middleware::intervehicle::protobuf::Header& header)
        {
            std::cout << "\nLink Header: " << header.ShortDebugString() << std::endl;
            // could add parts of header to message here which would then be included when on_nav callback is called
        };

        goby::middleware::Subscriber<NavigationWithID> nav_subscriber(
            cfg().nav_subscriber_cfg(), on_nav_subscribed, on_nav_subscribe_expired, on_nav_header);

        intervehicle().subscribe<groups::nav, NavigationWithID>(on_nav, nav_subscriber);
    }

    void loop() override
    {
        NavigationWithID nav_with_id;
        nav_with_id.set_vehicle_id(cfg().vehicle_id());
        dccl::NavigationReport& nav = *nav_with_id.mutable_nav();
        nav.set_x(95 + std::rand() % 20);
        nav.set_y(195 + std::rand() % 20);
        nav.set_z(-305 + std::rand() % 10);

        std::cout << "\n==== Published: " << nav_with_id.ShortDebugString() << std::endl;

        intervehicle().publish<groups::nav>(nav_with_id, nav_publisher_);
    }

  private:
    goby::middleware::Publisher<NavigationWithID> nav_publisher_;
};

int main(int argc, char* argv[]) { return goby::run<SameGroup>(argc, argv); }
