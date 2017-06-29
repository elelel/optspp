#pragma once

namespace optspp {
  const char* optspp_error::what() const noexcept {
    return message.c_str();
  }

  scheme_error::scheme_error(const std::string& msg) {
    message = msg;
  }

  value_required::value_required(const scheme::entity_ptr& e) :
    entity(e) {
    std::string name = entity->all_names_to_string();
    if (name == "") name = "<Unnamed>";
    message = "Argument " + name + " requires a value";
  }

  actual_counts_mismatch::actual_counts_mismatch(const std::vector<record>& rs) :
    records(rs) {
    std::cout << "Constructing mismatch\n";
    message = "Actual argument value counts mismatch. ";
    bool need_comma = false;
    for (const auto& r : records) {
      if (need_comma) message += " ";
      need_comma = true;
      std::string name = r.entity->all_names_to_string();
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

  value_not_found::value_not_found(const std::string& n) :
    name(n) {
    message = "Argument '" + name + "' has no values";
  }

  value_not_found::value_not_found(const std::string& n, const size_t idx) :
    name(n),
    index(idx) {
    message = "Index " + std::to_string(idx) + " for argument '" + name + "' is out bounds";
  }
}
