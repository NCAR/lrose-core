/**
 * @file NsslData.cc
 */
 
#include "NsslData.hh"
#include "Params.hh"
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
#include <sys/types.h>
#include <sys/stat.h>
using std::string;

//---------------------------------------------------------------------------
NsslData::NsslData(const Params &params, std::string &path) : _ok(true),
							      _fullPath(path)
{
  string header = params.input_dir;

  // the file path is of form:
  // <header>/<field>/yyyymmdd/yyyymmdd-hhmmss-xx.xx.netcdf
  // (xx.xx = elevation angle)
  size_t i = path.find(header);
  if (i != 0)
  {
    LOG(ERROR) << "Did not see header " << header << " in string " << path;
    _ok = false;
    return;
  }
  
  size_t pos0 = header.size();
  i = path.find("/", pos0);
  if (i != header.size())
  {
    LOG(ERROR) << "Did not see '/' in string";
    _ok = false;
    return;
  }    
  string s = path.substr(i+1);
  i = s.find("/");
  if (i == string::npos)
  {
    LOG(ERROR) << "Did not see '/'";
    _ok = false;
    return;
  }

  _field = s.substr(0, i);

  s = s.substr(i+1);
  i = s.find("/");
  s = s.substr(i+1);
  
  // parse out yyyymmdd-hhmmss-xx.xx
  int  y, m, d, h, min, sec;
  if (sscanf(s.c_str(), "%04d%02d%02d-%02d%02d%02d-%lf", 
	     &y, &m, &d, &h, &min, &sec, &_elev) != 7)
  {
    LOG(ERROR) << "Parsing contents of string " << s;
    _ok = false;
    return;
  }

  DateTime dt(y, m, d, h, min, sec);
  _time = dt.utime();

  LOG(DEBUG) << _field << ",t=" << y << m << d << h << min << sec 
	     << ",elev=" << _elev;
}

//---------------------------------------------------------------------------
NsslData::~NsslData()
{
}

//---------------------------------------------------------------------------
bool NsslData::pathAdjust(const std::string &input_dir)
{
  struct stat file_stat;
  
  if (stat(_fullPath.c_str(), &file_stat) != 0)
  {
    _fullPath = input_dir + "/" + _fullPath;
    if (stat(_fullPath.c_str(), &file_stat) != 0)
    {
      LOG(ERROR) << "Specified sweep file doesn't exist: " << _fullPath;
      return false;
    }
  }
  return true;
}

//---------------------------------------------------------------------------
bool NsslData::isMatch(const std::string &name, const time_t &time,
		       double elev) const
{
  return name == _field && time == _time && _elev == elev;
}
