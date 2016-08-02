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
 * @file WindshearAlpha.cc
 */
#include <rapformats/WindshearAlpha.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
#include <cmath>
using std::string;

//------------------------------------------------------------------
WindshearAlpha::WindshearAlpha(void) :
  _name("Unknown"), _type(NONE), _runwayType(BAD), _magnitude(0),
  _location(0), _wellFormed(false)
{
}

//------------------------------------------------------------------
WindshearAlpha::WindshearAlpha(const std::string &name, Alert_t type,
			       Runway_t runwayType, int magnitude,
			       int location) :
  _name(name), _type(type), _runwayType(runwayType), _magnitude(magnitude),
  _location(location), _wellFormed(true)
{
}

//------------------------------------------------------------------
WindshearAlpha::WindshearAlpha(const std::string &name, Runway_t runwayType) :
  _name(name), _type(NONE), _runwayType(runwayType), _magnitude(0),
  _location(0), _wellFormed(true)
{
}

//------------------------------------------------------------------
WindshearAlpha::WindshearAlpha(const std::string &msg, const int maxLength) :
  _name(""), _type(IMPAIRED), _runwayType(BAD), _magnitude(0),
  _location(0), _impairedMsg(msg), _wellFormed(true)
{
  size_t max = maxLength;
  if (_impairedMsg.size() > max)
  {
    _impairedMsg = _impairedMsg.substr(0, max);
  }
  else 
  {
    while (_impairedMsg.size() < max)
    {
      _impairedMsg += " ";
    }
  }
}

