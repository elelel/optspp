#pragma once

#include <queue>

namespace optspp {
  namespace scheme {
    parser::parser(definition& scheme_def,
                   const std::vector<std::string>& cmdl_args) :
      scheme_def_(scheme_def) {
      for (size_t i = 0; i < cmdl_args.size(); ++i) tokens_.push_back({i, 0, cmdl_args[i]});
      preprocess();
    }
  
    void parser::preprocess() {
      // This is not in parsing routine because it should not be recursive to make sure {xxx=yyy=zzz} stays as {xxx, yyy=zzz}
      std::list<token> ts;
      while (tokens_.size() > 0) {
        const auto& t = *(tokens_.begin());
        for (const auto& sep : scheme_def_.separators_) {
          auto pos = t.s.find(sep);
          if (pos != std::string::npos) {
            std::string key = t.s.substr(0, pos);
            size_t value_pos = pos + sep.length() + 1;
            std::string value = t.s.substr(value_pos, t.s.length() - value_pos);
            ts.push_back({t.pos_arg_num, t.pos_in_arg, key});
            ts.push_back({t.pos_arg_num, t.pos_in_arg + value_pos, value});
          } else {
            ts.push_back(t);
          }
        }
        tokens_.erase(tokens_.begin());
      }
      std::swap(ts, tokens_);
      // TODO: Add value(any) to arguments without value definition
    }

    // Return position, prefix, unprefixed
    std::tuple<size_t, std::string, std::string> parser::unprefix(const std::string& s) {
      for (const auto& prefix : scheme_def_.long_prefixes_) {
        if (s.find(prefix) == 0) {
          return {prefix.size(), prefix, s.substr(prefix.size(), s.size())};
        }
      }
      for (const auto& prefix : scheme_def_.short_prefixes_) {
        if (s.find(prefix) == 0) {
          return {prefix.size(), prefix, s.substr(prefix.size(), s.size())};
        }
      }
      return {0, "", s};
    }

    bool parser::is_prefixed(const std::string& s, const std::vector<std::string>& prefixes) {
      for (const auto& prefix : prefixes) {
        if (s.find(prefix) == 0) return true;
      }
      return false;
    }

    bool parser::is_long_prefixed(const std::string& s) const {
      return is_prefixed(s, scheme_def_.long_prefixes_);
    }
      
    bool parser::is_short_prefixed(const std::string& s) const {
      return is_prefixed(s, scheme_def_.short_prefixes_);
    }

    const std::string& parser::main_value(const entity_ptr& arg_def, const std::string& s) {
      for (const auto& c : arg_def->pending_) {
        if ((c->kind_ == entity::KIND::VALUE) && c->known_values_) {
          const auto& known_values = *c->known_values_;
          auto found = std::find(known_values.begin(), known_values.end(), s);
          if (found != known_values.end()) {
            return *found;
          }
        }
      }
      return s;
    }

    void parser::add_value(const entity_ptr& arg_def, const std::string& s) {
      auto& v = scheme_def_.values_[arg_def];
      v.push_back(s);
    }
  
    void parser::push_positional_value(const entity_ptr& arg_def, const std::string& s) {
      auto& v = scheme_def_.values_[arg_def];
      v.push_back(s);
      // TODO: Incorrect! It should be sorted, because push is now not in cmdl order
      scheme_def_.positional_.push_back(arg_def);
    }
  
    void parser::add_value_implicit(entity_ptr& arg_def, const token& token) {
      if (arg_def->implicit_values_) {
        auto& iv = *arg_def->implicit_values_;
        if (iv.size() > 0) {
          add_value(arg_def, main_value(arg_def, iv[0]));
          iv.erase(iv.begin());
        }
      }
      throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value (tried implicit)");
    }

    void parser::add_value_default(entity_ptr& arg_def, const parser::token& token) {
      if (arg_def->default_values_) {
        auto& iv = *arg_def->default_values_;
        if (iv.size() > 0) {
          add_value(arg_def, main_value(arg_def, iv[0]));
          iv.erase(iv.begin());
        }
      }
      throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value (tried default)");
    }

