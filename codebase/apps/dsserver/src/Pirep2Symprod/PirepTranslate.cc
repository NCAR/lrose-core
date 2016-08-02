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

// C++ include files

// System/RAP include files

// Local include files
#include "PirepTranslate.hh"

using namespace std;

PirepTranslate::PirepTranslate(bool f2s)
{
  _fewToScattered = f2s;
}

void PirepTranslate::pirep_t_to_Pirep(pirep_t in, Pirep* p)
{
  p->setObsTime(in.time);
  p->setLatitude(in.lat);
  p->setLongitude(in.lon);
  p->setAltitude(in.alt);

  Pirep::wx_obs_t weather;
  weather.clear_above = Pirep::MISSING_VALUE;

  weather.temperature = in.temp;
  weather.visibility = in.visibility;
  weather.wind_speed = in.wind_speed;
  weather.wind_dir = in.wind_dirn;
  p->setWeatherObs(weather);

  Pirep::turb_obs_t turb;
  turb.top = in.turb_fl_top;
  turb.base = in.turb_fl_base;
  
  turb.frequency = in.turb_freq; // ? 
  
  switch(in.turb_index)
  {
    case PIREP_TURB_NONE:
      turb.intensity = Pirep::NONE_SMOOTH_TI;
      break;
    case PIREP_TURB_TRACE:
      turb.intensity = Pirep::SMOOTH_LGHT_TI; // ?
      break;
    case PIREP_TURB_LIGHT:
      turb.intensity = Pirep::LGHT_TI;
      break;
    case PIREP_TURB_LIGHT_MODERATE:
      turb.intensity = Pirep::LGHT_MOD_TI;
      break;
    case PIREP_TURB_MODERATE:
      turb.intensity = Pirep::MOD_TI;
      break;
    case PIREP_TURB_MODERATE_SEVERE:
      turb.intensity = Pirep::MOD_SEVR_TI;
      break;
    case PIREP_TURB_SEVERE:
      turb.intensity = Pirep::SEVR_TI;
      break;
    case PIREP_TURB_SEVERE_EXTREME:
      turb.intensity = Pirep::SEVR_EXTRM_TI;
      break;
    case PIREP_TURB_EXTREME:
      turb.intensity = Pirep::EXTRM_TI;
      break;
  }
  

  turb.type = Pirep::FILL_TT; // ?
  p->setTurbObs1(turb);

  Pirep::ice_obs_t ice;
  ice.base = in.icing_fl_base;
  ice.top = in.icing_fl_top;
  switch(in.icing_index)
  {
    case PIREP_ICING_NONE:
      ice.intensity = Pirep::NONE_II;
      break;
    case PIREP_ICING_TRACE:
      ice.intensity = Pirep::TRC_II;
      break;
    case PIREP_ICING_LIGHT:
      ice.intensity = Pirep::LGHT_II;
      break;
    case PIREP_ICING_MODERATE:
      ice.intensity = Pirep::MOD_II;
      break;
    case PIREP_ICING_SEVERE:
      ice.intensity = Pirep::SEVR_II;
      break;
  }

  ice.type = Pirep::FILL_IT;
  p->setIceObs1(ice);

  Pirep::sky_cond_t sky;
  sky.base = in.sky_fl_base;
  sky.top = in.sky_fl_top;
  switch(in.sky_index)
  {
    case PIREP_SKY_CLEAR:
      sky.coverage = Pirep::CLEAR_SKY;
      break;
    case PIREP_SKY_SCATTERED:
      sky.coverage = Pirep::SCT_SKY;
      break;
    case PIREP_SKY_BROKEN:
      sky.coverage = Pirep::BKN_SKY;
      break;
    case PIREP_SKY_OVERCAST:
      sky.coverage = Pirep::OVC_SKY;
      break;
    case PIREP_SKY_OBSCURED:
      sky.coverage = Pirep::OBSCURD_SKY;
      break;
  }

  p->setSkyCondition1(sky);

  // how to handle callsign? 16 char -> 8 char
  string aid(in.callsign);
  p->setAircraftId(aid.substr(0,8));

  string raw(in.text);
  p->setRaw(raw);
}

void PirepTranslate::Pirep_to_pirep_t(Pirep in, pirep_t* p)
{
  p->time = in.getObsTime();
  p->lat = in.getLatitude();
  p->lon = in.getLongitude();
  p->alt = in.getAltitude();

  Pirep::wx_obs_t weather = in.getWeatherObs();
  p->temp = weather.temperature;
  p->visibility = weather.visibility;
  p->wind_speed = weather.wind_speed;
  p->wind_dirn = weather.wind_dir;

  Pirep::turb_obs_t turb = in.getTurbObs1();
  p->turb_fl_base = turb.base;
  p->turb_fl_top = turb.top;
  p->turb_freq = turb.frequency; // ?
  
  switch(turb.intensity)
  {
  case Pirep::NONE_SMOOTH_TI:
      p->turb_index = PIREP_TURB_NONE;
      break;
  case Pirep::SMOOTH_LGHT_TI:
      p->turb_index = PIREP_TURB_TRACE; // ?
      break;
  case Pirep::LGHT_TI:
      p->turb_index = PIREP_TURB_LIGHT;
      break;
  case Pirep::LGHT_MOD_TI:
      p->turb_index = PIREP_TURB_LIGHT_MODERATE;
      break;
  case Pirep::MOD_TI:
      p->turb_index = PIREP_TURB_MODERATE;
      break;
  case Pirep::MOD_SEVR_TI:
      p->turb_index = PIREP_TURB_MODERATE_SEVERE;
      break;
  case Pirep::SEVR_TI:
      p->turb_index = PIREP_TURB_SEVERE;
      break;
  case Pirep::SEVR_EXTRM_TI:
      p->turb_index = PIREP_TURB_SEVERE_EXTREME;
      break;
  case Pirep::EXTRM_TI:
      p->turb_index = PIREP_TURB_EXTREME;
      break;
  }
  

  Pirep::ice_obs_t ice = in.getIceObs1();
  p->icing_fl_base = ice.base;
  p->icing_fl_top = ice.top;
  p->icing_index = ice.intensity;

  Pirep::sky_cond_t sky = in.getSkyCondition1();
  p->sky_fl_base = sky.base;
  p->sky_fl_top = sky.top;
  switch(sky.coverage)
  {
    case Pirep::NO_REPORT_SKY:
      p->sky_index = PIREP_INT_MISSING;
      break;
    case Pirep::CLEAR_SKY:
      p->sky_index = PIREP_SKY_CLEAR;
      break;
    case Pirep::FEW_SKY:
      if(_fewToScattered)
      {
        p->sky_index = PIREP_SKY_SCATTERED;      
      }
      else
      {
        p->sky_index = PIREP_SKY_CLEAR;
      }
      break;
    case Pirep::SCT_SKY:
      p->sky_index = PIREP_SKY_SCATTERED;
      break;
    case Pirep::BKN_SKY:
      p->sky_index = PIREP_SKY_BROKEN;
      break;
    case Pirep::OVC_SKY:
      p->sky_index = PIREP_SKY_OVERCAST;
      break;
    case Pirep::OBSCURD_SKY:
      p->sky_index = PIREP_SKY_OBSCURED;
      break;
  }

  strncpy(p->callsign, in.getAircraftId(), sizeof(p->callsign));
  strncpy(p->text, in.getRaw(), sizeof(p->text));
}
