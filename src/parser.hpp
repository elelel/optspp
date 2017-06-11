#pragma once

#include "predeclare.hpp"

#include <list>

namespace optspp {
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
    std::list<token> tokens_;
    scheme::node_ptr node_;
    std::list<token>::iterator token_it_;
    scheme::node_ptr next_node_;
    bool ignore_option_prefixes_{false};

    void separate();
    
    // Extracts argument's value
    bool consume_value_sources(const std::shared_ptr<scheme::attributes>& arg);

    // Tries to parse current position as a long-prefixed argument
    bool consume_long_argument();
    // Tries to parse current position as a short-prefixed argument
    bool consume_short_argument();
    // Tries to parse current position as a positional argument
    bool consume_positional();
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
