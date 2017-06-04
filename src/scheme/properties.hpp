#pragma once

namespace optspp {
  namespace scheme {
    description::description(const std::string& desc) : node(node::KIND::NONE) {
      set_description(desc);
    }
    
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
      set_long_name(name);
      for (const auto& s : synonyms) add_long_name_synonym(s);
    }

    named::named(const char& name) {
      set_is_positional(false);
      set_short_name(name);
    }

    named::named(const char& name, std::initializer_list<char> synonyms) {
      set_is_positional(false);
      set_short_name(name);
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

    any_value::any_value() {
      set_allow_arbitrary_values(true);
    }
  }

  std::shared_ptr<scheme::description> description(const std::string& desc) {
    return std::make_shared<scheme::description>(desc);
  }
  
  std::shared_ptr<scheme::positional> positional(const std::string& name) {
    return std::make_shared<scheme::positional>(name);
  }

  std::shared_ptr<scheme::named> named(const std::string& name) {
    return std::make_shared<scheme::named>(name);
  }
  
  std::shared_ptr<scheme::named> named(const std::string& name, std::initializer_list<std::string> synonyms) {
    return std::make_shared<scheme::named>(name, synonyms);
  }
  
  std::shared_ptr<scheme::named> named(const char& name) {
    return std::make_shared<scheme::named>(name);    
  }

  std::shared_ptr<scheme::named> named(const char& name, std::initializer_list<char> synonyms) {
    return std::make_shared<scheme::named>(name, synonyms);
  }

  std::shared_ptr<scheme::min_count> min_count(const size_t& count) {
    return std::make_shared<scheme::min_count>(count);
  }
  
  std::shared_ptr<scheme::max_count> max_count(const size_t& count) {
    return std::make_shared<scheme::max_count>(count);
  }

  std::shared_ptr<scheme::default_value> default_value(const std::string& value) {
    return std::make_shared<scheme::default_value>(value);
  }

  std::shared_ptr<scheme::implicit_value> implicit_value(const std::string& value) {
    return std::make_shared<scheme::implicit_value>(value);
  }

  std::shared_ptr<scheme::any_value> any_value() {
    return std::make_shared<scheme::any_value>();
  }

  std::shared_ptr<scheme::value> value(const std::string& main_value) {
    return std::make_shared<scheme::value>(main_value);
  }

  std::shared_ptr<scheme::value> value(const std::string& main_value, std::initializer_list<std::string> synonyms) {
    return std::make_shared<scheme::value>(main_value, synonyms);
 }

  
}
