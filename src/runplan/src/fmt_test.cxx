#include <iostream>

#include <fmt/core.h>

#include <fmt/ostream.h>

R__LOAD_LIBRARY(libfmt.so)
 
void fmt_test() {

fmt::print(
    std::cout, "{:>5.3f}, {:>5.3f}\n", 1.0,2.0);

  std::cout << " soihasdfoisdoafij aosoaijweoifjaoefwoijaseofi j" << "oiajsdfoiajsidfoijsadflkj" <<
  "\n";

}

