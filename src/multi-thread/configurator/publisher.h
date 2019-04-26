#ifndef MULTITHREAD_PUBLISHER_H
#define MULTITHREAD_PUBLISHER_H

#include "config.pb.h"
#include "goby/middleware/multi-thread-application.h"
#include "messages/groups.h"

class BasicPublisher : public goby::SimpleThread<BasicMultithreadPubSubConfig>
{
  public:
    BasicPublisher(const BasicMultithreadPubSubConfig& config)
        : goby::SimpleThread<BasicMultithreadPubSubConfig>(config, 10 /*hertz*/)
    {
        // goby::glog.is_verbose() && goby::glog << "My configuration int is: " << cfg().my_value() << std::endl;
    }

    void loop() override
    {
        std::string msg("Hello: " + std::to_string(count_++));
        goby::glog.is_verbose() && goby::glog << "Tx: " << msg << std::endl;
        interthread().publish<groups::string_msg>(msg);
    }

  private:
    int count_{0};
};

#endif
