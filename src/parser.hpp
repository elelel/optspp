#pragma once

#include "predeclare.hpp"

namespace optspp {
  namespace detail {
    // Helper structure to split single command line argument into key/value
    struct separation {
      separation(const std::string& s,
                 const std::vector<std::string>& separators);
      // True if was separated
      explicit operator bool() const noexcept;

      std::string key;
      std::string value;
      size_t value_pos;
      
    private:
      bool was_separated_;
    };
  }
  
  // Token for parsing
  struct token {
    token();
    
    token(const size_t& _pos_arg_num,
          const size_t& _pos_in_arg,
          const std::string& _s);

    // True if contains data
    explicit operator bool() const noexcept;
                   
    const size_t pos_arg_num;
    const size_t pos_in_arg;
    const std::string s;
    
  private:
    bool some_{false};
  };
  
  struct parser {
    parser(scheme::arguments& args,
           const std::vector<std::string>& cmdl_args);
    void parse();

    static std::vector<std::string> tokenize(const std::vector<std::string>& cmdl_args);
    
  private:
    scheme::arguments& args_;
    std::deque<token> tokens_;
    std::vector<scheme::node_ptr> candidates_;
    bool take_as_positionals_{false};

    // Extracts argument's value
    bool parse_value_sources(const token& t, 
                             const std::shared_ptr<scheme::attributes>& arg,
                             const detail::separation& separated);
    // Tries to parse current position as a long-prefixed argument
    bool parse_long_argument();
    // Tries to parse current position as a short-prefixed argument
    bool parse_short_argument();
    // Tries to parse current position as a positional argument
    bool parse_positional();
    // True if s is prefixed with one of the strings in prefixes
    static bool is_prefixed(const std::string& s, const std::vector<std::string>& prefixes);
    // True if s is prefixed with one of the strings in long prefixes
    bool is_long_prefixed(const std::string& s) const;
    // True if s is prefixed with one of the strings in short prefixes
    bool is_short_prefixed(const std::string& s) const;
    // Set's argument's value to it's implicit value
    void add_value_implicit(const token& t, const std::shared_ptr<scheme::attributes>& arg);
    // Check if we exceeded how many times a value can be specified
    void check_value_counts(const token& t, const std::shared_ptr<scheme::attributes>& arg) const;
    // Add argument's value
    void add_value(const token& t, const std::shared_ptr<scheme::attributes>& arg, const std::string& s);

    static std::tuple<size_t, std::string> extract_unprefixed(const std::string& s, const std::vector<std::string>& prefixes);
      
  };
}
