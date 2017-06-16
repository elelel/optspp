#pragma once

namespace optspp {
  template <typename T>
  struct optional {
    optional() :
      some_(false) {
    }
    
    optional(const T& value) :
      some_(true),
      value_(value) {
    }
    
    T& operator*() {
      return value_;
    }

    const T& operator*() const {
      return value_;
    }

    constexpr operator bool() const noexcept {
      return some_;
    }

    optional<T>& operator=(T v) {
      value_ = v;
      some_ = true;
      return *this;
    }
  private:
    bool some_;
    T value_;
  };
}
