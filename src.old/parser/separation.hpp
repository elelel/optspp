#pragma once

namespace optspp {
  namespace detail {
    separation::separation(const std::string& s, const std::vector<std::string>& separators) :
      was_separated_(false),
      key(s) {
      for (const auto& sep : separators) {
        auto pos = s.find(sep);
        if (pos != std::string::npos) {
          key = s.substr(0, pos);
          value_pos = pos + sep.length() + 1;
          value = s.substr(value_pos, s.length() - value_pos);
          was_separated_ = true;
          break;
        }
      }
    }

    separation::operator bool() const noexcept {
      return was_separated_;
    }
  }
}

