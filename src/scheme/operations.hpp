#pragma once

#include "../scheme.hpp"

namespace optspp {
  // Create positional argument definition
  template <typename Property>
  void positional_(scheme::entity_ptr& e, Property p) {
    e->apply_property(p);
  }

  template <typename Property, typename... Properties>
  void positional_(scheme::entity_ptr& e, Property p, Properties&&... ps) {
    e->apply_property(p);
    positional_(e, std::forward<Properties>(ps)...);
  }
  
  template <typename... Properties>
  scheme::entity_ptr positional(Properties&&... ps) {
    scheme::entity_ptr e = std::make_shared<scheme::entity>(scheme::entity::KIND::ARGUMENT);
    e->set_positional();
    positional_(e, std::forward<Properties>(ps)...);
    return e;
  }

  // Create named argument definition
  template <typename Property>
  void named_(scheme::entity_ptr& e, Property p) {
    e->apply_property(p);
  }

  template <typename Property, typename... Properties>
  void named_(scheme::entity_ptr& e, Property p, Properties&&... ps) {
    e->apply_property(p);
    named_(e, std::forward<Properties>(ps)...);
  }
  
  template <typename... Properties>
  scheme::entity_ptr named(Properties&&... ps) {
    scheme::entity_ptr e = std::make_shared<scheme::entity>(scheme::entity::KIND::ARGUMENT);
    e->set_named();
    named_(e, std::forward<Properties>(ps)...);
    return e;
  }

  scheme::entity_ptr value(const std::string& val) {
    scheme::entity_ptr e = std::make_shared<scheme::entity>(scheme::entity::KIND::VALUE);
    e->set_known_value(std::vector<std::string>{val});
    return e;
  }
  
  scheme::entity_ptr value(const std::string& val, std::initializer_list<std::string> synonyms) {
    scheme::entity_ptr e = std::make_shared<scheme::entity>(scheme::entity::KIND::VALUE);
    std::vector<std::string> vs{val};
    for (const auto& s : synonyms) {
      vs.push_back(s);
    }
    e->set_known_value(vs);
    return e;
  }

  scheme::entity_ptr value(any) {
    scheme::entity_ptr e = std::make_shared<scheme::entity>(scheme::entity::KIND::VALUE);
    e->set_known_value(any());
    return e;
  }

  // TODO COPY by value to children???
  
  // Assign value definition to argument definition and argument definition to value definition; the children are xor-compatible
  std::shared_ptr<scheme::entity> operator<<(std::shared_ptr<scheme::entity> lhs, const std::shared_ptr<scheme::entity>& rhs) {
    if (((lhs->kind_ == scheme::entity::KIND::ARGUMENT) && (rhs->kind_ == scheme::entity::KIND::VALUE)) ||
        ((rhs->kind_ == scheme::entity::KIND::ARGUMENT) && (lhs->kind_ == scheme::entity::KIND::VALUE))) {
      if (std::find(lhs->pending_.begin(), lhs->pending_.end(), rhs) == lhs->pending_.end()) {
        auto e = std::make_shared<scheme::entity>(*rhs);
        e->siblings_group_ = scheme::SIBLINGS_GROUP::XOR;
        lhs->pending_.push_back(e);
      }
    } else {
      throw scheme_error("Scheme entity types are incompatible for combination");
    }
    return lhs;
  }
  
  // Assign value definition to argument definition and argument definition to value definition; the children are or-compatible
  std::shared_ptr<scheme::entity> operator|(std::shared_ptr<scheme::entity> lhs, const std::shared_ptr<scheme::entity>& rhs) {
    if (((lhs->kind_ == scheme::entity::KIND::ARGUMENT) && (rhs->kind_ == scheme::entity::KIND::VALUE)) ||
        ((rhs->kind_ == scheme::entity::KIND::ARGUMENT) && (lhs->kind_ == scheme::entity::KIND::VALUE))) {
      if (std::find(lhs->pending_.begin(), lhs->pending_.end(), rhs) == lhs->pending_.end()) {
        auto e = std::make_shared<scheme::entity>(*rhs);
        e->siblings_group_ = scheme::SIBLINGS_GROUP::OR;
        lhs->pending_.push_back(e);
      }
    } else {
      throw scheme_error("Scheme entity types are incompatible for combination: " +
                               std::to_string((int)lhs->kind_) + " and " +
                               std::to_string((int)rhs->kind_));
    }
    return lhs;
  }
  

  // Assign argument definition to scheme definition; the children are or-compatible
  scheme::definition& operator<<(scheme::definition& lhs, const std::shared_ptr<scheme::entity>& rhs) {
    if (rhs->kind_ == scheme::entity::KIND::ARGUMENT) {
      if (std::find(lhs.root_->pending_.begin(), lhs.root_->pending_.end(), rhs) == lhs.root_->pending_.end()) {
        auto e = std::make_shared<scheme::entity>(*rhs);
        e->siblings_group_ = scheme::SIBLINGS_GROUP::XOR;
        lhs.root_->pending_.push_back(e);
      }
    } else {
      throw scheme_error("Scheme entity type are incompatible for setting as scheme root element: " +
                               std::to_string((int)rhs->kind_));
    }
    return lhs;
  }
  
  // Assign argument definition to scheme definition; the children are or-compatible
  scheme::definition& operator|(scheme::definition& lhs, const std::shared_ptr<scheme::entity>& rhs) {
    if (rhs->kind_ == scheme::entity::KIND::ARGUMENT) {
      if (std::find(lhs.root_->pending_.begin(), lhs.root_->pending_.end(), rhs) == lhs.root_->pending_.end()) {
        auto e = std::make_shared<scheme::entity>(*rhs);
        e->siblings_group_ = scheme::SIBLINGS_GROUP::OR;
        lhs.root_->pending_.push_back(e);
      }
    } else {
      throw scheme_error("Scheme entity type are incompatible for setting as scheme root element: " +
                               std::to_string((int)rhs->kind_));
    }
    return lhs;
  }
  
  
}
