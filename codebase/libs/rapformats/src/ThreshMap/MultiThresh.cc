/**
 * @file MultiThresh.cc
 */

//------------------------------------------------------------------
#include <rapformats/MultiThresh.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <cstdio>

const std::string MultiThresh::_tag = "Lt";

//------------------------------------------------------------------
MultiThresh::MultiThresh(void) : _ok(false), _bias(-99.99),
				 _coldstart(true), _generatingTime(0),
				 _obsValue(-99.99), _fcstValue(-99.99)
{
}

//------------------------------------------------------------------
MultiThresh::MultiThresh(const std::string &xml,
			 const std::vector<std::string> &fields) :

  _ok(true)
{
  // read the thresholds array
  vector<string> vstring;
  if (TaXml::readStringArray(xml, FieldThresh::_tag, vstring))
  {
    LOG(ERROR) << "Reading tag as array " << FieldThresh::_tag;
    _ok = false;
  }
  else
  {
    if (vstring.size() != fields.size())
    {
      LOG(ERROR) << "Inconsistent number of fields "
		 << fields.size() << " " << vstring.size();
      _ok = false;
    }
    else
    {
      for (size_t i=0; i<vstring.size(); ++i)
      {
	FieldThresh f(vstring[i], fields[i]);
	if (!f.ok())
	{
	  _ok = false;
	}
	_thresh.push_back(f);
      }
    }

  }
  if (TaXml::readDouble(xml, "Bias", _bias))
  {
    LOG(ERROR) << "Parsing Bias XML as double";
    _ok = false;
  }

  if (TaXml::readBoolean(xml, "coldstart", _coldstart))
  {
    LOG(ERROR) << "no boolean coldstart";
    _ok = false;
  }
  if (_coldstart)
  {
    _generatingTime = 0;
    _obsValue = -99.99;
    _fcstValue = -99.99;
  }
  else
  {
    if (TaXml::readTime(xml, "generatingTime", _generatingTime))
    {
      LOG(ERROR) << "Reading generatingTime";
      _ok = false;
    }
    if (TaXml::readDouble(xml, "obsValue", _obsValue))
    {
      LOG(ERROR) << "Parsing obsValue XML as double";
      _ok = false;
    }
    if (TaXml::readDouble(xml, "fcstValue", _fcstValue))
    {
      LOG(ERROR) << "Parsing fcstValue XML as double";
      _ok = false;
    }
  }
}

//------------------------------------------------------------------
MultiThresh::
MultiThresh(const std::vector<FieldThresh> &fieldthresh) :
  _ok(true),
  _thresh(fieldthresh),
  _bias(-99.99),
  _coldstart(true),
  _generatingTime(0),
  _obsValue(-99.99),
  _fcstValue(-99.99)
{
}

//------------------------------------------------------------------
MultiThresh::
MultiThresh(const std::vector<FieldThresh> &fieldthresh,
	    double bias, const time_t &generatingTime,
	    double obsValue, double fcstValue) :
  _ok(true),
  _thresh(fieldthresh),
  _bias(bias),
  _coldstart(false),
  _generatingTime(generatingTime),
  _obsValue(obsValue),
  _fcstValue(fcstValue)
{
}

//------------------------------------------------------------------
MultiThresh::~MultiThresh()
{
}

//------------------------------------------------------------------
bool MultiThresh::update(const MultiThresh &item)
{
  if (_thresh.size() != item._thresh.size())
  {
    LOG(ERROR) << "Sizes don't match " << item._thresh.size() << " "
	       << _thresh.size();
    return false;
  }
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    if (!_thresh[i].fieldMatch(item._thresh[i]))
    {
      LOG(ERROR) << "Names do not match " << _thresh[i].getField() << ","
		 << item._thresh[i].getField();
      return false;
    }
    _thresh[i].setThreshFromInput(item._thresh[i]);
  }
  _coldstart = item._coldstart;
  _generatingTime = item._generatingTime;
  _bias = item._bias;
  _obsValue = item._obsValue;
  _fcstValue = item._fcstValue;
  return true;
}

