#ifndef DATASTAX_INTERNAL_OPTIONAL_HPP
#define DATASTAX_INTERNAL_OPTIONAL_HPP

#include "driver_config.hpp"

#if CASS_CPP_STANDARD >= 17
  #include "optional/optional_std.hpp"
#else
  #include "optional/optional_akrzemi.hpp"
#endif

#endif /* DATASTAX_INTERNAL_OPTIONAL_HPP */
