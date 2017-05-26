#include <catch.hpp>
#include <optspp/optspp>

SCENARIO("TDD") {
  using namespace optspp;
  
  WHEN("Creating options descriptor with constructor style") {
    auto opts =
      options(option(long_name("admin", {"administrator"}),
                     short_name('a'),
                     valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}}),
                     mutually_exclusive_value({"true", "false"}),
                     default_value("false"),
                     implicit_value("true"),
                     max_count(1),
                     description("Specifies whether the user is administrator")),
              option(long_name("login", {"username", "user"}),
                     short_name('l', {'u'}),
                     max_count(1),
                     description("User's login")),
              make_option(long_name("password", {"pw", "pass"}),
                          max_count(1),
                          short_name('p'))
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
    }
  }

  WHEN("Creating options descriptor with streamline style") {
    options opts;
    opts 
      << (option()
          << long_name("admin", {"administrator"})
          << short_name('a')
          << valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}})
          << mutually_exclusive_value({"true", "false"})
          << default_value("false")
          << implicit_value("true")
          << description("Specifies whether the user is administrator"))
      << (option()
          << long_name("login", {"username", "user"})
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

  /*
    std::vector<std::string> args{"--login", "User Name"};*/
}
