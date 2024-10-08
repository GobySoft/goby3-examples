#include <goby/middleware/marshalling/dccl.h>

#include <goby/middleware/protobuf/intervehicle.pb.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h"
#include "messages/groups.h"
#include "messages/nav_dccl.pb.h"

using Base = goby::zeromq::SingleThreadApplication<BroadcastSubscriberConfig>;
using dccl::NavigationReport;

using goby::middleware::intervehicle::protobuf::AckData;
using goby::middleware::intervehicle::protobuf::ExpireData;
using goby::middleware::intervehicle::protobuf::Subscription;

class BroadcastSubscriber : public Base
{
  public:
    BroadcastSubscriber()
    {
        // callback when data are received
        auto on_nav = [](const NavigationReport& nav)
        { std::cout << "\nReceived: " << nav.ShortDebugString() << std::endl; };

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

        goby::middleware::Subscriber<NavigationReport> nav_subscriber(
            cfg().nav_subscriber_cfg(), on_nav_subscribed, on_nav_subscribe_expired);

        intervehicle().subscribe<groups::nav, NavigationReport>(on_nav, nav_subscriber);
    }
};

int main(int argc, char* argv[]) { return goby::run<BroadcastSubscriber>(argc, argv); }
