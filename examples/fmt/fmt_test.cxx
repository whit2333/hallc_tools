#include <fmt/core.h>
#include <fmt/ostream.h>

R__LOAD_LIBRARY(libfmt.so)

void fmt_test() {

  fmt::print("Hello, {}!\n", "world");

  fmt::print("|{:<4.2f}|\n", 1.2345);

  fmt::print("{:<30}\n", "left aligned");
  //Result: "left aligned                  "
  fmt::print("{:>30}\n", "right aligned");
  // Result: "                 right aligned"
  fmt::print("{:^30}\n", "centered");
  // Result: "           centered           "
  fmt::print("{:*^30}\n", "centered");  // use '*' as a fill char
  // Result: "***********centered***********"

}


