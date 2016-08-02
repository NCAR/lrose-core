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
 * @file Windshear.cc
 */
#include <rapformats/Windshear.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
using std::string;

//------------------------------------------------------------------
Windshear::Windshear(void) :
  _time(0), _type(NONE), _magnitude(0), _wellFormed(false)
{
}

//------------------------------------------------------------------
Windshear::Windshear(const time_t &t, Event_t type, double magnitude, 
		     std::vector<std::pair<double,double> > &latlon) :
  _time(t), _type(type), _magnitude(magnitude), _latlon(latlon),
  _wellFormed(true)
{
}

//------------------------------------------------------------------
Windshear::Windshear(const std::string &s) : _wellFormed(true)
{
  if (TaXml::readTime(s, "Time", _time) != 0)
  {
    LOG(ERROR) << "reading tag Time";
    _wellFormed = false;
  }
  string t;
  if (TaXml::readString(s, "Type", t) == 0)
  {
    if (!setType(t, _type))
    {
      _wellFormed = false;
    }
  }
  else
  {
    LOG(ERROR) << "No Type key in XML data";
    _wellFormed = false;
  }
  if (TaXml::readDouble(s, "Magnitude", _magnitude) != 0)
  {
    LOG(ERROR) << "No Magnitude key in XML data";
    _wellFormed = false;
  }
  vector<string> latlon;
  if (TaXml::readStringArray(s, "Latlon", latlon) == 0)
  {
    for (size_t i=0; i<latlon.size(); ++i)
    {
      double lat, lon;
      bool ok = true;
      if (TaXml::readDouble(latlon[i], "Lat", lat) != 0)
      {
	LOG(ERROR) << "No Lat key in XML data " << latlon[i];
	_wellFormed = false;
	ok = false;
      }
      if (TaXml::readDouble(latlon[i], "Lon", lon) != 0)
      {
	LOG(ERROR) << "No Lon key in XML data " << latlon[i];
	_wellFormed = false;
	ok = false;
      }
      if (ok)
      {
	_latlon.push_back(pair<double,double>(lat, lon));
      }
    }
  }
  else
  {
    // can have no points (empty shape) which is ok.
  }
}

//------------------------------------------------------------------
Windshear::~Windshear()
{
}

//------------------------------------------------------------------
void Windshear::clear(void)
{
  _wellFormed = false;
  _time = 0;
  _type = NONE;
  _magnitude = 0;
  _latlon.clear();
}

//------------------------------------------------------------------
std::string Windshear::writeXml(void) const
{
  string s;
  s = TaXml::writeStartTag("Windshear", 0);
  s += TaXml::writeTime("Time", 0, _time);
  s += TaXml::writeString("Type", 0, sprintType(_type));
  s += TaXml::writeDouble("Magnitude", 0, _magnitude, "%.10lf");
  for (size_t i=0; i<_latlon.size(); ++i)
  {
    string slatlon;
    slatlon = TaXml::writeDouble("Lat",  0, _latlon[i].first, "%.10lf");
    slatlon = slatlon.substr(0, slatlon.size()-1);
    slatlon += TaXml::writeDouble("Lon", 0, _latlon[i].second, "%.10lf");
    slatlon = slatlon.substr(0, slatlon.size()-1);
    s += TaXml::writeString("Latlon", 0, slatlon);
  }
  s += TaXml::writeEndTag("Windshear", 0);
  return s;
}

//------------------------------------------------------------------
void Windshear::writeXml(const std::string &path) const
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
    LOG(ERROR) << "opening file " << path;
  }
}

//------------------------------------------------------------------
void Windshear::appendXml(const std::string &pth) const
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
    LOG(ERROR) << "opening file " << pth;
  }
}

//------------------------------------------------------------------
std::string Windshear::sprintType(Event_t type)
{
  string s;
  switch (type)
  {
  case WS_LOSS:
    s = "WS_LOSS";
    break;
  case WS_GAIN:
    s = "WS_GAIN";
    break;
  case MICROBURST:
    s = "MICROBURST";
    break;
  case MODERATE_TURB:
    s = "MODERATE_TURB";
    break;
  case SEVERE_TURB:
    s = "SEVERE_TURB";
    break;

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
    s = "ARENA_NONE";
    break;
  case RED_X:
    s = "RED_X";
    break;
  case EMPTY_X:
    s = "EMPTY_X";
    break;
  case NONE:
  default:
    s = "NONE";
    break;
  }
  return s;
}

//------------------------------------------------------------------
bool Windshear::setType(const std::string &name, Event_t &type)
{
  if (name == "WS_LOSS")
  {
    type = WS_LOSS;
    return true;
  }
  else if (name == "WS_GAIN")
  {
    type = WS_GAIN;
    return true;
  }
  else if (name == "MICROBURST")
  {
    type = MICROBURST;
    return true;
  }
  else if (name == "MODERATE_TURB")
  {
    type = MODERATE_TURB;
    return true;
  }
  else if (name == "SEVERE_TURB")
  {
    type = SEVERE_TURB;
    return true;
  }
  else if (name == "NONE")
  {
    type = NONE;
    return true;
  }
  else if (name == "ARENA_WS_LOSS")
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
  else if (name == "RED_X")
  {
    type = RED_X;
    return true;
  }
  else if (name == "EMPTY_X")
  {
    type = EMPTY_X;
    return true;
  }
  else
  {
    LOG(ERROR) << "Unknown type name " << name;
    type = NONE;
    return false;
  }
}

