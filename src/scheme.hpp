#pragma once

#include <string>
#include <vector>

namespace optspp {
  namespace scheme {
    struct description;

    namespace tree {
      struct node;
      struct descriptor;
      struct name;
      struct value;
    }

    std::shared_ptr<tree::name> operator|(std::shared_ptr<tree::name> n, const std::shared_ptr<tree::value>& v);
  }
}

namespace optspp {
  namespace scheme {
    namespace tree {
      struct node : std::enable_shared_from_this<node> {
        enum KIND {
          NONE,
          DESCRIPTOR,
          NAME,
          VALUE
        };

        virtual void validate() const;

        node(KIND node_kind);
        virtual ~node();

        node& set_description(const std::string& desc);

        // Find name in children names
        std::vector<std::shared_ptr<name>>::const_iterator
        find_iterator(const std::string& long_name) const;
        std::shared_ptr<name> find(const std::string& long_name) const;
        std::vector<std::shared_ptr<name>>::const_iterator
        find_iterator(const char& short_name) const;
        std::shared_ptr<name> find(const char& short_name) const;

        const std::string& description() const;

        node& add_parent(const std::shared_ptr<node>& parent);
        node& remove_parent(const std::shared_ptr<node>& parent);
        node& add_parents(const std::vector<std::shared_ptr<node>>& parents);
        node& remove_parents(const std::vector<std::shared_ptr<node>>& parents);

        node& add_child_name(const std::shared_ptr<name>& n);
      protected:      
        KIND node_kind_{KIND::NONE};
        std::string description_;
        std::vector<std::shared_ptr<node>> parents_;
        std::vector<std::shared_ptr<name>> child_names_;    
      };

      struct descriptor : node {
        descriptor();
        virtual ~descriptor();

        virtual void validate() const override;
      
        descriptor& operator<<(const std::shared_ptr<name>& other);
      };

      struct name : node {
        name();
        virtual ~name();
        virtual void validate() const override;

        friend std::shared_ptr<name> scheme::operator|(std::shared_ptr<name> n, const std::shared_ptr<tree::value>& v);

        name& operator+=(const scheme::description& other);
        name& operator+=(const name& other);

        bool operator==(const std::string& n) const;
        bool operator==(const char& n) const;
      
        name& set_long_name(const std::string& long_name);
        name& add_long_name_synonym(const std::string& long_name_synonym);
        name& set_short_name(const char& short_name);
        name& add_short_name_synonym(const char& short_name_synonym);
        name& add_default_value(const std::string& default_value);
        name& add_implicit_value(const std::string& implicit_value);
        name& set_max_count(const size_t& max_count);
        name& set_min_count(const size_t& min_count);
        name& set_is_positional(const bool& is_positional);
        name& set_allow_arbitrary_values(const bool& allow);

        // Name-related read accessors
        const std::string& long_name() const;
        const std::vector<std::string>& long_name_synonyms() const;
        std::vector<std::string> long_names() const;
        const char& short_name() const;
        const std::vector<char>& short_name_synonyms() const;
        std::vector<char> short_names() const;
        std::string all_names_to_string() const;
      
        const std::string& description() const;
        const std::string& to_main_value(const std::string& s) const;
        std::vector<std::string> default_values() const;
        std::vector<std::string> implicit_values() const;
        const size_t& max_count() const;
        const size_t& min_count() const;
        const bool allow_arbitrary_values() const;
    
      private:
        std::vector<std::shared_ptr<value>> child_values_;
        std::string long_name_;
        std::vector<std::string> long_name_synonyms_;
        char short_name_{0};
        std::vector<char> short_name_synonyms_;
        std::vector<std::string> default_values_;
        std::vector<std::string> implicit_values_;

        size_t max_count_{std::numeric_limits<size_t>::max()};
        size_t min_count_{std::numeric_limits<size_t>::min()};

        bool is_positional_{false};
        bool allow_arbitrary_values_{false};
      };

      struct value : node {
        value();
        virtual ~value();
        virtual void validate() const override;

        value& operator+=(const scheme::description& other);
        value& operator+=(const value& other);
      
        value& operator<<(const std::shared_ptr<name>& other);

        bool operator==(const std::string& s) const;
      
        value& set_main_value(const std::string& main_value);
        value& add_value_synonym(const std::string& synonym);

        value& add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_values);

        const std::string& main_value() const;
        const std::vector<std::string>& value_synonyms() const;

      private:
        std::string main_value_;
        std::vector<std::string> value_synonyms_;
        std::vector<std::vector<std::string>> mutually_exclusive_values_;
      
      };
    }

    // Properties

    struct description : tree::node {
      description(const std::string& desc);
    };
  
    struct positional : tree::name {
      positional(const std::string& name);
      positional(const std::string& name, std::initializer_list<std::string> synonyms);
    };

    struct named : tree::name {
      named(const std::string& name);
      named(const std::string& name, std::initializer_list<std::string> synonyms);
      named(const char& name);
      named(const char& name, std::initializer_list<char> synonyms);
    };

    struct min_count : tree::name {
      min_count(const size_t& count);
    };

    struct max_count : tree::name {
      max_count(const size_t& count);
    };

    struct default_value : tree::name {
      default_value(const std::string& default_value);
      template <typename... Args> default_value(const std::string& value, Args&&... args);
    };

    struct implicit_value : tree::name {
      implicit_value(const std::string& implicit_value);
      template <typename... Args> implicit_value(const std::string& value, Args&&... args);
    };

    struct any_value : tree::name {
      any_value();
    };
  
    struct value : tree::value {
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
