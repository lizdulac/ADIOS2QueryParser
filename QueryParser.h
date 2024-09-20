#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include "adiosheaders.hpp"

class QueryParser
{
public:
  QueryParser();
  QueryParser(std::vector<std::string>* varNames);
  ~QueryParser();

  void rotate(adios2::query::QueryBase* parent);
  void addChild(adios2::query::QueryBase* child);
  
  void parse(std::string strQuery);

  void IgnoreSpaces();
  
  bool IsVarName();
  bool IsNumber();
  bool IsCompare();
  bool IsRelation();
  bool IsLParen();
  bool IsRParen();

  std::string GetVarName();
  std::string GetNumber();
  adios2::query::Op GetCompare(bool reverse);
  adios2::query::Relation GetRelation();
  void GetLParen();
  void GetRParen();

  adios2::query::QueryBase* getRoot();

private:
  std::stack<adios2::query::QueryBase*> stack;
  std::string currString;
  std::vector<std::string> *varNames;
};

void parseQuery (std::string strQuery, std::vector<std::string> *varNames = nullptr);

/*
adios2::query::QueryBase* parseQuery (std::string strQuery)
{
  QueryParser parser;
  parser.parse(strQuery);
  return parser.getRoot();
}
*/
#endif
