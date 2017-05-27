#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "declaration.hpp"

namespace optspp {

  // Sets option's long name and synonyms
  struct long_name {
    long_name(const std::string& name) :
      name_(name) {
    }
    
    long_name(const std::string& name, std::initializer_list<std::string> synonyms) :
      name_(name),
      synonyms_(synonyms) {
    }

    void operator()(option& o) const {
      std::cout << "Property long_name\n";
      o.set_long_name(name_);
      if (synonyms_.size() > 0) {
        o.set_long_name_synonyms(synonyms_);
      }
    }
    
  private:
    std::string name_;
    std::vector<std::string> synonyms_;
  };

  // Sets option's short name and synonyms
  struct short_name {
    short_name(const char& name) :
      name_(name) {
    }

    short_name(const char& name, std::initializer_list<char> synonyms) :
      name_(name),
      synonyms_(synonyms) {
    }

    void operator()(option& o) const {
      std::cout << "Property short_name\n";
      o.set_short_name(name_);
      if (synonyms_.size() > 0) {
        o.set_short_name_synonyms(synonyms_);
      }
    }
    
  private:
    char name_;
    std::vector<char> synonyms_;
  };

  struct valid_value {
    valid_value(const std::string& value) :
      value_(value) {
    }
    
    valid_value(const std::string& value, std::initializer_list<std::string> synonyms) :
      value_(value),
      synonyms_(synonyms) {
    }

    void operator()(option& o) const {
      std::cout << "Property valid_value\n";
      o.add_valid_value(value_, synonyms_);
    }
    

  private:
    std::string value_;
    std::vector<std::string> synonyms_;
  };

  struct valid_values {
    valid_values(std::initializer_list<valid_value> vals) {
      for (const auto& v : vals) {
        values_.push_back(v);
      }
    }

    void operator()(option& o) const {
      std::cout << "Property valid_values\n";
      for (const auto& v : values_) {
        v.operator()(o);
      }
    }
  private:
    std::vector<valid_value> values_;
  };

  struct mutually_exclusive_value {
    mutually_exclusive_value(std::initializer_list<std::string> val) :
      val_(val) {
    }

    void operator()(option& o) const {
      std::cout << "Property mutually_exclusive_value\n";
      o.add_mutually_exclusive_value(val_);
    }
  private:
    std::vector<std::string> val_;
  };

  struct mutually_exclusive_values {
    mutually_exclusive_values(std::initializer_list<mutually_exclusive_value> vals) :
      vals_(vals) {
    }

    void operator()(option& o) const {
      for (const auto& v : vals_) {
        v.operator()(o);
      }
    }
  private:
    std::vector<mutually_exclusive_value> vals_;
  };

  // TODO: Add default_values
  struct default_value {
    default_value(const std::string& val) :
      val_(val) {
    }

    void operator()(option& o) const {
      std::cout << "Property default_value\n";
      o.add_default_value(val_);
    }
  private:
    std::string val_;
  };

  // TODO: Add implicit_values
  struct implicit_value {
    implicit_value(const std::string& val) :
      val_(val) {
    }

    void operator()(option& o) const {
      std::cout << "Property implicit_value\n";
      o.add_implicit_value(val_);
    }
  private:
    std::string val_;
  };

  struct description {
    description(const std::string& val) :
      val_(val) {
    }

    void operator()(option& o) const {
      std::cout << "Property description\n";
      o.set_description(val_);
    }
  private:
    std::string val_;
  };

  struct max_count {
    max_count(const size_t& n) :
      count_(n) {
    }

    void operator()(option& o) const {
      o.set_max_count(count_);
    }
  private:
    size_t count_;
  };

  struct min_count {
    min_count(const size_t& n) :
      count_(n) {
    }

    void operator()(option& o) const {
      o.set_min_count(count_);
    }
  private:
    size_t count_;
  };
}
