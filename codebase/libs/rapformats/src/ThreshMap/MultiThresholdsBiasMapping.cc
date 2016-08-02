/**
 * @file MultiThresholdsBiasMapping.cc
 */

//------------------------------------------------------------------
#include <rapformats/MultiThresholdsBiasMapping.hh>
#include <rapformats/MultiThreshItem.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>
#include <cstdio>
using std::string;
using std::vector;


//------------------------------------------------------------------
MultiThresholdsBiasMapping::MultiThresholdsBiasMapping(void)
{
}

//------------------------------------------------------------------
MultiThresholdsBiasMapping::
MultiThresholdsBiasMapping(const std::vector<double> &ltHours,
			   const std::vector<std::string> &fields)
{
  // _url = path;
  _fields = fields;
  for (size_t i=0; i<ltHours.size(); ++i)
  {
    _leadSeconds.push_back(static_cast<int>(ltHours[i]*3600.0));
  }

  // don't know anything about gen times yet
}

//------------------------------------------------------------------
MultiThresholdsBiasMapping::~MultiThresholdsBiasMapping()
{
}

//------------------------------------------------------------------
void MultiThresholdsBiasMapping::clearMapping(void)
{
  _fields.clear();
  _leadSeconds.clear();
  _fcst.clear();
}

//------------------------------------------------------------------
bool
MultiThresholdsBiasMapping::setColdStart(int genFrequencySeconds,
					 const std::vector<FieldThresh> &thresh)
{
  if (!_fieldThreshNamesOk(thresh))
  {
    LOG(ERROR) << "Mismatch";
    return false;
  }


  _fcst.clear();
  for (double hms = 0.0; hms < 24.0*3600.0; 
       hms += (double)genFrequencySeconds)
  {
    int hour = static_cast<int>(hms/3600.0);
    int min = static_cast<int>((hms - static_cast<double>(hour)*3600.0)/60.0);
    int sec = static_cast<int>((hms - static_cast<double>(hour)*3600.0 -
				static_cast<double>(min)*60.0)/60.0);
    _fcst.push_back(MultiThreshFcstBiasMapping(hour, min, sec,
					       _leadSeconds, thresh));
  }
  return true;
}
    
//------------------------------------------------------------------
bool MultiThresholdsBiasMapping::
setColdStart(const time_t &genTime, int leadTime,
	     const std::vector<FieldThresh> &fieldThresh)
{
  if (!_fieldThreshNamesOk(fieldThresh))
  {
    LOG(ERROR) << "Mismatch";
    return false;
  }

  vector<int>::const_iterator index;
  index = find(_leadSeconds.begin(), _leadSeconds.end(), leadTime);
  if (index == _leadSeconds.end())
  {
    LOG(ERROR) << "Lead time not on list " << leadTime;
    return false;
  }
  for (size_t i=0; i<_fcst.size(); ++i)
  {
    if (_fcst[i].hmsMatch(genTime))
    {
      _fcst[i].setColdstart(leadTime, fieldThresh);
      return true;
    }
  }

  LOG(ERROR) << "Gen time not in state " << DateTime::strn(genTime);
  return false;
}


//--------------------------------------------------------------------
bool
MultiThresholdsBiasMapping::update(const MultiThreshItem &item)
{
  if (!item._multiThresh.namesOk(_fields))
  {
    LOG(ERROR) << "Mismatch";
    return false;
  }

  vector<int>::const_iterator index;
  index = find(_leadSeconds.begin(), _leadSeconds.end(), item._leadTime);
  if (index == _leadSeconds.end())
  {
    LOG(ERROR) << "Lead time not on list " << item._leadTime;
    return false;
  }
  for (size_t i=0; i<_fcst.size(); ++i)
  {
    if (_fcst[i].hmsMatch(item._genHour, item._genMin, item._genSec))
    {
      return _fcst[i].update(item);
    }
  }
  LOG(ERROR) << "Gen time not found in state hms=" 
	     << item._genHour << "," << item._genMin << "," << item._genSec;
  return false;
}

//------------------------------------------------------------------
std::string MultiThresholdsBiasMapping::toXml(void) const
{
  string ret = _fieldsToXml();
  ret += _leadsToXml();
  ret += _mappingsToXml();
  return ret;
}

