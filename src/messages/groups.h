#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/group.h"

namespace groups
{
constexpr goby::middleware::Group nav{"navigation", goby::middleware::Group::broadcast_group};

constexpr goby::middleware::Group gps_raw_in{"gps_raw_in"};
constexpr goby::middleware::Group gps_raw_out{"gps_raw_out"};
constexpr goby::middleware::Group gps_data{"gps_data"};
constexpr goby::middleware::Group gps_control{"gps_control"};

constexpr goby::middleware::Group state_control{"state_control"};
constexpr goby::middleware::Group state_report{"state_report"};

constexpr goby::middleware::Group string_msg{"string_msg"};

constexpr goby::middleware::Group init{"init"};
} // namespace groups

// Used by the Julia examples. GOBY_DEFINE_GROUP sets the group's string value to
// the fully-qualified C++ name ("groups::julia_nav"), which is the string that
// publisher.jl/subscriber.jl pass to Goby.publish()/Goby.subscribe() - the Julia
// side matches a group by its string value rather than by the C++ symbol.
GOBY_DEFINE_GROUP(groups, julia_nav)

#endif
