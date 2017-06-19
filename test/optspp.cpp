#include <catch.hpp>
#include <optspp/optspp>

#include <map>

SCENARIO("Test optional") {
  using namespace optspp;
  WHEN("Created an empty optional<int>") {
    optional<int> x;
    REQUIRE(x == false);
    THEN("Assign a value") {
      x = 1;
      REQUIRE(x == true);
      REQUIRE(*x == 1);
    } 
  }
  WHEN("Created initialized optional<int>") {
    optional<int> x(1);
    REQUIRE(x == true);
    REQUIRE(*x == 1);
    THEN("Assigne none") {
      x = optional<int>();
      REQUIRE(x == false);
    }
  }
  WHEN("Copy-constructing") {
    optional<int> x(1);
    optional<int> y(x);
    REQUIRE(*x == 1);
    REQUIRE(*y == 1);
  }
  WHEN("Assigning optionals to each other") {
    optional<int> x;
    REQUIRE(x == false);
    optional<int> y(1);
    x = y;
    REQUIRE(*x == 1);
    REQUIRE(*y == 1);
    REQUIRE(*x == *y);
  }
}

SCENARIO("Test entity properties") {
  using namespace optspp;
  auto n1 = named(name("first"));
  REQUIRE(n1->kind() == scheme::entity::KIND::ARGUMENT);
  REQUIRE(n1->long_names());
  REQUIRE((*n1->long_names()).size() == 1);
  auto v1 = value("yes");
  REQUIRE(v1->known_values());
  REQUIRE(*v1->known_values() == std::vector<std::string>({"yes"}));
}

