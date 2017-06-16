#pragma once

#include "../scheme.hpp"

namespace optspp {
  name::name(const std::string& name_str) {
    long_names = optional<std::vector<std::string>>({name_str});
  }

  name::name(const std::string& name_str, std::initializer_list<std::string> synonyms) {
    std::vector<std::string> vs{name_str};
    for (const auto& s : synonyms) {
      vs.push_back(s);
    }
    long_names = vs;
  }
  
  name::name(const char& name_char) {
    short_names = optional<std::vector<char>>({name_char});
  }
  
  name::name(const char& name_char, std::initializer_list<char> synonyms) {
    std::vector<char> vs{name_char};
    for (const auto& s : synonyms) {
      vs.push_back(s);
    }
    short_names = vs;
  }

  default_values::default_values(const std::string& value) {
    if (values) (*values).push_back(value);
    else values = std::vector<std::string>{value};
  }
  
  template <typename... Values>
  default_values::default_values(const std::string& value, Values&&... _values) :
    default_values(std::forward<Values>(_values)...) {
    if (values) (*values).push_back(value);
    else values = std::vector<std::string>{value};
  }
  
  implicit_values::implicit_values(const std::string& value) {
    if (values) (*values).push_back(value);
    else values = std::vector<std::string>{value};
  }
  
  template <typename... Values>
  implicit_values::implicit_values(const std::string& value, Values&&... _values) :
    implicit_values(std::forward<Values>(_values)...) {
    if (values) (*values).push_back(value);
    else values = std::vector<std::string>{value};
  }

  min_count::min_count(const size_t _count) :
    count(_count) {
  }

  max_count::max_count(const size_t _count) :
    count(_count) {
  }

  description::description(const std::string& _text) :
    text(_text) {
  }

}
