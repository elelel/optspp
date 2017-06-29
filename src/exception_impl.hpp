#pragma once

namespace optspp {
  const char* optspp_error::what() const noexcept {
    return message.c_str();
  }

  scheme_error::scheme_error(const std::string& msg) {
    message = msg;
  }

  actual_counts_mismatch::actual_counts_mismatch(const std::vector<record>& rs) :
    records(rs) {
    std::cout << "Constructing mismatch\n";
    message = "Actual argument value counts mismatch. ";
    bool need_comma = false;
    for (const auto& r : records) {
      if (need_comma) message += " ";
      need_comma = true;
      std::cout << "All names to string, entity " << r.entity << "\n";
      std::string name = r.entity->all_names_to_string();
      std::cout << "All names to string done\n";
      if (name == "") name = "<Unnamed>";
      message += "Argument " + name + " ";
      bool need_inner_comma = false;
      if (r.entity->min_count()) {
        message += "minimum expected " + std::to_string(*r.entity->min_count());
        need_inner_comma = true;
      }
      if (r.entity->max_count()) {
        if (need_inner_comma) message += ", ";
        message += "maximum expected " + std::to_string(*r.entity->max_count());
        need_inner_comma = true;
      }
      if (need_inner_comma) message += ", ";
      message += "actual " + std::to_string(r.actual);
      message += ".";
    }
    std::cout << "Constructing mismatch done\n";
  }
}
