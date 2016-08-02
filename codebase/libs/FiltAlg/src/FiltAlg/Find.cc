// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 * @file Find.cc
 */
#include <FiltAlg/Find.hh>
#include <toolsa/LogStream.hh>

using std::string;
using std::vector;
using std::pair;

//------------------------------------------------------------------
Find::Find(void)
{
  _ok = false;
  _is_single = false;
}

//------------------------------------------------------------------
Find::Find(const string &s, const double vlevel_tolerance)
{
  _ok = true;
  _vlevel_tolerance = vlevel_tolerance;
  string::size_type n = s.size();
  string::size_type i=0;
  vector<string> tokens;
  while (i < n)
  {
    i = _nextToken(s, i, tokens);
  }
  int nt = static_cast<int>(tokens.size());
  _is_single = nt == 3;
  if (_is_single)
  {
    _single = FindSimple(tokens[0], tokens[1], tokens[2]);
    _ok = _single.ok();
  }    
  else
  {
    _tokenParse(tokens, 0, nt-1);
  }
  printTop();
}

//------------------------------------------------------------------
Find::Find(const vector<string> &token, const int ind0, const int ind1,
	   const double vlevel_tolerance)
{
  _ok = true;
  _vlevel_tolerance = vlevel_tolerance;
  _is_single = ind1-ind0+1 == 3;
  if (_is_single)
  {
    _single = FindSimple(token[ind0], token[ind0+1], token[ind1]);
    _ok = _single.ok();
  }
  else
  {
    _tokenParse(token, ind0, ind1);
  }
}

//------------------------------------------------------------------
Find::Find(const string &name, const string &comp, const string &value,
	   const double vlevel_tolerance)
{
  _is_single = true;
  _vlevel_tolerance = vlevel_tolerance;
  _single = FindSimple(name, comp, value);
  _ok = _single.ok();
}

//------------------------------------------------------------------
Find::~Find()
{
}

//------------------------------------------------------------------
void Find::printTop(void) const
{
  print();
  printf("\n");
}

//------------------------------------------------------------------
void Find::print(void) const
{
  if (_is_single)
  {
    _single.print();
  }
  else
  {
    for (int i=0; i<static_cast<int>(_multiple.size()); ++i)
    {
      printf("(");
      _multiple[i].first.print();
      printf(")");
      if (i < static_cast<int>(_multiple.size()) - 1)
      {
	printf(" %s ", logicalString(_multiple[i].second).c_str());
      }
    }
  }
}

//------------------------------------------------------------------
bool Find::isConsistent(const Comb &comb) const
{
  if (_is_single)
  {
    return _single.isConsistent(comb);
  }
  else
  {
    bool stat = true;
    for (int i=0; i<static_cast<int>(_multiple.size()); ++i)
    {
      if (!_multiple[i].first.isConsistent(comb))
      {
	stat = false;
      }
    }
    return stat;
  }
}

//------------------------------------------------------------------
bool Find::setPointers(const Comb &comb)
{
  if (_is_single)
  {
    return _single.setPointer(comb);
  }
  else
  {
    bool stat = true;
    for (int i=0; i<static_cast<int>(_multiple.size()); ++i)
    {
      if (!_multiple[i].first.setPointers(comb))
      {
	stat = false;
      }
    }
    return stat;
  }
}

//------------------------------------------------------------------
bool Find::satisfiesConditions(const int ipt, const double vlevel) const
{
  if (_is_single)
  {
    return _single.satisfiesCondition(ipt, vlevel, _vlevel_tolerance);
  }
  else
  {
    // implement left to right
    bool ret = _multiple[0].first.satisfiesConditions(ipt, vlevel);
    for (int i=1; i<static_cast<int>(_multiple.size()); ++i)
    {
      bool bi = _multiple[i].first.satisfiesConditions(ipt, vlevel);
      switch (_multiple[i-1].second)
      {
      case AND:
	ret = ret && bi;
	break;
      case OR:
	ret = ret || bi;
	break;
      default:
	LOG(ERROR) << "expect AND OR, got neither";
	break;
      }
    }
    return ret;
  }    
}

