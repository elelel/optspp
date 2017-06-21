#pragma once

#include <list>

namespace optspp {
  namespace scheme {
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
      
      parser(definition& scheme_def, const std::vector<std::string>& cmdl_args);

      void parse();

      
      // Clear color for a tree branch
      void clear_color(entity_ptr& e);
      // Test if there are nodes that still can be visited in this pass
      bool visitables_left(entity_ptr e);
      void initialize_pass();
      entity_ptr find_border_entity() const;
      // Find next node in tree to try parsing the argument
      bool pass_tree();
      // Add node to tree border, color taken entity_ptr as VISITED, and XOR-grouped siblings as BLOCKED
      void move_border(entity_ptr& parent, entity_ptr& entity);

      // Consume different types of tokens
      // Extracts named argument's value taking into considiration implicit values
      bool consume_named_value(entity_ptr& arg_def, const std::list<token>::iterator& value_token);
      // Finds token that matches named arg definition
      std::list<parser::token>::iterator find_token_for_named(const entity_ptr& arg_def);
      // Extracts positional argument's value, only for predefined positional values
      bool consume_positional_known_value(entity_ptr& arg_def,
                                          const std::list<token>::iterator& token);
      // Find token that looks like a positional arg
      std::list<parser::token>::iterator find_token_for_positional(const entity_ptr& arg_def);
      // Tries to parse current position as a prefixed named argument, then as positional with known value
      bool consume_argument(entity_ptr& parent);
    
    private:
      definition& scheme_def_;
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
      // Return position, prefix, unprefixed
      std::tuple<size_t, std::string, std::string> unprefix(const std::string& s);

      // Translates value to a main value, if available
      const std::string& main_value(const entity_ptr& arg_def, const std::string& s);
      // Adds named value to results
      void add_value(const entity_ptr& arg_def, const std::string& s);
      // Pushes positional argument value to results
      void push_positional_value(const entity_ptr& arg_def, const std::string& s);
      // Adds implicit value, throws if no implicit values left
      void add_value_implicit(entity_ptr& arg_def, const token& token);
      // Adds default value, throws if no default values left
      void add_value_default(entity_ptr& arg_def, const token& token);
    
    };
  }
}
