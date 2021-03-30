#include <goby/middleware/marshalling/json.h>
#include "nav.h"

#include <goby/zeromq/application/single_thread.h>

#include "messages/groups.h"

#include "config.pb.h"

using Base = goby::zeromq::SingleThreadApplication<JsonSubscriberConfig>;

class JsonSubscriber : public Base
{
  public:
    JsonSubscriber()
    {
        auto nav_callback = [this](const NavigationReport& nav) { this->incoming_nav(nav); };
        interprocess().subscribe<groups::nav>(nav_callback);
    }

    void incoming_nav(const NavigationReport& nav) { std::cout << "Rx: " << nav << std::endl; }
};

int main(int argc, char* argv[]) { return goby::run<JsonSubscriber>(argc, argv); }
