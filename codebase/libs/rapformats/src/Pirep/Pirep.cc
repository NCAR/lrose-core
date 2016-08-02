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
/////////////////////////////////////////////////////////////////////////
//
// Class:	Pirep
//
// Author:	G. M. Cunning
//
// Date:	Thu Dec 29 18:13:56 2005
//
// Description: Pirep contatiner class, based on pirep.c by Mike Dixon.
//
//


// C++ include files

// System/RAP include files
#include <toolsa/DateTime.hh>
#include <dataport/bigend.h>
#include <rapformats/Pirep.hh>
#include <toolsa/TaXml.hh>
// #include <tinyxml.h>

using namespace std;

// define any constants
const string Pirep::_className    = "Pirep";
const float Pirep::MISSING_VALUE = -999;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Pirep::Pirep()
{
  clear();
  
  _obs_time = 0;
  memset(&_report_type, '0', REP_STR_LEN);
  memset(&_acft_id, '0', AC_STR_LEN);

  _latitude = 0;
  _longitude = 0;
  _altitude = 0;
  //  memset(&_type, 0, sizeof(si08));
  _type = 0;
  //  memset(&_sky_cond_1, 0, sizeof(sky_cond_t));
  //  memset(&_sky_cond_2, 0, sizeof(sky_cond_t));
  _sky_cond_1.base = Pirep::MISSING_VALUE;
  _sky_cond_1.top = Pirep::MISSING_VALUE;
  _sky_cond_1.coverage = Pirep::NO_REPORT_SKY;
  _sky_cond_1.isSet = false;

  _sky_cond_2.base = Pirep::MISSING_VALUE;
  _sky_cond_2.top = Pirep::MISSING_VALUE;
  _sky_cond_2.coverage = Pirep::NO_REPORT_SKY;
  _sky_cond_2.isSet = false;

  memset(&_wx_obs, 0, sizeof(wx_obs_t));
  _wx_obs.isSet = false;
  _wx_obs.visibility = Pirep::MISSING_VALUE;
  _wx_obs.temperature = Pirep::MISSING_VALUE;
  _wx_obs.wind_dir = Pirep::MISSING_VALUE;
  _wx_obs.wind_speed = Pirep::MISSING_VALUE;
  _wx_obs.observation = NO_REPORT_WX;

  //  memset(&_ice_obs_1, 0, sizeof(ice_obs_t));
  //  memset(&_ice_obs_2, 0, sizeof(ice_obs_t));
  _ice_obs_1.base = Pirep::MISSING_VALUE;
  _ice_obs_1.top = Pirep::MISSING_VALUE;
  _ice_obs_1.intensity = Pirep::FILL_II;
  _ice_obs_1.type = Pirep::FILL_IT;
  _ice_obs_1.isSet = false;

  _ice_obs_2.base = Pirep::MISSING_VALUE;
  _ice_obs_2.top = Pirep::MISSING_VALUE;
  _ice_obs_2.intensity = Pirep::FILL_II;
  _ice_obs_2.type = Pirep::FILL_IT;
  _ice_obs_2.isSet = false;

  //  memset(&_turb_obs_1, 0, sizeof(turb_obs_t));
  //  memset(&_turb_obs_2, 0, sizeof(turb_obs_t));
  _turb_obs_1.base = Pirep::MISSING_VALUE;
  _turb_obs_1.top = Pirep::MISSING_VALUE;
  _turb_obs_1.frequency = Pirep::FILL_TF;
  _turb_obs_1.intensity = Pirep::FILL_TI;
  _turb_obs_1.type = Pirep::FILL_TT;
  _turb_obs_1.isSet = false;

  _turb_obs_2.base = Pirep::MISSING_VALUE;
  _turb_obs_2.top = Pirep::MISSING_VALUE;
  _turb_obs_2.frequency = Pirep::FILL_TF;
  _turb_obs_2.intensity = Pirep::FILL_TI;
  _turb_obs_2.type = Pirep::FILL_TT;
  _turb_obs_2.isSet = false;


  memset(&_raw, '0', PIREP_TEXT_LEN1);
  memset(&_memBuf, 0, sizeof(MemBuf));
  
}

