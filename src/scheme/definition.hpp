#pragma once

#include "../scheme.hpp"

namespace optspp {
  namespace scheme {
    definition::definition() {
      root_ = std::make_shared<entity>(entity::KIND::NONE);
    }
    
    void definition::parse(const std::vector<std::string>& cmdl_args) {
      if (parsed_) return;
      validate();
      parser p(*this, cmdl_args);
      p.parse();
      validate_results();
    }

    void definition::parse(const int argc, char* argv[]) {
      std::vector<std::string> args;
      if (argc > 1) {
        for (int i = 1; i < argc; ++i) args.push_back(std::string(argv[i]));
      }
      parse(args);
    }

    void definition::vertical_name_check(const std::vector<std::string>& taken_long_names,
                                         const std::vector<char>& taken_short_names,
                                         const entity_ptr& e) {
      auto taken_long = taken_long_names;
      auto taken_short = taken_short_names;
      if (e->kind_ == entity::KIND::ARGUMENT) {
        if (e->long_names_) {
          for (const auto& n : *e->long_names_) {
            if (std::find(taken_long.begin(), taken_long.end(), n) != taken_long.end())
              throw scheme_error("Argument's long name should not be used by it's descendant");
          }
          std::copy((*e->long_names_).begin(), (*e->long_names_).end(), std::back_inserter(taken_long));
        }
        if (e->short_names_) {
          for (const auto& n : *e->short_names_) {
            if (std::find(taken_short.begin(), taken_short.end(), n) != taken_short.end())
              throw scheme_error("Argument's short name should not be used by it's descendant");
          }
          std::copy((*e->short_names_).begin(), (*e->short_names_).end(), std::back_inserter(taken_short));
        }
      }
      for (const auto& c : e->pending_) vertical_name_check(taken_long, taken_short, c);
    }

    void definition::validate_entity(const entity_ptr& e) {
      if (e->kind_ == entity::KIND::ARGUMENT) {
        if (e->is_positional_ && *e->is_positional_) {
          if (e->short_names_)
            throw scheme_error("Positional argument should not have short names");
          if (e->implicit_values_)
            throw scheme_error("Positional argument should not implicit values");
          if (e->any_value_ && *e->any_value_) {
            for (const auto& c : e->pending_) {
              if ((c->kind_ == entity::KIND::ARGUMENT) && e->is_positional_ && !*e->is_positional_)
                throw scheme_error("Positional with any value should not have named child");
            }
          }
        } else {
          bool short_undefined = !e->short_names_ || (e->short_names_ && (*e->short_names_).size() == 0);
          bool long_undefined = !e->long_names_ || (e->long_names_ && (*e->long_names_).size() == 0);
          if (short_undefined && long_undefined )
            throw scheme_error("Named argument's both long and short names are empty");
        }
        for (const auto& c : e->pending_) {
          if (c->kind_ != entity::KIND::VALUE) {
            throw scheme_error("Argument entity shoud not have non-value child");
          }
        }
        // Implicitly allow any value
        if (std::find_if(e->pending_.begin(), e->pending_.end(), [] (const entity_ptr& v) {
              return v->kind_ == entity::KIND::VALUE;
            }) == e->pending_.end()) {
          std::cout << "Adding ANY value to " << e->all_names_to_string() << "\n";
          e->pending_.push_back(::optspp::value(any()));
        }
      }
      if (e->kind_ == entity::KIND::VALUE) {
        for (const auto& c : e->pending_) {
          if (c->kind_ != entity::KIND::ARGUMENT)
            throw scheme_error("Value entity should not have non-argument child");
        }
      }
      for (const auto& c : e->pending_) {
        validate_entity(c);
      }
    }
    
    void definition::validate() const {
      for (const auto& c : root_->pending_) {
        validate_entity(c);
        vertical_name_check(std::vector<std::string>(), std::vector<char>(), c);
      }
    }

    void definition::min_value_check(std::vector<actual_counts_mismatch::record>& acc, const entity_ptr& e) const {
      if (e->color_ != entity::COLOR::BLOCKED) {
        if (e->min_count_) {
          auto found = values_.find(e);
          if (found == values_.end()) {
            if (*e->min_count_ > 0) {
              acc.push_back({e, 0});
            }
          } else {
            std::cout << "checking size for " << found->first.get() << "\n";
            if (found->first->min_count_ &&
                (found->second.size() < *found->first->min_count_))
              acc.push_back({e, found->second.size()});
            std::cout << "done\n";
          }
      
        }
        for (const auto& c : e->pending_) min_value_check(acc, c);
      }
    }

    void definition::validate_results() const {
      std::vector<actual_counts_mismatch::record> acc;
      for (const auto& c : root_->pending_) {
        min_value_check(acc, c);
      }
      for (const auto& p : values_) {
        const auto& e = p.first;
        const auto& vs = p.second;
        if (e->max_count_ && (vs.size() > *e->max_count_)) {
          acc.push_back({e, vs.size()});
        }
      }
      if (acc.size() > 0) {
        throw actual_counts_mismatch(acc);
      }
    }

    bool definition::is_long_prefix(const std::string& s) const {
      return std::find(long_prefixes_.begin(), long_prefixes_.end(), s) != long_prefixes_.end();
    }

    bool definition::is_short_prefix(const std::string& s) const {
      return std::find(short_prefixes_.begin(), short_prefixes_.end(), s) != short_prefixes_.end();
    }

    const entity_ptr& definition::root() const {
      return root_;
    }

    const std::vector<std::string>& definition::operator[](const std::string& name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) return p.second;
      }
      static std::vector<std::string> empty;
      return empty;
    }

    const std::vector<std::string>& definition::operator[](const char name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) return p.second;
      }
      static std::vector<std::string> empty;
      return empty;
    }

    const std::string& definition::operator()(const std::string& name, const size_t idx) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() < idx) {
            return p.second[idx];
          } else {
            throw value_not_found(name, idx);
          }
        }
      }
      throw value_not_found("Argument '" + name + "' not found");
    }

    const std::string& definition::operator()(const std::string& name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() > 0) {
            return p.second[p.second.size() - 1];
          } else {
            throw value_not_found(name);
          }
        }
      }
      throw value_not_found("Argument '" + name + "' not found");
    }

    const std::string& definition::operator()(const char name, const size_t idx) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() < idx) {
            return p.second[idx];
          } else {
            throw value_not_found(std::string() + name, idx);
          }
        }
      }
      throw value_not_found(std::string("Argument '") + name + "' not found");
    }

    const std::string& definition::operator()(const char name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() > 0) {
            return p.second[p.second.size() - 1];
          } else {
            throw value_not_found(std::string() + name, 0);
          }
        }
      }
      throw value_not_found(std::string() + name);
    }

    
  }
}
