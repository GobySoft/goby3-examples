#include "goby/middleware/marshalling/dccl.h"
#include "goby/middleware/protobuf/intervehicle.pb.h"
#include "goby/zeromq/application/single_thread.h"

#include "messages/groups.h"
#include "messages/nav_dccl.pb.h"

#include "config.pb.h"

using Base = goby::zeromq::SingleThreadApplication<UnsubscriberConfig>;
using dccl::NavigationReport;

using goby::middleware::intervehicle::protobuf::AckData;
using goby::middleware::intervehicle::protobuf::ExpireData;
using goby::middleware::intervehicle::protobuf::Subscription;

class Unsubscriber : public Base
{
  public:
    Unsubscriber() : Base(1.0 / (30.0 * boost::units::si::seconds))
    {
        auto on_nav_subscribed = [](const Subscription& sub, const AckData& ack) {
            std::cout << "Received acknowledgment:\n\t" << ack.ShortDebugString()
                      << "\nfor subscription:\n\t" << sub.ShortDebugString() << std::endl;
        };

        auto on_nav_subscribe_expired = [](const Subscription& sub, const ExpireData& expire) {
            std::cout << "Received expiration notice:\n\t" << expire.ShortDebugString()
                      << "\nfor subscription:\n\t" << sub.ShortDebugString() << std::endl;
        };

        nav_subscriber_ = {cfg().nav_subscriber_cfg(), on_nav_subscribed, on_nav_subscribe_expired};
    }

  private:
    void subscribe()
    {
        auto on_nav = [](const NavigationReport& nav) {
            std::cout << "\nReceived: " << nav.ShortDebugString() << std::endl;
        };
        
        std::cout << "========= Subscribing ========= " << std::endl;
        intervehicle().subscribe<groups::nav, NavigationReport>(on_nav, nav_subscriber_);
        subscribed_ = true;
    }
    void unsubscribe()
    {
        std::cout << "========= Unsubscribing ========= " << std::endl;
        intervehicle().unsubscribe<groups::nav, NavigationReport>(nav_subscriber_);
        subscribed_ = false;
    }

    void loop() override
    {
        if (!subscribed_)
            subscribe();
        else
            unsubscribe();
    }

  private:
    bool subscribed_{false};
    goby::middleware::Subscriber<NavigationReport> nav_subscriber_;
};

int main(int argc, char* argv[]) { return goby::run<Unsubscriber>(argc, argv); }
