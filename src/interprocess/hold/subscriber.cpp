#include "goby/middleware/marshalling/protobuf.h"
#include "goby/zeromq/application/single_thread.h"

#include "messages/groups.h"
#include "messages/init.pb.h"

#include "config.pb.h"

using Base = goby::zeromq::SingleThreadApplication<HoldSubscriberConfig>;
using protobuf::Initial;

class HoldSubscriber : public Base
{
  public:
    HoldSubscriber()
    {
        interprocess().subscribe<groups::init>([](const Initial& init) {
            std::cout << "Received initial state: " << init.DebugString() << std::endl;
        });

        // after we have done all subscriptions that are required before we are ready for
        // publications, set ready
        interprocess().ready();
    }
};

int main(int argc, char* argv[]) { return goby::run<HoldSubscriber>(argc, argv); }
