#pragma once

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "exception.hpp"
#include "declaration.hpp"

namespace optspp {
  template <typename... Args>
  std::shared_ptr<option> make_option(Args&&... properties) {
    auto o = std::make_shared<option>(std::forward<Args>(properties)...);
    return o;
  }
  
  option::option() {
  }

  option::~option() {
  }

  template <typename... Args>
  option::option(Args&&... args) {
    reduce(std::forward<Args>(args)...);
  }
  
  void option::reduce(const option& o) {
    *this += o;
  }
    
  template <typename... Args>
  void option::reduce(const option& o, Args&&... args) {
    *this += o;
    reduce(std::forward<Args>(args)...);
  }
  
  option& option::operator+=(const option& other) {
    if (other.long_name_ != "")
      set_long_name(other.long_name_);
    if (other.long_name_synonyms_.size() != 0)
      for (const auto& x : other.long_name_synonyms_) 
        add_long_name_synonym(x);
    if (other.short_name_ != 0)
      set_short_name(other.short_name_);
    if (other.short_name_synonyms_.size() != 0) 
      for (const auto& x : other.short_name_synonyms_)
        add_short_name_synonym(x);

    if (other.valid_values_.size() != 0)
      for (const auto& x : other.valid_values_)
        add_valid_value(x.first, x.second);

    if (other.mutually_exclusive_values_.size() != 0)
      for (const auto& x : other.mutually_exclusive_values_)
        add_mutually_exclusive_values(x);

    if (other.default_values_.size() != 0)
      for (const auto& x : other.default_values_)
        add_default_value(x);
    if (other.implicit_values_.size() != 0)
      for (const auto& x : other.implicit_values_)
        add_implicit_value(x);

    if (other.description_ != "")
      set_description(other.description_);
    if (other.max_count_ != std::numeric_limits<size_t>::max())
      set_max_count(other.max_count_);
    if (other.min_count_ != std::numeric_limits<size_t>::min())
      set_min_count(other.min_count_);
    
    add_parent_containers(other.parent_containers_);
    
    return *this;
  }

  option& option::operator|(const option& other) {
    *this += other;
    return *this;
  }

  option& option::operator<<(const option& other) {
    *this += other;
    return *this;
  }
  
  option& option::set_long_name(const std::string& long_name) {
    long_name_ = long_name;
    invoke_parent_container_checks();
    return *this;
  }

  option& option::add_long_name_synonym(const std::string& synonym) {
    if (std::find(long_name_synonyms_.begin(),
                  long_name_synonyms_.end(), synonym) == long_name_synonyms_.end()) {
      long_name_synonyms_.push_back(synonym);
      invoke_parent_container_checks();
    }
  }

  option& option::set_short_name(const char& short_name) {
    if (short_name != 0) {
      short_name_ = short_name;
      invoke_parent_container_checks();
    }
    return *this;
  }

  option& option::add_short_name_synonym(const char& synonym) {
    if ((synonym != 0) &&
        std::find(short_name_synonyms_.begin(),
                  short_name_synonyms_.end(), synonym) == short_name_synonyms_.end()) {
      short_name_synonyms_.push_back(synonym);
      invoke_parent_container_checks();
    }
    return *this;
  }

  option& option::add_valid_value(const std::string& main_value, const std::vector<std::string>& synonyms) {
    valid_values_[main_value] = synonyms;
    check();
    return *this;
  }

