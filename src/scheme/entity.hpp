#pragma once

#include "../scheme.hpp"

namespace optspp {
  namespace scheme {
    entity::entity(const entity& other) :
      kind_(other.kind_),
      siblings_group_(other.siblings_group_),
      color_(other.color_),
      description_(other.description_),
      long_names_(other.long_names_),
      short_names_(other.short_names_),
      max_count_(other.max_count_),
      min_count_(other.min_count_),
      is_positional_(other.is_positional_),
      known_values_(other.known_values_),
      implicit_values_(other.implicit_values_),
      default_values_(other.default_values_),
      any_value_(other.any_value_) {
      for (const auto& p : other.pending_) {
        auto c = std::make_shared<entity>(*p);
        pending_.push_back(c);
      }
    }
    
    entity::entity(const KIND kind) :
      kind_(kind) {
    }

    void entity::set_siblings_group(const SIBLINGS_GROUP group) {
      siblings_group_ = group;
    }

    void entity::set_positional() {
      is_positional_ = true;
    }

    void entity::set_named() {
      is_positional_ = false;
    }

    void entity::set_known_value(const std::vector<std::string>& vs) {
      known_values_ = vs;
      any_value_ = false;
    }

    void entity::set_known_value(any) {
      known_values_ = optional<std::vector<std::string>>();
      any_value_ = true;
    }

    template <>
    void entity::apply_property(name p) {
      if (kind_ == KIND::ARGUMENT) {
        if (p.long_names) {
          if (!long_names_) long_names_ = std::vector<std::string>{};
          auto& lhs = *long_names_;
          auto& rhs = *p.long_names;
          lhs.push_back(rhs[0]);
          for (auto it = rhs.begin() + 1; it != rhs.end(); ++it) 
            if (std::find(lhs.begin(), lhs.end(), *it) == lhs.end()) lhs.push_back(*it);
        }
        if (p.short_names) {
          if (!short_names_) short_names_ = std::vector<char>{};
          auto& lhs = *short_names_;
          auto& rhs = *p.short_names;
          lhs.push_back(rhs[0]);
          for (auto it = rhs.begin() + 1; it != rhs.end(); ++it) 
            if (std::find(lhs.begin(), lhs.end(), *it) == lhs.end()) lhs.push_back(*it);
        }        
      } else {
        throw scheme_error("Can't assign name to a non-argument entity");
      }
    }

    template <>
    void entity::apply_property(optspp::default_values p) {
      if (kind_ == KIND::ARGUMENT) {
        if (p.values) {
          if (!default_values_) default_values_ = std::vector<std::string>{};
          auto& lhs = *default_values_;
          for (const auto& s : *p.values) {
            lhs.push_back(s);
          }
        }
      }
    }

    template <>
    void entity::apply_property(optspp::implicit_values p) {
      if (kind_ == KIND::ARGUMENT) {
        if (p.values) {
          if (!implicit_values_) implicit_values_ = std::vector<std::string>{};
          auto& lhs = *implicit_values_;
          for (const auto& s : *p.values) {
            lhs.push_back(s);
          }
        }
      }
    }
    
    template <>
    void entity::apply_property(description p) {
      if (p.text) {
        description_ = p.text;
      }
    }

    template <>
    void entity::apply_property(::optspp::min_count p) {
      if (p.count) {
        min_count_ = p.count;
      }
    }

    template <>
    void entity::apply_property(::optspp::max_count p) {
      if (p.count) {
        max_count_ = p.count;
      }
    }

    bool entity::name_matches(const std::string& s) const {
      if (long_names_) {
        auto& long_names = *long_names_;
        return std::find(long_names.begin(), long_names.end(), s) != long_names.end();
      }
      return false;
    }
    
    bool entity::name_matches(const char& c) const {
      if (short_names_) {
        auto& short_names = *short_names_;
        return std::find(short_names.begin(), short_names.end(), c) != short_names.end();
      }
      return false;
    }

    bool entity::value_matches(const std::string& s) const {
      if (kind_ == KIND::VALUE) {
        if (any_value_ && *any_value_) return true;
        if (known_values_) {
          auto found = std::find_if((*known_values_).begin(), (*known_values_).end(), [&s] (const std::string& val) {
              return val == s;
            });
          if (found != (*known_values_).end()) return true;
        }
      }
      return false;
    }

    auto entity::kind() const -> KIND {
      return kind_;
    }

    auto entity::siblings_group() const -> SIBLINGS_GROUP {
      return siblings_group_;
    }

    auto entity::color() const -> COLOR {
      return color_;
    }

    const std::vector<entity_ptr>& entity::children() const {
      return pending_;
    }
    
    const optional<std::vector<std::string>>& entity::long_names() const {
      return long_names_;
    }

    const optional<std::vector<char>>& entity::short_names() const {
      return short_names_;
    }

    const optional<bool>& entity::is_positional() const {
      return is_positional_;
    }

    const optional<bool>& entity::is_any_value() const {
      return any_value_;
    }
    
    const optional<std::vector<std::string>>& entity::known_values() const {
      return known_values_;
    }

    const optional<std::vector<std::string>>& entity::implicit_values() const {
      return implicit_values_;
    }

    const optional<std::vector<std::string>>& entity::default_values() const {
      return default_values_;
    }
    
    const optional<size_t>& entity::max_count() const {
      return max_count_;
    }
    
    const optional<size_t>& entity::min_count() const {
      return min_count_;
    }

    std::string entity::all_names_to_string() const {
      std::string rslt;
      if (kind_ == KIND::ARGUMENT) {
        bool need_comma{false};
        if (long_names_) {
          for (const auto& n : *long_names_) {
            if (need_comma) rslt += "/";
            rslt += n;
            need_comma = true;
          }
        }
        if (short_names_) {
          for (const auto& n : *short_names_) {
            if (need_comma) rslt += "/";
            rslt += n;
            need_comma = true;
          }
        }
      }
      return rslt;
    }
    
  }
}
