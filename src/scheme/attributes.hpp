#pragma once

#include "../exception.hpp"

namespace optspp {
  namespace scheme {
    attributes::attributes(const KIND kind) :
      kind_(kind) {
    }

    auto attributes::kind() const -> KIND {
      return kind_;
    }
        
    auto attributes::operator+=(const type& other) -> type& {
      switch (kind_) {
      case KIND::DESCRIPTION :
        if (other.description_ != "")
          set_description(other.description_);
        break;
      case KIND::NAME :
        if (other.description_ != "")
          set_description(other.description_);
        is_positional_ = other.is_positional_;
        allow_arbitrary_values_ = other.allow_arbitrary_values_;
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

        if (other.default_values_.size() != 0)
          for (const auto& x : other.default_values_)
            add_default_value(x);
        if (other.implicit_values_.size() != 0)
          for (const auto& x : other.implicit_values_)
            add_implicit_value(x);
        if (other.max_count_ != std::numeric_limits<size_t>::max())
          set_max_count(other.max_count_);
        if (other.min_count_ != std::numeric_limits<size_t>::min())
          set_min_count(other.min_count_);
        break;
      case KIND::VALUE :
        if (other.description_ != "")
          set_description(other.description_);
        if (other.main_value_ != "")
          set_main_value(other.main_value_);
        if (other.value_synonyms_.size() != 0)
          for (const auto& s : other.value_synonyms_)
            add_value_synonym(s);
        if (other.mutually_exclusive_values_.size() != 0)
          for (const auto& m : other.mutually_exclusive_values_)
            add_mutually_exclusive_values(m);
        break;
      }
      return *this;
    }

    auto attributes::set_description(const std::string& desc) -> type& {
      description_ = desc;
      return *this;
    }

    auto attributes::set_long_name(const std::string& long_name) -> type& {
      long_name_ = long_name;
      return *this;
    }
  
    auto attributes::add_long_name_synonym(const std::string& long_name_synonym) -> type& {
      if (std::find(long_name_synonyms_.begin(),
                    long_name_synonyms_.end(), long_name_synonym)
          == long_name_synonyms_.end()) {
          long_name_synonyms_.push_back(long_name_synonym);
        }
      return *this;
    }
  
    auto attributes::set_short_name(const char& short_name) -> type& {
      if (short_name != 0) {
          short_name_ = short_name;
        }
      return *this;
    }
  
    auto attributes::add_short_name_synonym(const char& short_name_synonym) -> type& {
      if ((short_name_synonym != 0) &&
          std::find(short_name_synonyms_.begin(),
                    short_name_synonyms_.end(), short_name_synonym)
          == short_name_synonyms_.end()) {
          short_name_synonyms_.push_back(short_name_synonym);
        }
      return *this;
    }

    auto attributes::add_default_value(const std::string& default_value) -> type& {
      default_values_.push_back(default_value);
      return *this;
    }
    
    auto attributes::add_implicit_value(const std::string& implicit_value) -> type& {
      implicit_values_.push_back(implicit_value);
      return *this;
    }
    
    auto attributes::set_max_count(const size_t& max_count) -> type& {
      if (max_count != std::numeric_limits<size_t>::max()) {
          max_count_ = max_count;
        }
      return *this;
    }
    
    auto attributes::set_min_count(const size_t& min_count) -> type& {
      if (min_count != std::numeric_limits<size_t>::min()) {
          min_count_ = min_count;
        }
      return *this;
    }

    auto attributes::set_is_positional(const bool& is_positional) -> type& {
      is_positional_ = is_positional;
      return *this;
    }

    auto attributes::set_allow_arbitrary_values(const bool& allow) -> type& {
      allow_arbitrary_values_ = allow;
      return *this;
    }
    
    // Read accessors
    const std::string& attributes::description() const {
      return description_;
    }
      
    std::string attributes::all_names_to_string() const {
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
  
    const std::string& attributes::long_name() const {
      return long_name_;
    }

    const std::vector<std::string>& attributes::long_name_synonyms() const {
      return long_name_synonyms_;
    }

    const char& attributes::short_name() const {
      return short_name_;
    }

    const std::vector<char>& attributes::short_name_synonyms() const {
      return short_name_synonyms_;
    }

    std::vector<std::string> attributes::all_long_names() const {
      std::vector<std::string> rslt;
      rslt.push_back(long_name_);
      for (const auto& n : long_name_synonyms_) rslt.push_back(n);
      return rslt;
    }

    std::vector<char> attributes::all_short_names() const {
      std::vector<char> rslt;
      rslt.push_back(short_name_);
      for (const auto& n : short_name_synonyms_) rslt.push_back(n);
      return rslt;
    }

    std::vector<std::string> attributes::all_values() const {
      std::vector<std::string> rslt;
      rslt.push_back(main_value_);
      for (const auto& v : value_synonyms_) rslt.push_back(v);
      return rslt;
    }

    /*
    std::vector<std::string> attributes::default_values() const {
      std::vector<std::string> rslt;
      for (const auto& s : default_values_) rslt.push_back(to_main_value(s));
      return rslt;
    }
  
    std::vector<std::string> attributes::implicit_values() const {
      std::vector<std::string> rslt;
      for (const auto& s : implicit_values_) rslt.push_back(to_main_value(s));
      return rslt;
      }*/

    const size_t& attributes::max_count() const {
      return max_count_;
    }

    const size_t& attributes::min_count() const {
      return min_count_;
    }

    // Value-related
    
    auto attributes::set_main_value(const std::string& main_value) -> type& {
      value_synonyms_.erase(std::remove(value_synonyms_.begin(), value_synonyms_.end(), main_value),
                            value_synonyms_.end());
      main_value_ = main_value;
      return *this;
    }

    auto attributes::add_value_synonym(const std::string& synonym) -> type& {
      if ((synonym != main_value_) &&
          (std::find(value_synonyms_.begin(), value_synonyms_.end(), synonym) == value_synonyms_.end()))
        value_synonyms_.push_back(synonym);
      return *this;
    }

    auto attributes::add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_value) -> type& {
      if (mutually_exclusive_value.size() > 0) {
        mutually_exclusive_values_.push_back(mutually_exclusive_value);
      }
      return *this;
    }

    const std::string& attributes::main_value() const {
      return main_value_;
    }

    const std::vector<std::string>& attributes::value_synonyms() const {
      return value_synonyms_;
    }
  }
}
