#include <goby/middleware/marshalling/json.h>

#include <goby/zeromq/application/single_thread.h>

#include "messages/groups.h"

#include "config.pb.h"
#include "nav.h"

using Base = goby::zeromq::SingleThreadApplication<JsonPublisherConfig>;

class JsonPublisher : public Base
{
  public:
    JsonPublisher() : Base(10 /*hertz*/) {}

    void loop() override
    {
        NavigationReport nav{95.0 + std::rand() % 20, 195.0 + std::rand() % 20,
                             -305.0 + std::rand() % 10};

        std::cout << "Tx: " << nav << std::endl;
        interprocess().publish<groups::nav>(nav);

        nlohmann::json nav2;
        nav2["x2"] = 10;
        nav2["y2"] = 20;
        nav2["z2"] = 30;
        std::cout << "Tx: " << nav2.dump() << std::endl;
        interprocess().publish<groups::nav>(nav2);
    }
};

int main(int argc, char* argv[]) { return goby::run<JsonPublisher>(argc, argv); }
