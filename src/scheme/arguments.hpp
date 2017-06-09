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
      std::vector<easytree::tree::node<std::shared_ptr<attributes>>::type_ptr> visited;
      while (true) {
        bool child_added{false};
        for (auto& c : breadth_first<std::shared_ptr<attributes>>(root_)) {
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
      if (!build_) {
        build_ = true;
        adopt_pending();
        validate_scheme();
      }
    }

    std::string arguments::main_value(const easytree::tree::node<std::shared_ptr<attributes>>::type_ptr n,
                                      const std::string& vs) const {
      using namespace easytree;
      for (const auto& c : n->children()) {
        if ((**c)->main_value_ != "") {
          if ((**c)->main_value_ == vs) return (**c)->main_value_;
          auto found = std::find((**c)->value_synonyms_.begin(), (**c)->value_synonyms_.end(), vs);
          if (found != (**c)->value_synonyms_.end()) return (**c)->main_value_;
        }
      }
      throw exception::value_not_found(vs);
    }
    
    void arguments::validate_scheme() {
      using namespace easytree;
      build();
      for (auto it1 = breadth_first<std::shared_ptr<attributes>>(root_).begin();
           it1 != breadth_first<std::shared_ptr<attributes>>(root_).end(); ++it1) {
        auto& n = *it1;
        // Validate same level entities
        std::vector<std::string> long_names;
        std::vector<char> short_names;
        std::vector<std::string> values;
        for (const auto& c : n->children()) {
          if ((**c)->kind_ == attributes::KIND::NAME) {
            for (const auto& x : (**c)->all_long_names())
              if (std::find(long_names.begin(), long_names.end(), x) == long_names.end())
                long_names.push_back(x);
              else
                throw exception::name_conflict(x);
            for (const auto& x : (**c)->all_short_names())
              if (std::find(short_names.begin(), short_names.end(), x) == short_names.end())
                short_names.push_back(x);
              else
                throw exception::name_conflict(x);
          }
          if ((**c)->kind_ == attributes::KIND::VALUE) {
            for (const auto& x : (**c)->all_values()) {
              if (std::find(values.begin(), values.end(), x) == values.end())
                values.push_back(x);
              else
                throw exception::value_conflict(x);
            }
          }
        }
        // Validate vertical uniqueness
        for (auto it2 = it1.depth_first_iterator();
             it2 != depth_first<std::shared_ptr<attributes>>(root_).end(); ++it2) {
          auto& o = *it2;
          if ((it1 != it2) &&
              ((**o)->kind_ == attributes::KIND::NAME) &&
              ((**n)->kind_ == attributes::KIND::NAME)) {
            for (const auto& lhs : (**n)->all_long_names()) {
              for (const auto& rhs : (**o)->all_long_names()) {
                if ((lhs != "") && (lhs == rhs))
                  throw exception::name_conflict(lhs);
              }
            }
            for (const auto& lhs : (**n)->all_short_names()) {
              for (const auto& rhs : (**o)->all_short_names()) {
                if ((lhs != 0) && (lhs == rhs))
                  throw exception::name_conflict(lhs);
              }
            }
          }
        }
      }
    }
    
    const easytree::tree::node<std::shared_ptr<attributes>>::type_ptr& arguments::root() const {
      return root_;
    }
    
  }

}
  
