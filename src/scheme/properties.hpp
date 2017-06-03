#pragma once

namespace optspp {
  positional::positional(const std::string& name) {
    set_is_positional(true);
    set_long_name(name);
  }

  positional::positional(const std::string& name, std::initializer_list<std::string> synonyms) {
    set_is_positional(true);
    set_long_name(name);
    for (const auto& s : synonyms) add_long_name_synonym(s);
  }

  named::named(const std::string& name) {
    set_is_positional(true);
    set_long_name(name);
  }

  named::named(const std::string& name, std::initializer_list<std::string> synonyms) {
    set_is_positional(false);
    for (const auto& s : synonyms) add_long_name_synonym(s);
  }

  named::named(const char& name) {
    set_is_positional(false);
    set_short_name(name);
  }

  named::named(const char& name, std::initializer_list<char> synonyms) {
    set_is_positional(false);
    for (const auto& s : synonyms) add_short_name_synonym(s);
  }


  min_count::min_count(const size_t& min_count) {
    set_min_count(min_count);
  }

  max_count::max_count(const size_t& max_count) {
    set_max_count(max_count);
  }
  
  value::value(const std::string& main_value) {
    set_main_value(main_value);
  }
  
  value::value(const std::string& main_value, std::initializer_list<std::string> synonyms) {
    set_main_value(main_value);
    for (const auto& s : synonyms) add_value_synonym(s);
  }

  description::description(const std::string& desc) : node(node::KIND::NONE) {
    set_description(desc);
  }

  
}
