#include "goby/middleware/single-thread-application.h" 

#include "machine.h"

#include "messages/state_control.pb.h" 
#include "messages/groups.h"

#include "config.pb.h" 

using protobuf::NavigationReport;

class StateMachineApp : public goby::SingleThreadApplication<StateMachineAppConfig, statechart::Machine<StateMachineApp>>
{
public:
    StateMachineApp()
        {
	    interprocess().subscribe<groups::state_control, protobuf::StateControl>
		([this](const protobuf::StateControl& ctrl)
		 {
		     std::cout << "Received message: " << ctrl.ShortDebugString() << std::enndl;
		     switch(ctrl.desired_state())
		     {
		     case protobuf::StateControl::ON:
			 state_machine().process_event(statechart::EvTurnOn);
			 break;
		     case protobuf::StateControl::OFF:
			 state_machine().process_event(statechart::EvTurnOff);
			 break;		    
		     }
		 });
	}
};

int main(int argc, char* argv[])
{ return goby::run<StateMachineApp>(argc, argv); }
