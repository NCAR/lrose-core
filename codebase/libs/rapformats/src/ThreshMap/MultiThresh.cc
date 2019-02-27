/**
 * @file MultiThresh.cc
 */

//------------------------------------------------------------------
#include <rapformats/MultiThresh.hh>
#include <rapformats/TileInfo.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <cstdio>
#include <algorithm>

const std::string MultiThresh::_tag = "TileData";

//------------------------------------------------------------------
MultiThresh::MultiThresh(void) : _ok(false), _bias(-99.99),
				 _coldstart(true), _motherTile(false),
				 _generatingTime(0), _obsValue(-99.99),
				 _fcstValue(-99.99)
{
}

//------------------------------------------------------------------
MultiThresh::MultiThresh(const std::string &xml,
			 const std::vector<std::string> &fields,
			 int &tileIndex) :
  _ok(true)
{
  // read the thresholds array
  vector<string> vstring;
  bool oldFormat = false;
  if (TaXml::readStringArray(xml, FieldThresh2::_tag2, vstring))
  {
    // try the older format
    if (TaXml::readStringArray(xml, FieldThresh::_tag, vstring))
    {
      LOG(ERROR) << "Reading tag as array using " << FieldThresh2::_tag2 
		 << " or " << FieldThresh::_tag;
      _ok = false;
    }
    else
    {
      oldFormat = true;
    }
  }
  if (_ok)
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
	if (oldFormat)
	{
	  FieldThresh f(vstring[i], fields[i]);
	  if (!f.ok())
	  {
	    _ok = false;
	  }
	  FieldThresh2 f2(f);
	  _thresh.push_back(f2);
	}
	else
	{
	  FieldThresh2 f(vstring[i], fields[i]);
	  if (!f.ok())
	  {
	    _ok = false;
	  }
	  _thresh.push_back(f);
	}
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
  if (TaXml::readBoolean(xml, "motherTile", _motherTile))
  {
    LOG(ERROR) << "no boolean motherTile";
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
  if (TaXml::readInt(xml, "tileIndex", tileIndex))
  {
    LOG(ERROR) << "No tileIndex tag";
    _ok = false;
  }
}

//------------------------------------------------------------------
MultiThresh::
MultiThresh(const std::vector<FieldThresh2> &fieldthresh, bool fromMother) :
  _ok(true),
  _thresh(fieldthresh),
  _bias(-99.99),
  _coldstart(true),
  _motherTile(fromMother),
  _generatingTime(0),
  _obsValue(-99.99),
  _fcstValue(-99.99)
{
}

//------------------------------------------------------------------
MultiThresh::
MultiThresh(const std::vector<FieldThresh2> &fieldthresh,
	    double bias, const time_t &generatingTime,
	    double obsValue, double fcstValue, bool fromMother) :
  _ok(true),
  _thresh(fieldthresh),
  _bias(bias),
  _coldstart(false),
  _motherTile(fromMother),
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
    _thresh[i].setThresh2FromInput(item._thresh[i]);
  }
  _coldstart = item._coldstart;
  _motherTile = item._motherTile;
  _generatingTime = item._generatingTime;
  _bias = item._bias;
  _obsValue = item._obsValue;
  _fcstValue = item._fcstValue;
  return true;
}

//------------------------------------------------------------------
bool MultiThresh::filterFields(const std::vector<std::string> &fieldNames)
{
  vector<FieldThresh2> newThresh;

  for (size_t i=0; i<fieldNames.size(); ++i)
  {
    int k = getThresholdIndex(fieldNames[i]);
    if (k < 0)
    {
      LOG(ERROR) << "Field Not found, cannot filter " << fieldNames[i];
      return false;
    }
    else
    {
      newThresh.push_back(_thresh[k]);
    }
  }
  _thresh = newThresh;
  return true;
}

//------------------------------------------------------------------
bool MultiThresh::replaceValues(const MultiThresh &filtMap,
				const std::vector<std::string> &filterFields)
{
  bool ret = true;
  for (size_t i=0; i<filterFields.size(); ++i)
  {
    if (!_replaceValue(filterFields[i], filtMap))
    {
      ret = false;
    }
  }
  return ret;
}

//------------------------------------------------------------------
std::string MultiThresh::toXml(int tileIndex, int indent) const
{
  string s = TaXml::writeStartTag(_tag, indent);
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    s += _thresh[i].toXml2(indent+1);
  }    
  s += TaXml::writeDouble("Bias", indent+1, _bias, "%010.7lf");
  s += TaXml::writeBoolean("coldstart", indent+1, _coldstart);
  s += TaXml::writeBoolean("motherTile", indent+1, _motherTile);
  s += TaXml::writeTime("generatingTime", indent+1, _generatingTime);
  s += TaXml::writeDouble("obsValue", indent+1, _obsValue, "%010.7lf");
  s += TaXml::writeDouble("fcstValue", indent+1, _fcstValue, "%010.7lf");
  s += TaXml::writeInt("tileIndex", indent+1, tileIndex);
  s += TaXml::writeEndTag(_tag, indent);
  return s;
}

//------------------------------------------------------------------
bool
MultiThresh::checkColdstart(const time_t &t,
			    int maxSecondsBeforeColdstart,
			    const std::vector<FieldThresh2> &coldstartThresh)
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
  // Note: mother tile is not changed here
  _generatingTime = 0;
  _thresh = coldstartThresh;
  _obsValue = -99.99;
  _fcstValue = -99.99;
  return true;
}

