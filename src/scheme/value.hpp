#pragma once

namespace optspp {
  namespace scheme {
    
    value::value() :
      node(KIND::VALUE) {
    }

    value::~value() {
    }

    void value::validate() const {
      node::validate();
    }

    value& value::operator|(const optspp::description& other) {
      const auto& d = static_cast<const node*>(&other)->description();
      if (d != "") description_ = d;
      return *this;
    }

    value& value::operator|(const value& other) {
      if (other.main_value_ != "")
        set_main_value(other.main_value_);
      if (other.value_synonyms_.size() != 0)
        for (const auto& s : other.value_synonyms_)
          add_value_synonym(s);
      if (other.mutually_exclusive_values_.size() != 0)
        for (const auto& m : other.mutually_exclusive_values_)
          add_mutually_exclusive_values(m);
      return *this;
    }

    value& value::operator<<(const name& other) {
      add_child_name(other);
      return *this;
    }
    
    value& value::operator<<(const std::shared_ptr<name>& other) {
      add_child_name(other);
      return *this;
    }
    
    value& value::set_main_value(const std::string& main_value) {
      value_synonyms_.erase(std::remove(value_synonyms_.begin(), value_synonyms_.end(), main_value),
                      value_synonyms_.end());
      main_value_ = main_value;
      return *this;
    }

    value& value::add_value_synonym(const std::string& synonym) {
      if ((synonym != main_value_) &&
          (std::find(value_synonyms_.begin(), value_synonyms_.end(), synonym) == value_synonyms_.end()))
        value_synonyms_.push_back(synonym);
      return *this;
    }

    value& value::add_mutually_exclusive_values(const std::vector<std::string>& mutually_exclusive_value) {
      if (mutually_exclusive_value.size() > 0) {
        mutually_exclusive_values_.push_back(mutually_exclusive_value);
      }
      return *this;
    }

    const std::string& value::main_value() const {
      return main_value_;
    }

    const std::vector<std::string>& value::value_synonyms() const {
      return value_synonyms_;
    }
    
    bool value::operator==(const std::string& s) const {
      if (main_value_ == s) return true;
      for (const auto& syn : value_synonyms_) {
        if (s == syn) return true;
      }
      return false;
    }

  }
}
