#include <catch.hpp>
#include <optspp/optspp>

SCENARIO("General test") {
  using namespace optspp;
  optspp::options opts;

  make_option(long_name("admin", {"administrator"}),
              short_name('a'),
              valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}}),
              mutually_exclusive_values("true", "false"),
              default_value("false"),
              implicit_value("true"),
              description("Specifies whether the user is administrator"));

  opts <<
    (option()
     << long_name("admin", {"administrator"})
     << short_name('a')
     << valid_values({{"true", {"on", "yes"}}, {"false", {"off", "no"}}})
     << mutually_exclusive_values("true", "false")
     << default_value("false")
     << implicit_value("true")
     << description("Specifies whether the user is administrator"));

  /*
  options.add_option(opt);
  options.by_long_name("login");
  options.by_short_name("l");
  options.add_constraint(mutually_exclusive(opt1, opt2, opt3));
  options.remove_constraint(mutually_exclusive(opt1, opt2));
  
  std::vector<std::string> args{"--login", "User Name"};*/
}
