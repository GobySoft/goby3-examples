#ifndef GOBY3EXAMPLES_GROUPS_H
#define GOBY3EXAMPLES_GROUPS_H

#include "goby/middleware/serialize_parse.h"

namespace groups
{

#ifdef __GNUC__  
#if __GNUC__ < 7 || __GNUC_MINOR__ < 2
  // bug in gcc < 7.2 requires extern
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52036
#define GCC_BUG
#endif
#endif

#ifndef GCC_BUG  
  constexpr goby::Group nav{"navigation"};
  constexpr goby::Group gps_data{"gps_data"};
  constexpr goby::Group gps_control{"gps_control"};
  constexpr goby::Group state_control{"state_control"};
  constexpr goby::Group state_report{"state_report"};
#else
  
  extern constexpr goby::Group nav{"navigation"};
  extern constexpr goby::Group gps_data{"gps_data"};
  extern constexpr goby::Group gps_control{"gps_control"};
  extern constexpr goby::Group state_control{"state_control"};
  extern constexpr goby::Group state_report{"state_report"};
#endif
  
}

#endif
