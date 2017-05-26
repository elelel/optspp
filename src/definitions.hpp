#pragma once

namespace optspp {
  void option::check_parents() const {
    for (const auto& os : parent_containers_) {
      os->check();
    }
  }

  std::string option_names_to_str(const option& o) {
    std::string rslt;
    bool need_comma{false};
    if (o.long_name() != "") {
      if (need_comma) rslt += "/";
      rslt += o.long_name();
      need_comma = true;
    }
    for (const auto& n : o.long_name_synonyms()) {
      if (need_comma) rslt += "/";
      rslt += n;
      need_comma = true;
    }
    if (o.short_name() != 0) {
      if (need_comma) rslt += "/";
      rslt += o.short_name();
      need_comma = true;
    }
    for (const auto& n : o.short_name_synonyms()) {
      if (need_comma) rslt += "/";
      rslt += n;
      need_comma = true;
    }
    return rslt;
  }
}
