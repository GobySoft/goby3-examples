#include "goby/middleware/single-thread-application.h" 

#include "machine.h"

#include "messages/state.pb.h" 
#include "messages/groups.h"

#include "config.pb.h" 

class StateMachineApp : public goby::SingleThreadApplication<StateMachineAppConfig, statechart::Machine<StateMachineApp>>
{
public:
    StateMachineApp()
        {
	    interprocess().subscribe<groups::state_control, protobuf::StateControl>
		([this](const protobuf::StateControl& ctrl)
		 {
		     std::cout << "Received message: " << ctrl.ShortDebugString() << std::endl;
		     switch(ctrl.desired_state())
		     {
		     case protobuf::ON:
			 state_machine().process_event(statechart::EvTurnOn());
			 break;
		     case protobuf::OFF:
			 state_machine().process_event(statechart::EvTurnOff());
			 break;		    
		     }
		 });
	}

    // allow these states to publish on our behalf
    template<typename App> friend struct statechart::Off;
    template<typename App> friend struct statechart::On;
};

int main(int argc, char* argv[])
{ return goby::run<StateMachineApp>(argc, argv); }
