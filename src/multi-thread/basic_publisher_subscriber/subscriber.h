#ifndef MULTITHREAD_SUBSCRIBER_H
#define MULTITHREAD_SUBSCRIBER_H

#include "goby/middleware/single-thread-application.h"

#include "messages/nav.pb.h"
#include "messages/groups.h"

#include "config.pb.h"

using ThreadBase = goby::SimpleThread<BasicMultithreadPubSubConfig>;

class BasicSubscriber : public ThreadBase
{
public:
BasicSubscriber(const BasicMultithreadPubSubConfig& config, int index)
    : ThreadBase(config, 0, index)
        {
            auto nav_callback = [this] (const protobuf::NavigationReport& nav)
                { this->incoming_nav(nav); };

            // subscribe only on the interthread layer
            interthread().subscribe<groups::nav, protobuf::NavigationReport>(nav_callback);
        }

    // called each time a NavigationReport on the Group "navigation" is received
    void incoming_nav(const protobuf::NavigationReport& nav)
        {
            using goby::glog;
            using namespace goby::common::logger;

            glog.is(VERBOSE) && glog <<  "Rx: " << ThreadBase::index() <<  ": " << nav.DebugString() << std::flush;
        }
};

#endif