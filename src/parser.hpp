#pragma once

#include <list>

namespace optspp {
  struct parser {
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

    struct node {
      enum COLOR {
        NONE,
        VISITED,
        BLOCKED
      };

      scheme::entity_ptr entity;
      COLOR color;
    };
      
    parser(scheme::definition& scheme_def,
           const std::vector<std::string>& cmdl_args);

    void parse();

  private:
    scheme::definition& scheme_def_;
    std::list<token> tokens_;
    std::list<token>::iterator token_;
    scheme::entity_ptr entity_;
    bool ignore_option_prefixes_{false};

    // Split name/values with custom separators
    void separate();

    // Prefix-related utils
    // True if s is prefixed with one of the strings in prefixes
    static bool is_prefixed(const std::string& s, const std::vector<std::string>& prefixes);
    // True if s is prefixed with one of the strings in long prefixes
    bool is_long_prefixed(const std::string& s) const;
    // True if s is prefixed with one of the strings in short prefixes
    bool is_short_prefixed(const std::string& s) const;
    // Extract unprefixed name and return it with it's position
    static std::tuple<size_t, std::string> extract_unprefixed(const std::string& s, const std::vector<std::string>& prefixes);

    // Consume different types of tokens
    // Extracts argument's value
    bool consume_value_sources(const scheme::entity_ptr& e);
    // Tries to parse current position as a long-prefixed argument
    bool consume_long_argument();
    // Tries to parse current position as a short-prefixed argument
    bool consume_short_argument();
    // Tries to parse current position as a positional argument
    bool consume_positional();
    
  };
}
