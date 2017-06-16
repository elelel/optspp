#pragma once

namespace optspp {
  parser::parser(scheme::definition& scheme_def,
                 const std::vector<std::string>& cmdl_args) :
    scheme_def_(scheme_def) {
    for (size_t i = 0; i < cmdl_args.size(); ++i) tokens_.push_back({i, 0, cmdl_args[i]});
    separate();
  }
  
  void parser::separate() {
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
  

  // Parse
  void parser::parse() {
    scheme_def_.values_.clear();
    scheme_def_.positional_.clear();
    bool done{false};
    // construct tree
    // it = depth_first(tree).begin();
    while (!done) {
      // track parent?
      
      // Check if kind == ARGUMENT
      // If (*it)->entity matches something in args
      //   consume it with value
      //   mark node and value node as taken
      //   mark node's and value node's xor-siblings as blocked (HOW?)
      //   it = value_node.chlidren[0]
      // else
      //   it++ (breadth)
    }
  }
}
