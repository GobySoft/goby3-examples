#ifndef MULTITHREAD_SUBSCRIBER_H
#define MULTITHREAD_SUBSCRIBER_H

#include "goby/middleware/single-thread-application.h"

#include "messages/groups.h"
#include "messages/nav.pb.h"

#include "config.pb.h"

using ThreadBase = goby::SimpleThread<BasicMultithreadPubSubConfig>;

class BasicSubscriber : public ThreadBase
{
  public:
    BasicSubscriber(const BasicMultithreadPubSubConfig& config, int index)
        : ThreadBase(config, 0, index)
    {
        interthread().subscribe<groups::string_msg, std::string>(
            [this](const std::string& msg) { this->incoming_msg(msg); });
    }

    void incoming_msg(const std::string& msg)
    {
        goby::glog.is_verbose() && goby::glog << "Rx: " << ThreadBase::index() << ": " << msg
                                              << std::endl;
    }
};

#endif
