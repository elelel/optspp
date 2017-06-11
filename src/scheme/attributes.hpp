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
        if (other.kind_ == KIND::NAME) {
          if (other.description_ != "") set_description(other.description_);
          std::cout << "Aggregating: " << long_names_[0] << " += " << other.long_names_[0] << " was " << is_positional_ << " will be " << other.is_positional_ << "\n";
          is_positional_ = other.is_positional_;
          allow_arbitrary_values_ = other.allow_arbitrary_values_;
          if (other.long_names_[0] != "")
            set_long_name(other.long_names_[0]);
          if (other.long_names_.size() > 1)
            for (auto it = other.long_names_.begin() + 1; it != other.long_names_.end(); ++it) 
              add_long_name_synonym(*it);
          if (other.short_names_[0] != 0)
            set_short_name(other.short_names_[0]);
          if (other.short_names_.size() > 1) 
            for (auto it = other.short_names_.begin() + 1; it != other.short_names_.end(); ++it) 
              add_short_name_synonym(*it);
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
        }
        break;
      case KIND::VALUE :
        if (other.kind_ == KIND::VALUE) {
          if (other.description_ != "")
            set_description(other.description_);
          if (other.known_values_[0] != "")
            set_main_value(other.known_values_[0]);
          if (other.known_values_.size() > 1) 
            for (auto it = other.known_values_.begin() + 1; it != other.known_values_.end(); ++it) 
              add_value_synonym(*it);        
          if (other.mutually_exclusive_values_.size() != 0)
            for (const auto& m : other.mutually_exclusive_values_)
              add_mutually_exclusive_values(m);
        }
        break;
      }
      return *this;
    }

    auto attributes::set_description(const std::string& desc) -> type& {
      description_ = desc;
      return *this;
    }

    auto attributes::set_long_name(const std::string& long_name) -> type& {
      long_names_[0] = long_name;
      long_names_.erase(std::remove(long_names_.begin() + 1, long_names_.end(), long_name),
                        long_names_.end());
      return *this;
    }
  
    auto attributes::add_long_name_synonym(const std::string& long_name_synonym) -> type& {
      if (std::find(long_names_.begin(),
                    long_names_.end(), long_name_synonym)
          == long_names_.end()) {
          long_names_.push_back(long_name_synonym);
        }
      return *this;
    }
  
    auto attributes::set_short_name(const char& short_name) -> type& {
      if (short_name != 0) {
        short_names_[0] = short_name;
      }
      return *this;
    }
  
    auto attributes::add_short_name_synonym(const char& short_name_synonym) -> type& {
      if ((short_name_synonym != 0) &&
          std::find(short_names_.begin(),
                    short_names_.end(), short_name_synonym)
          == short_names_.end()) {
          short_names_.push_back(short_name_synonym);
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
      for (const auto& n : long_names_) {
        if (need_comma) rslt += "/";
        rslt += n;
        need_comma = true;
      }
      for (const auto& n : short_names_) {
        if (need_comma) rslt += "/";
        rslt += n;
        need_comma = true;
      }
      return rslt;
    }
  
    const std::string& attributes::long_name() const {
      return long_names_[0];
    }

    const std::vector<std::string>& attributes::long_names() const {
      return long_names_;
    }

    const char& attributes::short_name() const {
      return short_names_[0];
    }

    const std::vector<char>& attributes::short_names() const {
      return short_names_;
    }

    const std::vector<std::string>& attributes::known_values() const {
      return known_values_;
    }

    const size_t& attributes::max_count() const {
      return max_count_;
    }

    const size_t& attributes::min_count() const {
      return min_count_;
    }

    bool attributes::is_positional() const {
      return is_positional_;
    }

    // Value-related
    
    auto attributes::set_main_value(const std::string& main_value) -> type& {
      known_values_[0] = main_value;
      known_values_.erase(std::remove(known_values_.begin() + 1, known_values_.end(), main_value),
                          known_values_.end());
      return *this;
    }

    auto attributes::add_value_synonym(const std::string& synonym) -> type& {
      if (std::find(known_values_.begin(),
                    known_values_.end(), synonym)
          == known_values_.end()) {
          known_values_.push_back(synonym);
        }
      return *this;
    }

    auto attributes::add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_value) -> type& {
      if (mutually_exclusive_value.size() > 0) {
        mutually_exclusive_values_.push_back(mutually_exclusive_value);
      }
      return *this;
    }

    const std::string& attributes::main_value() const {
      return known_values_[0];
    }

    const std::vector<std::string>& attributes::default_values() const {
      return default_values_;
    }
    
    const std::vector<std::string>& attributes::implicit_values() const {
      return implicit_values_;
    }
    
  }
}