//------------------------------------------------------------------
WindshearAlpha::WindshearAlpha(const std::string &s) : _wellFormed(true)
{
  _type = NONE;
  if (TaXml::readString(s, "Name", _name) != 0)
  {
    LOG(ERROR) << "ERROR reading tag Name";
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
  if (TaXml::readString(s, "RunwayType", t) == 0)
  {
    if (!setRunwayType(t, _runwayType))
    {
      _wellFormed = false;
    }
  }
  else
  {
    LOG(ERROR) << "No RunwayType key in XML data";
    _wellFormed = false;
  }

  if (TaXml::readInt(s, "Magnitude", _magnitude) != 0)
  {
    LOG(ERROR) << "No Magnitude key in XML data";
    _wellFormed = false;
  }

  if (TaXml::readInt(s, "Location", _location) != 0)
  {
    LOG(ERROR) <<  "No Location key in XML data";
    _wellFormed = false;
  }
  if (TaXml::readInt(s, "Location", _location) != 0)
  {
    LOG(ERROR) <<  "No Location key in XML data";
    _wellFormed = false;
  }
  if (_type == IMPAIRED)
  {
    if (TaXml::readString(s, "ImpairedMsg", _impairedMsg) != 0)
    {
      LOG(ERROR) << "No ImpairedMsg key in XML data";
      _wellFormed = false;
    }
  }
}

//------------------------------------------------------------------
WindshearAlpha::~WindshearAlpha()
{
}

//------------------------------------------------------------------
void WindshearAlpha::clear(void)
{
  _wellFormed = false;
  _name = "UNKNOWN";
  _type = NONE;
  _runwayType = BAD;
  _magnitude = 0;
  _location = 0;
}

//------------------------------------------------------------------
std::string WindshearAlpha::writeXml(void) const
{
  string s;
  s = TaXml::writeStartTag(tag(), 0);
  s += TaXml::writeString("Name", 0, _name);
  s += TaXml::writeString("Type", 0, sprintType(_type));
  s += TaXml::writeString("RunwayType", 0, sprintRunwayType(_runwayType));
  s += TaXml::writeInt("Magnitude", 0, _magnitude);
  s += TaXml::writeInt("Location", 0, _location);
  if (_type == IMPAIRED)
  {
    s += TaXml::writeString("ImpairedMsg", 0, _impairedMsg);
  }
  s += TaXml::writeEndTag(tag(), 0);
  return s;
}

//------------------------------------------------------------------
void WindshearAlpha::writeXml(const std::string &path) const
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
void WindshearAlpha::appendXml(const std::string &pth) const
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
std::string WindshearAlpha::sprintType(Alert_t type)
{
  string s;
  switch (type)
  {
  case WS_GAIN:
    s = "GAIN";
    break;
  case WS_LOSS:
    s = "LOSS";
    break;
  case MODERATE_TURB:
    s = "MODERATE_TURB";
    break;
  case SEVERE_TURB:
    s = "SEVERE_TURB";
    break;
  case MICROBURST:
    s = "MICROBURST";
    break;
  case IMPAIRED:
    s = "IMPAIRED";
    break;
  case NONE:
  default:
    s = "NONE";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string WindshearAlpha::sprintTypeShort(Alert_t type)
{
  string s;
  switch (type)
  {
  case WS_GAIN:
    s = "GAIN";
    break;
  case WS_LOSS:
    s = "LOSS";
    break;
  case MODERATE_TURB:
    s = "MOD ";
    break;
  case SEVERE_TURB:
    s = "SVR ";
    break;
  case MICROBURST:
    s = "MB  ";
    break;
  case NONE:
  case IMPAIRED:
  default:
    s = "    ";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string WindshearAlpha::sprintUnits(Alert_t type)
{
  string s;
  switch (type)
  {
  case WS_GAIN:
  case WS_LOSS:
  case MICROBURST:
    s = "kt";
    break;
  case MODERATE_TURB:
  case SEVERE_TURB:
  case NONE:
  case IMPAIRED:
  default:
    s = " ";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string WindshearAlpha::sprintRunwayType(Runway_t type)
{
  string s;
  switch (type)
  {
  case ARRIVAL:
    s = "ARRIVAL";
    break;
  case DEPART:
    s = "DEPART";
    break;
  case BAD:
  default:
    s = "BAD";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string WindshearAlpha::sprintRunwayTypeShort(Runway_t type)
{
  string s;
  switch (type)
  {
  case ARRIVAL:
    s = "A";
    break;
  case DEPART:
    s = "D";
    break;
  case BAD:
  default:
    s = "";
    break;
  }
  return s;
}

//----------------------------------------------------------------
std::string WindshearAlpha::sprintArenaDesignator(Runway_t type)
{
  std::string s="";
  switch (type)
  {
  case ARRIVAL:
    s = "F";
    break;
  case DEPART:
    s = "D";
    break;
  default:
    s = "";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string WindshearAlpha::sprintMagnitude(int mag)
{
  char buf[100];

  if (mag < 0)
  {
    sprintf(buf, "-%02d", -mag);
  }
  else
  {
    sprintf(buf, " %02d", mag);
  }

  string s = buf;
  return s;
}

//------------------------------------------------------------------
WindshearAlpha::Runway_t WindshearAlpha::oppositeRunwayType(Runway_t type)
{
  Runway_t t = BAD;
  switch (type)
  {
  case ARRIVAL:
    t = DEPART;
    break;
  case DEPART:
    t = ARRIVAL;
    break;
  default:
    t = BAD;
    break;
  }
  return t;
}

//------------------------------------------------------------------
bool WindshearAlpha::setType(const std::string &name, Alert_t &type)
{
  if (name == "LOSS")
  {
    type = WS_LOSS;
    return true;
  }
  else if (name == "GAIN")
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
  else if (name == "IMPAIRED")
  {
    type = IMPAIRED;
    return true;
  }
  else
  {
    LOG(ERROR) << "Unknown type name " << name;
    type = NONE;
    return false;
  }
}

//------------------------------------------------------------------
bool WindshearAlpha::setRunwayType(const std::string &name, Runway_t &type)
{
  if (name == "ARRIVAL")
  {
    type = ARRIVAL;
    return true;
  }
  else if (name == "DEPART")
  {
    type = DEPART;
    return true;
  }
  else if (name == "BAD")
  {
    type = BAD;
    return true;
  }
  else
  {
    LOG(ERROR) << "Unknown type name " << name;
    type = BAD;
    return false;
  }
}

//------------------------------------------------------------------
std::string WindshearAlpha::writeAlphanumeric(void) const
{
  if (_type == IMPAIRED)
  {
    return _impairedMsg;
  }
  else
  {
    string s = _name + sprintRunwayTypeShort(_runwayType);
    s += " ";
    s += sprintTypeShort(_type);
    s += " ";
  
    if (_type != NONE)
    {
      char buf[100];
      if (_location != 0)
      {
	sprintf(buf, "%s%s %3dM%s", sprintMagnitude(_magnitude).c_str(),
		sprintUnits(_type).c_str(), _location, 
		sprintArenaDesignator(_runwayType).c_str());
      }
      else
      {
	sprintf(buf, "%s%s    RW", sprintMagnitude(_magnitude).c_str(),
		sprintUnits(_type).c_str());
      }
      s += buf;
    }
    return s;
  }
}  

// //------------------------------------------------------------------
// std::string 
// WindshearAlpha::writeAlphanumeric(const std::string &impairedMsg) const
// {
//   if (_type != IMPAIRED)
//   {
//     LOG(ERROR) << "Wrong method for impaired";
//     return "";
//   }

//   string s = _name + sprintRunwayTypeShort(_runwayType);
//   s += " ";
//   s += impairedMsg;
//   return s;
// }  

