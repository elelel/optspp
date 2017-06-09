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
      while (true) {
        bool child_added{false};
        for (auto& c : breadth_first<std::shared_ptr<attributes>>(root_)) {
          std::cout << "Testing " << (**c)->long_name_ << " / " << (**c)->main_value_ << "\n";
          if ((**c)->pending_.size() != 0) {
            for (const auto& cc : (**c)->pending_) {
              std::cout << "Adding child\n";
              c->add_child(node(cc));
              child_added = true;
            }
            (**c)->pending_.clear();
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
      for (auto& c : breadth_first<std::shared_ptr<attributes>>(root_)) {
        std::cout << "Node " << (**c)->long_name_ << "\n";
      }
    }
  }
}
  
