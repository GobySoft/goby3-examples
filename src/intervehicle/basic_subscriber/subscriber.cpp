#include <goby/middleware/marshalling/dccl.h>

#include <goby/middleware/protobuf/intervehicle.pb.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h"
#include "messages/groups.h"
#include "messages/nav_dccl.pb.h"

using Base = goby::zeromq::SingleThreadApplication<BasicSubscriberConfig>;
using dccl::NavigationReport;

using goby::middleware::intervehicle::protobuf::AckData;
using goby::middleware::intervehicle::protobuf::ExpireData;
using goby::middleware::intervehicle::protobuf::Subscription;

class BasicSubscriber : public Base
{
  public:
    BasicSubscriber()
    {
        // callback when data are received
        auto on_nav = [](const NavigationReport& nav) {
            std::cout << "Received: " << nav.ShortDebugString() << std::endl;
        };

        // optional callback to indicate that our subscription was ack'd
        auto on_nav_subscribed = [](const Subscription& sub, const AckData& ack) {
            std::cout << "Received acknowledgment:\n\t" << ack.ShortDebugString()
                      << "\nfor subscription:\n\t" << sub.ShortDebugString() << std::endl;
        };

        // optional callback to indicate that our subscription expired before being ack'd
        auto on_nav_subscribe_expired = [](const Subscription& sub, const ExpireData& expire) {
            std::cout << "Received expiration notice:\n\t" << expire.ShortDebugString()
                      << "\nfor subscription:\n\t" << sub.ShortDebugString() << std::endl;
        };

        // optional callback to modify the data message using parts of the link header if desired
        auto on_nav_header = [](NavigationReport& nav,
                                const goby::middleware::intervehicle::protobuf::Header& header) {
            std::cout << "\nLink Header: " << header.ShortDebugString() << std::endl;
            // could add parts of header to message here which would then be included when on_nav callback is called
        };

        goby::middleware::Subscriber<NavigationReport> nav_subscriber(
            cfg().nav_subscriber_cfg(), on_nav_subscribed, on_nav_subscribe_expired, on_nav_header);

        intervehicle().subscribe<groups::nav, NavigationReport>(on_nav, nav_subscriber);
    }
};

int main(int argc, char* argv[]) { return goby::run<BasicSubscriber>(argc, argv); }
