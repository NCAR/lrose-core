/**
 * @file SpecialUserData.cc
 */

#include <rapmath/SpecialUserData.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
SpecialUserData::SpecialUserData(bool ownsPointers) :
  _ownsPointers(ownsPointers)
{
}

//------------------------------------------------------------------
SpecialUserData::~SpecialUserData(void)
{
  if (_ownsPointers)
  {
    Special_t::iterator i;
    for (i=_special.begin(); i!= _special.end(); ++i)
    {
      if (i->second != NULL)
      {
	delete i->second;
	i->second = NULL;
      }
    }
  }
}

//------------------------------------------------------------------
bool SpecialUserData::hasName(const std::string &name) const
{
  return (_special.find(name) != _special.end());
}

//------------------------------------------------------------------
MathUserData *SpecialUserData::matchingDataPtr(const std::string &name)
{
  Special_iterator_t i;
  i = _special.find(name);
  if (i == _special.end())
  {
    LOG(ERROR) << "out of whack";
    return NULL;
  }
  return i->second;
}

//------------------------------------------------------------------
const MathUserData *
SpecialUserData::matchingDataPtrConst(const std::string &name) const
{
  Special_const_iterator_t i;
  i = _special.find(name);
  if (i == _special.end())
  {
    LOG(ERROR) << "out of whack";
    return NULL;
  }
  return i->second;
}

//------------------------------------------------------------------
bool SpecialUserData::store(const std::string &name, MathUserData *v)
{
  if (hasName(name))
  {
    LOG(ERROR) << "double storing of " << name;
    return false;
  }
  else
  {
    _special[name] = v;
    return true;
  }
}