Pirep::Pirep(const Pirep &rhs)
{
  //  memcpy(&_pirep, &rhs._pirep, sizeof(pirep_t));
  _obs_time = rhs._obs_time;
  strncpy(_report_type, rhs._report_type, sizeof(_report_type));
  strncpy(_acft_id, rhs._acft_id, sizeof(_acft_id));
  _acft_id_string = rhs._acft_id_string;
  _latitude = rhs._latitude;
  _longitude = rhs._longitude;
  _altitude = rhs._altitude;
  _type = rhs._type;
  
  memcpy(&_wx_obs, &rhs._wx_obs, sizeof(wx_obs_t));

  memcpy(&_ice_obs_1, &rhs._ice_obs_1, sizeof(ice_obs_t));
  memcpy(&_ice_obs_2, &rhs._ice_obs_2, sizeof(ice_obs_t));

  memcpy(&_turb_obs_1, &rhs._turb_obs_1, sizeof(turb_obs_t));
  memcpy(&_turb_obs_2, &rhs._turb_obs_2, sizeof(turb_obs_t));

  strncpy(_raw, rhs._raw, PIREP_TEXT_LEN1);
  _raw_string = rhs._raw_string;

   _memBuf = rhs._memBuf;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Pirep::~Pirep()
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::operator=
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

 
Pirep& Pirep::operator=(const Pirep &rhs)
{
  if(this == &rhs) {
    return *this;
  }

  //  memcpy(&_pirep, &rhs._pirep, sizeof(pirep_t));
  //  _text = rhs._text;
  //  _memBuf = rhs._memBuf;
  _obs_time = rhs._obs_time;
  strncpy(_report_type, rhs._report_type, sizeof(_report_type));
  strncpy(_acft_id, rhs._acft_id, sizeof(_acft_id));
  _acft_id_string = rhs._acft_id_string;
  _latitude = rhs._latitude;
  _longitude = rhs._longitude;
  _altitude = rhs._altitude;
  _type = rhs._type;
  
  memcpy(&_wx_obs, &rhs._wx_obs, sizeof(wx_obs_t));

  memcpy(&_ice_obs_1, &rhs._ice_obs_1, sizeof(ice_obs_t));
  memcpy(&_ice_obs_2, &rhs._ice_obs_2, sizeof(ice_obs_t));

  memcpy(&_turb_obs_1, &rhs._turb_obs_1, sizeof(turb_obs_t));
  memcpy(&_turb_obs_2, &rhs._turb_obs_2, sizeof(turb_obs_t));

  strncpy(_raw, rhs._raw, PIREP_TEXT_LEN1);
  _raw_string = rhs._raw_string;

  _memBuf = rhs._memBuf;

  return *this;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::clear
//
// Description:	clears this object
//
// Returns:	
//
// Notes:
//
//

void 
Pirep::clear()
{
  _obs_time = 0;
  memset(&_report_type, '0', REP_STR_LEN);
  memset(&_acft_id, '0', AC_STR_LEN);
  _acft_id_string = "";
  _latitude = 0;
  _longitude = 0;
  _altitude = 0;
  _type = FILL_PIREP;
  memset(&_sky_cond_1, 0, sizeof(sky_cond_t));
  memset(&_sky_cond_2, 0, sizeof(sky_cond_t));
  _sky_cond_1.base = MISSING_VALUE;
  _sky_cond_1.top = MISSING_VALUE;
  _sky_cond_1.coverage = NO_REPORT_SKY;
  _sky_cond_2.base = MISSING_VALUE;
  _sky_cond_2.top = MISSING_VALUE;
  _sky_cond_2.coverage = NO_REPORT_SKY;
  _sky_cond_1.isSet = false;
  _sky_cond_2.isSet = false;

  memset(&_wx_obs, 0, sizeof(wx_obs_t));
  _wx_obs.isSet = false;
  _wx_obs.clear_above = Pirep::MISSING_VALUE;
  _wx_obs.visibility = Pirep::MISSING_VALUE;
  _wx_obs.temperature = Pirep::MISSING_VALUE;
  _wx_obs.wind_dir = Pirep::MISSING_VALUE;
  _wx_obs.wind_speed = Pirep::MISSING_VALUE;
  _wx_obs.observation = NO_REPORT_WX;

  //  memset(&_ice_obs_1, 0, sizeof(ice_obs_t));
  //  memset(&_ice_obs_2, 0, sizeof(ice_obs_t));
  _ice_obs_1.isSet = false;
  _ice_obs_1.base = Pirep::MISSING_VALUE;
  _ice_obs_1.top = Pirep::MISSING_VALUE;
  _ice_obs_1.intensity =  FILL_II;
  _ice_obs_1.type =  FILL_IT;
  _ice_obs_2.isSet = false;
  _ice_obs_2.base = Pirep::MISSING_VALUE;
  _ice_obs_2.top = Pirep::MISSING_VALUE;
  _ice_obs_2.intensity =  FILL_II;
  _ice_obs_2.type =  FILL_IT;

  memset(&_turb_obs_1, 0, sizeof(turb_obs_t));
  memset(&_turb_obs_2, 0, sizeof(turb_obs_t));
  _turb_obs_1.isSet = false;
  _turb_obs_2.isSet = false;

  memset(&_raw, '0', PIREP_TEXT_LEN1);
  _raw_string = "";

  _xml = "";
  _memBuf.reset();
  //  memset(&_memBuf, 0, sizeof(MemBuf));
}

#ifdef NOT_USED

TiXmlElement* Pirep::getItem(string name, TiXmlElement* xml)
{
  TiXmlElement* item = xml->FirstChildElement(name.c_str());
  if(item == NULL)
  {
    cout << "item " << name << " not found" << endl; 
  }

  return item;
}

TiXmlElement* Pirep::getNextItem(string name, TiXmlElement* xml)
{
  TiXmlElement* item = xml->NextSiblingElement(name.c_str());
  return item;
}



bool Pirep::parseObsTime(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  setObsTime(_itemToTimeT(item));
  return true;
}

bool Pirep::parseLatitude(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  setLatitude(_itemToDouble(item));
  return true;
}

bool Pirep::parseLongitude(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  setLongitude(_itemToDouble(item));
  return true;
}


bool Pirep::parseAltitude(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  setAltitude(_itemToInt(item));
  return true;
}

bool Pirep::parseRaw(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  setRaw(item->GetText());
  return true;
}

bool Pirep::parseAircraftId(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  setAircraftId(item->GetText());
  return true;
}

bool Pirep::parseReportType(string name, TiXmlElement* xml)
{
  TiXmlElement* item;

  if( (item = getItem(name,xml)) == NULL)
  {
    return false;
  }

  pirep_type_t type;

  switch(item->GetText())
  {
    case "VALID":
      type = VALID_PIREP;
      break;
    case "ICING":
      type = ICING_PIREP;
      break;
  case "TURB":
      type = TURB_PIREP;
      break;
  case "BOTH":
      type = BOTH_PIREP;
      break;
  case "CLEAR":
      type = CLEAR_PIREP;
      break;
  }
  setReportType(type);
  return true;
}

#endif

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::set*
//
// Description:	sets attribute of class
//
// Returns: none	
//
// Notes: These are the methods to set class members
//
//

void  
Pirep::setObsTime(const time_t &obs_time)
{
  _obs_time = static_cast<ti32>(obs_time);
}

void  
Pirep::setReportType(const string &report_type)
{
  memset(&_report_type, 0, REP_STR_LEN);
  strncpy(_report_type, report_type.c_str(), REP_STR_LEN);
}

void  
Pirep::setAircraftId(const string &acft_id)
{
  memset(&_acft_id, 0, AC_STR_LEN);
  strncpy(_acft_id, acft_id.c_str(), AC_STR_LEN-1);
  _acft_id_string = acft_id;
}

void  
Pirep::setLatitude(const double &lat)
{
  _latitude = static_cast<fl32>(lat);
}

void  
Pirep::setLongitude(const double &lon)
{
  _longitude = static_cast<fl32>(lon);
}

void  
Pirep::setAltitude(const double &alt)
{
  _altitude = static_cast<fl32>(alt);
}

void  
Pirep::setType(const int &type)
{
  if(!_checkPirepType(static_cast<pirep_type_t>(type))) {
    // cerr << "Unknown report type ... replacing with fill value." << endl;
    _type =  static_cast<si08>(FILL_PIREP);
  }
  else {
    _type =  static_cast<si08>(type);
  }
}

void  
Pirep::setSkyCondition1(const sky_cond_t &sky_cond)
{
  _setSkyCondition(sky_cond, _sky_cond_1);
}

void
Pirep::setSkyCondition2(const sky_cond_t &sky_cond)
{
  _setSkyCondition(sky_cond, _sky_cond_2);
}

void  
  Pirep::_setSkyCondition(const sky_cond_t &source, sky_cond_t &target)
{
  target.base = source.base;
  target.top = source.top;
  if(!_checkSkyCvrg(static_cast<sky_cvrg_t>(source.coverage))) {
    // cerr << "Unknown sky coverage ... replacing with fill value." << endl;
    target.coverage = static_cast<si08>(NO_REPORT_SKY);
  }
  else {
    target.coverage = source.coverage;
  }
  target.isSet = true;
}

void  
Pirep::setWeatherObs(const wx_obs_t &wx_obs)
{
  _wx_obs.clear_above = wx_obs.clear_above;
  _wx_obs.visibility = wx_obs.visibility;
  _wx_obs.temperature = wx_obs.temperature;
  _wx_obs.wind_dir = wx_obs.wind_dir;
  _wx_obs.wind_speed = wx_obs.wind_speed;

  if(!_checkObsWx(static_cast<obs_weather_t>(wx_obs.observation))) {
    // cerr << "Unknown weather observation ... replacing with fill value." << endl;
    _wx_obs.observation = static_cast<si08>(NO_REPORT_WX);
  }
  else {
    _wx_obs.observation = wx_obs.observation;
  }
  _wx_obs.isSet = true;
}

void  
Pirep::setIceObs1(const ice_obs_t &ice_obs)
{
  _setIceObs(ice_obs, _ice_obs_1);
}

void  
Pirep::setIceObs2(const ice_obs_t &ice_obs)
{
  _setIceObs(ice_obs, _ice_obs_2);
}

void  
Pirep::_setIceObs(const ice_obs_t &source, ice_obs_t &target)
{

  target.base = source.base;
  target.top = source.top;

  if(!_checkIceIntens(static_cast<ice_intens_t>(source.intensity))) {
    // cerr << "Unknown icing intensity ... replacing with fill value." << endl;
    target.intensity = static_cast<si08>(FILL_II);
  }
  else {
    target.intensity = source.intensity;
  }

  if(!_checkIceType(static_cast<ice_type_t>(source.type))) {
    // cerr << "Unknown icing type ... replacing with fill value." << endl;
    target.type = static_cast<si08>(FILL_IT);
  }
  else {
    target.type = source.type;
  }
  target.isSet = true;
}

void  
Pirep::setTurbObs1(const turb_obs_t &turb_obs)
{
  _setTurbObs(turb_obs, _turb_obs_1);
}

void  
Pirep::setTurbObs2(const turb_obs_t &turb_obs)
{
  _setTurbObs(turb_obs, _turb_obs_2);
}

void  
  Pirep::_setTurbObs(const turb_obs_t &source, turb_obs_t &target)
{
  target.base = source.base;
  target.top = source.top;

  if(!_checkTurbFreq(static_cast<turb_freq_t>(source.frequency))) {
    // cerr << "Unknown turbulence frequency: "
    // << (int) source.frequency << endl;
    // cerr << "  ... replacing with fill value." << endl;
    target.frequency = static_cast<si08>(FILL_TF);
  }
  else {
    target.frequency = source.frequency;
  }

  if(!_checkTurbIntens(static_cast<turb_intens_t>(source.intensity))) {
    // cerr << "Unknown turbulence intensity ... replacing with fill value." << endl;
    target.intensity = static_cast<si08>(FILL_TI);
  }
  else {
    target.intensity = source.intensity;
  }

  if(!_checkTurbType(static_cast<turb_type_t>(source.type))) {
    // cerr << "Unknown turbulence type ... replacing with fill value." << endl;
    target.type = static_cast<si08>(FILL_TT);
  }
  else {
    target.type = source.type;
  }
  target.isSet = true;
}

void  
Pirep::setRaw(const string &text)
{
  memset(&_raw, 0, PIREP_TEXT_LEN1);
  strncpy(_raw, text.c_str(), PIREP_TEXT_LEN1-1);

  _raw_string = text;
}

void Pirep::setXml(const string &text)
{
  _xml = text;
}

////////////////////////////////////////////////////
// convert to XML

void Pirep::toXml()
{
  _xml = "";

  _xml +=  TaXml::writeStartTag("PIREP", 0);
  _xml += TaXml::writeTime("obs_time", 1, _obs_time);
  _xml += TaXml::writeString("report_type", 1, _report_type);
  _xml += TaXml::writeString("aircraft_id", 1, _acft_id_string.c_str());
  _xml += TaXml::writeDouble("latitude", 1, _latitude);
  _xml += TaXml::writeDouble("longitude", 1, _longitude);
  _xml += TaXml::writeDouble("altitude", 1, _altitude);

  string type = "";
  switch(_type)
  {
    case VALID_PIREP:
      type = "VALID";
      break;
    case ICING_PIREP:
      type = "ICING";
      break;
    case TURB_PIREP:
      type = "TURB";
      break;
    case BOTH_PIREP:
      type = "BOTH";
      break;
    case CLEAR_PIREP:
      type = "CLEAR";
      break;
  }

  if(type != "")
  {
    _xml += TaXml::writeString("pirep_type", 1, type);
  }

  if(_ice_obs_1.isSet)
  {
    _xml += TaXml::writeStartTag("icing", 1);
    if((int)_ice_obs_1.type != static_cast<si08>(FILL_IT))
    {
      _xml += TaXml::writeInt("type", 2, (ice_type_t)_ice_obs_1.type);
    }
    if((int)_ice_obs_1.intensity != FILL_II)
    {
      _xml += TaXml::writeInt("intensity", 2, (ice_intens_t)_ice_obs_1.intensity);
    }
    if(_ice_obs_1.top != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("top", 2, _ice_obs_1.top);
    }
    if(_ice_obs_1.base != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("base", 2, _ice_obs_1.base);
    }
    _xml += TaXml::writeEndTag("icing", 1);
  }

  if(_ice_obs_2.isSet)
  {
    _xml += TaXml::writeStartTag("icing", 1);
    if(_ice_obs_2.type != static_cast<si08>(FILL_IT))
    {
      _xml += TaXml::writeInt("type", 2, (ice_type_t)_ice_obs_2.type);
    }
    if(_ice_obs_2.intensity != FILL_II)
    {
      _xml += TaXml::writeInt("intensity", 2, (ice_intens_t)_ice_obs_2.intensity);
    }
    if(_ice_obs_2.top != 0)
    {
      _xml += TaXml::writeDouble("top", 2, _ice_obs_2.top);
    }
    if(_ice_obs_2.base != 0)
    {
      _xml += TaXml::writeDouble("base", 2, _ice_obs_2.base);
    }
    _xml += TaXml::writeEndTag("icing", 1);
  }

  if(_turb_obs_1.isSet)
  {
    _xml += TaXml::writeStartTag("turbulence", 1);
    if(_turb_obs_1.type != FILL_TT)
    {
       _xml += TaXml::writeInt("type", 2, (turb_type_t)_turb_obs_1.type);
    }
    if(_turb_obs_1.intensity != FILL_TI)
    {
      _xml += TaXml::writeInt("intensity", 2, (turb_intens_t)_turb_obs_1.intensity);
    }
    if(_turb_obs_1.frequency != FILL_TF)
    {
      _xml += TaXml::writeInt("frequency", 2, (turb_freq_t)_turb_obs_1.frequency);
    }
    if(_turb_obs_1.top != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("top", 2, _turb_obs_1.top);
    }
    if(_turb_obs_1.base != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("base", 2, _turb_obs_1.base);
    }
    _xml += TaXml::writeEndTag("turbulence", 1);
  }

  if(_turb_obs_2.isSet)
  {
    _xml += TaXml::writeStartTag("turbulence", 1);
    if(_turb_obs_2.type != FILL_TT)
    {
      _xml += TaXml::writeInt("type", 2, (turb_type_t)_turb_obs_2.type);
    }
    if(_turb_obs_2.intensity != FILL_TI)
    {
      _xml += TaXml::writeInt("intensity", 2, (turb_intens_t)_turb_obs_2.intensity);
    }
    if(_turb_obs_2.frequency != FILL_TF)
    {
      _xml += TaXml::writeInt("frequency", 2, (turb_freq_t)_turb_obs_2.frequency);
    }
    if(_turb_obs_2.top != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("top", 2, _turb_obs_2.top);
    }
    if(_turb_obs_2.base != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("base", 2, _turb_obs_2.base);
    }
    _xml += TaXml::writeEndTag("turbulence", 1);
  }
  
  if(_sky_cond_1.isSet)
  {
    _xml += TaXml::writeStartTag("sky", 1);
    if(_sky_cond_1.coverage != NO_REPORT_SKY)
    {
      _xml += TaXml::writeInt("coverage", 2, (sky_cvrg_t)_sky_cond_1.coverage);
    }
    if(_sky_cond_1.top != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("top", 2, _sky_cond_1.top);
    }
    if(_sky_cond_1.base != MISSING_VALUE)
    {
     _xml += TaXml::writeDouble("base", 2, _sky_cond_1.base);
    }
      _xml += TaXml::writeEndTag("sky", 1);
  }

  if(_sky_cond_2.isSet)
  {
    _xml += TaXml::writeStartTag("sky", 1);
    if(_sky_cond_2.coverage != NO_REPORT_SKY)
    {
     _xml += TaXml::writeInt("coverage", 2, (sky_cvrg_t)_sky_cond_2.coverage);
    }
    if(_sky_cond_2.top != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("top", 2, _sky_cond_2.top);
    }
    if(_sky_cond_2.base != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("base", 2, _sky_cond_2.base);
    }
    _xml += TaXml::writeEndTag("sky", 1);
  }

  if(_wx_obs.isSet)
  {
    _xml += TaXml::writeStartTag("weather", 1);
    if(_wx_obs.clear_above != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("clear_above", 2, _wx_obs.clear_above);
    }
    if(_wx_obs.visibility != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("visibility", 2, _wx_obs.visibility);
    }
    if(_wx_obs.observation != NO_REPORT_WX)
    {
      _xml += TaXml::writeInt("observation", 2, (obs_weather_t)_wx_obs.observation);
    }
    if(_wx_obs.temperature != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("temp", 2, _wx_obs.temperature);
    }
    if(_wx_obs.wind_dir != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("wind_dir", 2, _wx_obs.wind_dir);
    }
    if(_wx_obs.wind_speed != MISSING_VALUE)
    {
      _xml += TaXml::writeDouble("wind_speed", 2, _wx_obs.wind_speed);
    }
    _xml += TaXml::writeEndTag("weather", 1);
  }

  _xml += TaXml::writeString("raw_text", 1, _raw_string.c_str());
  _xml += TaXml::writeEndTag("PIREP", 0);

  // add xml text to memory buffer
  _memBuf.free();
  _memBuf.add(_xml.c_str(), _xml.size() + 1);
}


#ifdef NOT_USED

////////////////////////////////////////////////////
// convert from XML using TinyXml

int Pirep::fromXml()
{
  TiXmlDocument doc;
  TiXmlElement* elem;
  TiXmlElement* item;
  const char* p = _xml.c_str();
  if(!doc.Parse(p, 0, TIXML_ENCODING_UTF8))
  {
    cerr << doc.ErrorDesc() << endl;
    return -1;
  }

  elem = doc.FirstChildElement();

  if(!parseObsTime("obs_time", elem))
  {
    return -2;
  }
  if(!parseLatitude("latitude", elem))
  {
    return -3;
  }
  if(!parseLongitude("longitude", elem))
  {
    return -4;
  }
  if(!parseAltitude("altitude", elem))
  {
    return -5;
  }

  setType(Pirep::VALID_PIREP);

  parseRaw("raw_text", elem);
  parseAircraftId("aircraft_id",elem);
  parseReportType("report_type", elem);

  // ice obs
  bool first = true;
  for(item = getItem("icing", elem); item != NULL; item = getNextItem("icing", elem))
  {
    setType(Pirep::ICING_PIREP);
    TiXmlElement* iceElem;
    ice_obs_t ice;

    if( (iceElem = getItem("type", item)) != NULL)
    {
      ice.type = atoi(iceElem->GetText());
    }
    iceElem = NULL;

    if( (iceElem = getItem("intensity", item)) != NULL)
    {
      ice.intensity = atoi(iceElem->GetText());
    }
    iceElem = NULL;

    if( (iceElem = getItem("base", item)) != NULL)
    {
      ice.base = atoi(iceElem->GetText());
    }
    else
    {
      ice.base = MISSING_VALUE;
    }
    iceElem = NULL;

    if( (iceElem = getItem("top", item)) != NULL)
    {
      ice.top = atoi(iceElem->GetText());
    }
    else
    {
      ice.top = MISSING_VALUE;
    }
    iceElem = NULL;
    
    if(first)
    {
      setIceObs1(ice);
      first = false;
    }
    else
    {
      setIceObs2(ice);
    }
  }

  // turbulence obs
  first = true;
  for(item = getItem("turbulence", elem); item != NULL; item = getNextItem("turbulence", elem))
  {
    TiXmlElement* subElem;
    turb_obs_t turb;

    if(getType() == ICING_PIREP)
    {
      setType(BOTH_PIREP);
    }
    else if(getType() == VALID_PIREP)
    {
      setType(TURB_PIREP);
    }

    if( (subElem = getItem("type", item)) != NULL)
    {
      turb.type = atoi(subElem->GetText());
    }
    else
    {
      turb.type = FILL_TT;
    }
    subElem = NULL;

    if( (subElem = getItem("intensity", item)) != NULL)
    {
      turb.intensity = atoi(subElem->GetText());
    }
    else
    {
      turb.intensity = FILL_TI;
    }
    subElem = NULL;

    if( (subElem = getItem("frequency", item)) != NULL)
    {
      turb.frequency = atoi(subElem->GetText());
    }
    else
    {
      turb.frequency = FILL_TF;
    }
    subElem = NULL;

    if( (subElem = getItem("base", item)) != NULL)
    {
      turb.base = atof(subElem->GetText());
    }
    else
    {
      turb.base = MISSING_VALUE;
    }
    subElem = NULL;

    if( (subElem = getItem("top", item)) != NULL)
    {
      turb.top = atof(subElem->GetText());
    }
    else
    {
      turb.top = MISSING_VALUE;
    }
    subElem = NULL;
    
    if(first)
    {
      setTurbObs1(turb);
      first = false;
    }
    else
    {
      setTurbObs2(turb);
    }
  }

  // sky obs
  first = true;
  for(item = getItem("sky", elem); item != NULL; item = getNextItem("sky", elem))
  {
    TiXmlElement* subElem;
    sky_cond_t sky;

    if( (subElem = getItem("coverage", item)) != NULL)
    {
      sky.coverage = atoi(subElem->GetText());
    }
    subElem = NULL;

    if(sky.coverage == CLEAR_SKY && getType() == VALID_PIREP)
    {
      setType(CLEAR_PIREP);
    }

    if( (subElem = getItem("base", item)) != NULL)
    {
      sky.base = atof(subElem->GetText());
    }
    else
    {
      sky.base = MISSING_VALUE;
    }
    subElem = NULL;

    if( (subElem = getItem("top", item)) != NULL)
    {
      sky.top = atof(subElem->GetText());
    }
    else
    {
      sky.top = MISSING_VALUE;
    }
    subElem = NULL;
    
    if(first)
    {
      setSkyCondition1(sky);
      first = false;
    }
    else
    {
      setSkyCondition2(sky);
    }
  }

  // weather obs
  if( (item = getItem("weather", elem)) != NULL)
  {
    TiXmlElement* subElem;
    wx_obs_t wx;
    if( (subElem = getItem("clear_above", item)) != NULL)
    {
      wx.clear_above = atof(subElem->GetText());
    }
    else
    {
      wx.clear_above = MISSING_VALUE;
    }
    subElem = NULL;

    if( (subElem = getItem("visibility", item)) != NULL)
    {
      wx.visibility = atof(subElem->GetText());
    }
    else
    {
      wx.visibility = MISSING_VALUE;
    }
    subElem = NULL;

    if( (subElem = getItem("observation", item)) != NULL)
    {
      wx.observation = atof(subElem->GetText());
    }
    else
    {
      wx.observation = NO_REPORT_WX;
    }

    subElem = NULL;

    if( (subElem = getItem("temp", item)) != NULL)
    {
      wx.temperature = atof(subElem->GetText());
    }
    else
    {
      wx.temperature = MISSING_VALUE;
    }
    subElem = NULL;

    if( (subElem = getItem("wind_dir", item)) != NULL)
    {
      wx.wind_dir = atof(subElem->GetText());
    }
    else
    {
      wx.wind_dir = MISSING_VALUE;
    }
    subElem = NULL;

    if( (subElem = getItem("wind_speed", item)) != NULL)
    {
      wx.wind_speed = atof(subElem->GetText());
    }
    else
    {
      wx.wind_speed = MISSING_VALUE;
    }
    setWeatherObs(wx);
  }
  return 0;
}

#endif

////////////////////////////////////////////////////
// convert from XML using TaXml

int Pirep::fromXml()
{
  // main tag

  string pirep;
  if (TaXml::readString(_xml, "PIREP", pirep)) {
    cerr << "ERROR - Pirep::fromXml" << endl;
    cerr << "  Cannot find PIREP tag" << endl;
    return -1;
  }

  // time

  time_t obsTime;
  if (TaXml::readTime(pirep, "obs_time", obsTime)) {
    cerr << "ERROR - Pirep::fromXml" << endl;
    cerr << "  No obs_time tag" << endl;
    return -2;
  }
  setObsTime(obsTime);

  // position

  double latitude;
  if (TaXml::readDouble(pirep, "latitude", latitude)) {
    cerr << "ERROR - Pirep::fromXml" << endl;
    cerr << "  No latitude tag" << endl;
    return -3;
  }
  setLatitude(latitude);

  double longitude;
  if (TaXml::readDouble(pirep, "longitude", longitude)) {
    cerr << "ERROR - Pirep::fromXml" << endl;
    cerr << "  No longitude tag" << endl;
    return -4;
  }
  setLongitude(longitude);

  double altitude;
  if (TaXml::readDouble(pirep, "altitude", altitude)) {
    cerr << "ERROR - Pirep::fromXml" << endl;
    cerr << "  No altitude tag" << endl;
    return -5;
  }
  setAltitude(altitude);

  // we have a valid pirep

  setType(Pirep::VALID_PIREP);
  
  // string info

  string strval;
  if (TaXml::readString(pirep, "raw_text", strval) == 0) {
    setRaw(strval);
  }
  if (TaXml::readString(pirep, "aircraft_id", strval) == 0) {
    setAircraftId(strval);
  }
  if (TaXml::readString(pirep, "report_type", strval) == 0) {
    setReportType(strval);
  }

  // icing

  vector<string> icingXmlVec;
  if (TaXml::readStringArray(pirep, "icing", icingXmlVec) == 0) {
    if (icingXmlVec.size() > 0) {
      setType(Pirep::ICING_PIREP);
    }
    for (size_t ii = 0; ii < icingXmlVec.size(); ii++) {
      const string &xml = icingXmlVec[ii];
      ice_obs_t ice;
      int ival;
      if (TaXml::readInt(xml, "type", ival) == 0) {
        ice.type = ival;
        ice.isSet = true;
      } else {
        ice.type = FILL_IT;
      }
      if (TaXml::readInt(xml, "intensity", ival) == 0) {
        ice.intensity = ival;
        ice.isSet = true;
      } else {
        ice.intensity = FILL_II;
      }
      double dval;
      if (TaXml::readDouble(xml, "base", dval) == 0) {
        ice.base = dval;
        ice.isSet = true;
      } else {
        ice.base = MISSING_VALUE;
      }
      if (TaXml::readDouble(xml, "top", dval) == 0) {
        ice.top = dval;
        ice.isSet = true;
      } else {
        ice.top = MISSING_VALUE;
      }
      if (ii == 0) {
        setIceObs1(ice);
      } else if (ii == 1) {
        setIceObs2(ice);
      }
    } // ii
  }

  // turbulence

  vector<string> turbXmlVec;
  if (TaXml::readStringArray(pirep, "turbulence", turbXmlVec) == 0) {
    if (turbXmlVec.size() > 0) {
      if(getType() == ICING_PIREP) {
        setType(BOTH_PIREP);
      } else if(getType() == VALID_PIREP) {
        setType(TURB_PIREP);
      }
    }
    for (size_t ii = 0; ii < turbXmlVec.size(); ii++) {
      const string &xml = turbXmlVec[ii];
      turb_obs_t turb;
      int ival;
      if (TaXml::readInt(xml, "type", ival) == 0) {
        turb.type = ival;
        turb.isSet = true;
      } else {
        turb.type = FILL_TT;
      }
      if (TaXml::readInt(xml, "intensity", ival) == 0) {
        turb.intensity = ival;
        turb.isSet = true;
      } else {
        turb.intensity = FILL_TI;
      }
      if (TaXml::readInt(xml, "frequency", ival) == 0) {
        turb.frequency = ival;
        turb.isSet = true;
      } else {
        turb.frequency = FILL_TF;
      }
      double dval;
      if (TaXml::readDouble(xml, "base", dval) == 0) {
        turb.base = dval;
        turb.isSet = true;
      } else {
        turb.base = MISSING_VALUE;
      }
      if (TaXml::readDouble(xml, "top", dval) == 0) {
        turb.top = dval;
        turb.isSet = true;
      } else {
        turb.top = MISSING_VALUE;
      }
      if (ii == 0) {
        setTurbObs1(turb);
      } else if (ii == 1) {
        setTurbObs2(turb);
      }
    } // ii
  }

  // sky condition
  
  vector<string> skyXmlVec;
  if (TaXml::readStringArray(pirep, "sky", skyXmlVec) == 0) {
    for (size_t ii = 0; ii < skyXmlVec.size(); ii++) {
      const string &xml = skyXmlVec[ii];
      sky_cond_t sky;
      int ival;
      if (TaXml::readInt(xml, "coverage", ival) == 0) {
        sky.coverage = ival;
        sky.isSet = true;
      } else {
        sky.coverage = NO_REPORT_SKY;
      }
      if(sky.coverage == CLEAR_SKY && getType() == VALID_PIREP) {
        setType(CLEAR_PIREP);
      }
      double dval;
      if (TaXml::readDouble(xml, "base", dval) == 0) {
        sky.base = dval;
        sky.isSet = true;
      } else {
        sky.base = MISSING_VALUE;
      }
      if (TaXml::readDouble(xml, "top", dval) == 0) {
        sky.top = dval;
        sky.isSet = true;
      } else {
        sky.top = MISSING_VALUE;
      }
      if (ii == 0) {
        setSkyCondition1(sky);
      } else if (ii == 1) {
        setSkyCondition2(sky);
      }
    } // ii
  }

  // weather obs

  string wxXml;
  if (TaXml::readString(pirep, "weather", wxXml) == 0) {
    wx_obs_t wx;
    int ival;
    if (TaXml::readInt(wxXml, "observation", ival) == 0) {
      wx.observation = ival;
      wx.isSet = true;
    } else {
      wx.observation = NO_REPORT_WX;
    }
    double dval;
    if (TaXml::readDouble(wxXml, "clear_above", dval) == 0) {
      wx.clear_above = dval;
      wx.isSet = true;
    } else {
      wx.clear_above = MISSING_VALUE;
    }
    if (TaXml::readDouble(wxXml, "visibility", dval) == 0) {
      wx.visibility = dval;
      wx.isSet = true;
    } else {
      wx.visibility = MISSING_VALUE;
    }
    if (TaXml::readDouble(wxXml, "temp", dval) == 0) {
      wx.temperature = dval;
      wx.isSet = true;
    } else {
      wx.temperature = MISSING_VALUE;
    }
    if (TaXml::readDouble(wxXml, "wind_dir", dval) == 0) {
      wx.wind_dir = dval;
      wx.isSet = true;
    } else {
      wx.wind_dir = MISSING_VALUE;
    }
    if (TaXml::readDouble(wxXml, "wind_speed", dval) == 0) {
      wx.wind_speed = dval;
      wx.isSet = true;
    } else {
      wx.wind_speed = MISSING_VALUE;
    }
    setWeatherObs(wx);
  }

  return 0;

}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::assemble
//
// Description:	Load up the buffer from the object. 
//
// Returns:	none	
//
// Notes:	Handles the byte swapping
//
//

void 
Pirep::assemble()
{
  /*  
  _memBuf.free();
  
  pirep_t pcopy = _pirep;
  _pirepToBE(pcopy);
  _memBuf.add(&pcopy, sizeof(pirep_t));
  */
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::disassemble
//
// Description:	Disassembles a buffer, sets the values in the object.
//
// Returns: 	true on success, false on failure
//
// Notes:	Handles the byte swapping
//
//

bool 
Pirep::disassemble(const void *buf, int len)
{
  /*
  int minLen = (int) sizeof(pirep_t);
  if (len < minLen) {
    cerr << "ERROR - Pirep::disassemble" << endl;
    cerr << "  Buffer header too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return false;
  }

  memcpy(&_pirep, (ui08 *) buf, sizeof(pirep_t));
  _pirepFromBE(_pirep);
  */
  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::print
//
// Description:	prints pirep contents.
//
// Returns:	none	
//
// Notes:
//
//

void 
Pirep::print(ostream &out, string spacer /* = "" */) const
{
  out << "===================================" << endl;
  out << spacer << "Pirep object" << endl;
  out << spacer << "  obs_time: " << DateTime::str(_obs_time) << endl;
  out << spacer << "  report_type: " << _report_type << endl;
  out << spacer << "  acft_id: " << _acft_id_string << endl;
  out << spacer << "  latitude: " << _latitude << endl;
  out << spacer << "  longitude: " << _longitude << endl;
  out << spacer << "  altitude: " << _altitude << endl;
  out << spacer << "  type: " << typeToString(static_cast<pirep_type_t>(_type)) << endl;
  out << spacer << "  sky condition 1: " << endl;
  out << spacer << "    base: " << _sky_cond_1.base << endl;
  out << spacer << "    top: " << _sky_cond_1.top << endl;
  out << spacer << "    coverage: " << skyCovrgToString(static_cast<sky_cvrg_t>(_sky_cond_1.coverage)) << endl;
  out << spacer << "  sky condition 2: " << endl;
  out << spacer << "    base: " << _sky_cond_2.base << endl;
  out << spacer << "    top: " << _sky_cond_2.top << endl;
  out << spacer << "    coverage: " << skyCovrgToString(static_cast<sky_cvrg_t>(_sky_cond_2.coverage)) << endl;
  out << spacer << "  observed weather: " << endl;
  out << spacer << "    clear_above: " << _wx_obs.clear_above << endl;
  out << spacer << "    visibility: " << _wx_obs.visibility << endl;
  out << spacer << "    observation: " << wxObsToString(static_cast<obs_weather_t>(_wx_obs.observation)) << endl;
  out << spacer << "    temperature: " << _wx_obs.temperature << endl;
  out << spacer << "    wind_dir: " << _wx_obs.wind_dir << endl;
  out << spacer << "    wind_speed: " << _wx_obs.wind_speed << endl;
  out << spacer << "  icing observation 1: " << endl;
  out << spacer << "    base: " << _ice_obs_1.base << endl;
  out << spacer << "    top: " << _ice_obs_1.top << endl;
  out << spacer << "    intensity: " << iceIntensityToString(static_cast<ice_intens_t>(_ice_obs_1.intensity)) << endl;
  out << spacer << "    type: " << iceTypeToString(static_cast<ice_type_t>(_ice_obs_1.type)) << endl;
  out << spacer << "  icing observation 2: " << endl;
  out << spacer << "    base: " << _ice_obs_2.base << endl;
  out << spacer << "    top: " << _ice_obs_2.top << endl;
  out << spacer << "    intensity: " << iceIntensityToString(static_cast<ice_intens_t>(_ice_obs_2.intensity)) << endl;
  out << spacer << "    type: " << iceTypeToString(static_cast<ice_type_t>(_ice_obs_2.type)) << endl;
  out << spacer << "  turbulence observation 1: " << endl;
  out << spacer << "    base: " << _turb_obs_1.base << endl;
  out << spacer << "    top: " << _turb_obs_1.top << endl;
  out << spacer << "    frequency: " << turbFreqToString(static_cast<turb_freq_t>(_turb_obs_1.frequency)) << endl;
  out << spacer << "    intensity: " << turbIntensityToString(static_cast<turb_intens_t>(_turb_obs_1.intensity)) << endl;
  out << spacer << "    type: " << turbTypeToString(static_cast<turb_type_t>(_turb_obs_1.type)) << endl;
  out << spacer << "  turbulence observation 2: " << endl;
  out << spacer << "    base: " << _turb_obs_2.base << endl;
  out << spacer << "    top: " << _turb_obs_2.top << endl;
  out << spacer << "    frequency: " << turbFreqToString(static_cast<turb_freq_t>(_turb_obs_2.frequency)) << endl;
  out << spacer << "    intensity: " << turbIntensityToString(static_cast<turb_intens_t>(_turb_obs_2.intensity)) << endl;
  out << spacer << "    type: " << turbTypeToString(static_cast<turb_type_t>(_turb_obs_2.type)) << endl;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::*ToString
//
// Description:	transforms enumerated values to tags for printing
//
// Returns:	
//
// Notes:
//
//

const char*
Pirep::typeToString(pirep_type_t type)
{
  switch(type) {
  case FILL_PIREP:
    return "fill value";
  case VALID_PIREP:
    return "valid pirep with no mention of icing or turbulence";
  case ICING_PIREP:
    return "icing data only";
  case TURB_PIREP:
    return "turbulence data only";
  case BOTH_PIREP:
    return "both icing and turbulence data";
  case CLEAR_PIREP:
    return "no icing or turbulence data, but clear skies";
  default:
    return "unknown value";
  }
}

const char*
Pirep::skyCovrgToString(sky_cvrg_t sky_cvrg)
{
  switch(sky_cvrg) {
  case NO_REPORT_SKY:
    return "no report";
  case CLEAR_SKY:
    return "clear sky";
  case FEW_SKY:
    return "few sky";
  case SCT_SKY:
    return "scattered sky";
  case BKN_SKY:
    return "broken sky";
  case OVC_SKY:
    return "overcast sky";
  case OBSCURD_SKY:
    return "obscured sky";
  default:
    return "unknown value";
  }
}

const char*
Pirep::wxObsToString(obs_weather_t obs_weather)
{
  switch(obs_weather) {
  case NO_REPORT_WX:
    return "no report";
  case VFR_GOOD_UNLMTD_WX:
    return "VFR good or unlimited visibility";
  case CLEAR_WX:
    return "clear";
  case SMOKE_WX:
    return "smoke";
  case HAZE_WX:
    return "haze";
  case DUST_ASH_WX:
    return "dust or ash";
  case TORNADO_WX:
    return "tornado";
  case SAND_WX:
    return "sand storm";
  case VIRGA_WX:
    return "virga";
  case LGHT_TSTRM_WX:
    return "lightning or thunderstorm";
  case FUNNEL_WX:
    return "funnel cloud ";
  case IFR_OBSCRD_WX:
    return "IFR obscured";
  case FOG_WX:
    return "fog or ground fog";
  case FRZ_FOG_WX:
    return "freezing fog";
  case DRZL_WX:
    return "drizzle";
  case FRZ_DRZL_WX:
    return "freezing drizzle";
  case RAIN_WX:
    return "rain";
  case FRZ_RAIN_WX:
    return "freezing rain";
  case SNOW_WX:
    return "snow";
  case GRAUPEL_WX:
    return "graupel";
  case RAIN_SHWR_WX:
    return "rain shower";
  case SNOW_SHWR_WX:
    return "snow shower";
  case HAIL_WX:
    return "hail";
  default:
    return "unknown value";
  }
}

const char*
Pirep::iceIntensityToString(ice_intens_t ice_intens)
{
  switch(ice_intens) {
  case FILL_II:
    return "fill value";
  case NONE_II:
    return "none";
  case TRC_II:
    return "trace";
  case TRC_LGHT_II:
    return "trace to light";
  case LGHT_II:
    return "light";
  case LGHT_MOD_II:
    return "light to moderate";
  case MOD_II:
    return "moderate";
  case MOD_HVY_II:
    return "moderate to heavy";
  case HVY_II:
    return "heavy";
  case SEVR_II:
    return "severe";
  default:
    return "unknown value";
  }
}

const char*
Pirep::iceTypeToString(ice_type_t ice_type)
{
  // cout << "ice_type " << ice_type << endl;
  // cout << "ice_typeint " << (int)ice_type << endl;
  switch(ice_type) {
  case FILL_IT:
    return "fill value";
  case RIME_IT:
    return "rime ice";
  case CLEAR_IT:
    return "clear ice";
  case MIXED_IT:
    return "rime and clear ice";
  default:
    return "unknown value";
  }
}

const char*
Pirep::turbFreqToString(turb_freq_t turb_freq)
{
  switch(turb_freq) {
  case FILL_TF:
    return "fill value";
  case ISOL_TF:
    return "isolated";
  case OCCNL_TF:
    return "occasional";
  case CONTNS_TF:
    return "continuous";
  default:
    return "unknown value";
  }
}

const char*
Pirep::turbIntensityToString(turb_intens_t turb_intens)
{
  switch(turb_intens) {
  case FILL_TI:
    return "fill value";
  case NONE_SMOOTH_TI:
    return "none or smooth";
  case LGHT_TI:
    return "light";
  case LGHT_MOD_TI:
    return "light to moderate";
  case MOD_TI:
    return "moderate";
  case MOD_SEVR_TI:
    return "moderate to severe";
  case SEVR_TI:
    return "severe";
  case SEVR_EXTRM_TI:
    return "severe to extreme";
  case EXTRM_TI:
    return "extreme";
  default:
    return "unknown value";
  }
}

const char*
Pirep::turbTypeToString(turb_type_t turb_type)
{
  switch(turb_type) {
  case FILL_TT:
    return "fill value";
  case CHOP_TT:
    return "chop";
  case CAT_TT:
    return "clear air";
  case LOWLVL_WNDSHR_TT:
    return "low-level wind shear";
  case MTN_WAVE_TT:
    return "mountain wave";
  default:
    return "unknown value";
  }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Pirep::_check*
//
// Description:	checks that value passed in for enum is valid.
//
// Returns:	
//
// Notes:
//
//

bool 
Pirep::_checkPirepType(const pirep_type_t& type)
{
  switch(type) {
  case FILL_PIREP:
  case VALID_PIREP:
  case ICING_PIREP:
  case TURB_PIREP:
  case BOTH_PIREP:
  case CLEAR_PIREP:
    return true;
  default:
    return false;
  }
}

bool 
Pirep::_checkSkyCvrg(const sky_cvrg_t& cvrg)
{
  switch(cvrg) {
  case NO_REPORT_SKY:
  case CLEAR_SKY:
  case FEW_SKY:
  case SCT_SKY:
  case BKN_SKY:
  case OVC_SKY:
  case OBSCURD_SKY:
    return true;
  default:
    return false;
  }

}

bool 
Pirep::_checkObsWx(const obs_weather_t& obs)
{
  switch(obs) {
  case NO_REPORT_WX:
  case VFR_GOOD_UNLMTD_WX:
  case CLEAR_WX:
  case SMOKE_WX:
  case HAZE_WX:
  case DUST_ASH_WX:
  case TORNADO_WX:
  case SAND_WX:
  case VIRGA_WX:
  case LGHT_TSTRM_WX:
  case FUNNEL_WX:
  case IFR_OBSCRD_WX:
  case FOG_WX:
  case FRZ_FOG_WX:
  case DRZL_WX:
  case FRZ_DRZL_WX:
  case RAIN_WX:
  case FRZ_RAIN_WX:
  case SNOW_WX:
  case GRAUPEL_WX:
  case RAIN_SHWR_WX:
  case SNOW_SHWR_WX:
  case HAIL_WX:
    return true;
  default:
    return false;
  }

}

bool 
Pirep::_checkIceIntens(const ice_intens_t& intens)
{
  switch(intens) {
  case FILL_II:
  case NONE_II:
  case TRC_II:
  case TRC_LGHT_II:
  case LGHT_II:
  case LGHT_MOD_II:
  case MOD_II:
  case MOD_HVY_II:
  case HVY_II:
  case SEVR_II:
    return true;
  default:
    return false;
  }

}

bool 
Pirep::_checkIceType(const ice_type_t& type)
{
  switch(type) {
  case FILL_IT:
  case RIME_IT:
  case CLEAR_IT:
  case MIXED_IT:
    return true;
  default:
    return false;
  }

}

bool 
Pirep::_checkTurbFreq(const turb_freq_t& freq)
{
  switch(freq) {
  case FILL_TF:
  case ISOL_TF:
  case OCCNL_TF:
  case CONTNS_TF:
    return true;
  default:
    return false;
  }

}

bool 
Pirep::_checkTurbIntens(const turb_intens_t& intens)
{
  switch(intens) {
  case FILL_TI:
  case NONE_SMOOTH_TI:
  case LGHT_TI:
  case LGHT_MOD_TI:
  case MOD_TI:
  case MOD_SEVR_TI:
  case SEVR_TI:
  case SEVR_EXTRM_TI:
  case EXTRM_TI:
    return true;
  default:
    return false;
  }

}

bool 
Pirep::_checkTurbType(const turb_type_t& type)
{
  switch(type) {
  case FILL_TT:
  case CHOP_TT:
  case CAT_TT:
  case LOWLVL_WNDSHR_TT:
  case MTN_WAVE_TT:
    return true;
  default:
    return false;
  }

}

#ifdef NOT_USED

//
// helper functions
//
int Pirep::_itemToInt(TiXmlElement* in)
{
  return atoi(in->GetText());
}

double Pirep::_itemToDouble(TiXmlElement* in)
{
  return atof(in->GetText());
}

time_t Pirep::_itemToTimeT(TiXmlElement* in)
{
  return convertStringToTimeT(in->GetText());
}

time_t Pirep::convertStringToTimeT(string in)
{
  int year, month, day, hour, min, sec;
  if (sscanf(in.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
         &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime dt(year, month, day, hour, min, sec);
    return dt.utime();
  }
  time_t tval;
  if (sscanf(in.c_str(), "%ld", &tval) == 1) {
    return tval;
  }

  return 0; //?
}
#endif
