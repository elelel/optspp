#pragma once

#include "../exception.hpp"

namespace optspp {
  namespace scheme {
    arguments::arguments() {
      using namespace easytree;
      root_ = node(std::make_shared<attributes>(attributes::KIND::ROOT));
    }
    
    void arguments::adopt_pending() {
      using namespace easytree;
      std::vector<scheme::node_ptr> visited;
      while (true) {
        bool child_added{false};
        for (auto& c : breadth_first<attributes_ptr>(root_)) {
          if (std::find(visited.begin(), visited.end(), c) == visited.end()) {
            for (const auto& cc : (**c)->pending_) {
              c->add_child(node(cc));
              child_added = true;
            }
            visited.push_back(c);
          }
        }
        if (!child_added) break;
      }
    }

    void arguments::build() {
      if (!built_) {
        built_ = true;
        adopt_pending();
        validate_scheme();
      } else {
        throw std::runtime_error("Arguments scheme is already built");
      }
    }
    
    const std::string& arguments::main_value(const node_ptr& arg_node,
                                             const std::string& value_str) const {
      using namespace easytree;
      for (const auto& c : arg_node->children()) {
        auto found = std::find((**c)->known_values_.begin(), (**c)->known_values_.end(), value_str);
        if (found != (**c)->known_values_.end()) return (**c)->known_values_[0];
      }
      throw exception::value_not_found(value_str);
    }
    
    void arguments::validate_scheme() {
      using namespace easytree;
      for (auto it1 = breadth_first<attributes_ptr>(root_).begin();
           it1 != breadth_first<attributes_ptr>(root_).end(); ++it1) {
        auto& n = *it1;
        // Validate same level entities
        std::vector<std::string> long_names;
        std::vector<char> short_names;
        std::vector<std::string> values;
        for (const auto& c : n->children()) {
          if ((**c)->kind_ == attributes::KIND::NAME) {
            for (const auto& x : (**c)->long_names())
              if (x != "") {
                if (std::find(long_names.begin(), long_names.end(), x) == long_names.end())
                  long_names.push_back(x);
                else
                  throw exception::name_conflict(x);
              }
            for (const auto& x : (**c)->short_names())
              if (x != 0) {
                if (std::find(short_names.begin(), short_names.end(), x) == short_names.end())
                  short_names.push_back(x);
                else
                  throw exception::name_conflict(x);
              }
          }
          if ((**c)->kind_ == attributes::KIND::VALUE) {
            for (const auto& x : (**c)->known_values()) {
              if (std::find(values.begin(), values.end(), x) == values.end())
                values.push_back(x);
              else
                throw exception::value_conflict(x);
            }
          }
        }
        // Validate vertical uniqueness
        for (auto it2 = it1.depth_first_iterator();
             it2 != depth_first<attributes_ptr>(root_).end(); ++it2) {
          auto& o = *it2;
          if ((it1 != it2) &&
              ((**o)->kind_ == attributes::KIND::NAME) &&
              ((**n)->kind_ == attributes::KIND::NAME)) {
            for (const auto& lhs : (**n)->long_names()) {
              for (const auto& rhs : (**o)->long_names()) {
                if ((lhs != "") && (lhs == rhs))
                  throw exception::name_conflict(lhs);
              }
            }
            for (const auto& lhs : (**n)->short_names()) {
              for (const auto& rhs : (**o)->short_names()) {
                if ((lhs != 0) && (lhs == rhs))
                  throw exception::name_conflict(lhs);
              }
            }
          }
        }
      }
    }
    
    const scheme::node_ptr& arguments::root() const {
      return root_;
    }


    /*
    std::vector<std::string> arguments::to_main_value(const scheme::node_ptr& n,
                                                      const std::string& s) const {
    }

    std::vector<std::string> arguments::default_values(const scheme::node_ptr& n) const {
      for (const auto& s : default_values_) rslt.push_back(to_main_value(s));
      return rslt;
    }
  
    std::vector<std::string> attributes::implicit_values(const scheme::node_ptr& n) const {
      std::vector<std::string> rslt;
      for (const auto& s : implicit_values_) rslt.push_back(to_main_value(s));
      return rslt;
    }
    */
    
    
  }

}
  
