/**
 * @file UrlParms.cc
 */
#include <FiltAlgVirtVol/UrlParms.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>
using std::vector;
using std::string;
using std::pair;

//------------------------------------------------------------------
UrlParms::UrlParms() : UrlParams(), _ok(false)
{
}

//------------------------------------------------------------------
UrlParms::UrlParms(const std::string &parmfileName) : UrlParams(), _ok(true)
{
  if (load(parmfileName.c_str(), NULL, TRUE, 0) != 0)
  {
    LOG(ERROR) << "Rading parm file " << parmfileName;
    _ok = false;
  }
  else
  {
    _init();
  }
}

//------------------------------------------------------------------
UrlParms::UrlParms(const UrlParams &P) : UrlParams(P), _ok(true)
{
  _init();
}

//------------------------------------------------------------------
UrlParms::~UrlParms()
{
}

//------------------------------------------------------------------
std::string UrlParms::sprintUrl(UrlParams::Url_t t)
{
  string s = "";
  switch (t)
  {
  case UrlParams::VOLUME:
    s = "VOLUME";
    break;
  case UrlParams::DATABASE:
    s = "DATABASE";
    break;
  case UrlParams::ASCII:
    s = "ASCII";
    break;
  default:
    s = "UNKNOWN";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string UrlParms::sprintData(UrlParams::Data_t t)
{
  string s = "";
  switch (t)
  {
  case UrlParams::GRID:
    s = "GRID";
    break;
  case UrlParams::VALUE:
    s = "VALUE";
    break;
  case UrlParams::NOT_SET:
  default:
    s = "NOT_SET";
    break;
  }
  return s;
}

//------------------------------------------------------------------
bool UrlParms::nameMatch(const std::string &name) const
{
  return find(_dataNames.begin(), _dataNames.end(), name) != _dataNames.end();
}

//------------------------------------------------------------------
bool UrlParms::_init(void)
{
  _ok = true;
  _dataNames.clear();
  for (int i=0; i<names_n; ++i)
  {
    _dataNames.push_back(_names[i]);
  }
  size_t maxLen = MDV_SHORT_FIELD_LEN-1;
  for (size_t i=0; i<_dataNames.size(); ++i)
  {
    if (_dataNames[i].size() > maxLen  && is_netCDF_output)
    {
      LOG(ERROR) << "Name too long, max netCDF length = " << maxLen
    		 << "   " << _dataNames[i];
      _ok = false;
    }
  }

  if (is_netCDF_output)
  {
    if (url_type != VOLUME)
    {
      LOG(ERROR) << "Need url_type = VOLUME for netCDF output, got " <<
	sprintUrl(url_type);
      _ok = false;
    }
    if (data_type != GRID)
    {
      LOG(ERROR) << "Need data_type = GRID for netCDF output, got " <<
	sprintData(data_type);
      _ok = false;
    }
  }
  return _ok;
}
