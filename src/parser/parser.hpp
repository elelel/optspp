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
    throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value");
  }

  void parser::add_value_default(scheme::entity_ptr& arg_def, const parser::token& token) {
    if (arg_def->default_values_) {
      auto& iv = *arg_def->default_values_;
      if (iv.size() > 0) {
        add_value(arg_def, main_value(arg_def, iv[0]));
        iv.erase(iv.begin());
      }
    }
    throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value");
  }
  
  scheme::entity_ptr parser::consume_value(scheme::entity_ptr& arg_def,
                                           const std::list<token>::iterator& token) {
    // Check if we don't have more tokens.
    if (tokens_.size() == 1) {
      add_value_implicit(arg_def, *token);
      tokens_.erase(token);
      return nullptr;
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
      return nullptr;
    }
    
    // Next token must be our the value
    // Find value entity that matches actual value
    auto& val_siblings = arg_def->pending_;
    auto found = find_if(val_siblings.begin(), val_siblings.end(), [&next_it] (const scheme::entity_ptr& e) {
        if ((e->kind_ == scheme::entity::KIND::VALUE) && (e->known_values_)) {
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
      found = find_if(val_siblings.begin(), val_siblings.end(), [] (const scheme::entity_ptr& e) {
          return e->any_value_ && *(e->any_value_);
        });
    }
    if (found != val_siblings.end()) {
      color_siblings(val_siblings, *found);
      //TODO add_value(*next_it, arg_def, next_it->s);
      // Remove tokens containing name and value
      tokens_.erase(tokens_.erase(token));
      return *found;
    }
    throw std::runtime_error("Argument " + arg_def->all_names_to_string() + " requires a value");
  }
  
  void parser::color_siblings(std::vector<scheme::entity_ptr>& siblings, const scheme::entity_ptr& taken) {
    taken->color_ = scheme::entity::COLOR::TAKEN;
    for (auto& s : siblings) {
      if ((s != taken) && (s->kind_ == taken->kind_) && (s->siblings_group_ == scheme::SIBLINGS_GROUP::XOR))
        s->color_ = scheme::entity::COLOR::BLOCKED;
    }
  }

  // Parse
  void parser::parse() {
    scheme_def_.values_.clear();
    scheme_def_.positional_.clear();

    scheme::entity_ptr parent;
    // Initialize siblings reference
    std::vector<scheme::entity_ptr>* arg_siblings;
    if (parent != nullptr) {
      arg_siblings = &parent->pending_;
    } else {
      arg_siblings = &scheme_def_.pending_;
    }
    while (true) {
      if (tokens_.size() > 0) {
        //TODO If no more nodes to walk, but tokens left - throw superflous paremeter
      } else {
        // Finish with success
        break;
      }

      bool arg_matched_long{false};
      bool arg_matched_short{false};
      bool arg_matched_positional{false};
      // Long-named
      while (true) {
        // Find matching argument
        for (auto& arg_def : *arg_siblings) {
          if ((arg_def->kind_ == scheme::entity::KIND::ARGUMENT) &&
              (arg_def->color_ != scheme::entity::COLOR::BLOCKED)) {
            // Find token that matches argument's definition by long name
            auto token = std::find_if(tokens_.begin(), tokens_.end(), [this, &arg_def] (const parser::token& t) {
                if (is_long_prefixed(t.s)) {
                  auto up = extract_unprefixed(t.s, scheme_def_.long_prefixes_);
                  if (arg_def->long_names_) {
                    auto& long_names = *arg_def->long_names_;
                    return std::find(long_names.begin(), long_names.end(), std::get<1>(up)) != long_names.end();
                  }
                }
                return false;
              });
            if (token != tokens_.end()) {
              color_siblings(*arg_siblings, arg_def);
              parent = consume_value(arg_def, token);
              // parent = value_entity_ptr
              arg_matched_long = true;
              break;
            } else {
              // Insert it as default value if available
              add_value_default(arg_def, *token);
              tokens_.erase(token);
              arg_matched_long = true;
              break;
            }
          } else {
            throw std::runtime_error("Scheme definition error: argument kind expected");
          }
        }
        // If no more next siblings, we have to move to another argument matching type
        if (!arg_matched_long) break;
      }
      if (arg_matched_long) {
        // TODO: Color arg node
        continue;
      }
      // Short-named
      
      // Positional


      if (!(arg_matched_long || arg_matched_short || arg_matched_positional)) {
        // throw unknown argument tokens
      }
    }
  }
}
