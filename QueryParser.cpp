#ifndef QUERYPARSER_CPP_
#define QUERYPARSER_CPP_

#include <string>
#include <iostream>

#include "QueryParser.h"

void parseQuery (std::string strQuery, std::vector<std::string> *varNames)
{
  QueryParser parser (varNames);
  parser.parse(strQuery);
  std::cout << "\n*****************************" << std::endl;
  std::cout << "ParseQuery of input string..." << std::endl;
  std::cout << "\"" << strQuery << "\"" << std::endl;
  std::cout << "with varNames vector:" << std::endl;
  if (!varNames)
    {
      std::cout << "  (No varNames vector)" << std::endl;
    }
  else
    {
      std::cout << "{ ";
      for (std::string var: *varNames)
	std::cout << var << " ";
      std::cout << "}" << std::endl;
    }
  std::cout << "...complete. Printing:\n" << std::endl;
  parser.getRoot()->Print("");
}

/*
adios2::query::QueryBase* parseQuery (std::string strQuery)
{
  QueryParser parser;
  parser.parse(strQuery);
  return parser.getRoot();
}
*/

/*
  private:
  std::stack<adios2::query::QueryBase*> stack;
  std::string currString;
*/

QueryParser::QueryParser()
{
  stack.push(nullptr);
}

QueryParser::QueryParser(std::vector<std::string> *v)
  : varNames(v)
{
  stack.push(nullptr);
}

QueryParser::~QueryParser()
{
}

// void rotate child parent/ insert parent
// check for null parent
void QueryParser::rotate(adios2::query::QueryBase* parent)
{
  std::cout << "rotating stack of size " << stack.size() << std::endl;
  std::cout << "   current stack root:" << std::endl;
  if (stack.size() > 0)
    {
      stack.top()->Print("    "); // debugging
      adios2::query::QueryBase *child = stack.top();
      stack.pop();
      stack.push(parent);
      addChild(child);
    }
  else
    {
      // error
      std::cout << "Cannot rotate. Stack empty." << std::endl;
    }
}

// add child/replace null
void QueryParser::addChild(adios2::query::QueryBase* child)
{
  std::cout << "adding Child to stack of size " << stack.size() << std::endl;
  std::cout << "   current stack root:" << std::endl;
  adios2::query::QueryBase *parent = stack.top();
  if (!parent)
    {
      std::cout << "  no current root. Child becomes root:" << std::endl;
      child->Print("    "); // debugging
      stack.pop();
      stack.push(child);
    }
  else if (dynamic_cast<adios2::query::QueryComposite*>(parent))
    {
      parent->Print("    "); // debugging
      dynamic_cast<adios2::query::QueryComposite*>(parent)->AddNode(child);
      std::cout << " post-child root:" << std::endl;
      stack.top()->Print("    "); // debugging
    }
  else
    {
      // error
      std::cout << "Error adding child - parent node is not QueryComposite object." << std::endl;
    }
}

