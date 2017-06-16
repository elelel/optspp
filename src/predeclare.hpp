#pragma once

#include <memory>

#include "../contrib/easytree/include/easytree/tree"

namespace optspp {
  namespace scheme {
    struct definition;
    struct entity;

    // Scheme element (argument definition, value definition)
    using entity_ptr = std::shared_ptr<entity>;
    // Scheme's tree node
    using node_ptr = easytree::tree::node<entity_ptr>::type_ptr;
  }
  // Properties
  struct name;
  struct min_count;
  struct max_count;
  struct description;
  struct default_value;
  struct implicit_value;
  struct any;

  struct parser;

  // Factory functions to create scheme definition nodes
  // Create positional argument definition
  template <typename... Properties>
  scheme::entity_ptr positional(Properties&&... ps);
  // Create named argument definition
  template <typename... Properties>
  scheme::entity_ptr named(Properties&&... ps);
  // Create value definition
  scheme::entity_ptr value(const std::string& val);
  scheme::entity_ptr value(const std::string& val, std::initializer_list<std::string> synonyms);
  

  // Assign argument node to scheme's root
  std::shared_ptr<scheme::definition>& operator<<(std::shared_ptr<scheme::definition> d, const std::shared_ptr<scheme::entity>& e);
  // Assign value definition to argument definition and argument definition to value definition; the children are xor-compatible
  scheme::entity_ptr operator<<(scheme::entity_ptr lhs, const scheme::entity_ptr& rhs);
  // Assign value definition to argument definition and argument definition to value definition; the children are or-compatible
  scheme::entity_ptr operator|(scheme::entity_ptr lhs, const scheme::entity_ptr& rhs);
  // Assign argument definition to scheme definition; the children are or-compatible
  scheme::definition& operator<<(scheme::definition& lhs, const std::shared_ptr<scheme::entity>& rhs);  
  // Assign argument definition to scheme definition; the children are or-compatible
  scheme::definition& operator|(scheme::definition& lhs, const std::shared_ptr<scheme::entity>& rhs);
  
}
