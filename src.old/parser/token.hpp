#pragma once

#include <limits>

namespace optspp {
  token::token() :
    some_(false),
    pos_arg_num(std::numeric_limits<size_t>::max()),
    pos_in_arg(std::numeric_limits<size_t>::max()) {
  }
    
  token::token(const size_t& _pos_arg_num,
               const size_t& _pos_in_arg,
               const std::string& _s) :
    some_(false),
    pos_arg_num(_pos_arg_num),
    pos_in_arg(_pos_in_arg),
    s(_s) {
  }

  token::operator bool() const noexcept {
    return some_;
  }
}