void QueryParser::parse(std::string strQuery)
{
  currString = strQuery;
  IgnoreSpaces();
  std::cout << "Parsing currString \"" << currString << "\"" << std::endl;
  std::cout << "Current stack top: ";
  if (stack.top())
    stack.top()->Print("");
  else
    std::cout << "(is nullptr)" << std::endl;

  if (IsLParen())
    {
      std::cout << "Is LParen" << std::endl;
      GetLParen();
      if (stack.top())
	stack.push(nullptr);
    }

  else if (IsRParen())
    {
      std::cout << "Is RParen" << std::endl;
      GetRParen();
      if (stack.size() > 1)
	{
	  adios2::query::QueryBase* child = stack.top();
	  stack.pop();
	  QueryParser::addChild(child);
	}
      else if (stack.top())
	{
	  stack.push(nullptr);
	}
      else
	{
	  // error?
	}
    }

  else if (IsNumber())
    {
      std::cout << "Is Number" << std::endl;
      std::string num = GetNumber();
      adios2::query::Op comp = GetCompare(true);
      std::string var = GetVarName();
      adios2::query::QueryVar *node = new adios2::query::QueryVar(var);
      node->m_RangeTree.AddLeaf(comp, num);

      if (IsCompare())
	{
	  adios2::query::Op comp2 = GetCompare(false);
	  std::string num2 = GetNumber();
	  node->m_RangeTree.AddLeaf(comp2, num2);
	}
      
      QueryParser::addChild(node);
    }

  else if (IsRelation())
    {
      std::cout << "Is Relation" << std::endl;
      adios2::query::Relation rel = GetRelation();
      // check for precedence
      adios2::query::QueryBase *parent = stack.top();
      if (!parent)
	{
	  stack.pop();
	  parent = stack.top();
	}
      if (dynamic_cast<adios2::query::QueryComposite*>(parent))
	{
	  adios2::query::QueryComposite* compParent = dynamic_cast<adios2::query::QueryComposite*>(parent);
	  adios2::query::Relation prevRel = compParent->GetRelation();
	  if ((prevRel == adios2::query::Relation::AND) and (rel == adios2::query::Relation::OR))
	    {
	      adios2::query::QueryComposite *node = new adios2::query::QueryComposite(rel);
	      QueryParser::rotate(node);	      
	    }
	  else if ((prevRel == adios2::query::Relation::OR) and (rel == adios2::query::Relation::AND))
	    {
	      adios2::query::QueryBase* child = compParent->GetLastChild();
	      adios2::query::QueryComposite *node;
	      if (dynamic_cast<adios2::query::QueryComposite*>(child))
		{
		  adios2::query::QueryComposite* compChild = dynamic_cast<adios2::query::QueryComposite*>(child);
		  adios2::query::Relation childRel = compChild->GetRelation();
		  if (childRel != rel)
		    {
		      node = new adios2::query::QueryComposite(rel);
		      node->AddNode(compChild);
		    }
		  else
		    {
		      node = compChild;
		    }
		  compParent->RemoveLastChild();
		  stack.push(node);
		}
	      else // QueryVar
		{
		  adios2::query::QueryComposite *node = new adios2::query::QueryComposite(rel);
		  node->AddNode(child);
		  compParent->RemoveLastChild();
		  stack.push(node);
		}
	    }
	  // if prevRel == rel, ignore (next query node will be added to parent)
	}
      else // QueryVar
	{
	  adios2::query::QueryComposite *node = new adios2::query::QueryComposite(rel);
	  node->AddNode(parent);
	  stack.pop();
	  stack.push(node);
	}
    }

  else if (IsVarName())
    {
      std::cout << "Is VarName" << std::endl;
      std::string var = GetVarName();
      adios2::query::Op comp = GetCompare(false);
      std::string num = GetNumber();
      adios2::query::QueryVar *node = new adios2::query::QueryVar(var);
      node->m_RangeTree.AddLeaf(comp, num);
      QueryParser::addChild(node);
    }

  else
    {
      // error
      std::cout << "Not sure what's next in string " << currString << std::endl;
      currString = currString.substr(1);
    }

  if (currString.size() > 0)
    {
      QueryParser::parse(currString);
    }
  else // finalize stack (merge into one node)
    {
      while (stack.size() > 1)
	{
	  adios2::query::QueryBase* top = stack.top();
	  stack.pop();
	  if ((dynamic_cast<adios2::query::QueryComposite*>(top) != nullptr) &
	      (dynamic_cast<adios2::query::QueryComposite*>(stack.top()) != nullptr) &
	      (dynamic_cast<adios2::query::QueryComposite*>(top)->GetRelation() == dynamic_cast<adios2::query::QueryComposite*>(stack.top())->GetRelation()))
	    {
	      for (adios2::query::QueryBase* q:dynamic_cast<adios2::query::QueryComposite*>(top)->m_Nodes)
		QueryParser::addChild(q);
	    }
	  else
	    QueryParser::addChild(top);
	}
    }
}

void QueryParser::IgnoreSpaces()
{
  size_t offset = 0;
  std::string ignore = " \t\n";
  while ((offset < currString.size() - 1) & (ignore.find(currString[offset]) != std::string::npos))
    {
      ++offset;
    }
  currString = currString.substr(offset);
}

bool QueryParser::IsVarName()
{
  return ((currString[0] >= 'a' & currString[0] <= 'z') | (currString[0] >= 'A' & currString[0] <= 'Z'))
    | (currString[0] == '@' & currString.size() > 1 & (currString[1] >= '0' & currString[1] <= '9'));
}

bool QueryParser::IsNumber()
{
  return ((currString[0] >= '0' & currString[0] <= '9')
	  | (currString[0] == '-' & currString.size() > 1
	     & (currString[1] >= '0' & currString[1] <= '9')));
}

bool QueryParser::IsCompare()
{
  std::string compare = "<>!=";
  return compare.find(currString[0]) != std::string::npos;
}

bool QueryParser::IsRelation()
{
  if (currString.length() < 2)
    return false;
  if (currString[0] == 'o' & currString[1] == 'r')
    return true;
  if (currString.length() < 3)
    return false;
  if (currString[0] == 'a' & currString[1] == 'n' & currString[2] == 'd')
    return true;
  if (currString[0] == 'n' & currString[1] == 'o' & currString[2] == 't')
    return true;
  return false;
}

