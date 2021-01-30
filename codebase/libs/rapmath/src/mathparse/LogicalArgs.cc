/**
 * @file LogicalArgs.cc
 */
#include <rapmath/LogicalArgs.hh>
#include <toolsa/LogStream.hh>

LogicalArg::LogicalArg(const std::string &name, double value, bool missingValue,
		       const MathFindSimple::Compare_t &test) :
  _variableName(name), _value(value), _valueIsMissing(missingValue), _op(test),
  _data(NULL)
{
}

bool LogicalArg::satisfiesCondition(int index) const
{
  double v;
  if (_valueIsMissing)
  {
    return !_data->getVal(index, v);
  }
  else
  {
    if (_data->getVal(index, v))
    {
      switch (_op)
      {
      case MathFindSimple::GT:
	return v > _value;
      case MathFindSimple::GE:
	return v >= _value;
      case MathFindSimple::EQ:
	return v == _value;
      case MathFindSimple::LE:
	return v <= _value;
      case MathFindSimple::LT:
	return v < _value;
      default:
	return false;
      }
    }
    else
    {
      return false;
    }
  }
}

bool LogicalArg::synch(MathData *rdata)
{
  MathLoopData *ldata = rdata->dataPtr(_variableName);
  if (ldata == NULL)
  {
    LOG(ERROR) << "No data for " << _variableName;
    return false;
  }

  _data = ldata;
  if (_valueIsMissing)
  {
    if (_op != MathFindSimple::EQ)
    {
      LOG(ERROR) << "Only equality for missing comparison";
      return false;
    }
    _value = ldata->getMissingValue();
  }
  return true;
}

void LogicalArgs::updateStatus(bool bi, int opIndex, bool &ret) const
{
  switch (_ops[opIndex])
  {
  case Find::AND:
    ret = ret && bi;
    break;
  case Find::OR:
    ret = ret || bi;
    break;
  case Find::NONE:
  default:
    ret = false;
    break;
  }
}
