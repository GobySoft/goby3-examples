#include "goby/middleware/marshalling/protobuf.h"

#include "goby/zeromq/application/single_thread.h"

#include "messages/groups.h"
#include "messages/init.pb.h"

#include "config.pb.h"

using Base = goby::zeromq::SingleThreadApplication<HoldPublisherConfig>;
using protobuf::Initial;

class HoldPublisher : public Base
{
  public:
    HoldPublisher() : Base(10 /*hertz*/)
    {
        // no subscriptions so we are ready immediately
        interprocess().ready();

        // publish some initial state once (that we want to ensure some/all processes get)
        Initial init;
        init.set_state(Initial::ON);
        interprocess().publish<groups::init>(init);
    }

    void loop() override
    {
        std::cout << "Hold is " << (interprocess().hold_state() ? "on" : "off") << std::endl;
    }
};

int main(int argc, char* argv[]) { return goby::run<HoldPublisher>(argc, argv); }
