# ADIOS2QueryParser

Use Makefile to build  `testQuery` executable. Examples are defined and passed to QueryParser in example `queryEx.cpp`.


### Sample

input: "(V/X < -10 or 0 <=V/Y < 5.03 )and (temp/^ != 0 or rad/% == 100) "

output:
```
Composite query (relation AND)
  Composite query (relation OR)
    QueryVar (V/X):
    ===> -10 (op <)
    QueryVar (V/Y):
    ===> 0 (op >=)
    ===> 5.03 (op <)
  Composite query (relation OR)
    QueryVar (temp/^):
    ===> 0 (op !=)
    QueryVar (rad/%):
    ===> 100 (op ==)
```
(These are ADIOS2 query objects approximated by `adiosheaders.hpp`.)

## Special Characters in Variable Names
Variable names are not parsed if they contain any of the following 7 characters `{' ', '(', ')', '<', '>', '!', '='}`, or if they begin with the character `'@'`. To use variables with these characters, pass a vector of variable names to the `parseQuery` function, and in the string query, replace variable names with '@' followed directly of the index of the variable of interest (0-based indexing, do not include a space).

For example:
```
  ex = "(@0 < -10 or 0 <=@1 < 5.03 )and (@2 != 0 or @3 == 100) ";
  std::vector<std::string> vars2 = {"V/(X)","V/<Y>","@temp/ ^","rad/!=%"};
  parseQuery(ex, &vars2);
```
output:
```
Composite query (relation AND)
  Composite query (relation OR)
    QueryVar (V/(X)):
    ===> -10 (op <)
    QueryVar (V/<Y>):
    ===> 0 (op >=)
    ===> 5.03 (op <)
  Composite query (relation OR)
    QueryVar (@temp/ ^):
    ===> 0 (op !=)
    QueryVar (rad/!=%):
    ===> 100 (op ==)
```
