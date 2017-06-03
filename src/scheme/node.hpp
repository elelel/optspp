#pragma once

#include "../exception.hpp"

namespace optspp {
  namespace scheme {
    node::node(KIND node_kind) :
      node_kind_(node_kind) {
    }

    node::~node() {
    }
    
    void node::validate() const {
      for (const auto& p : parents_) {
        std::cout << "node:;validate()\n";
        switch (p->node_kind_) {
        case KIND::DESCRIPTOR:
          dynamic_cast<const descriptor*>(p)->validate();
          break;
        case KIND::NAME:
          dynamic_cast<const name*>(p)->validate();
          break;
        case KIND::VALUE:
          dynamic_cast<const value*>(p)->validate();
          break;
        }
      }
    }

    node& node::set_description(const std::string& description) {
      description_ = description;
      return *this;
    }
    
    const std::string& node::description() const {
      return description_;
    }

    std::vector<std::shared_ptr<name>>::const_iterator
    node::find_iterator(const std::string& long_name) const {
      return std::find_if(child_names_.begin(), child_names_.end(),
                          [&long_name] (const std::shared_ptr<name>& o) {
                            const auto& syns = o->long_name_synonyms();
                            return (o->long_name() == long_name) ||
                              (std::find(syns.begin(), syns.end(), long_name) != syns.end());
                          });
    }

    std::shared_ptr<name>
    node::find(const std::string& long_name) const {
      auto found = find_iterator(long_name);
      if (found != child_names_.end()) return *found;
      return nullptr;
    }

    std::vector<std::shared_ptr<name>>::const_iterator
    node::find_iterator(const char& short_name) const {
      return std::find_if(child_names_.begin(), child_names_.end(),
                          [&short_name] (const std::shared_ptr<name>& o) {
                            const auto& syns = o->short_name_synonyms();
                            return (o->short_name() == short_name) ||
                              (std::find(syns.begin(), syns.end(), short_name) != syns.end());
                          });
    }
    
    std::shared_ptr<name>
    node::find(const char& short_name) const {
      auto found = find_iterator(short_name);
      if (found != child_names_.end()) return *found;
      return nullptr;
    }
    
    node& node::add_parent(const node* parent) {
      auto found = std::find(parents_.begin(), parents_.end(), parent);
      if (found == parents_.end()) parents_.push_back(parent);
      return *this;
    }
    
    node& node::add_parents(const std::vector<const node*>& parents) {
      for (const auto& parent : parents) add_parent(parent);
      return *this;
    }
    
    node& node::remove_parent(const node* parent) {
      parents_.erase(std::remove(parents_.begin(), parents_.end(), parent),
                     parents_.end());
      return *this;
    }
    
    node& node::remove_parents(const std::vector<const node*>& parents) {
      for (const auto& parent : parents) remove_parent(parent);
      return *this;
    }

    node& node::add_child_name(const std::shared_ptr<name>& n) {
      for (const auto& c : child_names_) {
        if (*c == n->long_name()) throw exception::argument_name_conflict(n->long_name());
        for (const auto& s : n->long_name_synonyms())
          if (*c == s) throw exception::argument_name_conflict(s);
        if (*c == n->short_name()) throw exception::argument_name_conflict(n->short_name());
        for (const auto& s : n->short_name_synonyms())
          if (*c == s) throw exception::argument_name_conflict(s);
      }
      child_names_.push_back(n);
      n->add_parent(this);
      return *this;
    }

    node& node::add_child_name(const name& n) {
      auto p = std::make_shared<name>(n);
      return add_child_name(p);
    }
  }

}
    
