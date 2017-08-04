#ifndef MULTITHREAD_PUBLISHER_H
#define MULTITHREAD_PUBLISHER_H

#include "goby/middleware/multi-thread-application.h"
#include "messages/nav.pb.h" 
#include "messages/groups.h"
#include "config.pb.h" 

using ThreadBase = goby::Thread<BasicMultithreadPubSubConfig, goby::InterProcessForwarder<goby::InterThreadTransporter>>;

class BasicPublisher : public ThreadBase
{
public:
BasicPublisher(const BasicMultithreadPubSubConfig& config, ThreadBase::Transporter* t)
    : ThreadBase(config, t, 10 /*hertz*/)
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
            // publish only on the interthread layer (transporter().inner(), since transporter is an InterProcessForwarder)
            transporter().inner().publish<groups::nav>(nav);
        }    
};

#endif
