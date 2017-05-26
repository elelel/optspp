#include <catch.hpp>
#include <optspp/optspp>

SCENARIO("TDD") {
  using namespace optspp;
  
  // Constructor-style options descripton
  optspp::options opts;

  WHEN("Creating options descriptor with constructor style") {
    opts
      // Using value-based option
      << option(long_name("admin", {"administrator"}),
                short_name('a'),
                valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}}),
                mutually_exclusive_values("true", "false"),
                default_value("false"),
                implicit_value("true"),
                description("Specifies whether the user is administrator"))
      // Using shared_ptr<option>
      << make_option(long_name("login", {"username", "user"}),
                     short_name('l', {'u'}),
                     description("User's login"))
      << make_option(long_name("password", {"pw", "pass"}),
                     short_name('p'));
  }

  WHEN("Creating options descriptor with streamline style") {
    opts << (option()
             << long_name("admin", {"administrator"})
             << short_name('a')
             << valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}})
             << mutually_exclusive_values("true", "false")
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
  }


  /*
  options.add_option(opt);
  options.by_long_name("login");
  options.by_short_name("l");
  options.add_constraint(mutually_exclusive(opt1, opt2, opt3));
  options.remove_constraint(mutually_exclusive(opt1, opt2));
  
  std::vector<std::string> args{"--login", "User Name"};*/
}
