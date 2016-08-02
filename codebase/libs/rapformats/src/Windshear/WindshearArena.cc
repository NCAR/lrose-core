// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 * @file WindshearArena.cc
 */
#include <rapformats/WindshearArena.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
using std::string;

//------------------------------------------------------------------
static bool _vectorLatlon(const std::string &s, const std::string &key,
			  vector<pair<double,double> > &out, bool showErrors)
{
  out.clear();

  vector<string> latlon;
  bool ret = true;
  if (TaXml::readStringArray(s, key, latlon) == 0)
  {
    for (size_t i=0; i<latlon.size(); ++i)
    {
      double lat, lon;
      bool ok = true;
      if (TaXml::readDouble(latlon[i], "Lat", lat) != 0)
      {
	if (showErrors)
	{
	  LOG(ERROR) << "No Lat key in XML data " << latlon[i];
	}
	ok = false;
      }
      if (TaXml::readDouble(latlon[i], "Lon", lon) != 0)
      {
	if (showErrors)
	{
	  LOG(ERROR) << "No Lon key in XML data " << latlon[i];
	}
	ok = false;
      }
      if (ok)
      {
	out.push_back(pair<double,double>(lat, lon));
      }
      else
      {
	ret = false;
      }
    }
  }
  else
  {
    if (showErrors)
    {
      LOG(ERROR) << "No " << key << " key in XML data";
    }
    ret = false;
  }

  return ret;
}

//------------------------------------------------------------------
static std::string _vectorLatlonWrite(const std::string &tag, 
				      const vector<pair<double,double> > &data)
{
  string s = "";

  for (size_t i=0; i<data.size(); ++i)
  {
    string slatlon;
    slatlon = TaXml::writeDouble("Lat",  0, data[i].first, "%.10lf");
    slatlon = slatlon.substr(0, slatlon.size()-1);
    slatlon += TaXml::writeDouble("Lon", 0, data[i].second, "%.10lf");
    slatlon = slatlon.substr(0, slatlon.size()-1);
    s += TaXml::writeString(tag, 0, slatlon);
  }
  return s;
}

//------------------------------------------------------------------
WindshearArena::WindshearArena(void) :
  _time(0), _type(ARENA_NONE), _magnitude(0), _onRunway(false),
  _wellFormed(false)
{
}

//------------------------------------------------------------------
WindshearArena::WindshearArena(const time_t &t, Event_t type, double magnitude, 
	     std::vector<std::pair<double,double> > &alertLatlon,
	     std::vector<std::pair<double,double> > &displayLatlon) :
  _time(t), _type(type), _magnitude(magnitude), _onRunway(true),
  _alertLatlon(alertLatlon),
  _displayLatlon(displayLatlon),
  _wellFormed(true)
{
}

//------------------------------------------------------------------
WindshearArena::WindshearArena(const time_t &t, Event_t type, double magnitude, 
	     std::vector<std::pair<double,double> > &alertLatlon,
	     std::vector<std::pair<double,double> > &displayLatlon,
	     std::vector<std::pair<double,double> > &center,
	     std::vector<std::pair<double,double> > &cross0,
	     std::vector<std::pair<double,double> > &cross1) :
  _time(t), _type(type), _magnitude(magnitude), _onRunway(false),
  _alertLatlon(alertLatlon),
  _displayLatlon(displayLatlon),
  _centerLatlon(center),
  _crossLatlon0(cross0),
  _crossLatlon1(cross1),
  _wellFormed(true)
{
  if (_crossLatlon0.size() != 2 || _crossLatlon1.size() != 2 ||
      _centerLatlon.size() != 2)
  {
    LOG(ERROR) << "Wrong size of input vectors to constructor";
    _wellFormed = false;
  }
}

//------------------------------------------------------------------
WindshearArena::WindshearArena(const std::string &s, bool showErrors) :
  _wellFormed(true)
{
  if (TaXml::readTime(s, "Time", _time) != 0)
  {
    if (showErrors)
    {
      LOG(ERROR) << "ERROR reading tag Time";
    }
    _wellFormed = false;
  }
  string t;
  if (TaXml::readString(s, "Type", t) == 0)
  {
    if (!setType(t, _type, showErrors))
    {
      _wellFormed = false;
    }
  }
  else
  {
    if (showErrors)
    {
      LOG(ERROR) << "No Type key in XML data";
    }
    _wellFormed = false;
  }
  if (TaXml::readDouble(s, "Magnitude", _magnitude) != 0)
  {
    if (showErrors)
    {
      LOG(ERROR) << "No Magnitude key in XML data";
    }
    _wellFormed = false;
  }

  if (TaXml::readBoolean(s, "OnRunway", _onRunway) != 0)
  {
    if (showErrors)
    {
      LOG(ERROR) << "No OnRunway key in XML data";
    }
    _wellFormed = false;
  }

  if (!_vectorLatlon(s, "AlertLatlon", _alertLatlon, false))
  {
    // For backward compatability
    if (!_vectorLatlon(s, "ComputeLatlon", _alertLatlon, false))
    {
      _wellFormed = false;
      if (showErrors) 
      {
	LOG(ERROR) << "No AlertLatlon or ComputeLatlon tags in XML data";
      }
    }
  }

  if (!_vectorLatlon(s, "DisplayLatlon", _displayLatlon, showErrors))
  {
    _wellFormed = false;
  }

  if (_onRunway)
  {
    return;
  }

  // additional stuff for non-runway arenas
  if (!_vectorLatlon(s, "CenterLine", _centerLatlon, showErrors))
  {
    _wellFormed = false;
  }
  else
  {
    if (_centerLatlon.size() != 2)
    {
      if (showErrors)
      {
	LOG(ERROR) <<  "Wrong length for CenterLine want 2 got " <<
	  _centerLatlon.size();
      }
      _wellFormed = false;
    }
  }

  if (!_vectorLatlon(s, "CrossLine0", _crossLatlon0, showErrors))
  {
    _wellFormed = false;
  }
  else
  {
    if (_crossLatlon0.size() != 2)
    {
      if (showErrors)
      {
	LOG(ERROR) << "Wrong length for CrossLine0 want 2 got " <<
	  _crossLatlon0.size();
      }
      _wellFormed = false;
    }
  }
  if (!_vectorLatlon(s, "CrossLine1", _crossLatlon1, showErrors))
  {
    _wellFormed = false;
  }
  else
  {
    if (_crossLatlon1.size() != 2)
    {
      if (showErrors)
      {
	LOG(ERROR) <<  "Wrong length for CrossLine1 want 2 got " <<
	  _crossLatlon1.size();
      }
      _wellFormed = false;
    }
  }
}

