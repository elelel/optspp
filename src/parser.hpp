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
      
    parser(scheme::definition& scheme_def, const std::vector<std::string>& cmdl_args);

    void parse();

  private:
    scheme::definition& scheme_def_;
    std::list<token> tokens_;
    bool ignore_option_prefixes_{false};

    // Split name/values with custom separators
    void preprocess();

    // Prefix-related utils
    // True if s is prefixed with one of the strings in prefixes
    static bool is_prefixed(const std::string& s, const std::vector<std::string>& prefixes);
    // True if s is prefixed with one of the strings in long prefixes
    bool is_long_prefixed(const std::string& s) const;
    // True if s is prefixed with one of the strings in short prefixes
    bool is_short_prefixed(const std::string& s) const;
    // Extract unprefixed name and return it with it's position
    static std::tuple<size_t, std::string> extract_unprefixed(const std::string& s, const std::vector<std::string>& prefixes);

    // Color taken entity_ptr as VISITED, and XOR-grouped siblings as BLOCKED
    void color_siblings(scheme::entity_ptr& entity, std::vector<scheme::entity_ptr>& siblings);
    // Revert visited state to none for the tree branch
    void clear_visited(scheme::entity_ptr& e);
    // Test if there are nodes that still can be visited in this pass
    bool visitables_left(scheme::entity_ptr e);
    // Find next node in tree to try parsing the argument
    bool pass_tree();

    // Translates value to a main value, if available
    const std::string& main_value(const scheme::entity_ptr& arg_def, const std::string& s);
    // Adds named value to results
    void add_value(const scheme::entity_ptr& arg_def, const std::string& s);
    // Pushes positional argument value to results
    void push_positional_value(const scheme::entity_ptr& arg_def, const std::string& s);
    // Adds implicit value, throws if no implicit values left
    void add_value_implicit(scheme::entity_ptr& arg_def, const token& token);
    // Adds default value, throws if no default values left
    void add_value_default(scheme::entity_ptr& arg_def, const token& token);
    
    // Consume different types of tokens
    // Extracts argument's value, returns matched value entity_ptr
    bool consume_value(scheme::entity_ptr& arg_def, const std::list<token>::iterator& token);
    // Extracts argument's value taking into considiration implicit values
    bool consume_value_with_implicit(scheme::entity_ptr& arg_def, const std::list<token>::iterator& value_token);
    // Tries to parse current position as a long-prefixed argument
    bool consume_long(std::vector<scheme::entity_ptr>& arg_siblings);
    // Tries to parse current position as a short-prefixed argument
    bool consume_short(std::vector<scheme::entity_ptr>& arg_siblings);
    // Tries to parse current position as a positional argument, only for predefined values
    bool consume_positonal_known(std::vector<scheme::entity_ptr>& arg_siblings);
    
  };
}
