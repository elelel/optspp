#pragma once

#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "exception.hpp"
#include "declaration.hpp"
#include "parse.hpp"

namespace optspp {
  template <typename... Args>
  std::shared_ptr<option> make_options(Args&&... opts) {
    auto o = std::make_shared<options>(std::forward<Args>(opts)...);
    return o;
  }

  options::options() {
  }
    
  template <typename... Args>
  options::options(Args&&... args) {
    apply(std::forward<Args>(args)...);
  }

  void options::apply() {
  }

  void options::apply(const std::shared_ptr<option>& o) {
    auto found = std::find(options_.begin(), options_.end(), o);
    if (found == options_.end()) {
      options_.push_back(o);
    }
    check_schema();
  }
    
  template <typename... Args>
  void options::apply(const std::shared_ptr<option>& o, Args&&... args) {
    apply(o);
    apply(std::forward<Args>(args)...);
  }

  template <typename... Args>
  void options::apply(const option& o, Args&&... args) {
    auto op = std::make_shared<option>(o);
    apply(op);
    apply(std::forward<Args>(args)...);
  }
    
  void options::add(const std::shared_ptr<option>& o) {
    options_.push_back(o);
  }

  void options::remove(const std::shared_ptr<option>& o) {
    options_.erase(std::remove(options_.begin(), options_.end(), o), options_.end());
  }

  void options::check_schema() const {
    // Name conflicts
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
    // TODO: Check name depenencies
  }

  std::vector<std::shared_ptr<option>>::const_iterator
  options::find_iterator(const std::string& long_name) const {
    return std::find_if(options_.begin(), options_.end(),
                        [&long_name] (const std::shared_ptr<option>& o) {
                          const auto& syns = o->long_name_synonyms();
                          return (o->long_name() == long_name) ||
                            (std::find(syns.begin(), syns.end(), long_name) != syns.end());
                        });
  }

  std::shared_ptr<option>
  options::find(const std::string& long_name) const {
    auto found = find_iterator(long_name);
    if (found != options_.end()) return *found;
    return nullptr;
  }

  std::vector<std::shared_ptr<option>>::const_iterator
  options::find_iterator(const char& short_name) const {
    return std::find_if(options_.begin(), options_.end(),
                        [&short_name] (const std::shared_ptr<option>& o) {
                          const auto& syns = o->short_name_synonyms();
                          return (o->short_name() == short_name) ||
                            (std::find(syns.begin(), syns.end(), short_name) != syns.end());
                        });
  }
    
  std::shared_ptr<option>
  options::find(const char& short_name) const {
    auto found = find_iterator(short_name);
    if (found != options_.end()) return *found;
    return nullptr;
  }

  template <typename Option>
  options& options::operator<<(const Option& o) {
    apply(o);
    return *this;
  }

  void options::parse(const std::vector<std::string>& args) {
    parser p(*this, args);
    p.parse();
    add_default_values();
    check_results();
  }

  void options::parse(const int argc, char* argv[]) {
    std::vector<std::string> args;
    if (argc > 1) {
      for (int i = 1; i < argc; ++i) args.push_back(std::string(argv[i]));
    }
    parse(args);
  }

  void options::check_results() {
    for (const auto& o : options_) {
      auto& v = values_[o];
      // Check value counts
      if (o->max_count() < v.size())
        throw exception::too_many_values(o);
      if (o->min_count() > v.size())
        throw exception::too_few_values(o);
      // Value mutual exclusiveness
      check_value_mutually_exclusive(o);
    }
    // TODO: Check name dependencies
    // TODO: Check value dependencies
  }
  
  void options::check_value_mutually_exclusive(const std::shared_ptr<option>& o) const {
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

  void options::add_default_values() {
    for (const auto& o : options_) {
      if ((o->default_values().size() != 0) && (values_[o].size() == 0))
        values_[o] = o->default_values();
    }
  }

  template <typename T>
  T options::as(const std::string& name, const size_t idx) const {
    std::istringstream i(this->operator[](name)[idx]);
    T value;
    i >> value;
    return value;
  }

  template <typename T>
  T options::as(const std::string& name) const {
    const auto& vs = this->operator[](name);
    std::istringstream i(vs[vs.size() - 1]);
    T value;
    i >> value;
    return value;
  }
  
  const std::vector<std::string>&
  options::operator[](const std::string& name) const {
    auto o = find(name);
    if (o != nullptr) {
      return values_.at(o);
    } else {
      throw exception::non_existent_option_value_requested(name);
    }
  }
   
  const std::vector<std::string>&
  options::operator[](const char& name) const {
    auto o = find(name);
    if (o != nullptr) {
      return values_.at(o);
    } else {
      throw exception::non_existent_option_value_requested(name);
    }
  };

  const size_t& options::max_positional_count() const {
    return max_positional_count_;
  }

  void options::set_max_positional_count(const size_t& n) {
    max_positional_count_ = n;
  }

  const size_t& options::min_positional_count() const {
    return min_positional_count_;
  }

  void options::set_min_positional_count(const size_t& n) {
    max_positional_count_ = n;
  }

  const std::vector<std::string>& options::positional() const {
    return positional_;
  }

  
}
