#pragma once

#include <exception>

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

    struct long_parameter_requires_value : value_exception {
      long_parameter_requires_value(const std::string& _name) :
        name(_name) {
        message = std::string("Long parameter '") + name + "' requires a value.";
      }

      std::string name;
    };

    struct unknown_long_parameter : value_exception {
      unknown_long_parameter(const std::string& _name) :
        name(_name) {
        message = std::string("Unknown long parameter '") + name + "'";
      }

      std::string name;
    };
    
    struct short_parameter_requires_value : value_exception {
      short_parameter_requires_value(const char& _name) :
        name(_name) {
        message = std::string("Short parameter '") + name + "' requires a value.";
      }

      char name;
    };

    struct unknown_short_parameter : value_exception {
      unknown_short_parameter(const char& _name) :
        name(_name) {
        message = std::string("Unknown short parameter '") + name + "'.";
      }

      char name;
    };

    struct invalid_long_parameter_value : value_exception {
      invalid_long_parameter_value(const std::string& _name, const std::string& _value) :
        name(_name), value(_value) {
        message = "Value '" + value + "' is not valid for long parameter '" + name + "'.";
      }

      std::string name;
      std::string value;
    };

    struct invalid_short_parameter_value : value_exception {
      invalid_short_parameter_value(const char& _name, const std::string& _value) :
        name(_name), value(_value) {
        message = std::string("Value '") + value + "' is not valid for short parameter '" + name + "'.";
      }

      char name;
      std::string value;
    };

    struct values_mutual_exclusiveness_violated : value_exception {
      values_mutual_exclusiveness_violated(const char& _name, const std::vector<std::vector<std::string>>& _values) :
        name_char(_name),
        values(_values) {
      }

      values_mutual_exclusiveness_violated(const std::string& _name, const std::vector<std::vector<std::string>>& _values) :
        name_str(_name),
        values(_values) {
        std::string name = name_str;
        if (name_str.size() == 0) name += name_char;

        message = "Mutual exclusiveness constraints violated for parameter '" + name + "': ";
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
      
      char name_char;
      std::string name_str;
      std::vector<std::vector<std::string>> values;
    };

    struct superflous_positional_parameter : value_exception {
      superflous_positional_parameter(const std::string& _value) :
        value(_value) {
        message = "Superflous positional parameter '" + value + "'";
      }

      std::string value;
    };
  }
}
