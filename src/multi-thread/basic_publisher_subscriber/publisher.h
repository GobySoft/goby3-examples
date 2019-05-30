#ifndef MULTITHREAD_PUBLISHER_H
#define MULTITHREAD_PUBLISHER_H

#include "config.pb.h"
#include "goby/middleware/multi-thread-application.h"
#include "messages/groups.h"
#include "messages/nav.pb.h"

class BasicPublisher : public goby::SimpleThread<BasicMultithreadPubSubConfig>
{
  public:
    BasicPublisher(const BasicMultithreadPubSubConfig& config)
        : goby::SimpleThread<BasicMultithreadPubSubConfig>(config, 1 /*hertz*/)
    {
        // goby::glog is thread safe in goby::MultiThreadApplication, std::cout is not
        goby::glog.is_verbose() && goby::glog << "My configuration int is: " << cfg().my_value()
                                              << std::endl;
    }

    void loop() override
    {
        protobuf::NavigationReport nav;
        nav.set_x(95 + std::rand() % 20);
        nav.set_y(195 + std::rand() % 20);
        nav.set_z(-305 + std::rand() % 10);

        goby::glog.is_verbose() && goby::glog << "Tx: " << nav.DebugString() << std::flush;
        interthread().publish<groups::nav>(nav);
    }
};

#endif
