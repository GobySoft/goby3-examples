#ifndef MULTITHREAD_SUBSCRIBER_H
#define MULTITHREAD_SUBSCRIBER_H

#include "goby/middleware/single-thread-application.h"

#include "messages/nav.pb.h"
#include "messages/groups.h"

#include "config.pb.h"

using AppBase = goby::MultiThreadApplication<BasicMultithreadPubSubConfig>;
using ThreadBase = AppBase::ThreadBase;

class BasicSubscriber : public ThreadBase
{
public:
    BasicSubscriber(const BasicMultithreadPubSubConfig& config, ThreadBase::Transporter* t)
        : ThreadBase(config, t)
        {
            auto nav_callback = [this] (const protobuf::NavigationReport& nav)
                { this->incoming_nav(nav); };

            // subscribe only on the interthread layer
            transporter().inner().subscribe<groups::nav, protobuf::NavigationReport>(nav_callback);
        }

    // called each time a NavigationReport on the Group "navigation" is received
    void incoming_nav(const protobuf::NavigationReport& nav)
        {
            std::cout << "Rx: " << nav.DebugString() << std::flush;
        }

    void loop() override
    {
        goby::glog.is(goby::common::logger::VERBOSE) && goby::glog << goby::common::goby_time() << std::endl;
    }
    
};

#endif
