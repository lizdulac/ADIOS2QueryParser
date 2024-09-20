#include <string>
#include <iostream>

#include "QueryParser.h"
//#include "adiosheaders.hpp"

int main()
{
  std::string ex = "B/x > 1.0 and (0 < B/y <= 0.5 or  -1 <= B/z < 1)";
  /*
  adios2::query::QueryBase* result = parseQuery(ex);
  result->Print();
  */
  parseQuery(ex);

  std::cout << "\n\nTest 2" << std::endl;
  ex = "(@0 == 5.0 and @6 != 4) or -10 <= @10 < 2";
  std::vector<std::string> vars = {"varZero", "varOne", "varTwo", "varThree", "varFour", "varFive", "varSix", "varSeven", "varEight", "varNine", "varTen", "varEleven"};
  parseQuery(ex, &vars);

  return 0;
}