  option& option::add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_value) {
    if (mutually_exclusive_value.size() > 0) {
      mutually_exclusive_values_.push_back(mutually_exclusive_value);
      check();
    }
    return *this;
  }

  option& option::add_default_value(const std::string& default_value) {
    default_values_.push_back(default_value);
    check();
    return *this;
  }

  option& option::add_implicit_value(const std::string& implicit_value) {
    implicit_values_.push_back(implicit_value);
    check();
    return *this;
  }
  
  option& option::set_description(const std::string& description) {
    if (description != "") {
      description_ = description;
      invoke_parent_container_checks();
    }
    return *this;
  }

  option& option::set_max_count(const size_t& max_count) {
    if (max_count != std::numeric_limits<size_t>::max()) {
      max_count_ = max_count;
      invoke_parent_container_checks();
    }
    return *this;
  }

  option& option::set_min_count(const size_t& min_count) {
    if (min_count != std::numeric_limits<size_t>::min()) {
      min_count_ = min_count;
      invoke_parent_container_checks();
    }
    return *this;
  }
  
  void option::check() const {
    // Checks below are valid only if valid_values_ are defined
    if (valid_values_.size() == 0) return;
      
    bool default_value_found{false};
    bool implicit_value_found{false};
    std::vector<std::set<std::string>> mes_to_find;
    for (const auto& me : mutually_exclusive_values_) {
      mes_to_find.push_back(std::set<std::string>(me.begin(), me.end()));
    }
    std::set<std::string> d_to_find(default_values_.begin(),
                                    default_values_.end());
    std::set<std::string> i_to_find(implicit_values_.begin(),
                                    implicit_values_.end());

    for (auto it1 = valid_values_.begin(); it1 != valid_values_.end(); ++it1) {
      std::vector<std::string> lhs{it1->second};
      lhs.push_back(it1->first);

      for (auto& me_to_find : mes_to_find) {
        if (me_to_find.find(it1->first) != me_to_find.end())
          me_to_find.erase(it1->first);
      }
      if (d_to_find.find(it1->first) != d_to_find.end())
        d_to_find.erase(it1->first);
      if (i_to_find.find(it1->first) != i_to_find.end())
        i_to_find.erase(it1->first);
        
      for (auto it2 = valid_values_.begin(); it2 != valid_values_.end(); ++it2) {
        if (it1 != it2) {
          std::vector<std::string> rhs{it2->second};
          rhs.push_back(it2->first);
          for (const auto& a : lhs) {
            for (const auto& b : rhs) {
              if (a == b) throw exception::non_distinct_valid_values(a);
            }
          }
        }
      }
    }

    if (d_to_find.size() > 0)
      throw exception::invalid_default_value(d_to_find);
    if (i_to_find.size() > 0)
      throw exception::invalid_implicit_value(i_to_find);

    // // Do not require mutually exclusive values to be listed in valid values yet
    // bool mes_ok{true};
    // for (const auto& me_to_find : mes_to_find) {
    //   if (me_to_find.size() > 0) mes_ok = false;
    // }
    // if (!mes_ok)
    //   throw exception::invalid_mutually_exclusive_value(mes_to_find);

  }

  option& option::add_parent_container(const std::shared_ptr<options>& os) {
    auto found = std::find(parent_containers_.begin(), parent_containers_.end(), os);
    if (found == parent_containers_.end()) parent_containers_.push_back(os);
    return *this;
  }
    
  option& option::remove_parent_container(const std::shared_ptr<options>& os) {
    parent_containers_.erase(std::remove(parent_containers_.begin(), parent_containers_.end(), os),
                             parent_containers_.end());
    return *this;
  }

  option& option::add_parent_containers(const std::vector<std::shared_ptr<options>>& oss) {
    for (const auto& os : oss) add_parent_container(os);
    return *this;
  }
    
  option& option::remove_parent_containers(const std::vector<std::shared_ptr<options>>& oss) {
    for (const auto& os : oss) remove_parent_container(os);
    return *this;
  }

  void option::invoke_parent_container_checks() const {
    for (const auto& os : parent_containers_) {
      os->check_schema();
    }
  }

  // --------- Read member accessors ---------
  std::string option::all_names_to_string() const {
    std::string rslt;
    bool need_comma{false};
    if (long_name_ != "") {
      if (need_comma) rslt += "/";
      rslt += long_name_;
      need_comma = true;
    }
    for (const auto& n : long_name_synonyms_) {
      if (need_comma) rslt += "/";
      rslt += n;
      need_comma = true;
    }
    if (short_name_ != 0) {
      if (need_comma) rslt += "/";
      rslt += short_name_;
      need_comma = true;
    }
    for (const auto& n : short_name_synonyms_) {
      if (need_comma) rslt += "/";
      rslt += n;
      need_comma = true;
    }
    return rslt;
  }
  
  const std::string& option::long_name() const {
    return long_name_;
  }

  const std::vector<std::string>& option::long_name_synonyms() const {
    return long_name_synonyms_;
  }

  const char& option::short_name() const {
    return short_name_;
  }

  const std::vector<char>& option::short_name_synonyms() const {
    return short_name_synonyms_;
  }
  
  std::string option::main_value(const std::string& v) const {
    if (valid_values_.size() == 0) return v;
    for (const auto& p : valid_values_) {
      if (p.first == v) return p.first;
      auto found = std::find(p.second.begin(), p.second.end(), v);
      if (found != p.second.end()) return p.first;
    }
    throw std::runtime_error("Unknown value requested. This should never have happened.");
  }
  
  std::vector<std::string> option::default_values() const {
    std::vector<std::string> rslt;
    std::transform(default_values_.begin(), default_values_.end(), std::back_inserter(rslt),
                   [this] (const std::string& s) {
                     return main_value(s);
                   });
    return rslt;
  }
  
  std::vector<std::string> option::implicit_values() const {
    std::vector<std::string> rslt;
    std::transform(implicit_values_.begin(), implicit_values_.end(), std::back_inserter(rslt),
                   [this] (const std::string& s) {
                     return main_value(s);
                   });
    return rslt;
  }

  const size_t& option::max_count() const {
    return max_count_;
  }

  const size_t& option::min_count() const {
    return min_count_;
  }

  bool option::is_valid_value(const std::string& v) const {
    if (valid_values_.size() == 0) return true;
    for (const auto& p : valid_values_) {
      if (p.first == v) return true;
      if (std::find(p.second.begin(), p.second.end(), v) != p.second.end()) return true;
    }
    return false;
  }

  const std::map<std::string, std::vector<std::string>>& option::valid_values() const {
    return valid_values_;
  }
  
  const std::vector<std::vector<std::string>>& option::mutually_exclusive_values() const {
    return mutually_exclusive_values_;
  }
  
  const std::string& option::description() const {
    return description_;
  }
}