bool QueryParser::IsLParen()
{
  return currString[0] == '(';
}

bool QueryParser::IsRParen()
{
  return currString[0] == ')';
}

std::string QueryParser::GetVarName()
{
  size_t nameLen = 0;
  bool charValid = true;
  std::string varname;
  std::string invalid = " ()<>!=";

  if (currString[0] == '@')
    {
      if (!varNames)
	{
	  // error
	  std::cout << "varNames is null. Cannot lookup variable." << std::endl;
	}
      size_t varIndex = 0;
      char charNext;
      while ((++nameLen < currString.length()) & charValid)
	{
	  charNext = currString[nameLen];
	  charValid = (charNext >= '0') & (charNext <= '9');
	  if (charValid)
	    varIndex = (varIndex * 10) + (charNext - '0');
	}
      if (!charValid)
	--nameLen;
      
      std::cout << "GetVarName looking for var " << varIndex << std::endl;
      if (varNames->size() > varIndex)
	varname = varNames->at(varIndex);
      else
	std::cout << "Can't find var. varNames size " << varNames->size() << std::endl;
    }
  else
    {
      while ((++nameLen < currString.length()) & charValid)
	{
	  charValid = invalid.find(currString[nameLen]) == std::string::npos;
	}
      if (!charValid)
	--nameLen;
      varname = currString.substr(0, nameLen);
    }
  
  currString = currString.substr(nameLen);
  IgnoreSpaces();
  return varname;
}

std::string QueryParser::GetNumber()
{
  size_t numLen = currString[0] == '-'? 1:0;
  bool decimalRead = false;
  bool charValid = true;

  while ((++numLen < currString.length()) & charValid)
    {
      if (currString[numLen] == '.')
	{
	  charValid = !decimalRead;
	  decimalRead = true;
	}
      else
	{
	  charValid = currString[numLen] >= '0' & currString[numLen] <= '9';
	}
    }
  if (!charValid)
    --numLen;

  std::string num = currString.substr(0, numLen);
  currString = currString.substr(numLen);
  IgnoreSpaces();
  return num;  
}

adios2::query::Op QueryParser::GetCompare(bool reverse)
{
  bool eq = currString.length() > 1 & currString[1] == '=';
  adios2::query::Op op;

  switch (currString[0]) {
  case '<':
    if (reverse)
      op = eq? adios2::query::Op::GE: adios2::query::Op::GT;
    else
      op = eq? adios2::query::Op::LE: adios2::query::Op::LT;
    break;
  case '>':
    if (reverse)
      op = eq? adios2::query::Op::LE: adios2::query::Op::LT;
    else
      op = eq? adios2::query::Op::GE: adios2::query::Op::GT;      
    break;
  case '=':
    if (eq)
      op = adios2::query::Op::EQ;
    break;
  case '!':
    if (eq)
      op = adios2::query::Op::NE;
    break;
  default:
    // error
    op = adios2::query::Op::EQ;
  }

  currString = currString.substr(eq? 2: 1);
  IgnoreSpaces();
  return op;
}

adios2::query::Relation QueryParser::GetRelation()
{
  size_t offset = 0;
  adios2::query::Relation rel;

  if (currString[0] == 'o' & currString[1] == 'r')
    {
      offset = 2;
      rel = adios2::query::Relation::OR;
    }
  else if (currString[0] == 'a' & currString[1] == 'n' & currString[2] == 'd')
    {
      offset = 3;
      rel = adios2::query::Relation::AND;
    }
  else if (currString[0] == 'n' & currString[1] == 'o' & currString[2] == 't')
    {
      offset = 3;
      rel = adios2::query::Relation::NOT;
    }

  currString = currString.substr(offset);
  IgnoreSpaces();
  return rel;
}

void QueryParser::GetLParen()
{
  currString = currString.substr(1);
  IgnoreSpaces();
}

void QueryParser::GetRParen()
{
  currString = currString.substr(1);
  IgnoreSpaces();
}

adios2::query::QueryBase* QueryParser::getRoot()
{
  if (stack.size() != 1)
    {
      // error
      std::cout << "Something went wrong - stack not compressed";
      while (stack.size() > 0)
	{
	  std::cout << "Stack size " << stack.size() << std::endl;
	  stack.top()->Print("");
	  stack.pop();
	}
      return nullptr;
    }
  return stack.top();
}

#endif