//------------------------------------------------------------------
void MultiThresh::print(int leadTime, int tileIndex, const TileInfo &info,
			bool verbose) const
{
  printf("        lt:%08d tile[%3d(%s)] ", leadTime,
	 tileIndex, info.latlonDebugString(tileIndex).c_str());
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    printf("%s ", _thresh[i].sprint2().c_str());
  }
  printf("bias:%10.8lf ", _bias);
  if (_motherTile)
  {
    printf("Mother ");
  }
  else
  {
    printf("       ");
  }
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
void MultiThresh::logDebug(int leadTime, int tileIndex, bool verbose) const
{
  LOG(DEBUG) << "   lt:" << leadTime << " tileIndex:" << tileIndex;
  string s= "";
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    s += _thresh[i].sprint2().c_str();
  }
  LOG(DEBUG) << "     " << s;
  LOG(DEBUG) << "      bias:" << _bias;
  if (_motherTile)
  {
    LOG(DEBUG) << "    fromMotherTile";
  }
  if (_coldstart)
  {
    LOG(DEBUG) << "      Coldstart";
  }
  else
  {
    LOG(DEBUG) << "      ObsTime: " <<  DateTime::strn(_generatingTime);
    if (verbose)
    {
      LOG(DEBUG) << "      ObsValue:" << _obsValue;
      LOG(DEBUG) << "      FcstValue:" <<  _fcstValue;
    }
  }
}

//------------------------------------------------------------------
std::string MultiThresh::sprint(int leadTime, int tileIndex,
				const TileInfo &info,
				bool verbose) const
{
  char buf[10000];
  sprintf(buf, "lt:%08d tile[%3d(%s)] ", leadTime,
	  tileIndex, info.latlonDebugString(tileIndex).c_str());
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    sprintf(buf+strlen(buf), "%s ", _thresh[i].sprint2().c_str());
  }
  sprintf(buf+strlen(buf), "bias:%10.8lf ", _bias);
  if (_motherTile)
  {
    sprintf(buf + strlen(buf), "fromMother ");
  }
  else
  {
    sprintf(buf + strlen(buf), "          ");
  }
  if (_coldstart)
  {
    sprintf(buf+strlen(buf), "Coldstart");
  }
  else
  {
    sprintf(buf+strlen(buf), "ObsTime:%s",
	    DateTime::strn(_generatingTime).c_str());
    if (verbose)
    {
      sprintf(buf+strlen(buf), " ObsValue:%.8lf", _obsValue);
      sprintf(buf+strlen(buf), " FcstValue:%.8lf", _fcstValue);
    }
  }

  string s = buf;
  return s;
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
bool MultiThresh::hasField(const std::string &name) const
{
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    if (_thresh[i].nameMatch(name))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
int MultiThresh::getThresholdIndex(const std::string &fieldName) const
{
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    if (_thresh[i].nameMatch(fieldName))
    {
      return static_cast<int>(i);
    }
  }
  LOG(ERROR) << "Name not found " << fieldName;
  return -1;
}


//------------------------------------------------------------------
std::string MultiThresh::fieldName2(int nameChars, int precision) const
{
  std::string ret = "";
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    ret += _thresh[i].dataFieldName2(nameChars, precision);
  }
  return ret;
}

//------------------------------------------------------------------
std::string
MultiThresh::fieldName2Limited(const std::vector<std::string> &ignore,
			       int nameChars, int precision) const
{
  if (ignore.empty())
  {
    return fieldName2(nameChars, precision);
  }

  std::string ret = "";
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    string fullName = _thresh[i].getField();
    if (find(ignore.begin(), ignore.end(), fullName) == ignore.end())
    {
      ret += _thresh[i].dataFieldName2(nameChars, precision);
    }
  }
  return ret;
}

//------------------------------------------------------------------
void MultiThresh::
update(double bias, const time_t &obsTime, double obsValue, double fcstValue,
       const std::vector<std::pair<std::string,double> >  &nameThresh)
{
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    _thresh[i].update(nameThresh);
  }
  _bias = bias;
  _coldstart = false;
  _generatingTime = obsTime;
  _obsValue = obsValue;
  _fcstValue = fcstValue;
}

//------------------------------------------------------------------
bool
MultiThresh::add(const std::vector<std::pair<std::string,double> > &nameThresh)
{
  bool ret = true;
  for (size_t i=0; i<nameThresh.size(); ++i)
  {
    if (hasField(nameThresh[i].first))
    {
      LOG(ERROR) << "Already have name " << nameThresh[i].first;
      ret = false;
    }
    else
    {
      _thresh.push_back(FieldThresh2(nameThresh[i].first,
				     nameThresh[i].second,
				     nameThresh[i].second));
    }
  }
  return ret;
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
bool MultiThresh::getIthName(int i, std::string &name) const
{
  if (i >= 0 && i < static_cast<int>(_thresh.size()))
  {
    name = _thresh[i].getField();
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
  
//------------------------------------------------------------------
bool MultiThresh::_replaceValue(const std::string &fieldName,
				const MultiThresh &filtMap)
{
  for (size_t i=0; i<_thresh.size(); ++i)
  {
    if (_thresh[i].nameMatch(fieldName))
    {
      for (size_t j=0; j<filtMap._thresh.size(); ++j)
      {
	if (filtMap._thresh[j].nameMatch(fieldName))
	{
	  _thresh[i].setThreshFromInput(filtMap._thresh[j]);
	  return true;
	}
      }
    }
  }
  LOG(ERROR) << "Field not found in state " << fieldName;
  return false;
}
