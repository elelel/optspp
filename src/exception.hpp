#pragma once

#include <exception>

#include "predeclare.hpp"

namespace optspp {
  struct optspp_error : std::exception {
    virtual const char* what() const noexcept override;
    std::string message;
  };

  struct scheme_error : optspp_error {
    scheme_error(const std::string& msg);
  };
  
  struct actual_counts_mismatch : optspp_error {
    struct record {
      scheme::entity_ptr entity;
      size_t actual;
    };
    actual_counts_mismatch(const std::vector<record>& rs);
    
    std::vector<record> records;
  };
}
