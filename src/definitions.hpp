#pragma once

namespace optspp {
  void option::check_parents() const {
    for (const auto& os : parent_containers_) {
      os->check();
    }
  }
}
