#include "goby/zeromq/single-thread-application.h"

#include "messages/groups.h"
#include "messages/nav_dccl.pb.h"

#include "config.pb.h"

using Base = goby::zeromq::SingleThreadApplication<BasicSubscriberConfig>;
using dccl::NavigationReport;

class BasicSubscriber : public Base
{
  public:
    BasicSubscriber()
    {
        auto nav_callback = [this](const NavigationReport& nav) {
            std::cout << "Rx: " << nav.DebugString() << std::flush;
        };
        intervehicle().subscribe<groups::nav, NavigationReport>(nav_callback);
    }
};

int main(int argc, char* argv[]) { return goby::run<BasicSubscriber>(argc, argv); }