//------------------------------------------------------------------
std::string MultiThresh::toXml(void) const
{
  string s = "";
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    s += _thresh[i].toXml();
  }    
  s += TaXml::writeDouble("Bias", 0, _bias, "%010.7lf");
  s += TaXml::writeBoolean("coldstart", 0, _coldstart);
  s += TaXml::writeTime("generatingTime", 0, _generatingTime);
  s += TaXml::writeDouble("obsValue", 0, _obsValue, "%010.7lf");
  s += TaXml::writeDouble("fcstValue", 0, _fcstValue, "%010.7lf");
  string ret = TaXml::writeString(_tag, 0, s);
  return ret;
}

//------------------------------------------------------------------
bool
MultiThresh::checkColdstart(const time_t &t,
			    int maxSecondsBeforeColdstart,
			    const std::vector<FieldThresh> &coldstartThresh)
{
  if (_coldstart)
  {
    // already set
    return true;
  }

  if (t - _generatingTime <= maxSecondsBeforeColdstart)
  {
    // not too old
    return true;
  }
  if (_thresh.size() != coldstartThresh.size())
  {
    LOG(ERROR) << "Sizes don't match " 
	       << coldstartThresh.size() << " "
	       << _thresh.size();
    return false;
  }
  bool good = true;
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    if (!_thresh[i].fieldMatch(coldstartThresh[i]))
    {
      LOG(ERROR) << "NAme mismatch " << _thresh[i].getField() << " "
		 << coldstartThresh[i].getField();
      good = false;
    }
  }
  if (!good)
  {
    return false;
  }
  _bias = -99.99;
  _coldstart = true;
  _generatingTime = 0;
  _thresh = coldstartThresh;
  _obsValue = -99.99;
  _fcstValue = -99.99;
  return true;
}

//------------------------------------------------------------------
void MultiThresh::print(int leadTime, bool verbose) const
{
  printf("        lt:%08d ", leadTime);
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    printf("%s ", _thresh[i].sprint().c_str());
  }
  printf("bias:%10.8lf ", _bias);
  if (_coldstart)
  {
    printf("Coldstart\n");
  }
  else
  {
    printf("ObsTime:%s",  DateTime::strn(_generatingTime).c_str());
    if (verbose)
    {
      printf(" ObsValue:%.8lf", _obsValue);
      printf(" FcstValue:%.8lf\n", _fcstValue);
    }
    else
    {
      printf("\n");
    }
  }
}


//------------------------------------------------------------------
bool MultiThresh::namesOk(const std::vector<std::string> &names,
			  bool printErrors) const
{
  if (names.size() != _thresh.size())
  {
    if (printErrors)
    {
      LOG(ERROR) << "Uneven sizes " << names.size() << " " 
		 << _thresh.size();
    }
    return false;
  }
  for (size_t i=0; i<names.size(); ++i)
  {
    if (_thresh[i].getField() != names[i])
    {
      if (printErrors)
      {
	LOG(ERROR) << "Name mismatch " << _thresh[i].getField() 
		   << " " << names[i];
      }
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
std::string MultiThresh::fieldName(int nameChars, int precision) const
{
  std::string ret = "";
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    ret += _thresh[i].dataFieldName(nameChars, precision);
  }
  return ret;
}

//------------------------------------------------------------------
void MultiThresh::update(double bias, const time_t &obsTime,
			 double obsValue, double fcstValue)
{
  _bias = bias;
  _coldstart = false;
  _generatingTime = obsTime;
  _obsValue = obsValue;
  _fcstValue = fcstValue;
}

//------------------------------------------------------------------
bool MultiThresh::getIthThreshold(int i, double &thresh) const
{
  if (i >= 0 && i < static_cast<int>(_thresh.size()))
  {
    thresh = _thresh[i].getThresh();
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool MultiThresh::get(const std::string &fieldName, FieldThresh &item) const
{
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    if (_thresh[i].nameMatch(fieldName))
    {
      item = _thresh[i];
      return true;
    }
  }
  return false;
}
  
