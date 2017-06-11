#pragma once

namespace optspp {
  parser::parser(scheme::arguments& args,
                 const std::vector<std::string>& cmdl_args) :
    args_(args),
    node_(args_.root_),
    next_node_(args_.root_) {
    for (size_t i = 0; i < cmdl_args.size(); ++i)
      tokens_.push_back({i, 0, cmdl_args[i]});
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

  void parser::parse() {
    args_.values_.clear();
    args_.positional_.clear();
    while (tokens_.size() > 0) {
      node_ = next_node_;
      if (parse_long_argument()) continue;
      if (parse_short_argument()) continue;
      if (parse_positional()) continue;
      throw exception::unknown_argument(tokens_.front(), tokens_.front().s);
    }
  }

  bool parser::parse_value_sources(const token& t, 
                                   const std::shared_ptr<scheme::attributes>& arg,
                                   const detail::separation& separated) {
    // We got the value from current token
    if (separated) {
      if (separated.value == "")
        add_value_implicit(t, arg);
      else
        add_value(t, arg, separated.value);
      tokens_.pop_front();
      return true;
    }
    // Continue looking for value in the next token.
    const auto& next_t = *(tokens_.begin() + 1);
    // Check if we don't have more tokens.
    if (tokens_.size() == 1) {
      add_value_implicit(t, arg);
      tokens_.pop_front();
      return true;
    }

    // Next token is a new option
    if (is_long_prefixed(next_t.s) ||
        is_short_prefixed(next_t.s)) {
      add_value_implicit(t, arg);
      tokens_.pop_front();
      return true;
    }
    // Next token must be our the value
    add_value(next_t, arg, next_t.s);
    tokens_.pop_front(); // Remove name
    tokens_.pop_front(); // Remove value
    return true;
  }
  
  bool parser::parse_long_argument() {
    const auto& t = tokens_.front();
    if (is_long_prefixed(t.s)) {
      size_t unprefixed_pos{0};
      std::string unprefixed;
      std::tie(unprefixed_pos, unprefixed) = extract_unprefixed(t.s, args_.long_prefixes_);
      // Check if we have a specially separated value
      auto separated = detail::separation(unprefixed, args_.separators_);
      // Find the option by name
      const auto& name = separated.key;

      auto found = std::find_if(node_->children().begin(), node_->children().end(),
                                [&name] (const scheme::node_ptr& n) {
                                  return
                                  (((**n)->kind() == scheme::attributes::KIND::NAME) &&
                                   (std::find((**n)->long_names().begin(), (**n)->long_names().end(), name) != (**n)->long_names().end()));
                                  // TODO: named/anyvalue
                                });
      
      if (found == node_->children().end()) throw exception::unknown_argument(t, name);
      next_node_ = *found;
      return parse_value_sources(t, ***found, separated);
    }
    return false;
  }
  
  bool parser::parse_short_argument() {
    // Copy-construct
    const auto t = tokens_.front();
    if (is_short_prefixed(t.s)) {
      size_t unprefixed_pos{0};
      std::string unprefixed;
      std::tie(unprefixed_pos, unprefixed) = extract_unprefixed(t.s, args_.short_prefixes_);
      // Check if we have a specially separated value
      auto separated = detail::separation(unprefixed, args_.separators_);
      // See if we have to unpack many short parameters with single prefix
      if (separated.key.size() > 1) {
        tokens_.pop_front();
        if (separated) { // Needed to push empty string in case we have -abcd=
          tokens_.push_front({
              t.pos_arg_num, t.pos_in_arg + unprefixed_pos + separated.value_pos, separated.value});
        }
        const auto& names = separated.key;
        size_t i = names.size();
        for (auto ri = names.rbegin(); ri != names.rend(); ++ri) {
          --i;
          tokens_.push_front({t.pos_arg_num, t.pos_in_arg + i,
                args_.short_prefixes_[0] + names[i]});
        }
        return true;
      }
      // We have a single option
      if (separated.key.size() == 1) {
        char name = separated.key[0];
        auto found = std::find_if(node_->children().begin(), node_->children().end(),
                                  [&name] (const scheme::node_ptr& n) {
                                    return
                                    (((**n)->kind() == scheme::attributes::KIND::NAME) &&
                                     (std::find((**n)->short_names().begin(), (**n)->short_names().end(), name) != (**n)->short_names().end()));
                                    // TODO: predefined value/anyvalue
                                  });
        if (found == node_->children().end()) throw exception::unknown_argument(t, std::string() + name);
        next_node_ = *found;
        return parse_value_sources(t, ***found, separated);
      }
    }
    return false;
  }
    
  bool parser::parse_positional() {
    const auto& t = tokens_.front();
    if ((is_short_prefixed(t.s) || is_short_prefixed(t.s)) && !take_as_positionals_) return false;
    args_.positional_.push_back(t.s);
    tokens_.pop_front();
    auto found = std::find_if(node_->children().begin(), node_->children().end(),
                              [] (const scheme::node_ptr& n) {
                                return
                                (((**n)->kind() == scheme::attributes::KIND::NAME) &&
                                 (**n)->is_positional());
                                // TODO: named/anyvalue
                              });
    next_node_ = *found;
    if (args_.max_positional_count_ < args_.positional_.size())
      throw exception::too_many_arguments(t, t.s);
    return true;
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
