/**
 * @file UrlSpec.cc
 */
#include <FiltAlgVirtVol/UrlSpec.hh>
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>

//------------------------------------------------------------------
UrlSpec::UrlSpec(const VirtVolParams::External_data_t &d) :
    _url(d.url), _type(d.url_type)
{
}

//------------------------------------------------------------------
UrlSpec::UrlSpec(const std::string &url,VirtVolParams::Url_t type) :
    _url(url), _type(type)
{
}

//------------------------------------------------------------------
UrlSpec::~UrlSpec(void)
{
}

//------------------------------------------------------------------
bool UrlSpec::add(const DataSpec &d)
{
  if (_allowed(d))
  {
    _data.push_back(d);
    return true;
  }
  else
  {
    LOG(ERROR) << "Invalid to have " << VirtVolParms::sprintUrl(_type) 
	       << " with " << VirtVolParms::sprintData(d._type);
    return false;
  }
}


//------------------------------------------------------------------
bool UrlSpec::internalNameMatch(const std::string &name) const
{
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._name.isInternalName(name))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool UrlSpec::externalNameMatch(const std::string &name) const
{
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._name.isExternalName(name))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool UrlSpec::external2Internal(const std::string externalName,
				std::string &internalName) const
{
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._name.isExternalName(externalName))
    {
      internalName = _data[i]._name._internal;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool UrlSpec::internal2External(const std::string internalName,
				std::string &externalName) const
{
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._name.isInternalName(internalName))
    {
      externalName = _data[i]._name._external;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
std::vector<std::string> UrlSpec::externalFieldNames(void) const
{
  std::vector<std::string> ret;
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._type == VirtVolParams::GRID)
    {
      ret.push_back(_data[i]._name._external);
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<std::string> UrlSpec::externalValueNames(void) const
{
  std::vector<std::string> ret;
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._type == VirtVolParams::VALUE)
    {
      ret.push_back(_data[i]._name._external);
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<std::string> UrlSpec::internalFieldNames(void) const
{
  std::vector<std::string> ret;
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._type == VirtVolParams::GRID)
    {
      ret.push_back(_data[i]._name._internal);
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<std::string> UrlSpec::internalValueNames(void) const
{
  std::vector<std::string> ret;
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._type == VirtVolParams::VALUE)
    {
      ret.push_back(_data[i]._name._internal);
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<NamePair> UrlSpec::fieldNames(void) const
{
  std::vector<NamePair> ret;
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._type == VirtVolParams::GRID)
    {
      ret.push_back(_data[i]._name);
    }
  }
  return ret;
}
//------------------------------------------------------------------
std::vector<NamePair> UrlSpec::valueNames(void) const
{
  std::vector<NamePair> ret;
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (_data[i]._type == VirtVolParams::VALUE)
    {
      ret.push_back(_data[i]._name);
    }
  }
  return ret;
}

//------------------------------------------------------------------
bool UrlSpec::_allowed(const DataSpec &d) const
{
  bool ret = true;
  switch (_type)
  {
  case VirtVolParams::VIRTUAL_VOLUME:
    // anything goes
    ret = true;
    break;
  case VirtVolParams::DATABASE:
  case VirtVolParams::ASCII:
    // must be non-gridded output
    ret = (d._type != VirtVolParams::GRID);
    break;
  default:
    // nothing will work
    ret = false;
  }

  return ret;
}
