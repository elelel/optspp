#pragma once

namespace optspp {
  parser::parser(scheme::definition& scheme_def,
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

  // Prefix-related
  std::tuple<size_t, std::string> parser::extract_unprefixed(const std::string& s, const std::vector<std::string>& prefixes) {
    for (const auto& prefix : prefixes) {
      if (s.find(prefix) == 0) {
        return {prefix.size(), s.substr(prefix.size(), s.size())};
      }
    }
    return {0, s};
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

  const std::string& parser::main_value(const scheme::entity_ptr& arg_def, const std::string& s) {
    for (const auto& c : arg_def->pending_) {
      if ((c->kind_ == scheme::entity::KIND::VALUE) && c->known_values_) {
        const auto& known_values = *c->known_values_;
        auto found = std::find(known_values.begin(), known_values.end(), s);
        if (found != known_values.end()) {
          return *found;
        }
      }
    }
    return s;
  }

  void parser::add_value(const scheme::entity_ptr& arg_def, const std::string& s) {
    auto& v = scheme_def_.values_[arg_def];
    v.push_back(s);
  }
  
  void parser::push_positional_value(const scheme::entity_ptr& arg_def, const std::string& s) {
    auto& v = scheme_def_.values_[arg_def];
    v.push_back(s);
    // TODO: Incorrect! It should be sorted, because push is now not in cmdl order
    scheme_def_.positional_.push_back(arg_def);
  }
  
  void parser::add_value_implicit(scheme::entity_ptr& arg_def, const token& token) {
    if (arg_def->implicit_values_) {
      auto& iv = *arg_def->implicit_values_;
      if (iv.size() > 0) {
        add_value(arg_def, main_value(arg_def, iv[0]));
        iv.erase(iv.begin());
      }
    }
    throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value (tried implicit)");
  }

  void parser::add_value_default(scheme::entity_ptr& arg_def, const parser::token& token) {
    if (arg_def->default_values_) {
      auto& iv = *arg_def->default_values_;
      if (iv.size() > 0) {
        add_value(arg_def, main_value(arg_def, iv[0]));
        iv.erase(iv.begin());
      }
    }
    throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value (tried default)");
  }

  bool parser::consume_value(scheme::entity_ptr& arg_def,
                             const std::list<token>::iterator& value_token) {
    std::cout << "Consuming value\n";
    // Next token must be our the value
    // Find value entity that matches actual value
    auto& val_siblings = arg_def->pending_;
    auto found = find_if(val_siblings.begin(), val_siblings.end(), [&value_token] (const scheme::entity_ptr& e) {
        if ((e->kind_ == scheme::entity::KIND::VALUE) && (e->known_values_)) {
          auto& known_values = *e->known_values_;
          auto found_value = find_if(known_values.begin(), known_values.end(), [&value_token] (const std::string& s) {
              return s == value_token->s;
            });
          return found_value != known_values.end();
        } else {
          return false;
        }
      });
    if (found == val_siblings.end()) {
      std::cout << "Trying to find any value definition in " << val_siblings.size() << " children of " << arg_def->all_names_to_string() << "\n";
      found = find_if(val_siblings.begin(), val_siblings.end(), [] (const scheme::entity_ptr& e) {
          return e->any_value_ && *(e->any_value_);
        });
    } 
    if (found != val_siblings.end()) {
      color_siblings(*found, val_siblings);
      add_value(arg_def, value_token->s);
      // Remove tokens containing name and value
      auto name_token(value_token);
      --name_token;
      tokens_.erase(tokens_.erase(name_token));
      return true;
    } else {
      std::cout << "Warning! Value definition for " << value_token->s << " (argument " << arg_def->all_names_to_string() << ") not found\n";
    }
    throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value");
  }
  
  bool parser::consume_value_with_implicit(scheme::entity_ptr& arg_def,
                                           const std::list<token>::iterator& token) {
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
    return consume_value(arg_def, next_it);
  }

  bool parser::consume_long(std::vector<scheme::entity_ptr>& arg_siblings) {
    if (ignore_option_prefixes_) return false;
    bool rslt{false};
    while (true) {
      std::cout << "Long named cycle\n";
      // Find matching argument
      for (auto& arg_def : arg_siblings) {
        if ((arg_def->kind_ == scheme::entity::KIND::ARGUMENT) &&
            (arg_def->color_ != scheme::entity::COLOR::BLOCKED) &&
            (arg_def->is_positional_ && !*arg_def->is_positional_)) {
          std::cout << "Trying arg def " << arg_def->all_names_to_string() << "\n";
          // Find token that matches argument's definition by long name
          auto token = std::find_if(tokens_.begin(), tokens_.end(), [this, &arg_def] (const parser::token& t) {
              std::cout << " Comparing to token " << t.s << "\n";
              if (is_long_prefixed(t.s)) {
                auto up = extract_unprefixed(t.s, scheme_def_.long_prefixes_);
                if (arg_def->long_names_) {
                  auto& long_names = *arg_def->long_names_;
                  //                  for (const auto& s : long_names) std::cout << " Candidate long name " << s << "\n";
                  return std::find(long_names.begin(), long_names.end(), std::get<1>(up)) != long_names.end();
                } else {
                  std::cout << "No long name for argument!\n";
                }
              } else {
                std::cout << "Not long prefixed " << t.s << "\n";
              }
              return false;
            });
          if (token != tokens_.end()) {
            color_siblings(arg_def, arg_siblings);
            consume_value_with_implicit(arg_def, token);
            return true;
          } else break;
        } else break;
      }
      // If no more next siblings, we have to move to another argument matching type
      if ((tokens_.size() == 0) || !rslt) break;
    }
    return rslt;
  }

  bool parser::consume_short(std::vector<scheme::entity_ptr>& arg_siblings) {
    if (ignore_option_prefixes_) return false;
    bool rslt{false};
    while (true) {
      std::cout << "Short named cycle\n";
      // Find matching argument
      for (auto& arg_def : arg_siblings) {
        if ((arg_def->kind_ == scheme::entity::KIND::ARGUMENT) &&
            (arg_def->color_ != scheme::entity::COLOR::BLOCKED) &&
            (arg_def->is_positional_ && !*arg_def->is_positional_)) {
          std::cout << "Trying arg def " << arg_def->all_names_to_string() << "\n";
          // Find token that matches argument's definition by short name
          auto token = std::find_if(tokens_.begin(), tokens_.end(), [this, &arg_def] (const parser::token& t) {
              std::cout << " Comparing to token " << t.s << "\n";
              if (is_short_prefixed(t.s)) {
                auto up = extract_unprefixed(t.s, scheme_def_.short_prefixes_);
                if (arg_def->short_names_) {
                  auto& short_names = *arg_def->short_names_;
                  //                  for (const auto& s : short_names) std::cout << " Candidate short name " << s << "\n";
                  return std::find(short_names.begin(), short_names.end(), std::get<1>(up)[0]) != short_names.end();
                } else {
                  std::cout << "No short name for argument!\n";
                }
              } else {
                std::cout << "Not short prefixed " << t.s << "\n";
              }
              return false;
            });
          if (token != tokens_.end()) {
            color_siblings(arg_def, arg_siblings);
            consume_value_with_implicit(arg_def, token);
            rslt = true;
            break;
          } else break;
        } else break;
      }
      // If no more next siblings, we have to move to another argument matching type
      if ((tokens_.size() == 0) || !rslt) break;
    }
    return rslt;
  }
  
  bool parser::consume_positonal_known(std::vector<scheme::entity_ptr>& arg_siblings) {
    bool rslt{false};
    while (true) {
      std::cout << "Positional named cycle\n";
      // Find matching argument
      for (auto& arg_def : arg_siblings) {
        if ((arg_def->kind_ == scheme::entity::KIND::ARGUMENT) &&
            (arg_def->color_ != scheme::entity::COLOR::BLOCKED) &&
            (arg_def->is_positional_ && *arg_def->is_positional_)) {
          std::cout << "Trying positional arg def " << arg_def->all_names_to_string() << "\n";
          auto& val_siblings = arg_def->pending_;
          for (auto& val_def : val_siblings) {
            // Find token that matches value definition, value def should not be "any"
            if ((val_def->kind_ == scheme::entity::KIND::VALUE) &&
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
                return true;
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
      if ((tokens_.size() == 0) || !rslt) return false;
    }
    return rslt;
  }

  void parser::clear_visited(scheme::entity_ptr& e) {
    if (e->color_ == scheme::entity::COLOR::VISITED)
      e->color_ = scheme::entity::COLOR::NONE;
    for (auto& c : e->pending_) clear_visited(c);
  }

  bool parser::pass_tree() {
    std::list<scheme::entity_ptr> q;
    if (scheme_def_.root_->color_ != scheme::entity::COLOR::BLOCKED)
      q.push_back(scheme_def_.root_);
    
    while (q.size() > 0) {
      std::cout << "Q size " << q.size() << "\n";
      for (const auto& x : q) {
        std::cout << "   " << x->all_names_to_string() << "\n";
      }
      auto p = q.front();
      q.pop_front();
      p->color_ = scheme::entity::COLOR::VISITED;
      // Find non-blocked child
      std::vector<scheme::entity_ptr> arg_siblings;
      for (const auto& x : p->pending_) {
        std::cout << "X kind " << (int)x->kind_ << "\n";
      }
      copy_if(p->pending_.begin(), p->pending_.end(),
              std::back_inserter(arg_siblings),
              [] (const scheme::entity_ptr& c) {
          return
          (c->color_ != scheme::entity::COLOR::BLOCKED) &&
          (c->color_ != scheme::entity::COLOR::VISITED) &&
          (c->kind_ == scheme::entity::KIND::ARGUMENT);
        });
      if (arg_siblings.size() > 0) {
        if (consume_long(arg_siblings) || consume_short(arg_siblings) || consume_positonal_known(arg_siblings))
          return true;
      } else {
        std::cout << "No arguments among childs\n";
        break;
      }
      std::cout << "Pushing more nodes\n";
      for (const auto& c : p->pending_) {
        if (c->color_ != scheme::entity::COLOR::BLOCKED) {
          q.push_back(p);
        }
      }
    }
    std::cout << "Returning false\n";
    return false;
  }

  void parser::color_siblings(scheme::entity_ptr& entity, std::vector<scheme::entity_ptr>& siblings) {
    entity->color_ = scheme::entity::COLOR::VISITED;
    if (entity->siblings_group_ == scheme::SIBLINGS_GROUP::XOR) {
      for (auto& s : siblings) {
        if ((s != entity) && (s->kind_ == entity->kind_) && (s->siblings_group_ == scheme::SIBLINGS_GROUP::XOR))
          s->color_ = scheme::entity::COLOR::BLOCKED;
      }
    }
  }

  bool parser::visitables_left(scheme::entity_ptr e) {
    bool rslt{false};
    if ((e->color_ != scheme::entity::COLOR::VISITED) &&
        (e->color_ != scheme::entity::COLOR::BLOCKED)) return true;
    for (auto& c : e->pending_)
      if (c->color_ != scheme::entity::COLOR::BLOCKED) {
        rslt = rslt | visitables_left(c);
      };
    return rslt;
  }
  
  // Parse
  void parser::parse() {
    scheme_def_.values_.clear();
    scheme_def_.positional_.clear();

    while (true) {
      // If we still have unparsed data
      if (tokens_.size() > 0) {
        // If this tree pass did not result in parsed argument
        if (!pass_tree()) {
          for (auto& e : scheme_def_.root_->pending_) {
            if (visitables_left(e)) continue;
          }
          std::cout << "Still " << tokens_.size() << " tokens left:\n";
          for (const auto& t : tokens_) std::cout << " * " << t.s << "\n";
          throw std::runtime_error("Unexpected arguments");
        } else {
          // Finish with success
          break;
        }
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
