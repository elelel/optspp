#pragma once

namespace optspp {
  namespace scheme {

    name::name() :
      node(KIND::NAME) {
    }

    name::~name() {
    }

    void name::validate() {
      // TODO: validate same level against self, then all children against self
      node::validate();
    }

    name& name::operator|(const optspp::description& other) {
      const auto& d = static_cast<const node*>(&other)->description();
      if (d != "") description_ = d;
      return *this;
    }
    
    name& name::operator|(const name& other) {
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

      if (other.description_ != "")
        set_description(other.description_);
      if (other.max_count_ != std::numeric_limits<size_t>::max())
        set_max_count(other.max_count_);
      if (other.min_count_ != std::numeric_limits<size_t>::min())
        set_min_count(other.min_count_);
    
      add_parents(other.parents_);
    
      return *this;
    }

    bool name::operator==(const std::string& n) const {
      if (long_name_ == n) return true;
      for (const auto& s : long_name_synonyms_)
        if (s == n) return true;
      return false;
    }

    bool name::operator==(const char& n) const {
      if (short_name_ == n) return true;
      for (const auto& s : short_name_synonyms_)
        if (s == n) return true;
      return false;
    }
    
    name& name::add_child_value(const std::shared_ptr<value>& n) {
      for (const auto& c : child_values_) {
        if (*c == n->main_value()) throw exception::argument_value_conflict(n->main_value());
        for (const auto& s : n->value_synonyms())
          if (*c == s) throw exception::argument_value_conflict(s);
      }
      return *this;
    }

    name& name::add_child_value(const value& n) {
      auto p = std::make_shared<value>(n);
      return add_child_value(n);
    }
    
    name& name::set_long_name(const std::string& long_name) {
      long_name_ = long_name;
      return *this;
    }
  
    name& name::add_long_name_synonym(const std::string& long_name_synonym) {
      if (std::find(long_name_synonyms_.begin(),
                    long_name_synonyms_.end(), long_name_synonym)
          == long_name_synonyms_.end()) {
        long_name_synonyms_.push_back(long_name_synonym);
      }
      return *this;
    }
  
    name& name::set_short_name(const char& short_name) {
      if (short_name != 0) {
        short_name_ = short_name;
      }
      return *this;
    }
  
    name& name::add_short_name_synonym(const char& short_name_synonym) {
      if ((short_name_synonym != 0) &&
          std::find(short_name_synonyms_.begin(),
                    short_name_synonyms_.end(), short_name_synonym)
          == short_name_synonyms_.end()) {
        short_name_synonyms_.push_back(short_name_synonym);
      }
      return *this;
    }

    name& name::add_default_value(const std::string& default_value) {
      default_values_.push_back(default_value);
      return *this;
    }
    
    name& name::add_implicit_value(const std::string& implicit_value) {
      implicit_values_.push_back(implicit_value);
      return *this;
    }
    
    name& name::set_max_count(const size_t& max_count) {
      if (max_count != std::numeric_limits<size_t>::max()) {
        max_count_ = max_count;
      }
      return *this;
    }
    
    name& name::set_min_count(const size_t& min_count) {
      if (min_count != std::numeric_limits<size_t>::min()) {
        min_count_ = min_count;
      }
      return *this;
    }

    name& name::set_is_positional(const bool& is_positional) {
      is_positional_ = is_positional;
      return *this;
    }

    name& name::set_allow_arbitrary_values(const bool& allow) {
      allow_arbitrary_values_ = allow;
      return *this;
    }
    
    // Read accessors
    std::string name::all_names_to_string() const {
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
  
    const std::string& name::long_name() const {
      return long_name_;
    }

    const std::vector<std::string>& name::long_name_synonyms() const {
      return long_name_synonyms_;
    }

    const char& name::short_name() const {
      return short_name_;
    }

    const std::vector<char>& name::short_name_synonyms() const {
      return short_name_synonyms_;
    }

    const std::string& name::to_main_value(const std::string& s) const {
      // Convert to main value, if we have one
      auto found = std::find_if(child_values_.begin(), child_values_.end(),
                                [&s] (const std::shared_ptr<value>& v) {
                                  return *v == s;
                                });
      if (found != child_values_.end()) return (*found)->main_value();
      return s;
    }
  
    std::vector<std::string> name::default_values() const {
      std::vector<std::string> rslt;
      for (const auto& s : default_values_) rslt.push_back(to_main_value(s));
      return rslt;
    }
  
    std::vector<std::string> name::implicit_values() const {
      std::vector<std::string> rslt;
      for (const auto& s : implicit_values_) rslt.push_back(to_main_value(s));
      return rslt;
    }

    const size_t& name::max_count() const {
      return max_count_;
    }

    const size_t& name::min_count() const {
      return min_count_;
    }
  
    
  }
  
}
