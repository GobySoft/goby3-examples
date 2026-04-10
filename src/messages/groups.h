#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/group.h"

namespace groups
{
constexpr goby::middleware::Group nav{"groups::nav", goby::middleware::Group::broadcast_group};
} // namespace groups

GOBY_DEFINE_GROUP(groups, gps_raw_in)
GOBY_DEFINE_GROUP(groups, gps_raw_out)
GOBY_DEFINE_GROUP(groups, gps_data)
GOBY_DEFINE_GROUP(groups, gps_control)

GOBY_DEFINE_GROUP(groups, state_control)
GOBY_DEFINE_GROUP(groups, state_report)

GOBY_DEFINE_GROUP(groups, string_msg)

GOBY_DEFINE_GROUP(groups, init)

#endif
