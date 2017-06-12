#include <catch.hpp>
#include <optspp/optspp>

#include <map>

SCENARIO("TDD") {
  using namespace optspp;
  WHEN("Testing basic attributes functionality") {
    REQUIRE(named("login")->long_name() == "login");
    REQUIRE(named("login", {"username", "user"})->long_names() ==
            std::vector<std::string>({"login", "username", "user"}));
  }

  WHEN("A sane argument definition is created") {
    auto arguments = make_arguments();
    arguments
      << (positional("command")
          | (value("useradd")
             << (named("login", {"username", "user"})
                 | named('l', {'u'})
                 | min_count(1)
                 | max_count(1)
                 | description("User's login")
                 )
             << (named("password", {"pw", "pass"})
                 | named('p')
                 | max_count(1))
             << (named("admin", {"administrator"})
                 | named('a')
                 | max_count(1)
                 | (value("true", {"on", "yes"})
                    << (named("superadmin")
                        | value("true", {"on", "yes"})
                        | value("false", {"off", "no"})
                        | default_value("false")
                        | implicit_value("true")
                        | description("Make this administrator a superadministrator")
                        ))
                 | value("false", {"off", "no"})
                 | default_value("false")
                 | implicit_value("true")
                 | description("Make this user administrator")
                 )
             << (named("force")
                 | named('f')
                 | max_count(1)
                 | value("true", {"on", "yes"})
                 | value("false", {"off", "no"})
                 | default_value("false")
                 | implicit_value("true")
                 | description("Force user creation, deleting the old user")
                 ))
          | (value("userdel")
             << (named("login", {"username", "user"})
                 | named('l', {'u'})
                 | min_count(1)
                 | max_count(1)
                 | description("User's login")
                 )
             << (named("force")
                 | named('f')
                 | max_count(1)
                 | value("true", {"on", "yes"})
                 | value("false", {"off", "no"})
                 | default_value("false")
                 | implicit_value("true")
                 | description("Force user deletion")
                 ))
          | any_value()  // Allow arbitraty value for "command" positional argument
          | min_count(1)
          );

    arguments->build();

    THEN("Walk the tree") {
      std::map<size_t, std::vector<std::string>> actual_names;
      std::map<size_t, std::vector<std::string>> actual_values;
      for (auto it = easytree::breadth_first<std::shared_ptr<optspp::scheme::attributes>>(arguments->root()).begin();
           it != easytree::breadth_first<std::shared_ptr<optspp::scheme::attributes>>(arguments->root()).end(); ++it) {
        if ((***it)->kind() == optspp::scheme::attributes::KIND::NAME) {
          actual_names[it.level()].push_back((***it)->long_name());
        }
        if ((***it)->kind() == optspp::scheme::attributes::KIND::VALUE) {
          actual_values[it.level()].push_back((***it)->main_value());
        }
      }
      REQUIRE(actual_names[1] == std::vector<std::string>({"command"}));
      REQUIRE(actual_names[3] == std::vector<std::string>({"login", "password", "admin", "force", "login", "force"}));
      REQUIRE(actual_names[5] == std::vector<std::string>({"superadmin"}));

      REQUIRE(actual_values[2] == std::vector<std::string>({"useradd", "userdel"}));
      REQUIRE(actual_values[4] == std::vector<std::string>({"true", "false", "true", "false", "true", "false"}));
      REQUIRE(actual_values[6] == std::vector<std::string>({"true", "false"}));
    }

    THEN("Parse") {
      std::vector<std::string> cmdl{"useradd", "--admin", "true", "--username", "john", "--password", "secret", "add"};
      arguments->parse(cmdl);
    }
  }
}

