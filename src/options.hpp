#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "exception.hpp"
#include "predeclare.hpp"

namespace optspp {
  struct options : std::enable_shared_from_this<options> {
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

    options& operator<<(const std::shared_ptr<option>& o) {
      if (std::find(options_.begin(), options_.end(), o) == options_.end()) {
        options_.push_back(o);
        check();
      }
    }

    options& operator<<(const option& o) {
      auto op = std::make_shared<option>(o);
      return operator<<(op);
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
              } else {
                throw exception::long_parameter_requires_value(name);
              }
            } else {
              if (o.is_valid_value(*it)) {
                values_[*o_it].push_back(*it);
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
                  } else {
                    throw exception::short_parameter_requires_value(name);
                  }
                } else {
                  if (o.is_valid_value(*it)) {
                    values_[*o_it].push_back(*it);
                    ++it;
                  } else {
                    throw exception::invalid_short_parameter_value(name, *it);
                  }
                }
                break;
              } else {  // Not the last short option in single pack
                if (o.implicit_values().size() > 0) {
                  values_[*o_it] = o.implicit_values();
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
    }
    
  };
}
