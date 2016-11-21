#include "goby/middleware/single-thread-application.h"

#include "messages/nav.pb.h"
#include "config.pb.h"

using goby::glog;
using namespace goby::common::logger;

// optional "using" declaration (reduces verbiage)
using Base = goby::SingleThreadApplication<BasicSubscriberConfig>;

class BasicSubscriber : public Base
{
public:
    BasicSubscriber() : Base(0.1 /*hertz*/)
        {
            auto nav_callback = [this](const NavigationReport& nav) { incoming_nav(nav); };
            portal().subscribe<nav_group, NavigationReport>(nav_callback);
        }

    // virtual method in goby::SingleThreadApplication called at a given frequency
    void loop() override
        {
        }

    void incoming_nav(const NavigationReport& nav)
        {
            std::cout << "Rx: " << nav.DebugString() << std::flush;
        }
    
    
private:
    // publish/subscribe group declaration (and definition, as a constexpr)
    static constexpr goby::Group nav_group{"nav::report"};
    
};

// publish/subscribe group "definition" (actual definition is in class body because this is a constexpr)
constexpr goby::Group BasicSubscriber::nav_group;


int main(int argc, char* argv[])
{ return goby::run<BasicSubscriber>(argc, argv); }

