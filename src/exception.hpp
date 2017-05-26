#pragma once

#include <exception>

namespace optspp {
  namespace exception {
    struct optspp_exception : std::exception {
    };

    struct option_exception : optspp_exception {
    };

    struct options_exception : optspp_exception {
    };
    
    struct value_exception : optspp_exception {
    };
    
    struct long_name_conflict : options_exception {
      long_name_conflict(const std::string& _name) :
        name(_name) {
      }

      virtual const char* what() const noexcept override {
        return ("Long name or synonym conflict for '" + name + "'").c_str();
      }

      std::string name;
    };

    struct short_name_conflict : options_exception {
      short_name_conflict(const char& _name) :
        name(_name) {
      }

      virtual const char* what() const noexcept override {
        return (std::string("Short name or synononym conflict for '") + name + "'").c_str();
      }
      char name;
    };

    struct invalid_default_value : option_exception {
      invalid_default_value(const std::set<std::string>& _values) :
        values(_values.begin(), _values.end()) {
      }

      virtual const char* what() const noexcept override {
        std::string s{"Default values not listed among valid values: "};
        bool need_comma{false};
        for (const auto& v : values) {
          if (need_comma) s += ", ";
          s += v;
          need_comma = true;
        }
        return s.c_str();
      }
      std::vector<std::string> values;
    };
    
    struct invalid_implicit_value : option_exception {
      invalid_implicit_value(const std::set<std::string>& _values) :
        values(_values.begin(), _values.end()) {
      }

      virtual const char* what() const noexcept override {
        std::string s{"Implicit values not listed among valid values: "};
        bool need_comma{false};
        for (const auto& v : values) {
          if (need_comma) s += ", ";
          s += v;
          need_comma = true;
        }
        return s.c_str();
      }
      
      std::vector<std::string> values;
    };

    struct invalid_mutually_exclusive_value : option_exception {
      invalid_mutually_exclusive_value(const std::set<std::string>& _values) :
        values(_values.begin(), _values.end()) {
      }

      virtual const char* what() const noexcept override {
        std::string s{"Mutually exclusive values not listed among valid values: "};
        bool need_comma{false};
        for (const auto& v : values) {
          if (need_comma) s += ", ";
          s += v;
          need_comma = true;
        }
        return s.c_str();
      }
      
      std::vector<std::string> values;
    };

    struct non_distinct_valid_values : option_exception {
      non_distinct_valid_values(const std::string& _value) :
        value(_value) {
      }

      virtual const char* what() const noexcept override {
        return ("Non distinct valid values, '" + value + "' specified more than once.").c_str();
      }
      std::string value;
    };

    struct short_parameter_requires_value : value_exception {
      short_parameter_requires_value(const std::string& _name) :
        name(_name) {
      }

      virtual const char* what() const noexcept override {
        return ("Short parameter '" + name + "' requires a value.").c_str();
      }
      
      std::string name;
    };
    
  }
}