    bool parser::consume_named_value(entity_ptr& arg_def,
                                     const std::list<token>::iterator& token) {
      std::cout << "Consuming value\n";
      // Check if we don't have more tokens.
      if (tokens_.size() == 1) {
        std::cout << "consume value no more tokens left\n";
        add_value_implicit(arg_def, *token);
        tokens_.erase(token);
        return false;
      }

      // Continue looking for value in the next token.
      std::list<parser::token>::iterator next_it(token);
      ++next_it;
      // Next token is a new option
      if ((is_long_prefixed(next_it->s) ||
           is_short_prefixed(next_it->s)) &&
          !ignore_option_prefixes_) {
        std::cout << "consume value next is prefixed\n";
        add_value_implicit(arg_def, *token);
        tokens_.erase(token);
        return false;
      }
      // Next token must be our the value
      // Find value entity that matches actual value
      auto& val_siblings = arg_def->pending_;
      auto found = find_if(val_siblings.begin(), val_siblings.end(), [&next_it] (const entity_ptr& e) {
          if ((e->kind_ == entity::KIND::VALUE) && (e->known_values_)) {
            auto& known_values = *e->known_values_;
            auto found_value = find_if(known_values.begin(), known_values.end(), [&next_it] (const std::string& s) {
                return s == next_it->s;
              });
            return found_value != known_values.end();
          } else {
            return false;
          }
        });
      if (found == val_siblings.end()) {
        std::cout << "Trying to find any value definition in " << val_siblings.size() << " children of " << arg_def->all_names_to_string() << "\n";
        found = find_if(val_siblings.begin(), val_siblings.end(), [] (const entity_ptr& e) {
            return e->any_value_ && *(e->any_value_);
          });
      } 
      if (found != val_siblings.end()) {
        move_border(arg_def, *found);
        add_value(arg_def, next_it->s);
        // Remove tokens containing name and value
        tokens_.erase(tokens_.erase(token));
        return true;
      } else {
        std::cout << "Warning! Value definition for " << next_it->s << " (argument " << arg_def->all_names_to_string() << ") not found\n";
      }
      throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value");
    }

    std::list<parser::token>::iterator parser::find_token_for_named(const entity_ptr& arg_def) {
      // Find token that matches argument's definition by long name
      for (auto t = tokens_.begin(); t != tokens_.end(); ++t) {
        std::cout << "find_token_for_named: Comparing to token " << t->s << "\n";
        auto up = unprefix(t->s);
        auto& prefix = std::get<1>(up);
        auto& name = std::get<2>(up);
        if ((scheme_def_.is_long_prefix(prefix)) && (arg_def->name_matches(name))) {
          return t;
        }
        if (scheme_def_.is_short_prefix(prefix)) {
          size_t pos = std::get<0>(up);
          if (name.size() > 1) {
            for (const auto& short_name : name) {
              parser::token new_token{t->pos_arg_num, pos, scheme_def_.short_prefixes_[0] + short_name};
              tokens_.insert(t, new_token);
              ++pos;
            }
            tokens_.erase(t);
            t = tokens_.begin();
          } else {
            if (arg_def->name_matches(name[0]))
              return t;
          }
        }
      }
      return tokens_.end();
    }

    bool parser::consume_named(entity_ptr& parent) {
      if (ignore_option_prefixes_) return false;
      std::vector<entity_ptr>& arg_siblings = parent->pending_;
      while (true) {
        std::cout << "consume_named: Named cycle\n";
        // Find matching argument
        for (auto& arg_def : arg_siblings) {
          if ((arg_def->kind_ == entity::KIND::ARGUMENT) &&
              (arg_def->color_ != entity::COLOR::BLOCKED) &&
              (arg_def->is_positional_ && !*arg_def->is_positional_)) {
            std::cout << "consume_named: Trying arg def " << arg_def->all_names_to_string() << "\n"
                      << " color " << (int)arg_def->color_ << "\n";
            auto token = find_token_for_named(arg_def);
            if (token != tokens_.end()) {
              move_border(parent, arg_def);
              consume_named_value(arg_def, token);
              std::cout << "consume_named: Consumed named arg " << arg_def->all_names_to_string() << " color is now " << (int)arg_def->color_ << "\n";
              return true;
            }
          }
        }
        std::cout << "consume_named: repeating\n";
        break;
      }
      return false;
    }
  
    entity_ptr parser::consume_positional_known(entity_ptr& parent) {
      /*  entity_ptr rslt;
      std::vector<entity_ptr>& arg_siblings = parent->pending_;
      while (true) {
        std::cout << "Positional named cycle\n";
        // Find matching argument
        for (auto& arg_def : arg_siblings) {
          if ((arg_def->kind_ == entity::KIND::ARGUMENT) &&
              (arg_def->color_ != entity::COLOR::BLOCKED) &&
              (arg_def->is_positional_ && *arg_def->is_positional_)) {
            std::cout << "Trying positional arg def " << arg_def->all_names_to_string() << "\n";
            auto& val_siblings = arg_def->pending_;
            for (auto& val_def : val_siblings) {
              // Find token that matches value definition, value def should not be "any"
              if ((val_def->kind_ == entity::KIND::VALUE) &&
                  (val_def->known_values_) && ((*val_def->known_values_).size() > 0)) {
                std::cout << " Trying val def " << (*val_def->known_values_)[0] << "\n";
                auto token = std::find_if(tokens_.begin(), tokens_.end(), [this, &val_def] (const parser::token& t) {
                    std::cout << "  Testing token " << t.s << "\n";
                    if (ignore_option_prefixes_ || (!is_long_prefixed(t.s) && !is_short_prefixed(t.s))) {
                      auto& known_values = *val_def->known_values_;
                      return std::find(known_values.begin(), known_values.end(), t.s) != known_values.end();
                    }
                  });
                if (token != tokens_.end()) {
                  color_siblings(arg_def, arg_siblings);
                  std::cout << "Adding as positional " << token->s << "\n";
                  color_siblings(val_def, val_siblings);
                  add_value(arg_def, token->s);
                  tokens_.erase(token);
                  return arg_def;
                }
              } else {
                std::cout << "no positionals found\n";
                break;
              }
              if (rslt) break;
            }
          } else break;
        }
        // If no more next siblings, we have to move to another argument matching type
        if ((tokens_.size() == 0) || !rslt) return nullptr;
      }
      return rslt;*/
    }

