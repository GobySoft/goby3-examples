#ifndef MULTITHREAD_SUBSCRIBER_H
#define MULTITHREAD_SUBSCRIBER_H


#include "goby/middleware/marshalling/protobuf.h"

#include "goby/middleware/application/multi_thread.h"

#include "messages/groups.h"
#include "messages/nav.pb.h"

#include "config.pb.h"

using ThreadBase = goby::middleware::SimpleThread<BasicMultithreadPubSubConfig>;

class BasicSubscriber : public ThreadBase
{
  public:
    BasicSubscriber(const BasicMultithreadPubSubConfig& config, int index)
        : ThreadBase(config, 0, index)
    {
        auto nav_callback = [this](const protobuf::NavigationReport& nav) {
            this->incoming_nav(nav);
        };

        // subscribe only on the interthread layer
        interthread().subscribe<groups::nav, protobuf::NavigationReport>(nav_callback);
        goby::glog.add_group("subscriber" + std::to_string(ThreadBase::index()),
                             goby::util::Colors::green);
    }

    // called each time a NavigationReport on the Group "navigation" is received
    void incoming_nav(const protobuf::NavigationReport& nav)
    {
        using namespace goby::util::tcolor;
        goby::glog.is_verbose() &&
            goby::glog << group("subscriber" + std::to_string(ThreadBase::index())) << green
                       << "Rx: " << nocolor << nav.DebugString() << std::flush;
    }
};

#endif