//------------------------------------------------------------------
Find::Logical_t Find::parseLogical(const string &s)
{
  if (s == "&" || s == "&&")
  {
    return AND;
  }
  else if (s == "|" || s == "||")
  {
    return OR;
  }
  else
  {
    return NONE;
  }
}

//------------------------------------------------------------------
string Find::logicalString(const Logical_t &l)
{
  string ret;
  switch (l)
  {
  case OR:
    ret = "|";
    break;
  case AND:
    ret = "&";
    break;
  case NONE:
  default:
    ret = "?";
    break;
  }
  return ret;
}

//------------------------------------------------------------------
string::size_type Find::_nextToken(const string &s, const string::size_type i,
				   vector<string> &tokens)
{
  string s1 = s.substr(i,1);
  if (s1 == " ")
  {
    return i+1;
  }

  string s2 = s.substr(i,2);
  if (s2 == "<=" || s2 == "<=" || s2 == ">=" || s2 == "==" || s2 == "&&" ||
      s2 == "||")
  {
    tokens.push_back(s2);
    return i+2;
  }

  if (s1 == "<" || s1 == ">" || s1 == "=" || s1 == "(" || s1 == ")" ||
      s1 == "&" || s1 == "|")
  {
    tokens.push_back(s1);
    return i+1;
  }

  string::size_type j;
  j = s.find_first_of("<>=()&| ", i);
  if (j == string::npos)
  {
    // all chars from i to end are one token
    tokens.push_back(s.substr(i));
    return s.size();
  }
  else
  {
    // all chars from i to j are one token
    tokens.push_back(s.substr(i,j-i));
    return j;
  }
}


//------------------------------------------------------------------
void Find::_tokenParse(const vector<string> &tokens, const int ind0,
			const int ind1)
{
  int k=ind0;
  while (k <= ind1)
  {
    if (tokens[k] == "(")
    {
      k = _parenParse(tokens, k, ind1);
    }
    else 
    {
      k = _comparisonParse(tokens, k, ind1);
    }
  }
}

//------------------------------------------------------------------
int Find::_parenParse(const vector<string> &tokens, const int leftParen,
		      const int ind1)
{
  int level = 1;
  int k2 = leftParen;
  while (level >= 1)
  {
    k2++;
    if (k2 > ind1)
    {
      LOG(ERROR) << "parens don't match";
      _ok = false;
      return ind1+1;
    }
    if (tokens[k2] == "(")
    {
      ++level;
    }
    else if (tokens[k2] == ")")
    {
      --level;
    }
  }

  // the stuff between parens:
  Find F(tokens, leftParen+1, k2-1, _vlevel_tolerance);
  return _finishOne(F, tokens, k2+1, ind1);
}

//------------------------------------------------------------------
int Find::_comparisonParse(const vector<string> &tokens, const int k, 
			   const int ind1)
{
  // should be a triple of name test value, then an operator (or end)
  if (k+2 >  ind1)
  {
    LOG(ERROR) << "not enough tokens Got " << ind1 << " looking for 3 at " << k;
    _ok = false;
    return ind1+1;
  }
  Find F(tokens[k], tokens[k+1], tokens[k+2], _vlevel_tolerance);
  return _finishOne(F, tokens, k+3, ind1);
}

//------------------------------------------------------------------
int Find::_finishOne(const Find &F, const vector<string> &tokens,
		     const int k2, const int ind1)
{
  if (!F.ok())
  {
    _ok = false;
    return ind1+1;
  }

  if (k2 >= ind1)
  {
    _multiple.push_back(pair<Find, Logical_t>(F, NONE));
    return k2+1;
  }
  else
  {
    // expect a match to a logical at position k2
    Logical_t l = parseLogical(tokens[k2]);
    if (l == NONE)
    {
      LOG(ERROR) << "expected a logical token (and or) and got " << tokens[k2]; 
      _ok = false;
      return ind1+1;
    }
    else
    {
      _multiple.push_back(pair<Find, Logical_t>(F, l));
      return k2+1;
    }
  }
}
