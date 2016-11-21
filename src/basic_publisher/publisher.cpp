#include "goby/middleware/single-thread-application.h"

#include "messages/nav.pb.h"
#include "config.pb.h"

using goby::glog;
using namespace goby::common::logger;

// optional "using" declaration (reduces verbiage)
using Base = goby::SingleThreadApplication<BasicPublisherConfig>;

class BasicPublisher : public Base
{
public:
    BasicPublisher() : Base(10 /*hertz*/)
        {
        }

    // virtual method in goby::SingleThreadApplication called at a given frequency
    void loop() override
        {
            NavigationReport nav;
            nav.set_x(95 + std::rand() % 20);
            nav.set_y(195 + std::rand() % 20);
            nav.set_z(-305 + std::rand() % 10);

            std::cout << "Tx: " << nav.DebugString() << std::flush;
            portal().publish<nav_group>(nav);
        }
    
    
private:
    // publish/subscribe group declaration (and definition, as a constexpr)
    static constexpr goby::Group nav_group{"nav::report"};
    
};

// publish/subscribe group "definition" (actual definition is in class body because this is a constexpr)
constexpr goby::Group BasicPublisher::nav_group;


int main(int argc, char* argv[])
{ return goby::run<BasicPublisher>(argc, argv); }

