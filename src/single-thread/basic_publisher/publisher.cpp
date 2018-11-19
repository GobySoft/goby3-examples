#include "goby/middleware/single-thread-application.h" // provides SingleThreadApplication

#include "messages/nav.pb.h" // Protobuf, defines NavigationReport
#include "messages/groups.h" // defines publish/subscribe groups

#include "config.pb.h" // Protobuf, defines BasicPublisherConfig

// optional "using" declaration (reduces verbiage)
using Base = goby::SingleThreadApplication<BasicPublisherConfig>;
using protobuf::NavigationReport;

class BasicPublisher : public Base
{
public:
    BasicPublisher() : Base(10 /*hertz*/)
        {
            // all configuration defined in BasicPublisherConfig is available using cfg();
            std::cout << "My configuration int is: " << cfg().my_value() << std::endl;
        }

    // virtual method in goby::SingleThreadApplication called at the frequency given to Base(freq)
    void loop() override
        {
            NavigationReport nav;
            nav.set_x(95 + std::rand() % 20);
            nav.set_y(195 + std::rand() % 20);
            nav.set_z(-305 + std::rand() % 10);

            std::cout << "Tx: " << nav.DebugString() << std::flush;
            interprocess().publish<groups::nav>(nav);
        }    
};


// reads command line parameters based on BasicPublisherConfig definition
// these can be set on the command line (try "basic_publisher --help" to see parameters)
// or in a configuration file (use "basic_publisher --example_config" for the correct syntax)
// edit "config.proto" to add parameters
int main(int argc, char* argv[])
{ return goby::run<BasicPublisher>(argc, argv); }
