#pragma once

namespace optspp {
  std::shared_ptr<scheme::tree::descriptor> make_scheme() {
    return std::make_shared<scheme::tree::descriptor>();
  }

  namespace scheme {
    std::shared_ptr<tree::name> operator|(std::shared_ptr<tree::name> n, const std::shared_ptr<tree::value>& v) {
      if (std::find(n->child_values_.begin(), n->child_values_.end(), v) != n->child_values_.end())
        return n;
        
      for (const auto& c : n->child_values_) {
        if ((v->main_value() != "") && (*c == v->main_value())) throw exception::argument_value_conflict(v->main_value());
        for (const auto& s : v->value_synonyms())
          if ((s != "") && (*c == s)) throw exception::argument_value_conflict(s);
      }
      n->child_values_.push_back(v);
      v->add_parent(n);
      return n;
    }

    std::shared_ptr<tree::name> operator|(std::shared_ptr<tree::name> lhs, const std::shared_ptr<description>& rhs) {
      std::cout << "name | description \n";
      *lhs += *rhs;
      return lhs;
    }
    
    std::shared_ptr<tree::name> operator|(std::shared_ptr<tree::name> lhs, const std::shared_ptr<tree::name>& rhs) {
      std::cout << "name | name \n";
      *lhs += *rhs;
      return lhs;
    }

    namespace tree {
      template <typename A, typename B>
      std::shared_ptr<A> operator<<(std::shared_ptr<A> lhs, const std::shared_ptr<B>& rhs) {
        std::cout << "shared << X " << rhs->long_name() << "\n";
        *lhs << rhs;
        return lhs;
      }

      template <typename NodeSpecialization>
      std::shared_ptr<node> node_shared_ptr_from(const NodeSpecialization& n) {
        return (std::shared_ptr<node>())n.shared_from_this();
      }
    }
  }
}
