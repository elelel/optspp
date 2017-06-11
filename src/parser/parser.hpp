#pragma once

namespace optspp {
  parser::parser(scheme::arguments& args,
                 const std::vector<std::string>& cmdl_args) :
    args_(args),
    next_node_(args_.root_) {
    for (size_t i = 0; i < cmdl_args.size(); ++i)
      tokens_.push_back({i, 0, cmdl_args[i]});
    separate();
    for (const auto& t : tokens_)
      std::cout << "Init token: " << t.s << "\n";
  }

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
    return is_prefixed(s, args_.long_prefixes_);
  }
      
  bool parser::is_short_prefixed(const std::string& s) const {
    return is_prefixed(s, args_.short_prefixes_);
  }
  
  void parser::separate() {
    // This is not in parsing routine because it should not be recursive to make sure {xxx=yyy=zzz} stays as {xxx, yyy=zzz}
    std::list<token> ts;
    while (tokens_.size() > 0) {
      const auto& t = *(tokens_.begin());
      for (const auto& sep : args_.separators_) {
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
  }

  void parser::parse() {
    args_.values_.clear();
    args_.positional_.clear();

    while (tokens_.size() > 0) {
      std::cout << "Using next_node " << next_node_ << "\n";
      if (next_node_ != nullptr) {
        node_ = next_node_;
      } else {
        throw exception::unknown_argument(*token_it_, token_it_->s);
      }
      for (token_it_ = tokens_.begin(); token_it_ != tokens_.end();) {
        std::cout << "Token: " << token_it_->s << "\n";
        std::cout << "Testing long\n";
        if (consume_long_argument()) continue;
        std::cout << "Testing short\n";
        if (consume_short_argument()) continue;
        std::cout << "Testing positional\n";
        if (consume_positional()) continue;
        std::cout << "End cycle\n";
      }
      throw exception::unknown_argument(tokens_.front(), tokens_.front().s);
    }
  }

  bool parser::consume_long_argument() {
    const auto& t = *token_it_;
    if (is_long_prefixed(t.s)) {
      size_t unprefixed_pos{0};
      std::string name;
      std::tie(unprefixed_pos, name) = extract_unprefixed(t.s, args_.long_prefixes_);
      // Find the option by name
      auto found = std::find_if(node_->children().begin(), node_->children().end(),
                                [&name] (const scheme::node_ptr& n) {
                                  return
                                  (((**n)->kind() == scheme::attributes::KIND::NAME) &&
                                   (std::find((**n)->long_names().begin(), (**n)->long_names().end(), name) != (**n)->long_names().end()));
                                  // TODO: named/anyvalue
                                });
      
      if (found == node_->children().end()) throw exception::unknown_argument(t, name);
      next_node_ = *found;
      return consume_value_sources(***found);
    }
    return false;
  }
  
  bool parser::consume_short_argument() {
    // Copy-construct
    if (is_short_prefixed(token_it_->s)) {
      size_t unprefixed_pos{0};
      std::string names;
      std::tie(unprefixed_pos, names) = extract_unprefixed(token_it_->s, args_.short_prefixes_);
      // See if we have to unpack many short parameters with single prefix
      if (names.size() > 1) {
        auto t = *token_it_;
        auto it = tokens_.erase(token_it_);
        for (auto i = 0; i < names.size(); ++i) {
          tokens_.insert(it, {t.pos_arg_num, t.pos_in_arg + i,
                args_.short_prefixes_[0] + names[i]});
        }
        token_it_ = tokens_.begin();
        return true;
      }
      // We have a single option
      if (names.size() == 1) {
        char name = names[0];
        auto found = std::find_if(node_->children().begin(), node_->children().end(),
                                  [&name] (const scheme::node_ptr& n) {
                                    return
                                    (((**n)->kind() == scheme::attributes::KIND::NAME) &&
                                     (std::find((**n)->short_names().begin(), (**n)->short_names().end(), name) != (**n)->short_names().end()));
                                    // TODO: predefined value/anyvalue
                                  });
        if (found == node_->children().end()) throw exception::unknown_argument(*token_it_, std::string() + name);
        next_node_ = *found;
        return consume_value_sources(***found);
      }
    }
    return false;
  }
  
  bool parser::consume_value_sources(const std::shared_ptr<scheme::attributes>& arg) {
    // Check if we don't have more tokens.
    if (tokens_.size() == 1) {
      add_value_implicit(*token_it_, arg);
      tokens_.erase(token_it_);
      token_it_ = tokens_.begin();
      return true;
    }

    // Continue looking for value in the next token.
    std::list<token>::iterator next_it(token_it_);
    ++next_it;
    // Next token is a new option
    if ((is_long_prefixed(next_it->s) ||
         is_short_prefixed(next_it->s)) &&
        !ignore_option_prefixes_) {
      add_value_implicit(*token_it_, arg);
      tokens_.erase(token_it_);
      token_it_ = tokens_.begin();
      return true;
    }
    // Next token must be our the value
    add_value(*next_it, arg, next_it->s);
    // Remove tokens containing name and value
    tokens_.erase(tokens_.erase(token_it_));
    token_it_ = tokens_.begin();
    return true;
  }
  
    
  bool parser::consume_positional() {
    const auto t = tokens_.front();
    if ((is_short_prefixed(t.s) || is_short_prefixed(t.s)) && !ignore_option_prefixes_) return false;
    std::cout << "Searching\n";
    auto found = std::find_if(node_->children().begin(), node_->children().end(),
                              [] (const scheme::node_ptr& n) {
                                return
                                (((**n)->kind() == scheme::attributes::KIND::NAME) &&
                                 (**n)->is_positional());
                                // TODO: named/anyvalue
                              });
    std::cout << "Done\n";
    if (found != node_->children().end()) {
      args_.positional_.push_back(t.s);
      next_node_ = *found;
      std::cout << "Assingned next node";
      return true;
    } else {
      throw exception::unknown_argument(t, t.s);
    }
    return false;
  }

  void parser::add_value_implicit(const token& t, const std::shared_ptr<scheme::attributes>& arg) {
    if (arg->implicit_values().size() > 0) {
      const auto& v = arg->implicit_values();
      std::copy(v.begin(), v.end(), std::back_inserter(args_.values_[arg]));
      check_value_counts(t, arg);
    } else {
      throw exception::missing_value(t, arg);
    }
  }

  void parser::check_value_counts(const token& t, const std::shared_ptr<scheme::attributes>& arg) const  {
    if (arg->max_count() < args_.values_[arg].size()) {
      throw exception::too_many_values(t, arg);
    }
  }

  void parser::add_value(const token& t, const std::shared_ptr<scheme::attributes>& arg, const std::string& s) {
    //    if (arg->is_valid_value(s)) {
    //      args_.values_[arg].push_back(arg->main_value(s));
      check_value_counts(t, arg);
      /*try {
        args_.check_value_mutually_exclusive(o);
      } catch (const exception::value_mutual_exclusiveness_violated& e) {
        throw exception::value_mutual_exclusiveness_violated(t, o, e.values);
        }*/
      //} else {
      //      throw exception::invalid_value(t, arg, s);
      //    }
  }

}
