#include "publisher.h"
#include "subscriber.h"

using AppBase = goby::MultiThreadStandaloneApplication<BasicMultithreadPubSubConfig>;


class ExampleMultiThreadApp : public AppBase
{
  public:
    ExampleMultiThreadApp() : AppBase()

    {
        // launch a publisher then two subscriber threads
        launch_thread<BasicPublisher>();
        launch_thread<BasicSubscriber>(0);
        launch_thread<BasicSubscriber>(1);
    }
};

int main(int argc, char* argv[])
{
    
    return goby::run<ExampleMultiThreadApp>(argc, argv);
}
