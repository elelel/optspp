#pragma once

#include <string>
#include <vector>

#include "../contrib/easytree/include/easytree/tree"
#include "../contrib/easytree/include/easytree/breadth_first"
#include "../contrib/easytree/include/easytree/depth_first"

#include "predeclare.hpp"

namespace optspp {
  namespace scheme {
    struct attributes {
      enum KIND {
        ROOT,
        NAME,
        VALUE,
        DESCRIPTION
      };

      attributes(const KIND);
      
      using type = attributes;

      KIND kind() const;
      
      // Set argument name's description
      type& operator+=(const scheme::description& other);
      // Reduce (categorical sum) two names
      type& operator+=(const type& other);

      // Sum attributes
      friend std::shared_ptr<attributes> optspp::operator|(std::shared_ptr<attributes> lhs,
                                                    const std::shared_ptr<attributes>& rhs);
      // Add a sub-argument (e.g. that is only allowed if this branch was selected)
      friend std::shared_ptr<attributes> optspp::operator<<(std::shared_ptr<attributes> lhs,
                                                     const std::shared_ptr<attributes>& rhs);

      type& set_description(const std::string& desc);
        
      // Modify arguments's attributes
      type& set_long_name(const std::string& long_name);
      type& add_long_name_synonym(const std::string& long_name_synonym);
      type& set_short_name(const char& short_name);
      type& add_short_name_synonym(const char& short_name_synonym);
      type& add_default_value(const std::string& default_value);
      type& add_implicit_value(const std::string& implicit_value);
      type& set_max_count(const size_t& max_count);
      type& set_min_count(const size_t& min_count);
      type& set_is_positional(const bool& is_positional);
      type& set_allow_arbitrary_values(const bool& allow);

      type& set_main_value(const std::string& main_value);
      type& add_value_synonym(const std::string& synonym);
      type& add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_values);

      // Read accessors
      const std::string& long_name() const;
      const std::vector<std::string>& long_names() const;
      const char& short_name() const;
      const std::vector<char>& short_names() const;
      std::string all_names_to_string() const;
      const std::string& description() const;
      const std::string& to_main_value(const std::string& s) const;
      std::vector<std::string> default_values() const;
      std::vector<std::string> implicit_values() const;
      const size_t& max_count() const;
      const size_t& min_count() const;
      const bool allow_arbitrary_values() const;
      const std::string& main_value() const;
      const std::vector<std::string>& value_synonyms() const;
      std::vector<std::string> all_values() const;

      friend struct arguments;
      
    private:
      KIND kind_;
      // Description
      std::string description_;
      // Argument's long name, which is expected after long prefix, e.g. --option (at index 0);
      // and long name's synonyms, e.g. --opt, --op, --program_option ...
      std::vector<std::string> long_names_{""};
      // Argument's short name, which is expexted after short prefix, e.g. -o (at index 0);
      // and short name's synonyms, e.g. -c, -i ...
      std::vector<char> short_names_{0};
      // Argument's default values, assumed if the arg was not specified on command line
      std::vector<std::string> default_values_;
      // Argument's implicit values, assumed if the arg was specified withouth a value on command line
      std::vector<std::string> implicit_values_;
      // Maximum number of times the argument may be specified on command line
      size_t max_count_{std::numeric_limits<size_t>::max()};
      // Minimum number of times the argument may be specified on command line
      size_t min_count_{std::numeric_limits<size_t>::min()};
      // Is it a positional argument (the one used without prefices)? 
      bool is_positional_{false};
      // Are arbitrary values allowed or only those specified in valid values
      bool allow_arbitrary_values_{true};
      // Main value, if a synonym specified this is the value queries will decay to
      std::string main_value_;
      // Alternative ways to name the value
      std::vector<std::string> value_synonyms_;
      // Values that are incompatible in same invocation
      std::vector<std::vector<std::string>> mutually_exclusive_values_;

      // Pending children, will be added to single tree by arguments container
      std::vector<std::shared_ptr<attributes>> pending_;
      
    };

    // Arguments container
    struct arguments {
      arguments();

      friend struct optspp::parser;
      friend std::shared_ptr<arguments> optspp::operator<<(std::shared_ptr<arguments> lhs,
                                                           const std::shared_ptr<scheme::attributes>& rhs);

      void build();
      void validate_scheme();

      std::string main_value(const easytree::tree::node<std::shared_ptr<attributes>>::type_ptr n,
                             const std::string& vs) const;

        
      const easytree::tree::node<std::shared_ptr<attributes>>::type_ptr& root() const;
    private:
      void adopt_pending();
      easytree::tree::node<std::shared_ptr<attributes>>::type_ptr root_;
      bool built_{false};

      std::vector<std::string> long_prefixes_{"--"};
      std::vector<std::string> short_prefixes_{"-"};
      std::vector<std::string> separators_{"="};
      std::vector<std::string> end_of_options_arg{"--"};

      // Actual value holders
      std::map<std::shared_ptr<scheme::attributes>, std::vector<std::string>> values_;
      std::vector<std::string> positional_;
      
    };
            

    // Properties
    struct description : attributes {
      description(const std::string& desc);
    };
  
    struct positional  : attributes {
      positional(const std::string& name);
      positional(const std::string& name, std::initializer_list<std::string> synonyms);
    };

    struct named : attributes {
      named(const std::string& name);
      named(const std::string& name, std::initializer_list<std::string> synonyms);
      named(const char& name);
      named(const char& name, std::initializer_list<char> synonyms);
    };

    struct min_count : attributes {
      min_count(const size_t& count);
    };

    struct max_count : attributes {
      max_count(const size_t& count);
    };

    struct default_value : attributes {
      default_value(const std::string& default_value);
      template <typename... Args> default_value(const std::string& value, Args&&... args);
    };

    struct implicit_value : attributes {
      implicit_value(const std::string& implicit_value);
      template <typename... Args> implicit_value(const std::string& value, Args&&... args);
    };

    struct any_value : attributes {
      any_value();
    };
  
    struct value : attributes {
      value(const std::string& main_value);
      value(const std::string& main_value, std::initializer_list<std::string> synonyms);
    };
  }

  std::shared_ptr<scheme::description> description(const std::string& desc);
  std::shared_ptr<scheme::positional> positional(const std::string& name);
  std::shared_ptr<scheme::named> named(const std::string& name);
  std::shared_ptr<scheme::named> named(const std::string& name, std::initializer_list<std::string> synonyms);
  std::shared_ptr<scheme::named> named(const char& name);
  std::shared_ptr<scheme::named> named(const char& name, std::initializer_list<char> synonyms);
  std::shared_ptr<scheme::min_count> min_count(const size_t& count);
  std::shared_ptr<scheme::max_count> max_count(const size_t& count);
  std::shared_ptr<scheme::default_value> default_value(const std::string& default_value);
  std::shared_ptr<scheme::implicit_value> implicit_value(const std::string& implicit_value);
  std::shared_ptr<scheme::any_value> any_value();
  std::shared_ptr<scheme::value> value(const std::string& main_value);
  std::shared_ptr<scheme::value> value(const std::string& main_value, std::initializer_list<std::string> synonyms);

}
