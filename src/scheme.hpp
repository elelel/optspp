#pragma once

#include <memory>

#include "predeclare.hpp"
#include "misc.hpp"

namespace optspp {
  namespace scheme {
    enum class SIBLINGS_GROUP {
      NONE,
      OR,
      XOR        
    };


    struct definition {
      void parse();
      
      friend struct ::optspp::parser;
      // Assign argument definition to scheme definition; the children are or-compatible
      friend definition& optspp::operator<<(definition& lhs, const std::shared_ptr<scheme::entity>& rhs);  
      // Assign argument definition to scheme definition; the children are or-compatible
      friend definition& optspp::operator|(definition& lhs, const std::shared_ptr<scheme::entity>& rhs);

    private:
      std::vector<std::string> long_prefixes_{"--"};
      std::vector<std::string> short_prefixes_{"-"};
      std::vector<std::string> separators_{"="};
      std::vector<std::string> take_as_positionals_args_{"--"};

      // Children
      std::vector<entity_ptr> pending_;
      std::map<entity_ptr, SIBLINGS_GROUP> pending_siblings_group_;

      // Actual value holders
      std::map<scheme::entity_ptr, std::vector<std::string>> values_;
      std::vector<scheme::entity_ptr> positional_;
    };

    struct entity {
      enum class KIND {
        NONE,
        ARGUMENT,
        VALUE
      };
      
      enum class COLOR {
        NONE,
        TAKEN,
        BLOCKED
      };

      entity(const KIND kind);
      entity(const entity& other);
      void set_siblings_group(const SIBLINGS_GROUP group);

      template <typename Property>
      void apply_property(Property p);

      void set_positional();
      void set_named();

      void set_value(const std::vector<std::string>& vs);
      void set_value(::optspp::any);

      // Assign value definition to argument definition and argument definition to value definition; the children are xor-compatible
      friend std::shared_ptr<entity> optspp::operator<<(std::shared_ptr<entity> lhs, const std::shared_ptr<entity>& rhs);
      // Assign value definition to argument definition and argument definition to value definition; the children are or-compatible
      friend std::shared_ptr<entity> optspp::operator|(std::shared_ptr<entity> lhs, const std::shared_ptr<entity>& rhs);
      // Assign argument definition to scheme definition; the children are or-compatible
      friend definition& optspp::operator<<(scheme::definition& lhs, const std::shared_ptr<entity>& rhs);  
      // Assign argument definition to scheme definition; the children are or-compatible
      friend definition& optspp::operator|(scheme::definition& lhs, const std::shared_ptr<entity>& rhs);

      std::string all_names_to_string() const;
      
      friend struct optspp::parser;
      
    private:
      KIND kind_{KIND::NONE};
      SIBLINGS_GROUP siblings_group_{SIBLINGS_GROUP::NONE};
      COLOR color_{COLOR::NONE};

      // Attributes
      //   Argument's attributes
      optional<std::string> description_;
      // Argument's long name, which is expected after long prefix, e.g. --option (at index 0);
      // and long name's synonyms, e.g. --opt, --op, --program_option ...
      optional<std::vector<std::string>> long_names_;
      // Argument's short name, which is expexted after short prefix, e.g. -o (at index 0);
      // and short name's synonyms, e.g. -c, -i ...
      optional<std::vector<char>> short_names_;
      // Maximum number of times the argument may be specified on command line
      optional<size_t> max_count_;
      // Minimum number of times the argument may be specified on command line
      optional<size_t> min_count_;
      // Is it a positional argument (the one used without prefices)? 
      optional<bool> is_positional_;

      //  Value's attributes
      // Main value, if a synonym specified this is the value queries will decay to (at index 0);
      // and alternative ways to name the value
      optional<std::vector<std::string>> known_values_;
      // Argument's default values, assumed if the arg was not specified on command line
      optional<std::vector<std::string>> default_values_;
      // Argument's implicit values, assumed if the arg was specified withouth a value on command line
      optional<std::vector<std::string>> implicit_values_;
      // Allow to take any value
      optional<bool> any_value_;

      // Children
      std::vector<entity_ptr> pending_;
      std::map<entity_ptr, SIBLINGS_GROUP> pending_siblings_group_;
    };
  }

  // Properties
  struct name {
    name(const std::string& name_str);
    name(const std::string& name_str, std::initializer_list<std::string> synonyms);
    name(const char& name_char);
    name(const char& name_char, std::initializer_list<char> synonyms);
    
    optional<std::vector<std::string>> long_names;
    optional<std::vector<char>> short_names;
  };

  struct default_values {
    default_values(const std::string& value);
    template <typename... Values>
    default_values(const std::string& value, Values&&... values);
    
    optional<std::vector<std::string>> values;
  };

  struct implicit_values {
    implicit_values(const std::string& value);
    template <typename... Values>
    implicit_values(const std::string& value, Values&&... values);
    
    optional<std::vector<std::string>> values;
  };

  struct min_count {
    min_count(const size_t count);

    optional<size_t> count;
  };

  struct max_count {
    max_count(const size_t count);

    optional<size_t> count;
  };

  struct description {
    description(const std::string& _text);

    optional<std::string> text;
  };

  struct any {
  };
}
