/**
 * @file MultiThreshFcstBiasMapping.cc
 */

//------------------------------------------------------------------
#include <rapformats/MultiThreshFcstBiasMapping.hh>
#include <rapformats/MultiThreshItem.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

const std::string MultiThreshFcstBiasMapping::_tag = "Forecast";

//------------------------------------------------------------------
MultiThreshFcstBiasMapping::
MultiThreshFcstBiasMapping(const std::string &xml,
			   const std::vector<std::string> &fields,
			   const std::vector<int> &leadSeconds) :
  _ok(true)
{
  // read in hour minute second
  if (TaXml::readInt(xml, "hour", _genHour))
  {
    LOG(ERROR) << "Reading tag hour";
    _ok = false;
  }
  if (TaXml::readInt(xml, "min", _genMinute))
  {
    LOG(ERROR) << "Reading tag min";
    _ok = false;
  }
  if (TaXml::readInt(xml, "sec", _genSecond))
  {
    LOG(ERROR) << "Reading tag sec";
    _ok = false;
  }
  
  // read in lead time array, and compare to input
  vector<string> vstring;
  if (TaXml::readStringArray(xml, MultiThresh::_tag, vstring))
  {
    LOG(ERROR) << "Reading tag as array " << MultiThresh::_tag;
    _ok = false;
    return;
  }
  if (vstring.size() != leadSeconds.size())
  {
    LOG(ERROR) << "Inconsistent lead second sizes "
	       << "input:" << leadSeconds.size()
	       << " xml:" << vstring.size();
    _ok = false;
    return;
  }

  // for every element in lead time array
  // parse it as a MultiThresh object
  for (size_t i=0; i<vstring.size(); ++i)
  {
    MultiThresh m(vstring[i], fields);
    _map[leadSeconds[i]] = m;
    if (!m.ok())
    {
      _ok = false;
    }
  }
}

//------------------------------------------------------------------
MultiThreshFcstBiasMapping::
MultiThreshFcstBiasMapping(int hour, int minute, int second,
			   const std::vector<int> &leadSeconds,
			   const std::vector<FieldThresh> &fieldThresh) :

  _ok(true),
  _genHour(hour), _genMinute(minute), _genSecond(second)
{
  for (size_t ilt=0; ilt<leadSeconds.size(); ++ilt)
  {
    _map[leadSeconds[ilt]] = MultiThresh(fieldThresh);
  }
}

//------------------------------------------------------------------
MultiThreshFcstBiasMapping::~MultiThreshFcstBiasMapping()
{
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::hmsMatch(const time_t &genTime) const
{
  DateTime dt(genTime);
  int h = dt.getHour();
  int m = dt.getMin();
  int s = dt.getSec();
  return hmsMatch(h, m, s);
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::hmsMatch(int h, int m, int s) const
{
  return h == _genHour && m == _genMinute && s == _genSecond;
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::update(const MultiThreshItem &item)
{
  return _map[item._leadTime].update(item._multiThresh);
}

//------------------------------------------------------------------
void MultiThreshFcstBiasMapping::
setColdstart(int leadTime, const std::vector<FieldThresh> &thresh)
{
  _map[leadTime] = MultiThresh(thresh);
}

//------------------------------------------------------------------
std::string MultiThreshFcstBiasMapping::toXml(void) const
{
  string s = TaXml::writeInt("hour", 0, _genHour, "%02d");
  s += TaXml::writeInt("min", 0, _genMinute, "%02d");
  s += TaXml::writeInt("sec", 0, _genSecond, "%02d");

  // this has a tag that makes it an array of 'Lt', need to pull that out
  // in constructor above
  std::map<int, MultiThresh>::const_iterator i;
  for (i = _map.begin(); i!= _map.end(); ++i)
  {
    s += i->second.toXml();
  }
  string ret = TaXml::writeString(_tag, 0, s);
  return ret;
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::
checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
	       const std::vector<FieldThresh> &coldstartThresh)
{
  bool ret = true;
  std::map<int, MultiThresh>::iterator i;
  for (i = _map.begin(); i!= _map.end(); ++i)
  {
    if (!i->second.checkColdstart(t, maxSecondsBeforeColdstart,
				  coldstartThresh))
    {
      ret = false;
    }
  }
  return ret;
}

//------------------------------------------------------------------
void MultiThreshFcstBiasMapping::print(bool verbose) const
{
  printf("     Gt_hms:%02d:%02d:%02d\n", _genHour, _genMinute, _genSecond);
  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    i->second.print(i->first, verbose);
  }
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::get(const time_t &genTime,
				     int leadTime,
				     MultiThreshItem &item) const
{
  DateTime dt(genTime);
  if (dt.getHour() == _genHour && dt.getMin() == _genMinute &&
      dt.getSec() == _genSecond)
  {
    const MultiThresh *mt = _mapFromLeadTime(leadTime);
    if (mt == NULL)
    {
      LOG(ERROR) << "Lead time not in state " << leadTime;
      return false;
    }    
    else
    {
      item = MultiThreshItem(*mt, _genHour, _genMinute, _genSecond, leadTime);
      return true;
    }
  }
  else
  {
    return false;
  }
}

const MultiThresh *
MultiThreshFcstBiasMapping::_mapFromLeadTime(int leadTime) const
{
  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (i->first == leadTime)
    {
      return &(i->second);
    }
  }
  return NULL;
}
      
