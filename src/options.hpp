#pragma once

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <iostream>

#include "exception.hpp"
#include "predeclare.hpp"
#include "parse.hpp"

namespace optspp {
  template <typename... Args>
  std::shared_ptr<option> make_options(Args&&... opts) {
    auto o = std::make_shared<options>(std::forward<Args>(opts)...);
    return o;
  }
  
  struct options : std::enable_shared_from_this<options> {
    options() {
    }

    void swap(options& other) {
      std::swap(options_, other.options_);
      std::swap(long_prefixes_, other.long_prefixes_);
      std::swap(short_prefixes_, other.short_prefixes_);

      std::swap(args_, other.args_);

      std::swap(values_, other.values_);
    }
    
    template <typename... Args>
    options(Args&&... args) {
      apply_options(std::forward<Args>(args)...);
    }
    
    void apply_options(const std::shared_ptr<option>& o) {
      auto found = std::find(options_.begin(), options_.end(), o);
      if (found == options_.end()) {
        std::cout << "pushing\n";
        options_.push_back(o);
        std::cout << "pushed\n";
      }
      check();
    }

    void apply_options() {
    }
    
    template <typename... Args>
    void apply_options(const std::shared_ptr<option>& o, Args&&... args) {
      apply_options(o);
      apply_options(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void apply_options(const option& o, Args&&... args) {
      auto op = std::make_shared<option>(o);
      apply_options(op);
      apply_options(std::forward<Args>(args)...);
    }
    
    void add(const std::shared_ptr<option>& o) {
      options_.push_back(o);
    }

    void erase(const std::shared_ptr<option>& o) {
      options_.erase(std::remove(options_.begin(), options_.end(), o), options_.end());
    }

    void check() const {
      for (auto it1 = options_.begin(); it1 != options_.end(); ++it1) {
        for (auto it2 = options_.begin(); it2 != options_.end(); ++it2) {
          if (it1 < it2) {
            { // Long names
              std::vector<std::string> lhs{(*it1)->long_name_synonyms()};
              std::vector<std::string> rhs{(*it2)->long_name_synonyms()};
              lhs.push_back((*it1)->long_name());
              rhs.push_back((*it2)->long_name());
              for (const auto& a : lhs) {
                for (const auto& b : rhs) {
                  if (a == b) throw exception::long_name_conflict(a);
                }
              }
            }
            { // Short names
              std::vector<char> lhs{(*it1)->short_name_synonyms()};
              std::vector<char> rhs{(*it2)->short_name_synonyms()};
              lhs.push_back((*it1)->short_name());
              rhs.push_back((*it2)->short_name());
              for (const auto& a : lhs) {
                for (const auto& b : rhs) {
                  if (a == b) throw exception::short_name_conflict(a);
                }
              }
            }
          }
        }
      }
    }

    std::vector<std::shared_ptr<option>>::const_iterator find_iterator(const std::string& long_name) const {
      return std::find_if(options_.begin(), options_.end(),
                          [&long_name] (const std::shared_ptr<option>& o) {
                            const auto& syns = o->long_name_synonyms();
                            return (o->long_name() == long_name) ||
                              (std::find(syns.begin(), syns.end(), long_name) != syns.end());
                          });
    }

    std::shared_ptr<option> find(const std::string& long_name) const {
      auto found = find_iterator(long_name);
      if (found != options_.end()) return *found;
      return nullptr;
    }

    std::vector<std::shared_ptr<option>>::const_iterator find_iterator(const char& short_name) const {
      return std::find_if(options_.begin(), options_.end(),
                               [&short_name] (const std::shared_ptr<option>& o) {
                                 const auto& syns = o->short_name_synonyms();
                                 return (o->short_name() == short_name) ||
                                 (std::find(syns.begin(), syns.end(), short_name) != syns.end());
                               });
    }
    
    std::shared_ptr<option> find(const char& short_name) const {
      auto found = find_iterator(short_name);
      if (found != options_.end()) return *found;
      return nullptr;
    }

    template <typename Option>
    options& operator<<(const Option& o) {
      apply_options(o);
      return *this;
    }

    void parse(const std::vector<std::string>& args) {
      args_.clear();
      std::copy(args.begin(), args.end(), std::back_inserter(args_));
      parse_();
    }

    const std::vector<std::string>& operator[](const std::string& name) const {
      auto o = find(name);
      if (o != nullptr) {
        return values_.at(o);
      } else {
        throw exception::non_existent_option_value_requested(name);
      }
    };
   
    const std::vector<std::string>& operator[](const char& name) const {
      auto o = find(name);
      if (o != nullptr) {
        return values_.at(o);
      } else {
        throw exception::non_existent_option_value_requested(name);
      }
    };

    const size_t& max_positional_args() const {
      return max_positional_args_;
    }

    void set_max_positional_args(const size_t& n) {
      max_positional_args_ = n;
    }

    const size_t& min_positional_args() const {
      return min_positional_args_;
    }

    void set_min_positional_args(const size_t& n) {
      max_positional_args_ = n;
    }

    const std::vector<std::string>& positional() const {
      return positional_;
    }
  private:
    std::vector<std::shared_ptr<option> > options_;
    std::vector<std::string> long_prefixes_{ {"--"} };
    std::vector<std::string> short_prefixes_{ {"-"} };
    std::vector<std::string> separators_{ {"="} };
    size_t max_positional_args_{std::numeric_limits<size_t>::max()};
    size_t min_positional_args_{std::numeric_limits<size_t>::min()};

    std::vector<std::string> args_;
    
    std::map<std::shared_ptr<option>, std::vector<std::string>> values_;
    std::vector<std::string> positional_;


    bool parse_long_option(std::vector<std::string>::iterator& it) {
      std::string name = parsing::as_long_name(*it, long_prefixes_);
      if (name != "") {
        auto o = find(name);
        if (o == nullptr) throw exception::unknown_parameter(name);
        ++it;
        // No more args
        if (it == args_.end()) {
          set_value_implicit(o);
          return true;
        }
        // Next arg is an option
        if (parsing::is_prefixed(*it, long_prefixes_) ||
            parsing::is_prefixed(*it, short_prefixes_)) {
          set_value_implicit(o);
          return true;
        }
        // Treat next arg as value
        set_value_to_string(o, *it);
        ++it;
        return true;
      }
      return false;
    }

    bool parse_short_options(std::vector<std::string>::iterator& it) {
      std::vector<char> names = parsing::as_short_names(*it, short_prefixes_);
      for (auto name_it = names.begin(); name_it != names.end(); ++name_it) {
        auto o = find(*name_it);
        if (o == nullptr) throw exception::unknown_parameter(*name_it);
        // last option in pack
        if (name_it == names.end() - 1) {
          ++it;
          // there-re no more args
          if (it == args_.end()) {
            set_value_implicit(o);
            return true;
          } 
          // Next arg is an option
          if (parsing::is_prefixed(*it, long_prefixes_) ||
              parsing::is_prefixed(*it, short_prefixes_)) {
            set_value_implicit(o);
            return true;
          }
          // Treat next arg as value
          set_value_to_string(o, *it);
          ++it;
          return true;
        }
      }
      return false;
    }

    bool parse_positional(std::vector<std::string>::iterator& it) {
      if (!parsing::is_prefixed(*it, long_prefixes_) &&
          !parsing::is_prefixed(*it, short_prefixes_)) {
        if ((max_positional_args_ != std::numeric_limits<size_t>::max()) &&
            (positional_.size() == max_positional_args_))
          throw exception::superflous_positional_parameter(*it);
        positional_.push_back(*it);
        ++it;
        return true;
      }
      return false;
    }

    void parse_() {
      values_.clear();
      positional_.clear();
      auto it = args_.begin();
      while (it != args_.end()) {
        if (parse_long_option(it)) continue;
        if (parse_short_options(it)) continue;
        parse_positional(it);
      }
      check_value_counts();
      add_default_values();
    }

    void set_value_implicit(const std::shared_ptr<option>& o) {
      if (o->implicit_values().size() > 0) {
        values_[o] = o->implicit_values();
      } else {
        throw exception::parameter_requires_value(o);
      }
      check_values_mutually_exclusive(o);
    }

    void set_value_to_string(const std::shared_ptr<option>& o, const std::string& s) {
      if (o->is_valid_value(s)) {
        values_[o].push_back(o->main_value(s));
      } else {
        throw exception::invalid_parameter_value(o, s);
      }
      check_values_mutually_exclusive(o);
    }
    
    void check_value_counts() {
      for (const auto& p : values_) {
        if (p.first->max_count() <  p.second.size())
          throw exception::too_many_values(p.first->long_name(), p.first->max_count(), p.second.size());
        if (p.first->min_count() > p.second.size())
          throw exception::too_few_values(p.first->long_name(), p.first->max_count(), p.second.size());
      }
    }
    
    void check_values_mutually_exclusive(const std::shared_ptr<option>& o) const {
      if (o->mutually_exclusive_values().size() == 0) return;
      std::vector<std::vector<std::string>> rslt;
      for (const auto& me : o->mutually_exclusive_values()) {
        std::vector<std::string> cur_set;
        for (const auto& v : values_.at(o)) {
          if (std::find(me.begin(), me.end(), v) != me.end()) cur_set.push_back(v);
        }
        if (cur_set.size() > 1) rslt.push_back(cur_set);
      }
      if (rslt.size() > 0) {
        throw exception::value_mutual_exclusiveness_violated(o, rslt);
      }
    }

    void add_default_values() {
      for (const auto& o : options_) {
        if ((o->default_values().size() != 0) && (values_[o].size() == 0))
          values_[o] = o->default_values();
      }
    }
    
  };
}
