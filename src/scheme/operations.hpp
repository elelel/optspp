#pragma once

namespace optspp {
  std::shared_ptr<scheme::arguments> make_arguments() {
    return std::make_shared<scheme::arguments>();
  }

  // Sum attributes or add values to names
  std::shared_ptr<scheme::attributes> operator|(std::shared_ptr<scheme::attributes> lhs,
                                                const std::shared_ptr<scheme::attributes>& rhs) {
    if ((lhs->kind_ == scheme::attributes::KIND::NAME) &&
        (rhs->kind_ == scheme::attributes::KIND::NAME)) {
      *lhs += *rhs;
    } else {
      using namespace easytree;
      auto n = node<std::shared_ptr<scheme::attributes>>(rhs);
      lhs->pending_.push_back(rhs);
    }
    return lhs;
  }
  
  // Add a sub-argument (e.g. that is only allowed if this branch was selected)
  std::shared_ptr<scheme::attributes> operator<<(std::shared_ptr<scheme::attributes> lhs,
                                                 const std::shared_ptr<scheme::attributes>& rhs) {
    using namespace easytree;
    auto n = node<std::shared_ptr<scheme::attributes>>(rhs);
    lhs->pending_.push_back(rhs);
    return lhs;
  }
    

  std::shared_ptr<scheme::arguments> operator<<(std::shared_ptr<scheme::arguments> lhs,
                                                const std::shared_ptr<scheme::attributes>& rhs) {
    using namespace easytree;
    lhs->root_->add_child(node(rhs));
    return lhs;
  }


}
