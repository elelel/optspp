#pragma once

namespace optspp {
  namespace scheme {
    struct description;

    // All arguments container
    struct arguments;
    // Argument's attributes
    struct attributes;
  }
  // Sum attributes
  std::shared_ptr<scheme::attributes> operator|(std::shared_ptr<scheme::attributes> lhs,
                                                const std::shared_ptr<scheme::attributes>& rhs);
  // Add a sub-argument (e.g. that is only allowed if this branch was selected)
  std::shared_ptr<scheme::attributes> operator<<(std::shared_ptr<scheme::attributes> lhs,
                                                 const std::shared_ptr<scheme::attributes>& rhs);

  std::shared_ptr<scheme::arguments> operator<<(std::shared_ptr<scheme::arguments> lhs,
                                                const std::shared_ptr<scheme::attributes>& rhs);

  // Parsing-related
  namespace detail {
    struct separation;
  }
  struct token;
  struct parser;


}

