#pragma once

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace optspp {
  struct option;
  struct options;
  struct token;
  struct parser;

  struct options : std::enable_shared_from_this<options> {
    friend struct parser;
    
    options();
    template <typename... Args>
    options(Args&&... args);
    
    void apply();
    void apply(const std::shared_ptr<option>& o);
    template <typename... Args>
    void apply(const std::shared_ptr<option>& o, Args&&... args);
    template <typename... Args>
    void apply(const option& o, Args&&... args);
    
    void add(const std::shared_ptr<option>& o);
    void remove(const std::shared_ptr<option>& o);
    void check_schema() const;
    std::vector<std::shared_ptr<option>>::const_iterator
    find_iterator(const std::string& long_name) const;
    std::shared_ptr<option> find(const std::string& long_name) const;
    std::vector<std::shared_ptr<option>>::const_iterator
    find_iterator(const char& short_name) const;
    std::shared_ptr<option> find(const char& short_name) const;
    template <typename Option>
    options& operator<<(const Option& o);
    
    void parse(const std::vector<std::string>& args);

    template <typename T>
    T as(const std::string& name) const;
    const std::vector<std::string>& operator[](const std::string& name) const;
    const std::vector<std::string>& operator[](const char& name) const;

    const size_t& max_positional_count() const;
    void set_max_positional_count(const size_t& n);
    const size_t& min_positional_count() const;
    void set_min_positional_count(const size_t& n);
    const std::vector<std::string>& positional() const;
    
  private:
    std::vector<std::shared_ptr<option> > options_;
    std::vector<std::string> long_prefixes_{ {"--"} };
    std::vector<std::string> short_prefixes_{ {"-"} };
    std::vector<std::string> separators_{ {"="} };
    size_t max_positional_count_{std::numeric_limits<size_t>::max()};
    size_t min_positional_count_{std::numeric_limits<size_t>::min()};

    std::map<std::shared_ptr<option>, std::vector<std::string>> values_;
    std::vector<std::string> positional_;

    void check_value_mutually_exclusive(const std::shared_ptr<option>& o) const;

    void check_results();

    void add_default_values();
  };

  struct token {
    token() {};
    
    token(const size_t& _pos_arg_num,
          const size_t& _pos_in_arg,
          const std::string& _s) :
      pos_arg_num(_pos_arg_num),
      pos_in_arg(_pos_in_arg),
      s(_s) {
    }

    explicit operator bool() const noexcept { return some_; }
                   
    const size_t pos_arg_num{0};
    const size_t pos_in_arg{0};
    const std::string s;
    
  private:
    bool some_{false};
  };

  struct option {
    option();
    virtual ~option();

    // For argument-style creation
    template <typename... Args>
    option(Args&&... args);
    void reduce(const option& o);
    template <typename... Args>
    void reduce(const option& o, Args&&... args);

    option& operator+=(const option& other);
    option& operator|(const option& other);
    option& operator<<(const option& other);

    // Option state mutators
    option& set_long_name(const std::string& long_name);
    option& add_long_name_synonym(const std::string& long_name_synonym);
    option& set_short_name(const char& short_name);
    option& add_short_name_synonym(const char& short_name_synonym);
    option& add_valid_value(const std::string& main_value, const std::vector<std::string>& synonyms);
    option& add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_values);
    option& add_default_value(const std::string& default_values);
    option& add_implicit_value(const std::string& implicit_values);
    option& set_description(const std::string& description);
    option& set_max_count(const size_t& max_count);
    option& set_min_count(const size_t& min_count);
    void check() const;

    option& add_parent_container(const std::shared_ptr<options>& os);
    option& remove_parent_container(const std::shared_ptr<options>& os);
    option& add_parent_containers(const std::vector<std::shared_ptr<options>>& oss);
    option& remove_parent_containers(const std::vector<std::shared_ptr<options>>& oss);
    void invoke_parent_container_checks() const;

    // Read member accessors
    std::string all_names_to_string() const;
    const std::string& long_name() const;
    const std::vector<std::string>& long_name_synonyms() const;
    const char& short_name() const;
    const std::vector<char>& short_name_synonyms() const;
    const std::map<std::string, std::vector<std::string>>& valid_values() const;
    const std::vector<std::vector<std::string>>& mutually_exclusive_values() const;
    const std::string& description() const;
    std::string main_value(const std::string& v) const;
    std::vector<std::string> default_values() const;
    std::vector<std::string> implicit_values() const;
    const size_t& max_count() const;
    const size_t& min_count() const;

    bool is_valid_value(const std::string& v) const;

  private:
    std::vector<std::shared_ptr<options>> parent_containers_;
    std::string long_name_;
    std::vector<std::string> long_name_synonyms_;
    char short_name_{0};
    std::vector<char> short_name_synonyms_;
    std::map<std::string, std::vector<std::string>> valid_values_;
    std::vector<std::vector<std::string>> mutually_exclusive_values_;
    std::vector<std::string> default_values_;
    std::vector<std::string> implicit_values_;
    std::string description_;
    size_t max_count_{std::numeric_limits<size_t>::max()};
    size_t min_count_{std::numeric_limits<size_t>::min()};
    
  };

  // --------- Option properties ---------
  struct long_name : option {
    long_name(const std::string& name);
    long_name(const std::string& name, std::initializer_list<std::string> synonyms);
  }; 
  
  struct short_name : option {
    short_name(const char& name);
    short_name(const char& name, std::initializer_list<char> synonyms);
  };

  struct valid_value : option {
    valid_value(const std::string& value);
    valid_value(const std::string& value, std::initializer_list<std::string> synonyms);
  };

  struct valid_values : option {
    valid_values(const valid_value& vv);
    valid_values(std::initializer_list<valid_value>);
  };

  struct mutually_exclusive_value : option {
    mutually_exclusive_value(std::initializer_list<std::string> mutually_exclusive_value);
    template <typename... Args>
    mutually_exclusive_value(std::initializer_list<std::string> mutually_exclusive_value, Args&&...);
  };

  struct default_value : option {
    default_value(const std::string& value);
    template <typename... Args>
    default_value(const std::string& value, Args&&...);
  };

  struct implicit_value : option {
    implicit_value(const std::string& value);
    template <typename... Args>
    implicit_value(const std::string& value, Args&&...);
  };

  struct description : option {
    description(const std::string& desc);
  };
    
  struct min_count : option {
    min_count(const size_t& count);
  };

  struct max_count : option {
    max_count(const size_t& count);
  };
}
