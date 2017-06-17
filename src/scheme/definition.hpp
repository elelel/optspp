#pragma once

#include "../scheme.hpp"

namespace optspp {
  namespace scheme {
    void definition::parse(const std::vector<std::string>& cmdl_args) {
      if (parsed_) return;
      validate();
      parser p(*this, cmdl_args);
      p.parse();
    }

    void definition::validate_entity(const entity_ptr& e) {
      if (e->kind_ == entity::KIND::ARGUMENT) {
        if (e->is_positional_ && *e->is_positional_) {
          if (e->short_names_)
            throw std::runtime_error("Scheme validation error: positional argument has short names");
        } else {
          bool short_undefined = !e->short_names_ || (e->short_names_ && (*e->short_names_).size() == 0);
          bool long_undefined = !e->long_names_ || (e->long_names_ && (*e->long_names_).size() == 0);
          if (short_undefined && long_undefined )
            throw std::runtime_error("Named argument's both long and short names are empty");
        }
        for (const auto& c : e->pending_) {
          if (c->kind_ != entity::KIND::VALUE) {
            throw std::runtime_error("Scheme validation error: argument entity has non-value child");
          }
        }
      }
      for (const auto& c : e->pending_) {
        validate_entity(c);
      }
    }
    
    void definition::validate() const {
      for (const auto& c : pending_) {
        validate_entity(c);
      }
    }
  }
}
