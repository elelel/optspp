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

    struct scheme_error : optspp_exception {
    };

    struct options_exception : optspp_exception {
    };

    struct option_exception : optspp_exception {
    };
    
    struct value_exception : optspp_exception {
    };

    // Scheme errors
    
    struct name_conflict : scheme_error {
      name_conflict(const std::string& n) :
        name_str(n) {
        message = "Argument name conflict ('" + name_str + "').";
      }
      
      name_conflict(const char& n) :
        name_char(n) {
        message = "Argument name conflict ('" + std::string() + name_char + "').";
      }
      
      std::string name_str;
      char name_char{0};
    };

    struct value_conflict : scheme_error {
      value_conflict(const std::string& n) :
        value_str(n) {
        message = "Argument value conflict ('" + value_str + "').";
      }

      std::string value_str;
    };

    struct value_not_found : scheme_error {
      value_not_found(const std::string& n) :
        value_str(n) {
        message = "Argument value '" + n + "' not found.";
      }
      
      std::string value_str;
    };
    
    /*
    // --------- Options exceptions ---------

    struct long_name_not_found : options_exception {
      long_name_not_found(const std::string& _name) :
        name(_name) {
        message = "Long name '" + _name + "' not found";
      }
      std::string name;
    };

    struct short_name_not_found : options_exception {
      short_name_not_found(const char& _name) :
        name(_name) {
        message = "Short name '" + std::string() + _name + "' not found";
      }
      char name;
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

    // --------- Option exceptions ---------
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

    struct option_dependency_conflict : option_exception {
      option_dependency_conflict(const std::string& _name) :
        name_str(_name) {
        message = "Option dependency conflict for option '" + name_str + "'.";
      }

      option_dependency_conflict(const char& _name) :
        name_char(_name) {
        message = "Option dependency conflict for option '" + std::string() + _name + "'.";
      }
      
      std::string name_str;
      char name_char;
    };

    // --------- Value exceptions ---------
    struct unknown_parameter : value_exception {
      unknown_parameter(const token& _t, const std::string& _parameter) :
        t(_t),
        parameter(_parameter) {
        message = "Unknown parameter '" + parameter + "'.";
      }

      token t;
      std::string parameter;
    };

    struct too_many_parameters : value_exception {
      too_many_parameters(const token& _t, const std::string& _argument) :
        t(_t),
        argument(_argument) {
        message = "Too many parameters, superflous argument '" + argument + "'.";
      }

      token t;
      std::string argument;
    };

    struct too_many_values : value_exception {
      too_many_values(const std::shared_ptr<option>& _o) :
        o(_o) {
        message = "Too many values for parameter '" + o->all_names_to_string() + ".";
      }
      
      too_many_values(const token& _t, const std::shared_ptr<option>& _o) :
        t(_t),
        o(_o) {
        message = "Too many values for parameter '" + o->all_names_to_string() + ".";
      }
    private:
      token t;
      std::shared_ptr<option> o;
    };

    struct too_few_values : value_exception {
      too_few_values(const std::shared_ptr<option>& _o) :
        o(_o) {
        message = "Too few values for parameter '" + o->all_names_to_string() + ".";
      }
      
      too_few_values(const token& _t, const std::shared_ptr<option>& _o) :
        t(_t),
        o(_o) {
        message = "Too few values for parameter '" + o->all_names_to_string() + ".";
      }
    private:
      token t;
      std::shared_ptr<option> o;
    };
    
    struct parameter_requires_value : value_exception {
      parameter_requires_value(const token& _t, const std::shared_ptr<option>& _o) :
        t(_t),
        o(_o) {
        message = "Parameter " + o->all_names_to_string() + " requires a value.";
      }
      
    private:
      token t;
      std::shared_ptr<option> o;
    };

    struct invalid_parameter_value : value_exception {
      invalid_parameter_value(const token& _t, const std::shared_ptr<option>& _o, const std::string& s) :
        t(_t),
        o(_o) {
        message = "Invalied value '" + s + "' for parameter " + o->all_names_to_string() + ".";
      }
      
    private:
      token t;
      std::shared_ptr<option> o;
    };


    
    struct value_mutual_exclusiveness_violated : value_exception {
      value_mutual_exclusiveness_violated(const std::shared_ptr<option>& _o,
                                          const std::vector<std::vector<std::string>>& _values) :
        o(_o),
        values(_values) {
        message = "Mutual exclusiveness constraints violated for parameter '" + o->all_names_to_string() +
          "' values: " + values_to_string();
      }

      value_mutual_exclusiveness_violated(const token& _t,
                                          const std::shared_ptr<option>& _o,
                                          const std::vector<std::vector<std::string>>& _values) :
        t(_t),
        o(_o),
        values(_values) {
        message = "Mutual exclusiveness constraints violated for parameter '" + o->all_names_to_string() +
          "' values: " + values_to_string();
      }
      
      std::string values_to_string() const {
        std::string s;
        for (const auto& vs : values) {
          bool need_outer_comma{false};
          if (values.size() > 0) {
            if (need_outer_comma) s += ", ";
            s += "[";
            bool need_comma{false};
            for (const auto& v : vs) {
              if (need_comma) s += ", ";
              s += v;
              need_comma = true;
            }
            s += "]";
            need_outer_comma = true;
          }
        }
        return s;
      }

      token t;
      std::vector<std::vector<std::string>> values;
      std::shared_ptr<option> o;
      
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
    */
    } 
}
