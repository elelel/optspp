# Optspp: parse command line arguments in C++
The library parses [GNU-style command line arguments](https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html) according to a dependency tree scheme.
This aleviates the burden of options dependency checking by programmer, if incompatible options are specified an exception is thrown at parse time.

Consider scheme difinition for a fictional file manipulation utility:

```c++
using namespace optspp;
scheme::definition arguments;
  arguments
    // Declare a named argument node
    | (named(name("force"),  // Set long name attribute to "force"
             name('f'),      // Set short name attribute to 'f'
             default_values("false"),   // Which value to assume if not given on the cmd line
             implicit_values("true"),   // Which value to assume if given on cmd line without value
             max_count(1))    // Maximum number of times the argument may be specified
       << value("true", {"on", "yes"})  // Possible value with synonyms
       << value("false", {"off", "no"}))
    | (named(name("recursive"),
             name('r', {'R'}),  // Short name with one synonym
             default_values("false"),
             implicit_values("true"))
       << value("true", {"on", "yes"})
       << value("false", {"off", "no"}))
    | (positional(name("filename"),
                  min_count(1))
       << (value(any())));         
```
Here | operator designates a branch in the tree with OR relationship with siblings,
and << operator is used to designate XOR relationship.
All of the following:
```
myutil -rf on file1 file2
myutil -rf file1 file2
myutil -r -f file1 file2
myutil -R --force file1 file2
```
will be parsed identically:
```c++
REQUIRE(arguments["force"].size() == 1);
REQUIRE(arguments["force"][0] == "true");
REQUIRE(arguments["recursive"].size() == 1);
REQUIRE(arguments["recursive"][0] == "true");
REQUIRE(arguments["filename"].size() == 2);
REQUIRE(arguments["filename"][0] == "file1");
REQUIRE(arguments["filename"][1] == "file2");
```
However if the user violates the scheme, an exception will be thrown.
For example, if the user violates max_count(1) constraint for "force" argument, an exception will be thrown:
```
myutil -f true -f true file1
Actual argument value counts mismatch. Argument force/f maximum expected 1, actual 2.
```
Same error will be thrown if the user forgets to specify a filename:
```
myutil -rf
Actual argument value counts mismatch. Argument filename minimum expected 1, actual 0.
```
The parser will also ensure that mutually exclusive scheme branches (XOR branches) were not taken:
```
myutil --force yes --force no file1 file2
Argument force/f specified with value 'no' conflicts with other argument values
```

Some features are not available yet, consult the issues for more info.
