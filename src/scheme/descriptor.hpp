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
      add_child_name(other);
      return *this;
    }
    
  }
}
