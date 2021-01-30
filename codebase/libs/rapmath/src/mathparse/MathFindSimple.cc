/**
 * @file MathFindSimple.cc
 */
#include <rapmath/MathFindSimple.hh>
#include <rapmath/LogicalArgs.hh>
#include <toolsa/LogStream.hh>
using std::string;

//------------------------------------------------------------------
MathFindSimple::MathFindSimple(void)
{
  _ok = false;
}

//------------------------------------------------------------------
MathFindSimple::MathFindSimple(const string &name, const string &comp,
                               const string &value)
{
  _ok = true;
  _data = LeafContent(name);
  if (value == "missing")
  {
    _missingValue = true;
    _value = 0.0;
  }
  else
  {
    _missingValue = false;
    if (sscanf(value.c_str(), "%lf", &_value) != 1)
    {
      LOG(ERROR) << "scanning '" << value << "' as a numerical value";
      _value = 0.0;
      _ok = false;
    }
  }
  if (comp == ">")
  {
    _test = GT;
  }
  else if (comp == ">=")
  {
    _test = GE;
  }
  else if (comp == "<")
  {
    _test = LT;
  }
  else if (comp == "<=")
  {
    _test = LE;
  }
  else if (comp == "=" || comp == "==")
  {
    _test = EQ;
  }
  else
  {
    _ok = false;
    LOG(ERROR) << "unknown comparison operator " << comp;
    _test = GT;
  }
}

//------------------------------------------------------------------
MathFindSimple::~MathFindSimple(void)
{
}

//------------------------------------------------------------------
void MathFindSimple::print(void) const
{
  _data.print();
  if (_missingValue)
  {
    printf("%s Missing", comparisonString(_test).c_str());
  }
  else
  {
    printf(" %s %.5lf", comparisonString(_test).c_str(), _value);
  }
}

//------------------------------------------------------------------
bool MathFindSimple::satisfiesCondition(const MathData *data,
                                        const int ipt) const
{
  if (_missingValue)
  {
    if (_test != EQ)
    {
      return false;
    }
  }    
  double v;
  if (!_data.getValue(data, ipt, v))
  {
    // data missing at ipt
    if (_missingValue)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    if (_missingValue)
    {
      return false;
    }
    bool stat = false;
    switch (_test)
    {
      case GT:
        stat = v > _value;
        break;
      case GE:
        stat = v >= _value;
        break;
      case LT:
        stat = v < _value;
        break;
      case LE:
        stat = v <= _value;
        break;
      case EQ:
        stat = v == _value;
        break;
      default:
        stat = false;
        break;
    }
    return stat;
  }
}

//------------------------------------------------------------------
string MathFindSimple::comparisonString(const Compare_t &c)
{
  string ret;
  switch (c)
  {
    case GT:
      ret = ">";
      break;
    case GE:
      ret = ">=";
      break;
    case EQ:
      ret = "=";
      break;
    case LE:
      ret = "<=";
      break;
    case LT:
      ret = "<";
      break;
    default:
      ret = "?";
      break;
  }
  return ret;
}

//------------------------------------------------------------------
LogicalArg MathFindSimple::getLogicalArg(void) const
{
  return LogicalArg(_data.getName(), _value, _missingValue, _test);
}

//------------------------------------------------------------------
bool MathFindSimple::getSimpleCompare(std::string &compareName,
                                      MathFindSimple::Compare_t &c,
                                      double &compareV,
                                      bool &compareMissing) const
{
  compareName = _data.getName();
  c = _test;
  compareMissing = _missingValue;
  compareV = _value;
  return true;
}