SCENARIO("Validation errors") {
  using namespace optspp;
  auto arguments = make_arguments();
  WHEN("Two same level non-unique names") {
    arguments << named("non_unique")
              << named("unique")
              << named("non_unique");
    THEN("Name conflic exception should be thrown on scheme validation") {
      REQUIRE_THROWS_AS(arguments->build(), exception::name_conflict);
    }
  }
  WHEN("Two different level, same vertical path non-unique names") {
    arguments << named("unique")
              << (named("non_unique")
                  | value("a")
                  | (value("b")
                     << named("non_unique")));
    THEN("Name conflict exception should be thrown on scheme validation") {
      REQUIRE_THROWS_AS(arguments->build(), exception::name_conflict);
    }
  }
  WHEN("Two different level, different vertical paths non-unique names") {
    arguments << (named("unique 1")
                  | value("a")
                  | (value("b")
                     << named("non_unique")))
              << (named("unique 2")
                  | value("a")
                  | (value("b")
                     << named("non_unique")));
    THEN("Name conflict exception shouldn't be thrown on scheme validation") {
      REQUIRE_NOTHROW(arguments->build());
    }
  }
  WHEN("Two same level non-unique values") {
    arguments << (named("name")
                  | value("non-unique")
                  | value("non-unique"));
    THEN("Name conflict exception should be thrown on scheme validation") {
      REQUIRE_THROWS_AS(arguments->build(), exception::value_conflict);
    }
  }
  
}



  /*
  WHEN("Creating min property") {
    option source = min_count(1);
    REQUIRE(source.min_count() == 1);
    auto applied = long_name("test") | min_count(1);
    REQUIRE(applied.min_count() == 1);
  }
  
  WHEN("Creating options descriptor with constructor style") {
    auto opts =
      options(option(long_name("login", {"username", "user"}),
                     short_name('l', {'u'}),
                     min_count(1),
                     max_count(1),
                     description("User's login")),
              option(long_name("password", {"pw", "pass"}),
                     max_count(1),
                     short_name('p')),
              option(long_name("admin", {"administrator"}),
                     short_name('a'),
                     valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}}),
                     mutually_exclusive_value({"true", "false"}),
                     default_value("false"),
                     implicit_value("true"),
                     max_count(1),
                     description("Specifies whether the user is administrator")),
              make_option(long_name("active"),
                     valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}}),
                     default_value("true"))
              );

    THEN("Non-existent options must stay non-existent") {
      auto option_non_existent = opts.find("non-existent");
      REQUIRE(option_non_existent.get() == nullptr);
      auto option_n = opts.find('n');
      REQUIRE(option_n.get() == nullptr);
    }
    
    THEN("Options have to be findable") {
      auto option_admin = opts.find("admin");
      REQUIRE(option_admin.get() != nullptr);
      auto option_administrator = opts.find("administrator");
      auto option_a = opts.find('a');
      REQUIRE(option_admin == option_administrator);
      REQUIRE(option_admin == option_a);

      auto option_login = opts.find("login");
      REQUIRE(option_login.get() != nullptr);
      auto option_l = opts.find('l');
      auto option_u = opts.find('u');
      REQUIRE(option_login == option_l);
      REQUIRE(option_login == option_u);
    }

    THEN("Parse normal") {
      std::vector<std::string> args{"--admin", "true", "--username", "john", "--password", "secret", "add"};
      opts.parse(args);
      
      REQUIRE(opts["admin"].size() == 1);
      REQUIRE(opts["admin"][0] == "true");
      REQUIRE(opts["administrator"][0] == "true");
      REQUIRE(opts['a'][0] == "true");

      REQUIRE(opts["username"].size() == 1);
      REQUIRE(opts["username"][0] == "john");
      REQUIRE(opts['u'][0] == "john");

      REQUIRE(opts["password"].size() == 1);
      REQUIRE(opts["password"][0] == "secret");
      REQUIRE(opts["pw"][0] == "secret");
      REQUIRE(opts["pass"][0] == "secret");
      REQUIRE(opts['p'][0] == "secret");

      REQUIRE(opts["active"].size() == 1);
      REQUIRE(opts["active"][0] == "true");

      REQUIRE(opts.positional().size() == 1);
      REQUIRE(opts.positional()[0] == "add");
    }

    THEN("Parse normal with value synonym") {
      std::vector<std::string> args{"--admin", "yes", "--username", "john", "--password", "secret", "add"};
      opts.parse(args);
      
      REQUIRE(opts["admin"].size() == 1);
      REQUIRE(opts["admin"][0] == "true");
      REQUIRE(opts["administrator"][0] == "true");
      REQUIRE(opts['a'][0] == "true");
    }

    THEN("Parse normal with implicit") {
      std::vector<std::string> args{"--admin", "--username", "john", "--password", "secret", "add"};
      opts.parse(args);
      
      REQUIRE(opts["admin"].size() == 1);
      REQUIRE(opts["admin"][0] == "true");
      REQUIRE(opts["administrator"][0] == "true");
      REQUIRE(opts['a'][0] == "true");

      REQUIRE(opts["username"].size() == 1);
      REQUIRE(opts["username"][0] == "john");
      REQUIRE(opts['u'][0] == "john");

      REQUIRE(opts["password"].size() == 1);
      REQUIRE(opts["password"][0] == "secret");
      REQUIRE(opts["pw"][0] == "secret");
      REQUIRE(opts["pass"][0] == "secret");
      REQUIRE(opts['p'][0] == "secret");

      REQUIRE(opts["active"].size() == 1);
      REQUIRE(opts["active"][0] == "true");

      REQUIRE(opts.positional().size() == 1);
      REQUIRE(opts.positional()[0] == "add");
    }
    
    THEN("Parse normal with short name") {
      std::vector<std::string> args{"--admin", "--username", "john", "-p", "secret", "add"};
      opts.parse(args);
      
      REQUIRE(opts["admin"].size() == 1);
      REQUIRE(opts["admin"][0] == "true");
      REQUIRE(opts["administrator"][0] == "true");
      REQUIRE(opts['a'][0] == "true");

      REQUIRE(opts["username"].size() == 1);
      REQUIRE(opts["username"][0] == "john");
      REQUIRE(opts['u'][0] == "john");

      REQUIRE(opts["password"].size() == 1);
      REQUIRE(opts["password"][0] == "secret");
      REQUIRE(opts["pw"][0] == "secret");
      REQUIRE(opts["pass"][0] == "secret");
      REQUIRE(opts['p'][0] == "secret");

      REQUIRE(opts["active"].size() == 1);
      REQUIRE(opts["active"][0] == "true");

      REQUIRE(opts.positional().size() == 1);
      REQUIRE(opts.positional()[0] == "add");
    }

    THEN("Parse normal with short names in pack") {
      std::vector<std::string> args{"--username", "john", "-ap", "secret", "add"};
      opts.parse(args);
      REQUIRE(opts["admin"].size() == 1);
      REQUIRE(opts["admin"][0] == "true");
      REQUIRE(opts["administrator"][0] == "true");
      REQUIRE(opts['a'][0] == "true");

      REQUIRE(opts["username"].size() == 1);
      REQUIRE(opts["username"][0] == "john");
      REQUIRE(opts['u'][0] == "john");

      REQUIRE(opts["password"].size() == 1);
      REQUIRE(opts["password"][0] == "secret");
      REQUIRE(opts["pw"][0] == "secret");
      REQUIRE(opts["pass"][0] == "secret");
      REQUIRE(opts['p'][0] == "secret");

      REQUIRE(opts["active"].size() == 1);
      REQUIRE(opts["active"][0] == "true");

      REQUIRE(opts.positional().size() == 1);
      REQUIRE(opts.positional()[0] == "add");
    }

    THEN("Invalid invocation: required parameter missed") {
      std::vector<std::string> args{"-ap", "secret", "add"};
      REQUIRE_THROWS_AS(opts.parse(args), exception::too_few_values);
    }

    THEN("Invalid invocation: invalid value specified") {
      std::vector<std::string> args{"--admin", "yeah", "--username", "john", "--password", "secret", "add"};
      REQUIRE_THROWS_AS(opts.parse(args), exception::invalid_parameter_value);
     }

    THEN("Invalid invocation: no value specified") {
      std::vector<std::string> args{"--admin", "true", "--username", "--password", "secret", "add"};
      REQUIRE_THROWS_AS(opts.parse(args), exception::parameter_requires_value);
    }

    
  }

  WHEN("Creating options descriptor with streamline/pipeline style") {
    options opts;
    opts << (long_name("admin", {"administrator"})
             | short_name('a')
             | valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}})
             | mutually_exclusive_value({"true", "false"})
             | default_value("false")
             | implicit_value("true")
             | description("Specifies whether the user is administrator"))
         << (long_name("login", {"username", "user"})
             << short_name('l', {'u'})
             << description("User's login"))
         << (option()
             << long_name("password", {"pw", "pass"})
             << short_name('p'));
    THEN("Non-existent options must stay non-existent") {
      auto option_non_existent = opts.find("non-existent");
      REQUIRE(option_non_existent.get() == nullptr);
      auto option_n = opts.find('n');
      REQUIRE(option_n.get() == nullptr);
    }
    THEN("Options have to be findable") {
      auto option_admin = opts.find("admin");
      REQUIRE(option_admin.get() != nullptr);
      auto option_administrator = opts.find("administrator");
      auto option_a = opts.find('a');
      REQUIRE(option_admin == option_administrator);
      REQUIRE(option_admin == option_a);

      auto option_login = opts.find("login");
      REQUIRE(option_login.get() != nullptr);
      auto option_l = opts.find('l');
      auto option_u = opts.find('u');
      REQUIRE(option_login == option_l);
      REQUIRE(option_login == option_u);
    }
  }
  */

