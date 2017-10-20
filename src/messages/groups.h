#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/group.h"

namespace groups
{    
#ifdef __clang__
    constexpr goby::Group nav{"navigation"};
    constexpr goby::Group gps_data{"gps_data"};
    constexpr goby::Group gps_control{"gps_control"};
#else
    // bug in gcc requires extern
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52036
    extern constexpr goby::Group nav{"navigation"};
    extern constexpr goby::Group gps_data{"gps_data"};
    extern constexpr goby::Group gps_control{"gps_control"};
#endif
    
}

#endif
