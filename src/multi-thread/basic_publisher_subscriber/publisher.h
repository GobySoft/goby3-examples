#ifndef MULTITHREAD_PUBLISHER_H
#define MULTITHREAD_PUBLISHER_H

#include "goby/middleware/multi-thread-application.h"
#include "messages/nav.pb.h" 
#include "messages/groups.h"
#include "config.pb.h" 

using AppBase = goby::MultiThreadApplication<BasicMultithreadPubSubConfig>;
using ThreadBase = AppBase::ThreadBase;

class BasicPublisher : public ThreadBase
{
public:
BasicPublisher(const BasicMultithreadPubSubConfig& config, ThreadBase::Transporter* t)
    : ThreadBase(config, t, 0.01 /*hertz*/)
        {
            std::cout << "My configuration int is: " << cfg().my_value() << std::endl;
        }

    void loop() override
        {
            protobuf::NavigationReport nav;
            nav.set_x(95 + std::rand() % 20);
            nav.set_y(195 + std::rand() % 20);
            nav.set_z(-305 + std::rand() % 10);

            std::cout << "Tx: " << nav.DebugString() << std::flush;
            // publish only on the interthread layer
            // transporter().inner().publish<groups::nav>(nav);
        }    
};

#endif
