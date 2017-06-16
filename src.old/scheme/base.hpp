#pragma once

namespace optspp {
  namespace scheme {
    namespace entity {
      base::base(KIND kind) :
        kind_(kind) {
      }

      base::~base() {
      }

      auto base::kind() const -> const KIND& {
        return kind_;
      }
    }
  }
}
