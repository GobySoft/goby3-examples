#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/serialize_parse.h"

namespace groups
{
constexpr goby::middleware::Group nav{"navigation", goby::middleware::Group::broadcast_group};

constexpr goby::middleware::Group gps_data{"gps_data"};
constexpr goby::middleware::Group gps_control{"gps_control"};
constexpr goby::middleware::Group state_control{"state_control"};
constexpr goby::middleware::Group state_report{"state_report"};
constexpr goby::middleware::Group string_msg{"string_msg"};

} // namespace groups

#endif