//------------------------------------------------------------------
bool MultiThresholdsBiasMapping::fromXml(const std::string &xml,
					 bool fieldsAndLeadSet)
{
  vector<string> vstring;
  string str;

  vstring = _fieldsFromXml(xml);
  if (fieldsAndLeadSet)
  {
    if (vstring != _fields)
    {
      LOG(ERROR) << "Fields in XML does not match local state";
      return false;
    }
  }
  else
  {
    _fields = vstring;
  }

  vector<int> leads = _leadSecondsFromXml(xml);
  if (fieldsAndLeadSet)
  {
    if (leads != _leadSeconds)
    {
      LOG(ERROR) << "Lead Times in XML does not match local state";
      return false;
    }
  }
  else
  {
    _leadSeconds = leads;
  }

  if (TaXml::readStringArray(xml, MultiThreshFcstBiasMapping::_tag, vstring))
  {
    LOG(ERROR) << "String array tag missing, " 
	       << MultiThreshFcstBiasMapping::_tag;
    return false;
  }
  
  _fcst.clear();
  for (size_t i=0; i<vstring.size(); ++i)
  {
    MultiThreshFcstBiasMapping m(vstring[i], _fields, _leadSeconds);
    if (m.ok())
    {
      _fcst.push_back(m);
    }
    else
    {
      _fcst.clear();
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
bool
MultiThresholdsBiasMapping::
checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
	       const std::vector<FieldThresh> &coldstartThresh)
{
  bool ret = true;
  for (size_t i=0;i<_fcst.size(); ++i)
  {
    if (!_fcst[i].checkColdstart(t, maxSecondsBeforeColdstart,
				 coldstartThresh))
    {
      ret = false;
    }
  }
  return ret;
}

//------------------------------------------------------------------
void MultiThresholdsBiasMapping::printState(const time_t &t,
					    bool verbose) const
{
  printf("---------Threshold/bias information %s ----------\n",
	 DateTime::strn(t).c_str());
  printf("Fields:");
  for (size_t i=0; i<_fields.size(); ++i)
  {
    printf("%s ", _fields[i].c_str());
  }
  printf("\nLeadtimes:");
  for (size_t i=0; i<_leadSeconds.size(); ++i)
  {
    printf("%d,", _leadSeconds[i]);
  }
  printf("\n");
  for (size_t i=0; i<_fcst.size(); ++i)
  {
    _fcst[i].print(verbose);
  }
  printf("-----End Threshold/bias information ------------------\n");
}

//------------------------------------------------------------------
bool MultiThresholdsBiasMapping::get(const time_t &genTime,
				     int leadTime,
				     MultiThreshItem &item) const
{
  if (find(_leadSeconds.begin(), _leadSeconds.end(),
	   leadTime) == _leadSeconds.end())
  {
    LOG(ERROR) << "Lead time not found in state " << leadTime;
    return false;
  }
  for (size_t i=0; i<_fcst.size(); ++i)
  {
    if (_fcst[i].get(genTime, leadTime, item))
    {
      return true;
    }
  }
  LOG(ERROR) << "No matching gen time in state " << DateTime::strn(genTime);
  return false;
}


//------------------------------------------------------------------
bool MultiThresholdsBiasMapping::get(const time_t &genTime,
				     int leadTime, const std::string &fieldName,
				     FieldThresh &item) const
{
  MultiThreshItem m;

  if (get(genTime, leadTime, m))
  {
    return m.get(fieldName, item);
  }
  else
  {
    LOG(ERROR) << "No data at " << DateTime::strn(genTime) 
	       << " + " << leadTime;
    return false;
  }
}

//------------------------------------------------------------------
std::vector<std::string>
MultiThresholdsBiasMapping::_fieldsFromXml(const std::string &xml)
{
  std::vector<std::string> ret;

  string str;
  if (TaXml::readString(xml, "Fields", str))
  {
    LOG(ERROR) << "No XML with key Fields";
    return ret;
  }
  if (TaXml::readStringArray(str, "field", ret))
  {
    LOG(ERROR) << "No XML string array with key field";
  }
  return ret;
}


//------------------------------------------------------------------
std::vector<int>
MultiThresholdsBiasMapping::_leadSecondsFromXml(const std::string &xml)
{
  std::vector<int> ret;

  string str;
  if (TaXml::readString(xml, "Lead", str))
  {
    LOG(ERROR) << "No XML with key Lead";
    return ret;
  }

  std::vector<std::string> vstring;
  if (TaXml::readStringArray(str, "lt", vstring))
  {
    LOG(ERROR) << "No XML string array with key lt";
  }
  for (size_t i=0; i<vstring.size(); ++i)
  {
    int lt;
    if (sscanf(vstring[i].c_str(), "%d", &lt) != 1)
    {
      LOG(ERROR) << "Scanning " << vstring[i] << " As an int";
      ret.clear();
      return ret;
    }
    ret.push_back(lt);
  }
  return ret;
}

//------------------------------------------------------------------
std::string MultiThresholdsBiasMapping::_fieldsToXml(void) const
{
  string s = "";
  for (size_t i=0; i<_fields.size(); ++i)
  {
    s += TaXml::writeString("field", 0, _fields[i]);
  }
  string ret = TaXml::writeString("Fields", 0, s);
  return ret;
}

//------------------------------------------------------------------
std::string MultiThresholdsBiasMapping::_leadsToXml(void) const
{
  string s = "";
  for (size_t i=0; i<_leadSeconds.size(); ++i)
  {
    s += TaXml::writeInt("lt", 0, _leadSeconds[i], "%08d");
  }
  string ret = TaXml::writeString("Lead", 0, s);
  return ret;
}

//------------------------------------------------------------------
std::string MultiThresholdsBiasMapping::_mappingsToXml(void) const
{
  string s = "";

  for (size_t i=0; i<_fcst.size(); ++i)
  {
    s += _fcst[i].toXml();
  }
  return s;
}
//------------------------------------------------------------------
bool MultiThresholdsBiasMapping::
_fieldThreshNamesOk(const std::vector<FieldThresh> &fieldThresh) const
{
  if (fieldThresh.size() != _fields.size())
  {
    LOG(ERROR) << "Uneven sizes " << fieldThresh.size() << " " 
               << _fields.size();
    return false;
  }
  for (size_t i=0; i<fieldThresh.size(); ++i)
  {
    if (fieldThresh[i].getField() != _fields[i])
    {
      LOG(ERROR) << "Name mismatch " << fieldThresh[i].getField() 
                 << " " << _fields[i];
      return false;
    }
  }
  return true;
}



