#include "publisher.h"
#include "subscriber.h"

using AppBase = goby::MultiThreadApplication<BasicMultithreadPubSubConfig>;

class BasicMultiThreadPubSub : public AppBase
{
public:
    BasicMultiThreadPubSub() :
        AppBase()
        
        {
            // launch a publisher then two subscriber threads
            launch_thread<BasicPublisher>();
            launch_thread<BasicSubscriber<0>>();
            launch_thread<BasicSubscriber<1>>();
        }
};




int main(int argc, char* argv[])
{ return goby::run<BasicMultiThreadPubSub>(argc, argv); }
