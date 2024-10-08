#include <goby/middleware/marshalling/dccl.h>

#include <goby/middleware/protobuf/intervehicle.pb.h>
#include <goby/zeromq/application/single_thread.h>

#include "config.pb.h" // Protobuf, defines BroadcastPublisherConfig
#include "messages/groups.h"
#include "messages/nav_dccl.pb.h" // DCCL version of NavigationReport

// optional "using" declaration (reduces verbiage)
using Base = goby::zeromq::SingleThreadApplication<BroadcastPublisherConfig>;
using dccl::NavigationReport;
using goby::middleware::intervehicle::protobuf::AckData;
using goby::middleware::intervehicle::protobuf::ExpireData;

class BroadcastPublisher : public Base
{
  public:
    BroadcastPublisher() : Base(0.1 * boost::units::si::hertz) {}

    void loop() override
    {
        NavigationReport nav;
        nav.set_x(95 + std::rand() % 20);
        nav.set_y(195 + std::rand() % 20);
        nav.set_z(-305 + std::rand() % 10);

        std::cout << "\nPublished: " << nav.ShortDebugString() << std::endl;

        intervehicle().publish<groups::nav>(nav);
    }

  private:
};

int main(int argc, char* argv[]) { return goby::run<BroadcastPublisher>(argc, argv); }