//------------------------------------------------------------------
WindshearArena::~WindshearArena()
{
}

//------------------------------------------------------------------
void WindshearArena::clear(void)
{
  _wellFormed = false;
  _time = 0;
  _type = ARENA_NONE;
  _magnitude = 0;
  _onRunway = false;
  _alertLatlon.clear();
  _displayLatlon.clear();
  _centerLatlon.clear();
  _crossLatlon0.clear();
  _crossLatlon1.clear();
}

//------------------------------------------------------------------
std::string WindshearArena::writeXml(void) const
{
  string s;
  s = TaXml::writeStartTag("Arena", 0);
  s += TaXml::writeTime("Time", 0, _time);
  s += TaXml::writeString("Type", 0, sprintType(_type));
  s += TaXml::writeDouble("Magnitude", 0, _magnitude, "%.10lf");
  s += TaXml::writeBoolean("OnRunway", 0, _onRunway);
  s += _vectorLatlonWrite("AlertLatlon", _alertLatlon);
  s += _vectorLatlonWrite("DisplayLatlon", _displayLatlon);
  if (!_onRunway)
  {
    s += _vectorLatlonWrite("CenterLine", _centerLatlon);
    s += _vectorLatlonWrite("CrossLine0", _crossLatlon0);
    s += _vectorLatlonWrite("CrossLine1", _crossLatlon1);
  }

  s += TaXml::writeEndTag("Arena", 0);
  return s;
}

//------------------------------------------------------------------
void WindshearArena::writeXml(const std::string &path) const
{
  FILE *fp = fopen(path.c_str(), "w");
  if (fp != NULL)
  {
    string s = writeXml();
    fprintf(fp, "%s\n", s.c_str());
    fclose(fp);
  }
  else
  {
    LOG(ERROR) << "Opening file " << path;
  }
}

//------------------------------------------------------------------
void WindshearArena::appendXml(const std::string &pth) const
{
  FILE *fp = fopen(pth.c_str(), "a");
  if (fp != NULL)
  {
    string s = writeXml();
    fprintf(fp, "%s\n", s.c_str());
    fclose(fp);
  }
  else
  {
    LOG(ERROR) << "Opening file " << pth;
  }
}

//------------------------------------------------------------------
std::string WindshearArena::sprintType(Event_t type)
{
  string s;
  switch (type)
  {
  case ARENA_WS_LOSS:
    s = "ARENA_WS_LOSS";
    break;
  case ARENA_WS_GAIN:
    s = "ARENA_WS_GAIN";
    break;
  case ARENA_MICROBURST:
    s = "ARENA_MICROBURST";
    break;
  case ARENA_MODERATE_TURB:
    s = "ARENA_MODERATE_TURB";
    break;
  case ARENA_SEVERE_TURB:
    s = "ARENA_SEVERE_TURB";
    break;
  case ARENA_NONE:
  default:
    s = "ARENA_NONE";
    break;
  }
  return s;
}

//------------------------------------------------------------------
bool WindshearArena::setType(const std::string &name, Event_t &type,
			     bool showErrors)
{
  if (name == "ARENA_WS_LOSS")
  {
    type = ARENA_WS_LOSS;
    return true;
  }
  else if (name == "ARENA_WS_GAIN")
  {
    type = ARENA_WS_GAIN;
    return true;
  }
  else if (name == "ARENA_MICROBURST")
  {
    type = ARENA_MICROBURST;
    return true;
  }
  else if (name == "ARENA_MODERATE_TURB")
  {
    type = ARENA_MODERATE_TURB;
    return true;
  }
  else if (name == "ARENA_SEVERE_TURB")
  {
    type = ARENA_SEVERE_TURB;
    return true;
  }
  else if (name == "ARENA_NONE")
  {
    type = ARENA_NONE;
    return true;
  }
  else
  {
    if (showErrors)
    {
      LOG(ERROR) << "Unknown type name " << name;
    }
    type = ARENA_NONE;
    return false;
  }
}
