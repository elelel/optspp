#pragma once


#include <limits>

namespace optspp {
  namespace scheme {
    parser::token::token() :
      some_(false),
      pos_arg_num(std::numeric_limits<size_t>::max()),
      pos_in_arg(std::numeric_limits<size_t>::max()) {
    }
    
    parser::token::token(const size_t& _pos_arg_num,
                         const size_t& _pos_in_arg,
                         const std::string& _s) :
      some_(false),
      pos_arg_num(_pos_arg_num),
      pos_in_arg(_pos_in_arg),
      s(_s) {
    }

    parser::token::token(const token& other) :
      some_(other.some_),
      pos_arg_num(other.pos_arg_num),
      pos_in_arg(other.pos_in_arg),
      s(other.s) {
    }

    void parser::token::swap(token& other) {
      std::swap(some_, other.some_);
      std::swap(pos_arg_num, other.pos_arg_num);
      std::swap(pos_in_arg, other.pos_in_arg);
      std::swap(s, other.s);
    }

    parser::token::operator bool() const noexcept {
      return some_;
    }
  }
}

