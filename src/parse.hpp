#pragma once

#include <deque>
#include <string>
#include <vector>

#include "options.hpp"

namespace optspp {
  struct separation {
    separation(const std::string& s,
               const std::vector<std::string>& separators) :
      key(s) {
      for (const auto& sep : separators) {
        auto pos = s.find(sep);
        if (pos != std::string::npos) {
          key = s.substr(0, pos);
          value_pos = pos + sep.length() + 1;
          value = s.substr(value_pos, s.length() - value_pos);
          was_separated_ = true;
          break;
        }
      }
    }

    explicit operator bool() const {
      return was_separated_;
    }

    std::string key;
    std::string value;
    size_t value_pos;
  private:
    bool was_separated_{false};
  };

  struct parser {
    parser(options& os,
           const std::vector<std::string>& args) :
      os_(os) {
      for (size_t i = 0; i < args.size(); ++i)
        tokens_.push_back({i, 0, args[i]});
    }

    void parse() {
      os_.values_.clear();
      os_.positional_.clear();
      while (tokens_.size() > 0) {
        if (parse_long_option()) continue;
        if (parse_short_options()) continue;
        if (parse_positional()) continue;
        throw exception::unknown_parameter(tokens_.front(), tokens_.front().s);
      }
    }
      
  private:
    options& os_;
    std::deque<token> tokens_;

    bool parse_value_sources(const token& t, 
                             const std::shared_ptr<option>& o,
                             const separation& separated) {
      // We got the value from current token
      if (separated) {
        if (separated.value == "")
          add_value_implicit(t, o);
        else
          add_value(t, o, separated.value);
        tokens_.pop_front();
        return true;
      }
      // Continue looking for value in the next token.
      const auto& next_t = *(tokens_.begin() + 1);
      // Check if we don't have more tokens.
      if (tokens_.size() == 1) {
        add_value_implicit(t, o);
        tokens_.pop_front();
        return true;
      }

      // Next token is a new option
      if (is_long_prefixed(next_t.s) ||
          is_short_prefixed(next_t.s)) {
        add_value_implicit(t, o);
        tokens_.pop_front();
        return true;
      }
      // Next token must be our the value
      add_value(next_t, o, next_t.s);
      tokens_.pop_front(); // Remove name
      tokens_.pop_front(); // Remove value
      return true;
    }
    
    bool parse_long_option() {
      const auto& t = tokens_.front();
      if (is_long_prefixed(t.s)) {
        auto unprefixed = extract_unprefixed(t.s, os_.long_prefixes_);
        // Check if we have a specially separated value
        auto separated = separation(unprefixed, os_.separators_);
        // Find the option by name
        const auto& name = separated.key;
        auto o = os_.find(name);
        if (!o) throw exception::unknown_parameter(t, name);
        return parse_value_sources(t, o, separated);
      }
      return false;
    }

    bool parse_short_options() {
      // Copy-construct
      const auto t = tokens_.front();
      if (is_short_prefixed(t.s)) {
        auto unprefixed = extract_unprefixed(t.s, os_.short_prefixes_);
        // Check if we have a specially separated value
        auto separated = separation(unprefixed, os_.separators_);
        // See if we have to unpack many short parameters with single prefix
        if (separated.key.size() > 1) {
          tokens_.pop_front();
          if (separated) { // Needed to push empty string in case we have -abcd=
            tokens_.push_front({
                t.pos_arg_num, t.pos_in_arg + separated.value_pos, separated.value});
          }
          const auto& names = separated.key;
          size_t i = names.size();
          for (auto ri = names.rbegin(); ri != names.rend(); ++ri) {
            --i;
            tokens_.push_front({t.pos_arg_num, t.pos_in_arg + i, std::string("-") + names[i]});
          }
          return true;
        }
        // We have a single option
        if (separated.key.size() == 1) {
          char name = separated.key[0];
          auto o = os_.find(name);
          if (!o) throw exception::unknown_parameter(t, std::string() + name);
          return parse_value_sources(t, o, separated);
        }
      }
      return false;
    }

    bool parse_positional() {
      const auto& t = tokens_.front();
      if (!is_short_prefixed(t.s) && !(is_short_prefixed(t.s))) {
        os_.positional_.push_back(t.s);
        tokens_.pop_front();
        if (os_.max_positional_count_ < os_.positional_.size())
          throw exception::too_many_parameters(t, t.s);
        return true;
      }
      return false;
    }
      
    static const std::string extract_unprefixed(const std::string& s, const std::vector<std::string>& prefixes) {
      std::string name;
      for (const auto& prefix : prefixes) {
        if (s.find(prefix) == 0) {
          name = s.substr(prefix.size(), s.size());
          break;
        }
      }
      return name;
    }
    
    static bool is_prefixed(const std::string& s, const std::vector<std::string>& prefixes) {
      for (const auto& prefix : prefixes) {
        if (s.find(prefix) == 0) return true;
      }
      return false;
    }

    bool is_long_prefixed(const std::string& s) const {
      return is_prefixed(s, os_.long_prefixes_);
    }
      
    bool is_short_prefixed(const std::string& s) const {
      return is_prefixed(s, os_.short_prefixes_);
    }

    void add_value_implicit(const token& t, const std::shared_ptr<option>& o) {
      if (o->implicit_values().size() > 0) {
        const auto& v = o->implicit_values();
        std::copy(v.begin(), v.end(), std::back_inserter(os_.values_[o]));
        check_value_counts(t, o);
      } else {
        throw exception::parameter_requires_value(t, o);
      }
    }

    void check_value_counts(const token& t, const std::shared_ptr<option>& o) const  {
      if (o->max_count() < os_.values_[o].size()) {
        throw exception::too_many_values(t, o);
      }
    }

    void add_value(const token& t, const std::shared_ptr<option>& o, const std::string& s) {
      if (o->is_valid_value(s)) {
        os_.values_[o].push_back(o->main_value(s));
        check_value_counts(t, o);
        try {
          os_.check_value_mutually_exclusive(o);
        } catch (const exception::value_mutual_exclusiveness_violated& e) {
          throw exception::value_mutual_exclusiveness_violated(t, o, e.values);
        }
      } else {
        throw exception::invalid_parameter_value(t, o, s);
      }
    }

  };
}
