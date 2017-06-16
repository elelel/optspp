#pragma once

#include "../scheme.hpp"

namespace optspp {
  namespace scheme {
    definition::definition(const entity_ptr e) {
      root_entity_ = e;
    }
  
    definition& definition::operator=(const entity_ptr e) {
      root_entity_ = e;
      return *this;
    }
  }
}
