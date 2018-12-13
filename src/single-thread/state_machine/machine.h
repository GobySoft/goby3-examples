#ifndef MACHINE_20180321_H
#define MACHINE_20180321_H

#include <boost/mpl/list.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>

#include "messages/groups.h"
#include "messages/state.pb.h"

namespace statechart
{
// events
struct EvTurnOn : boost::statechart::event<EvTurnOn>
{
};
struct EvTurnOff : boost::statechart::event<EvTurnOff>
{
};

template <typename App> struct Machine;
template <typename App> struct On;
template <typename App> struct Off;

template <typename App> struct Machine : boost::statechart::state_machine<Machine<App>, Off<App> >
{
  public:
    Machine(App& app) : app_(app) {}
    App& app() { return app_; }

  private:
    App& app_;
};

template <typename App> struct On : boost::statechart::state<On<App>, Machine<App> >
{
    using StateBase = boost::statechart::state<On<App>, Machine<App> >;
    // state enter
    On(typename StateBase::my_context c) : StateBase(c)
    {
        auto& interprocess = this->outermost_context().app().interprocess();
        protobuf::StateReport report;
        report.set_current_state(protobuf::ON);
        interprocess.template publish<groups::state_report>(report);

        std::cout << "Entering state: ON" << std::endl;
    }
    // state exit
    ~On() { std::cout << "Leaving state: ON" << std::endl; }
    typedef boost::mpl::list<boost::statechart::transition<EvTurnOff, Off<App> > > reactions;
};

template <typename App> struct Off : boost::statechart::state<Off<App>, Machine<App> >
{
    using StateBase = boost::statechart::state<Off<App>, Machine<App> >;
    Off(typename StateBase::my_context c) : StateBase(c)
    {
        auto& interprocess = this->outermost_context().app().interprocess();
        protobuf::StateReport report;
        report.set_current_state(protobuf::OFF);
        interprocess.template publish<groups::state_report>(report);

        std::cout << "Entering state: OFF" << std::endl;
    }
    ~Off() { std::cout << "Leaving state: OFF" << std::endl; }

    typedef boost::mpl::list<boost::statechart::transition<EvTurnOn, On<App> > > reactions;
};
} // namespace statechart

#endif
