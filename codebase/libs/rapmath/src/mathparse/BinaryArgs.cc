#include <rapmath/BinaryArgs.hh>
#include <rapmath/MathLoopData.hh>

BinaryArg::BinaryArg(const std::string &name)
{
  _isVariable = true;
  _variableName = name;
  _value = 0;
  _valueIsMissing = false;
  _data = NULL;
}

BinaryArg::BinaryArg(double value, bool missing)
{
  _isVariable = false;
  _variableName = "none";
  _value = value;
  _valueIsMissing = missing;
  _data = NULL;
}

bool BinaryArg::value(int index, double &v) const
{
  if (_isVariable)
  {
    if (_data != NULL)
    {
      return _data->getVal(index, v);
    }
    else
    {
      return false;
    }
  }
  else
  {
    if (_valueIsMissing)
    {
      return false;
    }
    else
    {
      v = _value;
      return true;
    }
  }
}
