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
      adopt_pending();
      for (auto& n : breadth_first<std::shared_ptr<attributes>>(root_)) {
        std::cout << "Validating node " << (**n)->long_name_ << "\n";
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
      }
    }
    
    const easytree::tree::node<std::shared_ptr<attributes>>::type_ptr& arguments::root() const {
      return root_;
    }
    
  }

}
  
