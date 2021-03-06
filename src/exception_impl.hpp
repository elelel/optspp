#pragma once

#include "scheme.hpp"
#include "parser.hpp"

namespace optspp {
  const char* optspp_error::what() const noexcept {
    return message.c_str();
  }

  scheme_error::scheme_error(const std::string& msg) {
    message = msg;
  }

  consume_value_failed::consume_value_failed(const scheme::entity_ptr& e) :
    entity(e) {
    std::string name = entity->all_names_to_string();
    if (name == "") name = "<Unnamed>";
    message = "Consume value failed for argument " + name;
  }
  
  no_implicit_value::no_implicit_value(const scheme::entity_ptr& e) :
    entity(e) {
    std::string name = entity->all_names_to_string();
    if (name == "") name = "<Unnamed>";
    message = "Argument " + name + " specified without a value, but no implicit value is defined";
  }

  argument_conflict::argument_conflict(const scheme::entity_ptr& e) :
    entity(e) {
    std::string name = entity->all_names_to_string();
    if (name == "") name = "<Unnamed>";
    message = "Argument " + name + " conflicts with other arguments";
  }
  
  value_conflict::value_conflict(const scheme::entity_ptr& e, const std::string& v) :
    entity(e),
    value(v) {
    std::string name = entity->all_names_to_string();
    if (name == "") name = "<Unnamed>";
    message = "Argument " + name + " specified with value '" + value + "' conflicts with other argument values";
  }

  unparsed_tokens::unparsed_tokens(const std::list<scheme::parser::token>& ts) :
    tokens(ts) {
    message = "Unparsed tokens left: ";
    bool need_comma = false;
    for (const auto& t : ts) {
      if (need_comma) message += " ";
      need_comma = true;
      message += t.s;
    }
  }

  actual_counts_mismatch::actual_counts_mismatch(const std::vector<record>& rs) :
    records(rs) {
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
