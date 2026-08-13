#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/group.h"

namespace groups
{
constexpr goby::middleware::Group nav{"navigation", goby::middleware::Group::broadcast_group};

// used by the Julia examples: the Julia code refers to a group by this string
// value (not by the C++ symbol name), so it must be kept in sync with the group
// passed to Goby.publish()/Goby.subscribe() in publisher.jl/subscriber.jl.
// Note that a group used from Julia must not have a numeric value, as
// Group::operator std::string() then appends ";<numeric>" to the string.
constexpr goby::middleware::Group julia_nav{"julia_navigation"};

constexpr goby::middleware::Group gps_raw_in{"gps_raw_in"};
constexpr goby::middleware::Group gps_raw_out{"gps_raw_out"};
constexpr goby::middleware::Group gps_data{"gps_data"};
constexpr goby::middleware::Group gps_control{"gps_control"};

constexpr goby::middleware::Group state_control{"state_control"};
constexpr goby::middleware::Group state_report{"state_report"};

constexpr goby::middleware::Group string_msg{"string_msg"};

constexpr goby::middleware::Group init{"init"};
} // namespace groups

#endif
