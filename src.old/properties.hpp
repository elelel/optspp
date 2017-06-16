#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "declaration.hpp"

namespace optspp {
  long_name::long_name(const std::string& name) {
    set_long_name(name);
  }

  long_name::long_name(const std::string& name, std::initializer_list<std::string> synonyms) {
    set_long_name(name);
    for (const auto& s : synonyms) add_long_name_synonym(s);
  }

  short_name::short_name(const char& name) {
    set_short_name(name);
  }

  short_name::short_name(const char& name, std::initializer_list<char> synonyms) {
    set_short_name(name);
    for (const auto& s : synonyms) add_short_name_synonym(s);
  }
  
  valid_value::valid_value(const std::string& value) {
    add_valid_value(value, {});
  }
    
  valid_value::valid_value(const std::string& value, std::initializer_list<std::string> synonyms) {
    add_valid_value(value, synonyms);
  }

  valid_values::valid_values(const valid_value& vv) {
    *this += vv;
  }
  
  valid_values::valid_values(std::initializer_list<valid_value> vvs) {
    for (const auto& vv : vvs) 
      *this += vv;
  }
  
  mutually_exclusive_value::mutually_exclusive_value(std::initializer_list<std::string> values) {
    add_mutually_exclusive_values(values);
  }
  
  template <typename... Args>
  mutually_exclusive_value::mutually_exclusive_value(std::initializer_list<std::string> values, Args&&... args)
    : mutually_exclusive_value(std::forward<Args>(args)...) {
    add_mutually_exclusive_values(values);
  }

  default_value::default_value(const std::string& value) {
    add_default_value(value);
  }
  
  template <typename... Args>
  default_value::default_value(const std::string& value, Args&&... args) : default_value(std::forward<Args>(args)...) {
    add_default_value(value);
  }

  implicit_value::implicit_value(const std::string& value) {
    add_implicit_value(value);
  }
  
  template <typename... Args>
  implicit_value::implicit_value(const std::string& value, Args&&... args) : implicit_value(std::forward<Args>(args)...) {
    add_implicit_value(value);
  }

  description::description(const std::string& desc) {
    set_description(desc);
  }
  
  max_count::max_count(const size_t& count) {
    set_max_count(count);
  }
  
  min_count::min_count(const size_t& count) {
    set_min_count(count);
  }

  any_value::any_value() {
    set_allow_arbitrary_value(true);
  }

}
