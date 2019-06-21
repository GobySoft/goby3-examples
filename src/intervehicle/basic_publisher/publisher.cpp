#include "goby/zeromq/single-thread-application.h"

#include "messages/groups.h"
#include "messages/nav_dccl.pb.h" // DCCL version of NavigationReport

#include "config.pb.h" // Protobuf, defines BasicPublisherConfig

// optional "using" declaration (reduces verbiage)
using Base = goby::zeromq::SingleThreadApplication<BasicPublisherConfig>;
using dccl::NavigationReport;

class BasicPublisher : public Base
{
  public:
    BasicPublisher() : Base(0.1 * boost::units::si::hertz) {}

    void loop() override
    {
        NavigationReport nav;
        nav.set_x(95 + std::rand() % 20);
        nav.set_y(195 + std::rand() % 20);
        nav.set_z(-305 + std::rand() % 10);

        std::cout << "Tx: " << nav.DebugString() << std::flush;
        intervehicle().publish<groups::nav>(nav);
    }
};

int main(int argc, char* argv[]) { return goby::run<BasicPublisher>(argc, argv); }
