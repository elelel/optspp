#pragma once

namespace optspp {
  namespace scheme {
    descriptor::descriptor() :
      node(KIND::DESCRIPTOR) {
    }

    descriptor::~descriptor() {
    }

    void descriptor::validate() const {
      node::validate();
    }

    descriptor& descriptor::operator<<(const std::shared_ptr<name>& other) {
      add_child_name(other);
      return *this;
    }

    descriptor& descriptor::operator<<(const name& other) {
      std::cout << "descriptor:: adding child\n";
      add_child_name(other);
      std::cout << "descriptor:: added child\n";
      return *this;
    }
    
  }
}
