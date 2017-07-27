#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/serialize_parse.h"

namespace groups
{    
    // extern is currently required by g++ but not clang++
    extern constexpr goby::Group nav{"navigation"};
    extern constexpr goby::Group gps_data{"gps_data"};
    extern constexpr goby::Group gps_control{"gps_control"};
    
}

#endif
