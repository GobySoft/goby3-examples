#include "goby/middleware/protobuf/intervehicle.pb.h"
#include "goby/zeromq/single-thread-application.h"

#include "messages/groups.h"
#include "messages/nav_dccl.pb.h" // DCCL version of NavigationReport

#include "config.pb.h" // Protobuf, defines BasicPublisherConfig

// optional "using" declaration (reduces verbiage)
using Base = goby::zeromq::SingleThreadApplication<BasicPublisherConfig>;
using dccl::NavigationReport;
using goby::middleware::intervehicle::protobuf::AckData;
using goby::middleware::intervehicle::protobuf::ExpireData;

class BasicPublisher : public Base
{
  public:
    BasicPublisher() : Base(0.1 * boost::units::si::hertz)
    {
        auto on_nav_ack = [](const NavigationReport& nav, const AckData& ack) {
            std::cout << "Received acknowledgment:\n\t" << ack.ShortDebugString()
                      << "\nfor message:\n\t" << nav.ShortDebugString() << std::endl;
        };

        auto on_nav_expired = [](const NavigationReport& nav, const ExpireData& expire) {
            std::cout << "Received expiration notice:\n\t" << expire.ShortDebugString()
                      << "\nfor subscription:\n\t" << nav.ShortDebugString() << std::endl;
        };

        
        nav_publisher_ = {{}, on_nav_ack, on_nav_expired};
    }

    void loop() override
    {
        NavigationReport nav;
        nav.set_x(95 + std::rand() % 20);
        nav.set_y(195 + std::rand() % 20);
        nav.set_z(-305 + std::rand() % 10);

        std::cout << "\nPublished: " << nav.ShortDebugString() << std::endl;

        intervehicle().publish<groups::nav>(nav, nav_publisher_);
    }

  private:
    goby::middleware::Publisher<NavigationReport> nav_publisher_;
};

int main(int argc, char* argv[]) { return goby::run<BasicPublisher>(argc, argv); }
