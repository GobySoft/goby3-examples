#ifndef MULTITHREAD_PUBLISHER_H
#define MULTITHREAD_PUBLISHER_H

#include "goby/middleware/multi-thread-application.h"
#include "messages/nav.pb.h" 
#include "messages/groups.h"
#include "config.pb.h"

class BasicPublisher : public goby::SimpleThread<BasicMultithreadPubSubConfig>
{
public:
BasicPublisher(const BasicMultithreadPubSubConfig& config)
    : goby::SimpleThread<BasicMultithreadPubSubConfig>(config, 10 /*hertz*/)
        {
            // goby::glog is thread safe in goby::MultiThreadApplication, std::cout is not
            using goby::glog;
            using namespace goby::common::logger;
            glog.is(VERBOSE) && glog << "My configuration int is: " << cfg().my_value() << std::endl;
        }

    void loop() override
        {
            using goby::glog;
            using namespace goby::common::logger;

            protobuf::NavigationReport nav;
            nav.set_x(95 + std::rand() % 20);
            nav.set_y(195 + std::rand() % 20);
            nav.set_z(-305 + std::rand() % 10);
            
            glog.is(VERBOSE) && glog << "Tx: " << nav.DebugString() << std::flush;
            interthread().publish<groups::nav>(nav);
        }    
};

#endif