    void parser::clear_color(entity_ptr& e) {
      if (e->color_ != entity::COLOR::BLOCKED)
        e->color_ = entity::COLOR::NONE;
      for (auto& c : e->pending_) clear_color(c);
    }

    void parser::initialize_pass() {
      clear_color(scheme_def_.root_);
      if (scheme_def_.root_->color_ != entity::COLOR::BLOCKED) {
        scheme_def_.root_->color_ = entity::COLOR::BORDER;
      }
    }

    entity_ptr parser::find_border_entity() const {
      std::queue<entity_ptr> q;
      if (scheme_def_.root_->color_ != entity::COLOR::BLOCKED) q.push(scheme_def_.root_);
      while (q.size() > 0) {
        auto p = q.front();
        q.pop();
        if (p->color_ == entity::COLOR::BORDER) {
          std::cout << "find_border_arg_def: trying kind " << (int)p->kind_ << "\n";
          for (const auto& e : p->pending_) {
            if ((e->kind_ == entity::KIND::ARGUMENT) && (e->color_ != entity::COLOR::VISITED))
              return p;
          }
        }
        for (const auto& c : p->pending_) q.push(c);
      }
      return nullptr;
    }

    void parser::move_border(entity_ptr& parent, entity_ptr& child) {
      child->color_ = entity::COLOR::BORDER;
      std::cout << "Marking siblings";
      if (child->kind_ == entity::KIND::ARGUMENT) std::cout << " of " << child->all_names_to_string();
      std::cout << "\n";
      if (child->siblings_group_ == SIBLINGS_GROUP::XOR) {
        parent->color_ = entity::COLOR::VISITED;
        for (auto& s : parent->pending_) {
          if ((s != child) && (s->kind_ == child->kind_) && (s->siblings_group_ == SIBLINGS_GROUP::XOR))
            s->color_ = entity::COLOR::BLOCKED;
        }
      }
      if (child->siblings_group_ == SIBLINGS_GROUP::OR) {
      }
    }

    bool parser::visitables_left(entity_ptr e) {
      bool rslt{false};
      if ((e->color_ != entity::COLOR::VISITED) &&
          (e->color_ != entity::COLOR::BLOCKED)) return true;
      for (auto& c : e->pending_)
        if (c->color_ != entity::COLOR::BLOCKED) {
          rslt = rslt | visitables_left(c);
        };
      return rslt;
    }


    bool parser::pass_tree() {
      std::cout << "pass_tree: starting\n";
      bool rslt = false;
      initialize_pass();

      while (true) {
        std::cout << "pass_tree: cycle start\n";
        auto parent = find_border_entity();
        if (parent == nullptr) {
          return rslt;
        }
        if (consume_named(parent)) {
          rslt = true;
        } else {
          parent->color_ = entity::COLOR::VISITED;
        }
      }
      std::cout << " Tree pass returning " << rslt << "\n";
      return rslt;
    }
    
    // Parse
    void parser::parse() {
      scheme_def_.values_.clear();
      scheme_def_.positional_.clear();

      while (true) {
        std::cout << "parse: cycle start\n";
        // If we still have unparsed data
        if (tokens_.size() > 0) {
          std::cout << "parse: Tokens left: " << tokens_.size() << "\n";
          if (!pass_tree()) {
            throw std::runtime_error("Tokens left, but tree pass did not consume anything");
          }
        } else {
          // Success
          std::cout << "parse: Success, no more tokens left\n";
          break;
        }
      }
      
      //TODO: Check if all parents for dead-ends
      
    }

    /*
      TODO: move this to after parsing func
      // Insert it as default value if available
      add_value_default(arg_def, *token);
      tokens_.erase(token);
      arg_matched_long = true;*/
  }
}
