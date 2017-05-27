#pragma once

#include <string>
#include <vector>

namespace optspp {
  namespace parsing {
    std::vector<std::string> tokenize(const std::vector<std::string>& args, const std::vector<std::string>& separators) {
      std::vector<std::string> rslt;
      for (const auto& arg : args) {
        for (const auto& sep : separators) {
          auto pos = arg.find(sep);
          if (pos != std::string::npos) {
            rslt.push_back(arg.substr(0, pos));
            rslt.push_back(arg.substr(pos + sep.length() + 1,
                                      arg.length() - (pos + sep.length() + 1)));
          } else {
            rslt.push_back(arg);
          }
        }
      }
      return rslt;
    }

    const std::string as_long_name(const std::string& token, const std::vector<std::string>& long_prefixes) {
      std::string name;
      for (const auto& long_prefix : long_prefixes) {
        if (token.find(long_prefix) == 0) {
          name = token.substr(long_prefix.size(), token.size());
          break;
        }
      }
      return name;
    }

    const std::vector<char> as_short_names(const std::string& token, const std::vector<std::string>& short_prefixes) {
      std::vector<char> names;
      for (const auto& short_prefix : short_prefixes) {
        if (token.find(short_prefix) == 0) {
          auto names_str = token.substr(short_prefix.size(), token.size());
          for (const auto& c : names_str) names.push_back(c);
          break;
        }
      }
      return names;
    }
    
    bool is_prefixed(const std::string& token, const std::vector<std::string>& prefixes) {
      for (const auto& prefix : prefixes) {
        if (token.find(prefix) == 0) return true;
      }
      return false;
    }
  }
}