SCENARIO("Test XOR scheme without 'any' values and positional args") {
  using namespace optspp;
  WHEN("Created the scheme") {
    scheme::definition args;
    args
      << (named(name("Arg_L1_1"))
          << (value("Val_L2_1")
              << (named(name("Arg_L3_1"))
                  << (value("Val_L4_1"))
                  << (value("Val_L4_2")))
              << (named(name("Arg_L3_2"))))
          << (value("Val_L2_2"))
          )
      << (named(name("Arg_L1_2"))
          << (value("Val_L2_2_1"))
          << (value("Val_L2_2_2")));
    REQUIRE(args.root()->children().size() == 2);
    auto& arg_l1_1 = args.root()->children()[0];
    auto& val_l2_1 = arg_l1_1->children()[0];
    auto& arg_l3_1 = val_l2_1->children()[0];
    auto& val_l4_1 = arg_l3_1->children()[0];
    auto& val_l4_2 = arg_l3_1->children()[1];
    auto& arg_l3_2 = val_l2_1->children()[1];
    auto& val_l2_2 = arg_l1_1->children()[1];
    auto& arg_l1_2 = args.root()->children()[1];


    REQUIRE(arg_l1_1->long_names());
    REQUIRE((*arg_l1_1->long_names())[0] == "Arg_L1_1");
    REQUIRE(arg_l1_1->siblings_group() == scheme::SIBLINGS_GROUP::XOR);
    REQUIRE(arg_l1_2->long_names());
    REQUIRE((*arg_l1_2->long_names())[0] == "Arg_L1_2");
    REQUIRE(arg_l1_2->siblings_group() == scheme::SIBLINGS_GROUP::XOR);
    REQUIRE(val_l2_1->known_values());
    REQUIRE((*val_l2_1->known_values())[0] == "Val_L2_1");
    REQUIRE(val_l2_2->known_values());
    REQUIRE((*val_l2_2->known_values())[0] == "Val_L2_2");
    
    THEN("Command line: --Arg_L1_1 Val_L2_1 --Arg_L3_1 Val_L4_2") {
      scheme::parser p(args, {"--Arg_L1_1", "Val_L2_1",  "--Arg_L3_1", "Val_L4_2"});
      p.initialize_pass();
      auto parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(p.consume_named(parent));
      REQUIRE(arg_l1_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l1_2->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l2_1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(val_l2_2->color() == scheme::entity::COLOR::BLOCKED);
      parent = p.find_border_entity();
      REQUIRE(parent == val_l2_1);
      REQUIRE(p.consume_named(parent));
      REQUIRE(arg_l3_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l3_2->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l4_1->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l4_2->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(p.find_border_entity() == nullptr);
    }
  }            
}

SCENARIO("Test XOR/OR scheme without 'any' values and positional args") {
  using namespace optspp;
  WHEN("Created the scheme") {
    scheme::definition args;
    args
      | (named(name("Arg_L1_1"))
          << (value("Val_L2_1")
              << (named(name("Arg_L3_1"))
                  | (value("Val_L4_1"))
                  | (value("Val_L4_2")))
              << (named(name("Arg_L3_2"))))
          << (value("Val_L2_2"))
          )
      | (named(name("Arg_L1_2"))
          << (value("Val_L2_2_1"))
          << (value("Val_L2_2_2")));
    REQUIRE(args.root()->children().size() == 2);
    auto& arg_l1_1 = args.root()->children()[0];
    auto& val_l2_1 = arg_l1_1->children()[0];
    auto& arg_l3_1 = val_l2_1->children()[0];
    auto& val_l4_1 = arg_l3_1->children()[0];
    auto& val_l4_2 = arg_l3_1->children()[1];
    auto& arg_l3_2 = val_l2_1->children()[1];
    auto& val_l2_2 = arg_l1_1->children()[1];
    auto& arg_l1_2 = args.root()->children()[1];


    REQUIRE(arg_l1_1->long_names());
    REQUIRE((*arg_l1_1->long_names())[0] == "Arg_L1_1");
    REQUIRE(arg_l1_1->siblings_group() == scheme::SIBLINGS_GROUP::OR);
    REQUIRE(arg_l1_2->long_names());
    REQUIRE((*arg_l1_2->long_names())[0] == "Arg_L1_2");
    REQUIRE(arg_l1_2->siblings_group() == scheme::SIBLINGS_GROUP::OR);
    REQUIRE(val_l2_1->known_values());
    REQUIRE((*val_l2_1->known_values())[0] == "Val_L2_1");
    REQUIRE(val_l2_2->known_values());
    REQUIRE((*val_l2_2->known_values())[0] == "Val_L2_2");
    
    THEN("Command line: --Arg_L1_1 Val_L2_1 --Arg_L3_1 Val_L4_2") {
      scheme::parser p(args, {"--Arg_L1_1", "Val_L2_1",  "--Arg_L3_1", "Val_L4_2", "--Arg_L3_1", "Val_L4_1", "--Arg_L1_2", "Val_L2_2_2" });
      p.initialize_pass();
      auto parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(p.consume_named(parent));
      REQUIRE(arg_l1_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l1_2->color() == scheme::entity::COLOR::NONE);
      REQUIRE(val_l2_1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(val_l2_2->color() == scheme::entity::COLOR::BLOCKED);
      parent = p.find_border_entity();
      REQUIRE(parent == val_l2_1);
      REQUIRE(p.consume_named(parent));
      REQUIRE(arg_l3_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l3_2->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l4_1->color() == scheme::entity::COLOR::NONE);
      REQUIRE(val_l4_2->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(p.find_border_entity() == nullptr);
      THEN("Second tree pass") {
        p.initialize_pass();
        REQUIRE(arg_l1_1->color() == scheme::entity::COLOR::NONE);
        REQUIRE(arg_l1_2->color() == scheme::entity::COLOR::NONE);
        REQUIRE(val_l2_1->color() == scheme::entity::COLOR::NONE);
        REQUIRE(val_l2_2->color() == scheme::entity::COLOR::BLOCKED);
        REQUIRE(arg_l3_1->color() == scheme::entity::COLOR::NONE);
        REQUIRE(arg_l3_2->color() == scheme::entity::COLOR::BLOCKED);
        REQUIRE(val_l4_1->color() == scheme::entity::COLOR::NONE);
        REQUIRE(val_l4_2->color() == scheme::entity::COLOR::NONE);
      }
    }
  }            
}

/*

SCENARIO("TDD") {
  using namespace optspp;
  WHEN("A sane argument definition is created") {
    scheme::definition args1;
    args1 << (named(name("first"))
              << (value("yes"))
              << (value("no")))
          << (named(name("second"))
              << (value("yes"))
              << (value("no")));
    const std::vector<std::string> args_input2{"--first", "yes", "--second", "no"};
    args1.parse(args_input2);
    
    scheme::definition args2;
    args2 | (named(name("first"))
              << (value("yes"))
              << (value("no")))
          | (named(name("second"))
              << (value("yes"))
              << (value("no")));

    
    auto option_force =
      named(name("force"),
            name('f'),
            default_values("false"),
            implicit_values("true"))
      << value("true", {"on", "yes"})
      << value("false", {"off", "no"});

    
    scheme::definition arguments;
    arguments <<
      (positional(name("command"))
       << (value("useradd")
           | (named(name("login", {"username", "user"}),
                    name('l', {'u'}),
                    min_count(1),
                    max_count(1),
                    description("User's login")))
           | (named(name("password", {"pw", "pass"}),
                    name('p'),
                    max_count(1),
                    description("User's password")))
           | (named(name("admin", {"administrator"}),
                    name('a'),
                    description("Make this user administrator"))
              | (value("true", {"on", "yes"})
                 << (named(name("super-admin"),
                           default_values("false"),
                           implicit_values("true"),
                           description("Make this administrator a superadministrator"))
                     << value("true", {"on", "yes"})
                     << value("false", {"off", "no"}))
                 << (named(name("rookie-admin"),
                           default_values("true"),
                           implicit_values("true"),
                           description("Make this administrator a rookie administrator"))
                     << value("true", {"on", "yes"})
                     << value("false", {"off", "no"})))
              | (value("false", {"off", "no"})))
           | (option_force))
       << (value("userdel")
           | (named(name("login", {"username", "user"}),
                    name('l', {'u'}),
                    min_count(1),
                    max_count(1),
                    description("User's login")))
           | (option_force))
       << (value(any())));

    std::vector<std::string> arguments_input{"useradd", "--super-admin", "--admin", "yes", "--login", "mylogin"};
    arguments.parse(arguments_input);
    
  }
  }*/
