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
 * @file WindshearAlphas.cc
 */
#include <rapformats/WindshearAlphas.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

//------------------------------------------------------------------
WindshearAlphas::WindshearAlphas(void) :
  _time(0), _numDisplayLinesARC(0), _wellFormed(false)
{
}

//------------------------------------------------------------------
WindshearAlphas::WindshearAlphas(const time_t &time,
				 const std::vector<WindshearAlpha> alphas) :
  _time(time), _alpha(alphas), _numDisplayLinesARC(0), _wellFormed(true)
{
  _color[WindshearAlpha::NONE] = "0x000000";
  _color[WindshearAlpha::IMPAIRED] = "0xFF0000";
  _color[WindshearAlpha::WS_GAIN] = "0x000000";
  _color[WindshearAlpha::WS_LOSS] = "0x000000";
  _color[WindshearAlpha::MODERATE_TURB] = "0x000000";
  _color[WindshearAlpha::SEVERE_TURB] = "0x000000";
  _color[WindshearAlpha::MICROBURST] = "0x000000";
}

//------------------------------------------------------------------
WindshearAlphas::WindshearAlphas(const std::string &s) : 
  _numDisplayLinesARC(0), _wellFormed(true)
{
  if (TaXml::readTime(s, "Time", _time) != 0)
  {
    LOG(ERROR) << "reading tag Time";
    _wellFormed = false;
  }

  vector<string> a;
  if (TaXml::readStringArray(s, WindshearAlpha::tag(), a) == 0)
  {
    for (size_t i=0; i<a.size(); ++i)
    {
      WindshearAlpha alpha(a[i]);
      if (alpha.isWellFormed())
      {
	_alpha.push_back(alpha);
      }
      else
      {
	_wellFormed = false;
      }
    }
  }
  else
  {
    LOG(ERROR) << "ERROR reading tag Alpha";
    _wellFormed = false;
  }
}

//------------------------------------------------------------------
WindshearAlphas::~WindshearAlphas()
{
}

//------------------------------------------------------------------
void WindshearAlphas::clear(void)
{
  _wellFormed = false;
  _time = 0;
  _alpha.clear();
}

//------------------------------------------------------------------
std::string WindshearAlphas::writeXml(void) const
{
  string s;
  s = TaXml::writeStartTag("WindshearAlphas", 0);
  s += TaXml::writeTime("Time", 0, _time);
  for (size_t i=0; i<_alpha.size(); ++i)
  {
    s += _alpha[i].writeXml();
  }
  s += TaXml::writeEndTag("WindshearAlphas", 0);
  return s;
}

//------------------------------------------------------------------
void WindshearAlphas::writeXml(const std::string &path) const
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
void WindshearAlphas::appendXml(const std::string &pth) const
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
void WindshearAlphas::changeToRealTime(void)
{
  _time = time(0);
}

//------------------------------------------------------------------
void WindshearAlphas::setNumDisplayLinesForARCFormat(int numLines)
{
  _numDisplayLinesARC = numLines;
}

//------------------------------------------------------------------
void WindshearAlphas::setColorForARCFormat(WindshearAlpha::Alert_t type,
					   const std::string &color)
{
  _color[type] = color;
}

//------------------------------------------------------------------
std::vector<std::string> WindshearAlphas::writeAlphanumeric(void) const
{
  std::vector<string> ret;

  string s = DateTime::strn(_time);
  ret.push_back(s);
  for (size_t i=0; i<_alpha.size(); ++i)
  {
    ret.push_back(_alpha[i].writeAlphanumeric());
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<std::string> WindshearAlphas::writeAlphanumericARC(bool ymd)
{
  std::vector<string> ret;

  string o = "MSG,0," + _color[WindshearAlpha::NONE];
  o += ",   ";
  if (ymd)
  {
    o += DateTime::strn(_time);
  }
  else
  {
    DateTime dt(_time);
    o += "        ";
    o += dt.getTimeStr(false);
  }
  ret.push_back(o);
  for (size_t i=0; i<_alpha.size(); ++i)
  {
    ostringstream oi;
    oi << "MSG," << i+1 << "," 
       << _color[_alpha[i].getType()] << ","
       << _alpha[i].writeAlphanumeric();
    ret.push_back(oi.str());
    oi.flush();
  }

  // so we have _alpha.size() + 1 lines, put in blanks
  int nextIndex = static_cast<int>(_alpha.size()) + 1;
  for (int i=nextIndex; i<_numDisplayLinesARC; ++i)
  {
    ostringstream oi;
    oi << "MSG," << i << "," 
       << _color[WindshearAlpha::NONE] << ","
       << " ";
    ret.push_back(oi.str());
    oi.flush();
  }

  return ret;
}

//------------------------------------------------------------------
std::vector<std::string> 
WindshearAlphas::initialMessageARC(bool ymd)
{
  std::vector<string> ret;

  string o = "MSG,0," + _color[WindshearAlpha::NONE];
  o += ",   ";
  if (ymd)
  {
    o += DateTime::strn(_time);
  }
  else
  {
    DateTime dt(_time);
    o += "        ";
    o += dt.getTimeStr(false);
  }
  ret.push_back(o);
  o = "MSG,1," + _color[WindshearAlpha::NONE];
  o += ",  Initializing";
  ret.push_back(o);

  for (int i=2; i<_numDisplayLinesARC; ++i)
  {
    ostringstream oi;
    oi << "MSG," << i << "," 
       << _color[WindshearAlpha::NONE] << ","
       << " ";
    ret.push_back(oi.str());
    oi.flush();
  }
  return ret;
}

//------------------------------------------------------------------
bool WindshearAlphas::worstAlert(std::string &alert) const
{
  WindshearAlpha::Alert_t type = WindshearAlpha::NONE;
  int mag = 0;

  for (size_t i=0 ; i<_alpha.size(); ++i)
  {
    WindshearAlpha::Alert_t t = _alpha[i].getType();
    int m = _alpha[i].getMagnitude();
    if (t > type)
    {
      type = t;
      mag = m;
    }
    else if (t == type)
    {
      if (m > mag)
      {
	mag = m;
      }
    }
  }

  alert = WindshearAlpha::sprintType(type);
  alert += " ";
  alert += WindshearAlpha::sprintMagnitude(mag);
  return type > WindshearAlpha::NONE;
}
