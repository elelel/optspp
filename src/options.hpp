#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <iostream>

#include "exception.hpp"
#include "predeclare.hpp"

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

    std::shared_ptr<option> find(const std::string& long_name) const {
      std::cout << "Searching for long name " << long_name << " in " << options_.size() << "\n";
      for (const auto& o : options_) {
        if (o->long_name() == long_name) return o;
        const auto& syns = o->long_name_synonyms();
        if (std::find(syns.begin(), syns.end(), long_name) != syns.end())
          return o;
      }
      return nullptr;
    }

    std::shared_ptr<option> find(const char& short_name) const {
      std::cout << "Searching for short name " << short_name << " in " << options_.size() << "\n";
      for (const auto& o : options_) {
        if (o->short_name() == short_name) return o;
        const auto& syns = o->short_name_synonyms();
        if (std::find(syns.begin(), syns.end(), short_name) != syns.end())
          return o;
      }
      return nullptr;
    }

    template <typename Option>
    options& operator<<(const Option& o) {
      apply_options(o);
      return *this;
    }
    
  private:
    std::vector<std::shared_ptr<option> > options_;
    std::vector<std::string> long_prefixes_{ {"--"} };
    std::vector<std::string> short_prefixes_{ {"-"} };

    std::vector<std::string> args_;

    std::map<std::shared_ptr<option>, std::vector<std::string>> values_;

    void parse_() {
      auto it = args_.cbegin();
      while (it != args_.cend()) {
        try_long_name_(it);
        if (it == args_.cend()) break;
        try_short_name_(it);
        if (it == args_.cend()) break;
        try_positional_(it);
      }
    }

    void try_long_name_(std::vector<std::string>::const_iterator& it) {
      for (const auto& long_prefix : long_prefixes_) {
        if (it->find(long_prefix) == 0) {
          auto name = it->substr(long_prefix.size(), it->size());
          auto o_it = std::find_if(options_.begin(), options_.end(),
                                   [&name] (const std::shared_ptr<option>& o) {
                                     const auto& syns = o->long_name_synonyms();
                                     return (o->long_name() == name) ||
                                     (std::find(syns.begin(), syns.end(), name) != syns.end());
                                   });
          if (o_it != options_.end()) {
            auto& o = **o_it;
            ++it;
            if (it == args_.end()) {
              if (o.implicit_values().size() > 0) {
                values_[*o_it] = o.implicit_values();
                check_values_mutually_exclusive(name, o, values_[*o_it]);
              } else {
                throw exception::long_parameter_requires_value(name);
              }
            } else {
              if (o.is_valid_value(*it)) {
                values_[*o_it].push_back(*it);
                check_values_mutually_exclusive(name, o, values_[*o_it]);
                ++it;
              } else {
                throw exception::invalid_long_parameter_value(name, *it);
              }
            }
            break;
          } else {
            throw exception::unknown_long_parameter(name);
          }
        }
      }
    }

    void try_short_name_(std::vector<std::string>::const_iterator& it) {
      // Check if it is a short param
      for (const auto& short_prefix : short_prefixes_) {
        if (it->find(short_prefix) == 0) {
          auto names = it->substr(short_prefix.size(), it->size());
          for (size_t i = 0; i < names.size(); ++i) {
            char name = names[i];
            auto o_it = std::find_if(options_.begin(), options_.end(),
                                     [&name] (const std::shared_ptr<option>& o) {
                                       const auto& syns = o->short_name_synonyms();
                                       return (o->short_name() == name) ||
                                       (std::find(syns.begin(), syns.end(), name) != syns.end());
                                     });
            if (o_it != options_.end()) {
              auto& o = **o_it;
              if (i == names.size() - 1) {  // Last short option in single pack
                ++it;
                if (it == args_.cend()) {
                  if (o.implicit_values().size() > 0) {
                    values_[*o_it] = o.implicit_values();
                    check_values_mutually_exclusive(name, o, values_[*o_it]);
                  } else {
                    throw exception::short_parameter_requires_value(name);
                  }
                } else {
                  if (o.is_valid_value(*it)) {
                    values_[*o_it].push_back(*it);
                    check_values_mutually_exclusive(name, o, values_[*o_it]);
                    ++it;
                  } else {
                    throw exception::invalid_short_parameter_value(name, *it);
                  }
                }
                break;
              } else {  // Not the last short option in single pack
                if (o.implicit_values().size() > 0) {
                  values_[*o_it] = o.implicit_values();
                  check_values_mutually_exclusive(name, o, values_[*o_it]);
                } else {
                  throw exception::short_parameter_requires_value(name);
                }
              }
            } else {
              throw exception::unknown_short_parameter(name);
            }
          }
        }
      } 
    }

    void try_positional_(std::vector<std::string>::const_iterator& it) {
      // TODO: positional
    }
    
    void check_values_mutually_exclusive(const std::string& name,
                                         const option& o,
                                         const std::vector<std::string>& values) const {
      if (o.mutually_exclusive_values().size() == 0) return;
      std::vector<std::vector<std::string>> rslt;
      for (const auto& me : o.mutually_exclusive_values()) {
        std::vector<std::string> cur_set;
        for (const auto& v : values) {
          if (std::find(me.begin(), me.end(), v) != me.end()) cur_set.push_back(v);
        }
        if (cur_set.size() > 1) rslt.push_back(cur_set);
      }
      if (rslt.size() > 0) {
        throw exception::values_mutual_exclusiveness_violated(name, rslt);
      }
    }
    
    void check_values_mutually_exclusive(const char& name,
                                         const option& o,
                                         const std::vector<std::string>& values) const {
      std::string s;
      s += name;
      check_values_mutually_exclusive(s, o, values);
    }
  };
}
