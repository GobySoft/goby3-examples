#include "publisher.h"
#include "subscriber.h"

using AppBase = goby::MultiThreadApplication<BasicMultithreadPubSubConfig>;

class BasicMultiThreadPubSub : public AppBase
{
public:
    BasicMultiThreadPubSub()
        {
            launch_thread<BasicPublisher>();
            launch_thread<BasicSubscriber>();
        }
};




int main(int argc, char* argv[])
{ return goby::run<BasicMultiThreadPubSub>(argc, argv); }
