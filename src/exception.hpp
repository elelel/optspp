#pragma once

#include <exception>

#include "predeclare.hpp"

namespace optspp {
  namespace exception {
    struct optspp_exception : std::exception {
      virtual const char* what() const noexcept override {
        return message.c_str();
      }

      std::string message;
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
        message = "Long name or synonym conflict for '" + name + "'";
      }
      std::string name;
    };

    struct short_name_conflict : options_exception {
      short_name_conflict(const char& _name) :
        name(_name) {
        message = std::string("Short name or synononym conflict for '") + name + "'";
      }
      char name;
    };

    struct invalid_default_value : option_exception {
      invalid_default_value(const std::set<std::string>& _values) :
        values(_values.begin(), _values.end()) {
        message = "Default values not listed among valid values: ";
        bool need_comma{false};
        for (const auto& v : values) {
          if (need_comma) message += ", ";
          message += v;
          need_comma = true;
        }
      }

      std::vector<std::string> values;
    };
    
    struct invalid_implicit_value : option_exception {
      invalid_implicit_value(const std::set<std::string>& _values) :
        values(_values.begin(), _values.end()) {
        message = "Implicit values not listed among valid values: ";
        bool need_comma{false};
        for (const auto& v : values) {
          if (need_comma) message += ", ";
          message += v;
          need_comma = true;
        }
      }

      std::vector<std::string> values;
    };

    struct invalid_mutually_exclusive_value : option_exception {
      invalid_mutually_exclusive_value(const std::vector<std::set<std::string>>& _values) {
        for (const auto& v : _values) {
          values.push_back(std::vector<std::string>(v.begin(), v.end()));
        }
        message = "Values specified in mutual exclusiveness constraints are not listed among valid values: ";
        for (const auto& vs : values) {
          bool need_outer_comma{false};
          if (vs.size() > 0) {
            if (need_outer_comma) message += ", ";
            message += "[";
            bool need_comma{false};
            for (const auto& v : vs) {
              if (need_comma) message += ", ";
              message += v;
              need_comma = true;
            }
            message += "]";
            need_outer_comma = true;
          }
        }
      }

      std::vector<std::vector<std::string>> values;
    };

    struct non_distinct_valid_values : option_exception {
      non_distinct_valid_values(const std::string& _value) :
        value(_value) {
        message = "Non distinct valid values, '" + value + "' specified more than once.";
      }

      std::string value;
    };

    struct unknown_long_parameter : value_exception {
      unknown_long_parameter(const std::string& _name) :
        name(_name) {
        message = std::string("Unknown long parameter '") + name + "'";
      }

      std::string name;
    };

    struct parameter_requires_value : value_exception {
      parameter_requires_value(const std::shared_ptr<option>& _o) :
        o(_o) {
        message = "Parameter " + option_names_to_str(*o) + " requires a value.";
      }
        
      std::shared_ptr<option> o;
    };

    struct unknown_short_parameter : value_exception {
      unknown_short_parameter(const char& _name) :
        name(_name) {
        message = std::string("Unknown short parameter '") + name + "'.";
      }

      char name;
    };

    struct invalid_parameter_value : value_exception {
      invalid_parameter_value(const std::shared_ptr<option>& _o, const std::string& _value) :
        o(_o),
        value(_value) {
        message = "Value '" + value + "' is not valid parameter " + option_names_to_str(*o) + ".";
      }

    private:
      std::shared_ptr<option> o;
      std::string value;
    };

    struct value_mutual_exclusiveness_violated : value_exception {
      value_mutual_exclusiveness_violated(const std::shared_ptr<option>& _o,
                                          const std::vector<std::vector<std::string>>& _values) :
        o(_o),
        values(_values) {
        message = "Mutual exclusiveness constraints violated for parameter '" + option_names_to_str(*o) +
          "' values: ";
        for (const auto& vs : values) {
          bool need_outer_comma{false};
          if (values.size() > 0) {
            if (need_outer_comma) message += ", ";
            message += "[";
            bool need_comma{false};
            for (const auto& v : vs) {
              if (need_comma) message += ", ";
              message += v;
              need_comma = true;
            }
            message += "]";
            need_outer_comma = true;
          }
        }
      }

      std::vector<std::vector<std::string>> values;
      std::shared_ptr<option> o;
    };

    struct superflous_positional_parameter : value_exception {
      superflous_positional_parameter(const std::string& _value) :
        value(_value) {
        message = "Superflous positional parameter '" + value + "'";
      }

      std::string value;
    };

    struct non_existent_option_value_requested : value_exception {
      non_existent_option_value_requested(const std::string& _name) :
        name_str(_name) {
        message = "Non-existent option's '" + name_str + "' value requested.";
      }
      
      non_existent_option_value_requested(const char& _name) :
        name_char(_name) {
        message = std::string("Non-existent option's '") + name_char + "' value requested.";
      }

      std::string name_str;
      char name_char;
    };

    struct unknown_parameter : value_exception {
      unknown_parameter(const std::string& _name) :
        name_str(_name) {
        message = "Unknown parameter '" + name_str + "'.";
      }
      
      unknown_parameter(const char& _name) :
        name_char(_name) {
        message = std::string("Unknown parameter '") + name_char + "'.";
      }

      std::string name_str;
      char name_char;
    };

    struct too_few_values : value_exception {
      too_few_values(const std::string& _name, const size_t& _min, const size_t& _actual):
        name(_name),
        min(_min),
        actual(_actual) {
        message = "Too few values for parameter '" + name +
          "', should be at least " + std::to_string(min) +
          ", but actually " + std::to_string(actual) + ".";
      }

      std::string name;
      size_t min;
      size_t actual;
    };
    
    struct too_many_values : value_exception {
      too_many_values(const std::string& _name, const size_t& _max, const size_t& _actual):
        name(_name),
        max(_max),
        actual(_actual) {
        message = "Too many values for parameter '" + name +
          "', should be at most " + std::to_string(max) +
          ", but actually " + std::to_string(actual) + ".";
      }

      std::string name;
      size_t max;
      size_t actual;
    };
  }
}
