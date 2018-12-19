/**
 * @file LeafContent.cc
 */
#include <rapmath/LeafContent.hh>
#include <rapmath/VolumeData.hh>
#include <rapmath/MathData.hh>
#include <rapmath/MathLoopData.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
LeafContent::LeafContent() : _isVariable(false), _value(-99), _name("unknown"),
			     _missingData(false) {}

//------------------------------------------------------------------
LeafContent::LeafContent(const std::string &s) :
  _isVariable(true), _value(-99), _name(s),  _missingData(false)
{
  if (s == "PI" || s == "pi")
  {
    _isVariable = false;
    _value = 3.14159;
  }
  if (s == "missing")
  {
    _isVariable = false;
    _missingData = true;
  }
}

//------------------------------------------------------------------
LeafContent::LeafContent(const std::string &s, double v) :
  _isVariable(false), _value(v), _name(s),  _missingData(false)
{

}

//------------------------------------------------------------------
LeafContent::~LeafContent()
{
}

//------------------------------------------------------------------
void LeafContent::print(void) const
{
  if (_isVariable)
  {
    printf("%s", _name.c_str());
  }
  else
  {
    if (_missingData)
    {
      printf("missing");
    }
    else
    {
      printf("%lf", _value);
    }
  }
}

//------------------------------------------------------------------
bool LeafContent::getValue(const MathData *data,  int ipt, double &v) const
{
  if (!_isVariable)
  {
    if (_missingData)
    {
      return false;
    }
    v = _value;
    return true;
  }

  const MathLoopData *l = data->dataPtrConst(_name);
  if (l == NULL)
  {
    LOG(ERROR) << "No named data in data object for " << _name;
    return false;
  }
  else
  {
    return l->getVal(ipt, v);
  }
}

//------------------------------------------------------------------
bool LeafContent::getValue(double &v) const
{
  if (!_isVariable)
  {
    if (_missingData)
    {
      return false;
    }
    v = _value;
    return true;
  }
  else
  {
    LOG(ERROR) << "non-indexed method with indexed data";
    return false;
  }
}

//------------------------------------------------------------------
