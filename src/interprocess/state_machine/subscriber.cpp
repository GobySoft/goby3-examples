#include <goby/middleware/marshalling/protobuf.h>

#include <goby/zeromq/application/single_thread.h>

#include "machine.h"
#include "messages/groups.h"
#include "messages/state.pb.h"
#include "config.pb.h"

class StateMachineApp : public goby::zeromq::SingleThreadApplication<StateMachineAppConfig>
{
  public:
    StateMachineApp()
    {
        interprocess().subscribe<groups::state_control>(
            [this](const protobuf::StateControl& ctrl) {
                std::cout << "Received message: " << ctrl.ShortDebugString() << std::endl;
                switch (ctrl.desired_state())
                {
                    case protobuf::ON: machine_->process_event(statechart::EvTurnOn()); break;
                    case protobuf::OFF: machine_->process_event(statechart::EvTurnOff()); break;
                }
            });
    }

    void initialize() override
    {
        machine_.reset(new statechart::Machine<StateMachineApp>(*this));
        machine_->initiate();
    }

    void finalize() override
    {
        machine_->terminate();
        machine_.reset();
    }

    // allow these states to publish on our behalf
    template <typename App> friend struct statechart::Off;
    template <typename App> friend struct statechart::On;

  private:
    std::unique_ptr<statechart::Machine<StateMachineApp> > machine_;
};

int main(int argc, char* argv[]) { return goby::run<StateMachineApp>(argc, argv); }
