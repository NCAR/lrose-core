/**
 * @file Find.cc
 */
#include <rapmath/Find.hh>
#include <rapmath/LogicalArgs.hh>
#include <rapmath/MathParser.hh>
#include <toolsa/LogStream.hh>

using std::string;
using std::vector;
using std::pair;

//------------------------------------------------------------------
Find::Find(void)
{
  _ok = false;
  _isSimple = false;
  _pattern = UNKNOWN;
}

//------------------------------------------------------------------
Find::Find(const string &sInp)
{
  _ok = true;
  string s = sInp;

  // first thing..remove leading and training parens
  MathParser::parenRemove(s);
  
  string::size_type n = s.size();
  string::size_type i=0;
  vector<string> tokens;
  while (i < n)
  {
    i = _nextToken(s, i, tokens);
  }
  int nt = static_cast<int>(tokens.size());
  _isSimple = nt == 3;
  if (_isSimple)
  {
    _simple = MathFindSimple(tokens[0], tokens[1], tokens[2]);
    if (_simple.isSimpleCompareToNumber())
    {
      _pattern = SIMPLE_COMPARE_TO_NUMBER;
    }
    else
    {
      _pattern = COMPLEX;
    }
    _ok = _simple.ok();
  }    
  else
  {
    _tokenParse(tokens, 0, nt-1);
    _pattern = SIMPLE_MULTIPLES;
    for (size_t i=0; i<_multiple.size(); ++i)
    {
      if (!(_multiple[i].first._isSimple))
      {
	_pattern = COMPLEX;
	break;
      }
    }	
  }
  // printTop();
}

//------------------------------------------------------------------
Find::Find(const vector<string> &token, const int ind0, const int ind1)
{
  _ok = true;
  _isSimple = ind1-ind0+1 == 3;
  if (_isSimple)
  {
    _simple = MathFindSimple(token[ind0], token[ind0+1], token[ind1]);
    _ok = _simple.ok();
  }
  else
  {
    _tokenParse(token, ind0, ind1);
  }
}

//------------------------------------------------------------------
Find::Find(const string &name, const string &comp, const string &value)
{
  _isSimple = true;
  _simple = MathFindSimple(name, comp, value);
  _ok = _simple.ok();
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
  if (_isSimple)
  {
    _simple.print();
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
bool Find::satisfiesConditions(const MathData *data, int ipt) const
{
  if (_isSimple)
  {
    return _simple.satisfiesCondition(data, ipt);
  }
  else
  {
    // implement left to right
    bool ret = _multiple[0].first.satisfiesConditions(data, ipt);
    for (int i=1; i<static_cast<int>(_multiple.size()); ++i)
    {
      bool bi = _multiple[i].first.satisfiesConditions(data, ipt);
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
void Find::fields(std::vector<std::string> &names) const
{
  if (_isSimple)
  {
    names.push_back(_simple.getName());
  }
  else
  {
    for (size_t i=0; i<_multiple.size(); ++i)
    {
      _multiple[i].first.fields(names);
    }
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
bool Find::getSimpleCompare(std::string &compareName,
			    MathFindSimple::Compare_t &c,
			    double &compareV,
			    bool &compareMissing) const
{
  if (_isSimple)
  {
    return _simple.getSimpleCompare(compareName, c, compareV, compareMissing);
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool Find::getMultiCompare(LogicalArgs &args) const
{
  args = LogicalArgs();
  if (_isSimple)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  else
  {
    if (_pattern != SIMPLE_MULTIPLES)
    {
      LOG(ERROR) << "Wrong pattern";
      return false;
    }
    else
    {
      for (size_t i=0; i<_multiple.size(); ++i)
      {
	LogicalArg a;
	if (_multiple[i].first._isSimple)
	{
	  a = _multiple[i].first._simple.getLogicalArg();
	  args.appendArg(a);
	}
	else
	{
	  LOG(ERROR) << "Not good";
	  return false;
	}
	if (i+1 != _multiple.size())
	{
	  args.appendOp(_multiple[i].second);
	}
      }
    }
  }
  return true;
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
  Find F(tokens, leftParen+1, k2-1);
  _pattern = F.pattern();
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
  Find F(tokens[k], tokens[k+1], tokens[k+2]);
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
