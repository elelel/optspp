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
            return *known_values.begin();
          }
        }
      }
      return s;
    }

    void parser::add_value(const entity_ptr& arg_def, const std::string& s) {
      auto& v = scheme_def_.values_[arg_def];
      v.push_back(main_value(arg_def, s));
    }
  
    void parser::add_positional_value(const entity_ptr& arg_def, const parser::token& t) {
      add_value(arg_def, t.s);
      positionals_tmp_.push_back({arg_def, t});
    }

    void parser::add_value_implicit(entity_ptr& arg_def, const token& token) {
      if (arg_def->implicit_values_) {
        auto& iv = *arg_def->implicit_values_;
        if (iv.size() > 0) {
          add_value(arg_def, main_value(arg_def, iv[0]));
          iv.erase(iv.begin());
          return;
        }
      }
      throw value_required(arg_def);
    }

    void parser::add_value_default(entity_ptr& arg_def, const parser::token& token) {
      if (arg_def->default_values_) {
        auto& iv = *arg_def->default_values_;
        if (iv.size() > 0) {
          add_value(arg_def, main_value(arg_def, iv[0]));
          iv.erase(iv.begin());
          return;
        }
      }
      throw value_required(arg_def);
    }

    void parser::consume_named_value(entity_ptr& arg_def,
                                     const std::list<token>::iterator& token) {
      // Check if we don't have more tokens.
      if (tokens_.size() == 1) {
        add_value_implicit(arg_def, *token);
        tokens_.erase(token);
        return;
      }

      // Continue looking for value in the next token.
      std::list<parser::token>::iterator next_it(token);
      ++next_it;
      // Next token is a new option
      if ((is_long_prefixed(next_it->s) ||
           is_short_prefixed(next_it->s)) &&
          !ignore_option_prefixes_) {
        add_value_implicit(arg_def, *token);
        tokens_.erase(token);
        return;
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
        found = find_if(val_siblings.begin(), val_siblings.end(), [] (const entity_ptr& e) {
            return e->any_value_ && *(e->any_value_);
          });
      } 
      if (found != val_siblings.end()) {
        move_border(arg_def, *found);
        add_value(arg_def, next_it->s);
        // Remove tokens containing name and value
        tokens_.erase(tokens_.erase(token));
        return;
      } else {
        try {
          add_value_implicit(arg_def, *token);
          tokens_.erase(token);
          return;
        } catch (...) {
        }
      }
      throw value_required(arg_def);
    }

    bool parser::consume_positional(entity_ptr& arg_def,
                                                const std::list<token>::iterator& token,
                                                const bool only_known_value) {
      if (arg_def->is_positional_ && *arg_def->is_positional_) {
        if (!ignore_option_prefixes_ &&
            (is_prefixed(token->s, scheme_def_.long_prefixes_) ||
             is_prefixed(token->s, scheme_def_.short_prefixes_))) return false;
        std::vector<entity_ptr>& val_siblings = arg_def->pending_;
        while (true) {
          auto& val_siblings = arg_def->pending_;
          auto found = find_if(val_siblings.begin(), val_siblings.end(), [&token] (const entity_ptr& e) {
              if ((e->kind_ == entity::KIND::VALUE) && (e->known_values_)) {
                auto& known_values = *e->known_values_;
                auto found_value = find_if(known_values.begin(), known_values.end(), [&token] (const std::string& s) {
                    return s == token->s;
                  });
                return found_value != known_values.end();
              } else {
                return false;
              }
            });
          if ((!only_known_value) && (found == val_siblings.end())) {
            found = find_if(val_siblings.begin(), val_siblings.end(), [&token] (const entity_ptr& e) {
                return (e->kind_ == entity::KIND::VALUE) && e->is_any_value() && *e->is_any_value();
              });
          }
          if (found != val_siblings.end()) {
            move_border(arg_def, *found);
            add_positional_value(arg_def, *token);
            // Remove tokens containing name and value
            tokens_.erase(token);
            return true;
          } else {
            break;
          }
        }
      }
      return false;
    }

    std::list<parser::token>::iterator parser::find_token_for_named(const entity_ptr& arg_def) {
      // Find token that matches argument's definition by long name
      for (auto t = tokens_.begin(); t != tokens_.end(); ++t) {
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

    bool parser::consume_argument(entity_ptr& parent) {
      auto& arg_siblings = parent->pending_;
      if (!ignore_option_prefixes_) {
        // Find matching argument
        for (auto& arg_def : arg_siblings) {
          if ((arg_def->kind_ == entity::KIND::ARGUMENT) &&
              (arg_def->color_ != entity::COLOR::BLOCKED) &&
              (arg_def->is_positional_ && !*arg_def->is_positional_)) {
            auto t = find_token_for_named(arg_def);
            if (t != tokens_.end()) {
              move_border(parent, arg_def);
              consume_named_value(arg_def, t);
              return true;
            }
          }
        }
      }
      // Positional
      for (auto& arg_def : arg_siblings) {
        if ((arg_def->kind_ == entity::KIND::ARGUMENT) &&
            (arg_def->color_ != entity::COLOR::BLOCKED) &&
            (arg_def->is_positional_ && *arg_def->is_positional_)) {
          auto& val_siblings = arg_def->pending_;
          move_border(parent, arg_def);
          for (auto& val_def : val_siblings) {
            if ((val_def->kind_ == entity::KIND::VALUE) &&
                (val_def->color_ != entity::COLOR::BLOCKED)
                //&& (val_def->any_value_ && (*val_def->any_value_ == false))
                ) {
              for (auto t = tokens_.begin(); t != tokens_.end(); ++t) {
                if ((std::get<1>(unprefix(t->s)) == "") || (ignore_option_prefixes_)) {
                  if (val_def->value_matches(t->s)) {
                    consume_positional(arg_def, t, false);
                    return true;
                  }
                }
              }
            }
          }
        }
      }
      // Nothing consumed
      return false;
    }

    bool parser::consume_argument_positional_any(entity_ptr& parent) {
      auto& arg_siblings = parent->pending_;
      for (auto& arg_def : arg_siblings) {
        if ((arg_def->kind_ == entity::KIND::ARGUMENT) &&
            (arg_def->color_ != entity::COLOR::BLOCKED) &&
            (arg_def->is_positional_ && *arg_def->is_positional_)) {
          auto& val_siblings = arg_def->pending_;
          move_border(parent, arg_def);
          for (auto& val_def : val_siblings) {
            if ((val_def->kind_ == entity::KIND::VALUE) &&
                (val_def->color_ != entity::COLOR::BLOCKED) &&
                (val_def->any_value_ && (*val_def->any_value_))) {
              //              consume_positional(arg_def, t, false);
              return true;
            }
          }
        }
      }
      return false;
    }
    
    void parser::clear_color(entity_ptr& e) {
      if (e->color_ == entity::COLOR::VISITED)
        e->color_ = entity::COLOR::BORDER;
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
          for (const auto& e : p->pending_) {
            if ((e->kind_ == entity::KIND::ARGUMENT) && (e->color_ != entity::COLOR::VISITED)) {
              p->color_ = entity::COLOR::VISITED;
              return p;
            }
          }
        }
        for (const auto& c : p->pending_) q.push(c);
      }
      return nullptr;
    }

    void parser::move_border(entity_ptr& parent, entity_ptr& child) {
      child->color_ = entity::COLOR::BORDER;
      if (child->siblings_group_ == SIBLINGS_GROUP::XOR) {
        //        parent->color_ = entity::COLOR::VISITED;
        for (auto& s : parent->pending_) {
          if ((s != child) && (s->kind_ == child->kind_) && (s->siblings_group_ == SIBLINGS_GROUP::XOR))
            s->color_ = entity::COLOR::BLOCKED;
        }
      }
      if (child->siblings_group_ == SIBLINGS_GROUP::OR) {
        //  parent->color_ = entity::COLOR::VISITED;
      }
    }

    bool parser::pass_tree() {
      bool rslt = false;
      initialize_pass();

      while (true) {
        auto parent = find_border_entity();
        if (parent == nullptr) {
          // No more parents found
          return rslt;
        }
        if (consume_argument(parent)) {
          rslt = true;
        }
      }
      return rslt;
    }
    
    // Parse
    void parser::parse() {
      scheme_def_.values_.clear();
      scheme_def_.positionals_.clear();

      while (true) {
        // If we still have unparsed data
        if (tokens_.size() > 0) {
          if (!pass_tree()) {
            throw unparsed_tokens(tokens_);
          }
        } else {
          // Success
          break;
        }
      }
      
      std::sort(positionals_tmp_.begin(), positionals_tmp_.end(),
                [] (const std::pair<entity_ptr, token>& a,
                    const std::pair<entity_ptr, token>& b) {
                  return (a.second.pos_arg_num < b.second.pos_arg_num);
                });
      scheme_def_.positionals_.clear();
      for (const auto& p : positionals_tmp_) {
        scheme_def_.positionals_.push_back(p.first);
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
