#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "exception.hpp"
#include "predeclare.hpp"

namespace optspp {

  namespace detail {
    void apply_option_property(option& o) {
    }
    
    template <typename Arg>
    void apply_option_property(option& o, Arg property) {
      property(o);
    }
    
    template <typename Arg, typename... Args>
    void apply_option_property(option& o, Arg property, Args&&... args) {
      property(o);
      apply_option_property(o, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  std::shared_ptr<option> make_option(Args&&... properties) {
    auto o = std::make_shared<option>();
    std::cout << "Applying properties\n";
    detail::apply_option_property(*o, std::forward<Args>(properties)...);
    std::cout << "Applied properties\n";
    return o;
  }

  struct option : public std::enable_shared_from_this<option> {
    option() {
      std::cout << "option()\n";
    }

    void swap(option& other) {
      std::cout << "Swapping\n";
      std::swap(parent_containers_, other.parent_containers_);
      std::swap(long_name_, other.long_name_);
      std::swap(long_name_synonyms_, other.long_name_synonyms_);
      std::swap(short_name_, other.short_name_);
      std::swap(short_name_synonyms_, other.short_name_synonyms_);
      std::swap(valid_values_, other.valid_values_);
      std::swap(mutually_exclusive_values_, other.mutually_exclusive_values_);
      std::swap(default_values_, other.default_values_);
      std::swap(implicit_values_, other.implicit_values_);
      std::swap(description_, other.description_);
    }

    template <typename... Args>
    option(Args&&... args) {
      std::cout << "option(...)\n";
      auto o = make_option(std::forward<Args>(args)...);
      swap(*o);
      std::cout << "Created " << long_name_ << "\n";
    }

    const std::string& long_name() const {
      return long_name_;
    }
    
    void set_long_name(const std::string& long_name) {
      check_parents();
      long_name_ = long_name;
    }

    const std::vector<std::string> long_name_synonyms() const {
      return long_name_synonyms_;
    }
    
    void set_long_name_synonyms(const std::vector<std::string>& long_name_synonyms) {
      check_parents();
      long_name_synonyms_ = long_name_synonyms;
    }

    const char& short_name() const {
      return short_name_;
    }
    
    void set_short_name(const char& short_name) {
      check_parents();
      short_name_ = short_name;
    }

    const std::vector<char> short_name_synonyms() const {
      return short_name_synonyms_;
    }
    
    void set_short_name_synonyms(const std::vector<char>& short_name_synonyms) {
      check_parents();
      short_name_synonyms_ = short_name_synonyms;
    }

    const std::map<std::string, std::vector<std::string>> valid_values() const {
      return valid_values_;
    }

    void add_valid_value(const std::string& val, const std::vector<std::string>& synonyms) {
      valid_values_[val] = synonyms;
      check();
    }

    const std::vector<std::vector<std::string>>& mutually_exclusive_values() const {
      return mutually_exclusive_values_;
    }

    void add_mutually_exclusive_value(const std::vector<std::string>& vals) {
      mutually_exclusive_values_.push_back(vals);
      check();
    }

    const std::vector<std::string>& default_values() const {
      return default_values_;
    }

    void add_default_value(const std::string& val) {
      default_values_.push_back(val);
      check();      
    }
    
    const std::vector<std::string>& implicit_values() const {
      return implicit_values_;
    }

    void add_implicit_value(const std::string& val) {
      implicit_values_.push_back(val);
      check();      
    }
    
    const std::string& description() const {
      return description();
    }

    void set_description(const std::string& desc) {
      description_ = desc;
    }

    void add_parent_container(const std::shared_ptr<options>& os) {
      auto found = std::find(parent_containers_.begin(), parent_containers_.end(), os);
      if (found == parent_containers_.end()) parent_containers_.push_back(os);
    }
    
    void remove_parent_container(const std::shared_ptr<options>& os) {
      parent_containers_.erase(std::remove(parent_containers_.begin(), parent_containers_.end(), os),
                               parent_containers_.end());
    }

    bool is_valid_value(const std::string& v) const {
      if (valid_values_.size() == 0) return true;
      for (const auto& p : valid_values_) {
        if (p.first == v) return true;
        if (std::find(p.second.begin(), p.second.end(), v) != p.second.end()) return true;
      }
      return false;
    }

    void check_parents() const;

    void check() const {
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

    template <typename Property>
    option& operator<<(const Property& property) {
      property(*this);
      return *this;
    }

  private:
    std::vector<std::shared_ptr<options>> parent_containers_;
    std::string long_name_;
    std::vector<std::string> long_name_synonyms_;
    char short_name_;
    std::vector<char> short_name_synonyms_;
    std::map<std::string, std::vector<std::string>> valid_values_;
    std::vector<std::vector<std::string>> mutually_exclusive_values_;
    std::vector<std::string> default_values_;
    std::vector<std::string> implicit_values_;
    std::string description_;
  };


  
}
