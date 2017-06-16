#pragma once

namespace optspp {
  namespace scheme {
    namespace entity {
      desc::desc() :
        base(KIND::DESCRIPTION) {
      }

      desc::~desc() {
      }
    
      desc& desc::set_description(const std::string& s) {
        description_ = s;
        return *this;
      }

      const std::string& desc::description() const {
        return description_;
      }
                                    
    }
  }
}
