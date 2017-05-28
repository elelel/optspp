#pragma once

#include <deque>
#include <map>
#include <memory>
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
    
    void update();
    void update(const std::shared_ptr<option>& o);
    template <typename... Args>
    void update(const std::shared_ptr<option>& o, Args&&... args);
    template <typename... Args>
    void update(const option& o, Args&&... args);
    
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

  struct option : public std::enable_shared_from_this<option> {
    option();
    template <typename... Args>
    option(Args&&... args);
    
    void update();
    template <typename Arg>
    void update(Arg property);
    template <typename Arg, typename... Args>
    void update(Arg property, Args&&... args);
    template <typename Property>
    option& operator<<(const Property& property);

    bool is_valid_value(const std::string& v) const;
    std::string main_value(const std::string& v) const;

    const std::string& long_name() const;
    void set_long_name(const std::string& long_name);
    const std::vector<std::string> long_name_synonyms() const;
    void set_long_name_synonyms(const std::vector<std::string>& long_name_synonyms);
    const char& short_name() const;
    void set_short_name(const char& short_name);
    const std::vector<char> short_name_synonyms() const;
    void set_short_name_synonyms(const std::vector<char>& short_name_synonyms);
    const std::map<std::string, std::vector<std::string>> valid_values() const;
    void add_valid_value(const std::string& val, const std::vector<std::string>& synonyms);
    const std::vector<std::vector<std::string>>& mutually_exclusive_values() const;
    void add_mutually_exclusive_value(const std::vector<std::string>& vals);
    std::vector<std::string> default_values() const;
    void add_default_value(const std::string& val);
    std::vector<std::string> implicit_values() const;
    void add_implicit_value(const std::string& val);
    const std::string& description() const;
    void set_description(const std::string& desc);
    const size_t& max_count() const;
    void set_max_count(const size_t& n);
    const size_t& min_count() const;
    void set_min_count(const size_t& n);
    
    void add_parent_container(const std::shared_ptr<options>& os);
    void remove_parent_container(const std::shared_ptr<options>& os);

    std::string all_names_to_string();
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
  
    void check_parents() const;
    void check() const;
  };
}
  


