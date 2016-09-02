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
/////////////////////////////////////////////////////////////
// WxObs.cc
//
// C++ class for dealing with weather observations.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Feb 2007
//////////////////////////////////////////////////////////////

#define _in_wx_obs_cc

#include <cctype>
#include <algorithm>

#include <rapformats/WxObs.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/SignBit.hh>
using namespace std;

const double WxObs::CLOUD_SKC = 0.0;
const double WxObs::CLOUD_CLR = 0.01;
const double WxObs::CLOUD_FEW = 0.1875;
const double WxObs::CLOUD_SCT = 0.4375;
const double WxObs::CLOUD_BKN = 0.75;
const double WxObs::CLOUD_OVC = 1.0;
const double WxObs::CLOUD_OVX = -1.0;


///////////////
// constructor

WxObs::WxObs()

{

  // initialize relevant data members

  _observation_time = 0;
  _latitude = 0;
  _longitude = 0;
  _elevation_m = 0;
  _metar_rem_info_set = false;
  _metar_rem_pwi_down = false;
  _metar_rem_fzra_down = false;
  _metar_rem_ts_down = false;
  _ceiling_is_minimum = false;
  _visibility_is_minimum = false;
  _basedOnStationReport = false;
  MEM_zero(_report);

}

/////////////
// destructor

WxObs::~WxObs()

{
  reset();
}

//////////////////////////
// reset all data members

void WxObs::reset()

{

  _station_id = "";
  _observation_time = 0;
  _latitude = 0;
  _longitude = 0;
  _elevation_m = 0;

  _metar_text = "";
  _metar_wx = "";
  _metar_remarks = "";

  _metar_rem_info_set = false;
  _metar_rem_stn_indicator = "";
  _metar_rem_pwi_down = false;
  _metar_rem_fzra_down = false;
  _metar_rem_ts_down = false;

  _temp_c.clear();
  _min_temp_c.clear();
  _max_temp_c.clear();
  _sea_surface_temp_c.clear();
  _dewpoint_c.clear();
  _rh_percent.clear();
  _wind_dirn_degt.clear();
  _wind_speed_mps.clear();
  _wind_gust_mps.clear();
  _visibility_km.clear();
  _vert_vis_km.clear();
  _ceiling_km.clear();
  _rvr_km.clear();
  _pressure_mb.clear();
  _msl_pressure_mb.clear();
  _press_tend_mb.clear();
  _precip_liquid_mm.clear();
  _snow_depth_mm.clear();
  _sky_obsc.clear();
  _wx_type.clear();

  _basedOnStationReport = false;
  MEM_zero(_report);

  _memBuf.free();

}

///////////////////////////////////////////
// assemble()
// Assembles buffer as XML.

void WxObs::assemble()

{
  assembleAsXml();
}

///////////////////////////////////////////
// assemble as XML
// Load up an XML buffer from the object.

void WxObs::assembleAsXml()
  
{

  // check mem buffer is free
  
  _memBuf.free();
  
  // convert to XML string

  loadXml(_xml);

  // add xml string to buffer, including trailing null
  
  _memBuf.add(_xml.c_str(), _xml.size() + 1);

}

///////////////////////////////////////////
// assemble as station_report_t, optionally
// followed by XML.
//
// msgId should be one of:
//
//  SENSOR_REPORT
//  STATION_REPORT
//  METAR_REPORT
//  PRESSURE_STATION_REPORT
//  METAR_WITH_REMARKS_REPORT
//  REPORT_PLUS_METAR_XML,
//  REPORT_PLUS_FULL_XML
//
// See station_reports.h for more details.

void WxObs::assembleAsReport(msg_id_t msgId /* = STATION_REPORT */)
  
{

  // load station report

  station_report_t report;
  loadStationReport(report, msgId);

  // byte swap

  station_report_t be_report;
  memcpy(&be_report, &report, sizeof(station_report_t));
  station_report_to_be(&be_report);
  
  // check mem buffer is free
  
  _memBuf.free();
  
  // add byte-swapped station report to buffer
  
  _memBuf.add(&be_report, sizeof(station_report_t));

  // check if we need to add XML to the buffer

  if (msgId == REPORT_PLUS_METAR_XML) {

    // load METAR XML string
    
    loadMetarStringsXml(_metarStringsXml);

    // add xml string to buffer, including trailing null
    
    _memBuf.add(_metarStringsXml.c_str(), _metarStringsXml.size() + 1);

    // free up _xml since it is not being used

    _xml = "";
    
  } else if (msgId == REPORT_PLUS_FULL_XML) {

    // load XML string
    
    loadXml(_xml);
    
    // add xml string to buffer, including trailing null
    
    _memBuf.add(_xml.c_str(), _xml.size() + 1);
    
    // free up _metarStringsXml since it is not being used

    _metarStringsXml = "";
    
  }

}

///////////////////////////////////////////
// load XML string
// includeWxString defaults to false

void WxObs::loadXml(string &xml, bool includeWxStr,
                    int startIndentLevel) const
  
{

  int sil = startIndentLevel;

  // print object to string as XML

  xml = "";
  
  xml += TaXml::writeStartTag("weather_observation", sil+0);
  
  xml += TaXml::writeString("station_id", sil+1, _station_id);
  if (_long_name.size() > 0) {
    xml += TaXml::writeString("long_name", sil+1, _long_name);
  }
  xml += TaXml::writeTime("observation_time", sil+1, _observation_time);
  xml += TaXml::writeDouble("latitude", sil+1, _latitude);
  xml += TaXml::writeDouble("longitude", sil+1, _longitude);
  xml += TaXml::writeDouble("elevation_m", sil+1, _elevation_m);
  
  if (_metar_text.size() > 0) {
    xml += TaXml::writeString("metar_text", sil+1, _metar_text);
  }
  if (_metar_wx.size() > 0) {
    xml += TaXml::writeString("metar_wx", sil+1, _metar_wx);
  }
  if (_metar_remarks.size() > 0) {
    xml += TaXml::writeString("metar_remarks", sil+1, _metar_remarks);
  }
  
  if (includeWxStr && _metar_wx.size() > 0 ) {
    xml += TaXml::writeString("wx_string", sil+1, _metar_wx);
  }

  if (_metar_rem_info_set) {
    xml += TaXml::writeString("metar_rem_stn_indicator", sil+1,
                              _metar_rem_stn_indicator);
    xml += TaXml::writeBoolean("metar_rem_pwi_down", sil+1, _metar_rem_pwi_down);
    xml += TaXml::writeBoolean("metar_rem_fzra_down", sil+1, _metar_rem_fzra_down);
    xml += TaXml::writeBoolean("metar_rem_ts_down", sil+1, _metar_rem_ts_down);
  }

  if (getCeilingKmSize() > 0) {
    xml += TaXml::writeBoolean("ceiling_is_minimum", sil+1, 
                               _ceiling_is_minimum);
  }
  if (getVisibilityKmSize() > 0) {
    xml += TaXml::writeBoolean("visibility_is_minimum", sil+1,
                               _visibility_is_minimum);
  }

  _addFieldAsXml(_temp_c, "temp_c", sil+1, xml);
  _addFieldAsXml(_min_temp_c, "min_temp_c", sil+1, xml);
  _addFieldAsXml(_max_temp_c, "max_temp_c", sil+1, xml);
  _addFieldAsXml(_sea_surface_temp_c, "sea_surface_temp_c", sil+1, xml);
  _addFieldAsXml(_dewpoint_c, "dewpoint_c", sil+1, xml);
  _addFieldAsXml(_rh_percent, "rh_percent", sil+1, xml);
  _addFieldAsXml(_wind_dirn_degt, "wind_dirn_degt", sil+1, xml);
  _addFieldAsXml(_wind_speed_mps, "wind_speed_mps", sil+1, xml);
  _addFieldAsXml(_wind_gust_mps, "wind_gust_mps", sil+1, xml);
  _addFieldAsXml(_visibility_km, "visibility_km", sil+1, xml);
  _addFieldAsXml(_extinction_per_km, "extinction_per_km", sil+1, xml);
  _addFieldAsXml(_vert_vis_km, "vert_vis_km", sil+1, xml);
  _addFieldAsXml(_ceiling_km, "ceiling_km", sil+1, xml);
  _addFieldAsXml(_rvr_km, "rvr_km", sil+1, xml);
  _addFieldAsXml(_pressure_mb, "pressure_mb", sil+1, xml);
  _addFieldAsXml(_msl_pressure_mb, "msl_pressure_mb", sil+1, xml);
  _addFieldAsXml(_msl_pressure_QFF_mb, "msl_pressure_QFF_mb", sil+1, xml);
  _addFieldAsXml(_msl_pressure_inHg, "msl_pressure_inHg", sil+1, xml);
  _addFieldAsXml(_press_tend_mb, "press_tend_mb", sil+1, xml);
  _addFieldAsXml(_precip_liquid_mm, "precip_liquid_mm", sil+1, xml);
  _addFieldAsXml(_precip_rate_mmph, "precip_rate_mmph", sil+1, xml);
  _addFieldAsXml(_snow_depth_mm, "snow_depth_mm", sil+1, xml);
  _addFieldAsXml(_sky_obsc, "sky_obsc", sil+1, xml);
  _addWxTypeAsXml(_wx_type, "wx_type", sil+1, xml);

  xml += TaXml::writeEndTag("weather_observation", sil+0);
  
}

///////////////////////////////////////////
// load METAR strings XML

void WxObs::loadMetarStringsXml(string &xml) const
  
{

  xml = "";
  xml += TaXml::writeStartTag("metar_strings", 0);
  
  if (_metar_text.size() > 0) {
    xml += TaXml::writeString("metar_text", 1, _metar_text);
  }
  if (_metar_wx.size() > 0) {
    xml += TaXml::writeString("metar_wx", 1, _metar_wx);
  }
  if (_metar_remarks.size() > 0) {
    xml += TaXml::writeString("metar_remarks", 1, _metar_remarks);
  }
  if (getWindDirnVariable()) {
    xml += TaXml::writeBoolean("wdirn_variable", 1, true);
  }

  xml += TaXml::writeBoolean("ceiling_is_minimum", 1, _ceiling_is_minimum);
  xml += TaXml::writeBoolean("visibility_is_minimum", 1, _visibility_is_minimum);

  
  xml += TaXml::writeEndTag("metar_strings", 0);
  
}

///////////////////////////////////////////
// convert to station_report_t
//
// msgId should be one of:
//
//  SENSOR_REPORT
//  STATION_REPORT
//  METAR_REPORT
//  PRESSURE_STATION_REPORT
//  METAR_WITH_REMARKS_REPORT
//
// See station_reports.h for more details.

void WxObs::loadStationReport(station_report_t &report,
                              msg_id_t msgId) const
  
{

  // if this object was based on a station report, use it
  // directly

  if (_basedOnStationReport) {
    memcpy(&report, &_report, sizeof(station_report_t));
    return;
  }

  MEM_zero(report);

  report.msg_id = msgId;
  
  report.time = getObservationTime();

  report.lat = getLatitude();
  report.lon = getLongitude();
  report.alt = getElevationMeters();

  if (_temp_c.getSize() == 0) {
    report.temp = STATION_NAN;
  } else {
    report.temp = getTempC();
  }

  if (_dewpoint_c.getSize() == 0) {
    report.dew_point = STATION_NAN;
  } else {
    report.dew_point = getDewpointC();
  }

  if (_rh_percent.getSize() == 0) {
    report.relhum = STATION_NAN;
  } else {
    report.relhum = getRhPercent();
  }

  if (_wind_speed_mps.getSize() == 0) {
    report.windspd = STATION_NAN;
  } else {
    report.windspd = getWindSpeedMps();
  }

  if (_wind_dirn_degt.getSize() == 0) {
    report.winddir = STATION_NAN;
  } else {
    report.winddir = getWindDirnDegt();
  }

  if (_wind_gust_mps.getSize() == 0) {
    report.windgust = STATION_NAN;
  } else {
    report.windgust = getWindGustMps();
  }

  if (_msl_pressure_mb.getSize() == 0) {
    report.pres = STATION_NAN;
  } else {
    report.pres = getSeaLevelPressureMb();
  }

  if (_precip_liquid_mm.getSize() == 0) {
    report.liquid_accum = STATION_NAN;
    report.accum_start_time = report.time - 86400;
  } else {
    report.liquid_accum = getPrecipLiquidMm();
    report.accum_start_time =
      report.time - (ui32) getPrecipLiquidMmAccumSecs();
  }

  if (_precip_rate_mmph.getSize() == 0) {
    report.precip_rate = STATION_NAN;
  } else {
    report.precip_rate = getPrecipRateMmPerHr();
  }

  report.visibility = STATION_NAN;
  if (_visibility_km.getSize() > 0) {
    report.visibility = getVisibilityKm();
  } else if (_extinction_per_km.getSize() > 0) {
    report.visibility = getExtinctionPerKm() * -1.0;
  }

  if (_rvr_km.getSize() == 0) {
    report.rvr = STATION_NAN;
  } else {
    double minRvr = getRvrKm(0);
    for (int ii = 1; ii < (int) getRvrKmSize(); ii++) {
      double rvr = getRvrKm(ii);
      if (rvr >= 0 && rvr < minRvr) {
        minRvr = rvr;
      }
    }
    report.rvr = minRvr;
  }

  if (_ceiling_km.getSize() == 0) {
    report.ceiling = STATION_NAN;
  } else {
    report.ceiling = getCeilingKm();
  }

  switch (msgId) {

    case SENSOR_REQUEST:
    case SENSOR_REPORT:
      /* do nothing */
      break;
    
    case STATION_REPORT:
    case STATION_REPORT_ARRAY:
      if (_precip_liquid_mm.getSize() < 2) {
        report.shared.station.liquid_accum2 = STATION_NAN;
      } else {
        report.shared.station.liquid_accum2 = getPrecipLiquidMm(1);
      }
      report.shared.station.Spare1 = STATION_NAN;
      report.shared.station.Spare2 = STATION_NAN;
      break;
      
    case METAR_REPORT:
    case REPORT_PLUS_METAR_XML:
    case REPORT_PLUS_FULL_XML:
      STRncopy(report.shared.metar.weather_str,
               _metar_wx.c_str(), METAR_WX_STR_LEN);
      break;
      
    case PRESSURE_STATION_REPORT:
      if (_pressure_mb.getSize() == 0) {
        report.shared.pressure_station.stn_pres = STATION_NAN;
      } else {
        report.shared.pressure_station.stn_pres = getPressureMb();
      }
      report.shared.pressure_station.Spare1 = STATION_NAN;
      report.shared.pressure_station.Spare2 = STATION_NAN;
      break;

    case METAR_WITH_REMARKS_REPORT:
      strncpy(report.shared.remark_info.stn_indicator,
              getMetarRemStnIndicator().c_str(), STN_INDC_LEN);
      report.shared.remark_info.pwi_no = (ui08) getMetarRemPwiDown();
      report.shared.remark_info.fzra_no = (ui08) getMetarRemFzraDown();
      report.shared.remark_info.ts_no = (ui08) getMetarRemTsDown();
      break;

  } // switch

  // set weather type

  report.weather_type = 0;

  for (int ii = 0; ii < _wx_type.getSize(); ii++) {
    
    wx_type_t wxType = _wx_type._measurements[ii]._value;
    
    switch (wxType) {
    case WxT_RA:
      report.weather_type |= WT_RA;
      break;
    case WxT_SN:
      report.weather_type |= WT_SN;
      break;
    case WxT_UP:
      report.weather_type |= WT_UP;
      break;
    case WxT_FG:
      report.weather_type |= WT_FG;
      break;
    case WxT_DS:
      report.weather_type |= WT_DS;
      break;
    case WxT_FZFG:
      report.weather_type |= WT_FZFG;
      break;
    case WxT_BR:
      report.weather_type |= WT_BR;
      break;
    case WxT_HZ:
      report.weather_type |= WT_HZ;
      break;
    case WxT_SQ:
      report.weather_type |= WT_SQ;
      break;
    case WxT_FC:
      report.weather_type |= WT_FC;
      break;
    case WxT_TS:
      report.weather_type |= WT_TS;
      break;
    case WxT_GR:
      report.weather_type |= WT_GR;
      break;
    case WxT_GS:
      report.weather_type |= WT_GS;
      break;
    case WxT_MFZDZ:
      report.weather_type |= WT_MFZDZ;
      break;
    case WxT_FZRA:
      report.weather_type |= WT_FZRA;
      break;
    case WxT_VA:
      report.weather_type |= WT_VA;
      break;
    case WxT_CLR:
      report.weather_type |= WT_CLR;
      break;
    case WxT_FROST:
      report.weather_type |= WT_FROST;
      break;
    case WxT_SCT:
      report.weather_type |= WT_SCT;
      break;
    case WxT_BKN:
      report.weather_type |= WT_BKN;
      break;
    case WxT_OVC:
      report.weather_type |= WT_OVC;
      break;
    case WxT_FEW:
      report.weather_type |= WT_FEW;
      break;
    case WxT_PE:
      report.weather_type |= WT_PE;
      break;
    case WxT_BLSN:
      report.weather_type |= WT_BLSN;
      break;
    case WxT_FZDZ:
      report.weather_type |= WT_FZDZ;
      break;
    case WxT_DZ:
      report.weather_type |= WT_DZ;
      break;
    case WxT_MRA:
      report.weather_type |= WT_MRA;
      break;
    case WxT_PRA:
      report.weather_type |= WT_PRA;
      break;
    case WxT_MSN:
      report.weather_type |= WT_MSN;
      break;
    case WxT_PSN:
      report.weather_type |= WT_PSN;
      break;
    case WxT_PTS:
      report.weather_type |= WT_PTS;
      break;
    case WxT_MFZRA:
      report.weather_type |= WT_MFZRA;
      break;
    case WxT_PFZRA:
      report.weather_type |= WT_PFZRA;
      break;
    case WxT_SG:
      report.weather_type |= WT_SG;
      break;
    case WxT_PFZDZ:
      report.weather_type |= WT_PFZDZ;
      break;
      default: {}
    } // case

  } // ii

  // station label

  STRncopy(report.station_label, _station_id.c_str(), ST_LABEL_SIZE);

}

///////////////////////////////////////////
// set from station_report_t

void WxObs::setFromStationReport(const station_report_t &report)

{
  
  _basedOnStationReport = true;
  memcpy(&_report, &report, sizeof(station_report_t));

  // id

  char id[ST_LABEL_SIZE + 4];
  MEM_zero(id);
  memcpy(id, report.station_label, ST_LABEL_SIZE);
  _station_id = id;

  // time and location

  _observation_time = report.time;
  _latitude = report.lat;
  _longitude = report.lon;
  _elevation_m = report.alt;

  // weather type

  if (report.weather_type != (ui32) STATION_NAN) {
    int nwx = 0;
    if (report.weather_type & WT_RA) {
      setWeatherType(WxT_RA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_SN) {
      setWeatherType(WxT_SN, nwx);
      nwx++;
    }
    if (report.weather_type & WT_UP) {
      setWeatherType(WxT_UP, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FG) {
      setWeatherType(WxT_FG, nwx);
      nwx++;
    }
    if (report.weather_type & WT_DS) {
      setWeatherType(WxT_DS, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FZFG) {
      setWeatherType(WxT_FZFG, nwx);
      nwx++;
    }
    if (report.weather_type & WT_BR) {
      setWeatherType(WxT_BR, nwx);
      nwx++;
    }
    if (report.weather_type & WT_HZ) {
      setWeatherType(WxT_HZ, nwx);
      nwx++;
    }
    if (report.weather_type & WT_SQ) {
      setWeatherType(WxT_SQ, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FC) {
      setWeatherType(WxT_FC, nwx);
      nwx++;
    }
    if (report.weather_type & WT_TS) {
      setWeatherType(WxT_TS, nwx);
      nwx++;
    }
    if (report.weather_type & WT_GR) {
      setWeatherType(WxT_GR, nwx);
      nwx++;
    }
    if (report.weather_type & WT_GS) {
      setWeatherType(WxT_GS, nwx);
      nwx++;
    }
    if (report.weather_type & WT_MFZDZ) {
      setWeatherType(WxT_MFZDZ, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FZRA) {
      setWeatherType(WxT_FZRA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_VA) {
      setWeatherType(WxT_VA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_CLR) {
      setWeatherType(WxT_CLR, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FROST) {
      setWeatherType(WxT_FROST, nwx);
      nwx++;
    }
    if (report.weather_type & WT_SCT) {
      setWeatherType(WxT_SCT, nwx);
      nwx++;
    }
    if (report.weather_type & WT_BKN) {
      setWeatherType(WxT_BKN, nwx);
      nwx++;
    }
    if (report.weather_type & WT_OVC) {
      setWeatherType(WxT_OVC, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FEW) {
      setWeatherType(WxT_FEW, nwx);
      nwx++;
    }
    if (report.weather_type & WT_PE) {
      setWeatherType(WxT_PE, nwx);
      nwx++;
    }
    if (report.weather_type & WT_BLSN) {
      setWeatherType(WxT_BLSN, nwx);
      nwx++;
    }
    if (report.weather_type & WT_FZDZ) {
      setWeatherType(WxT_FZDZ, nwx);
      nwx++;
    }
    if (report.weather_type & WT_DZ) {
      setWeatherType(WxT_DZ, nwx);
      nwx++;
    }
    if (report.weather_type & WT_MRA) {
      setWeatherType(WxT_MRA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_PRA) {
      setWeatherType(WxT_PRA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_MSN) {
      setWeatherType(WxT_MSN, nwx);
      nwx++;
    }
    if (report.weather_type & WT_PSN) {
      setWeatherType(WxT_PSN, nwx);
      nwx++;
    }
    if (report.weather_type & WT_PTS) {
      setWeatherType(WxT_PTS, nwx);
      nwx++;
    }
    if (report.weather_type & WT_MFZRA) {
      setWeatherType(WxT_MFZRA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_PFZRA) {
      setWeatherType(WxT_PFZRA, nwx);
      nwx++;
    }
    if (report.weather_type & WT_SG) {
      setWeatherType(WxT_SG, nwx);
      nwx++;
    }
    if (report.weather_type & WT_PFZDZ) {
      setWeatherType(WxT_PFZDZ, nwx);
      nwx++;
    }
  }

  // fields

  if (report.temp != (fl32) STATION_NAN) {
    setTempC(report.temp);
  }
  if (report.dew_point != (fl32) STATION_NAN) {
    setDewpointC(report.dew_point);
  }
  if (report.relhum != (fl32) STATION_NAN) {
    setRhPercent(report.relhum);
  }
  if (report.windspd != (fl32) STATION_NAN) {
    setWindSpeedMps(report.windspd);
  }
  if (report.winddir != (fl32) STATION_NAN) {
    setWindDirnDegT(report.winddir);
  }
  if (report.windgust != (fl32) STATION_NAN) {
    setWindGustMps(report.windgust);
  }
  if (report.pres != (fl32) STATION_NAN) {
    setSeaLevelPressureMb(report.pres);
  }
  if (report.liquid_accum != (fl32) STATION_NAN) {
    double accum_secs = (double) report.time - report.accum_start_time;
    setPrecipLiquidMm(report.liquid_accum, accum_secs);
  }
  if (report.precip_rate != (fl32) STATION_NAN) {
    setPrecipRateMmPerHr(report.precip_rate);
  }
  if (report.visibility != (fl32) STATION_NAN) {
    if (report.visibility >= 0) {
      setVisibilityKm(report.visibility);
      setExtinctionPerKm(3.0 / report.visibility);
    } else {
      setExtinctionPerKm(report.visibility * -1.0);
      setVisibilityKm(-3.0 / report.visibility);
    }
  }
  if (report.rvr != (fl32) STATION_NAN) {
    setRvrKm(report.rvr);
  }
  if (report.ceiling != (fl32) STATION_NAN) {
    setCeilingKm(report.ceiling);
  }

  switch (report.msg_id) {

    case SENSOR_REQUEST:
    case SENSOR_REPORT:
      setMetarWx(wxTypes2Str());
      break;
      
    case STATION_REPORT:
    case STATION_REPORT_ARRAY:
      if(report.shared.station.liquid_accum2 != STATION_NAN) {
        double accum_secs = (double) report.time - report.accum_start_time;
        setPrecipLiquidMm(report.shared.station.liquid_accum2, accum_secs, 1);
      }
      setMetarWx(wxTypes2Str());
      break;
      
    case METAR_REPORT:
    case REPORT_PLUS_METAR_XML:
    case REPORT_PLUS_FULL_XML:
      setMetarWx(report.shared.metar.weather_str);
      break;
    
    case PRESSURE_STATION_REPORT:
      if(report.shared.pressure_station.stn_pres != STATION_NAN) {
        setPressureMb(report.shared.pressure_station.stn_pres);
      }
      setMetarWx(wxTypes2Str());
      break;

    case METAR_WITH_REMARKS_REPORT:
      char stn_indicator[STN_INDC_LEN + 4];
      MEM_zero(stn_indicator);
      strncpy(stn_indicator, report.shared.remark_info.stn_indicator,
              STN_INDC_LEN);
      setMetarRemStnIndicator(stn_indicator);
      setMetarRemPwiDown(report.shared.remark_info.pwi_no);
      setMetarRemFzraDown(report.shared.remark_info.fzra_no);
      setMetarRemTsDown(report.shared.remark_info.ts_no);
      setMetarWx(wxTypes2Str());
      break;

  }


}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int WxObs::disassemble(const void *buf, int len)

{

  reset();

  // check if buffer starts with station_report

  msg_id_t msgId;
  bool startsWithReport = checkForReport(buf, len, msgId);
  
  if (!startsWithReport) {
    // no report, must be XML only
    const char *xml = (const char *) buf;
    return _disassembleXml(xml, len);
  }
  
  // starts with report
  
  int iret = 0;
  switch (msgId) {
    case REPORT_PLUS_METAR_XML: {
      // first decode station report, setting internal members
      if (_disassembleReport(buf, len, true)) {
        iret = -1;
      }
      // METAR XML follows report
      int xmlLen = len - sizeof(station_report_t);
      const char *xml = (const char *) buf + sizeof(station_report_t);
      if (_disassembleMetarStringsXml(xml, xmlLen)) {
        iret = -1;
      }
      break;
    }
    case REPORT_PLUS_FULL_XML: {
      // first decode station report, NOT setting internal members
      if (_disassembleReport(buf, len, true)) {
        iret = -1;
      }
      // full XML follows report, decode XML to set members
      int xmlLen = len - sizeof(station_report_t);
      const char *xml = (const char *) buf + sizeof(station_report_t);
      if (_disassembleXml(xml, xmlLen)) {
        iret = -1;
      }
      break;
    }
    default: {
      if (_disassembleReport(buf, len, true)) {
        iret = -1;
      }
    }
  }
  
  return iret;

}

///////////////////////////////////////////////////////////
// Disassembles an XML buffer.
// Returns 0 on success, -1 on failure

int WxObs::_disassembleXml(const char *buf, int len)

{

  _xml = "";
  _xml.append(buf, len - 1);
  string wxObs;
  if (TaXml::readString(_xml, "weather_observation", wxObs)) {
    return -1;
  }

  // find the tags

  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(wxObs, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = wxObs.substr(start, len);

    if (tag == "station_id") {
      TaXml::readString(content, tag, _station_id);
    } else if (tag == "long_name") {
      TaXml::readString(content, tag, _long_name);
    } else if (tag == "observation_time") {
      TaXml::readTime(content, tag, _observation_time);
    } else if (tag == "latitude") {
      TaXml::readDouble(content, tag, _latitude);
    } else if (tag == "longitude") {
      TaXml::readDouble(content, tag, _longitude);
    } else if (tag == "elevation_m") {
      TaXml::readDouble(content, tag, _elevation_m);
    } else if (tag == "metar_text") {
      TaXml::readString(content, tag, _metar_text);
    } else if (tag == "metar_wx") {
      TaXml::readString(content, tag, _metar_wx);
    } else if (tag == "metar_remarks") {
      TaXml::readString(content, tag, _metar_remarks);
    } else if (tag == "metar_rem_stn_indicator") {
      _metar_rem_info_set = true;
      TaXml::readString(content, tag, _metar_rem_stn_indicator);
    } else if (tag == "metar_rem_pwi_down") {
      TaXml::readBoolean(content, tag, _metar_rem_pwi_down);
    } else if (tag == "metar_rem_fzra_down") {
      TaXml::readBoolean(content, tag, _metar_rem_fzra_down);
    } else if (tag == "metar_rem_ts_down") {
      TaXml::readBoolean(content, tag, _metar_rem_ts_down);
    } else if (tag == "ceiling_is_minimum") {
      TaXml::readBoolean(content, tag, _ceiling_is_minimum);
    } else if (tag == "visibility_is_minimum") {
      TaXml::readBoolean(content, tag, _visibility_is_minimum);
    } else if (tag == "temp_c") {
      _setFieldFromXml(tag, content, _temp_c);
    } else if (tag == "min_temp_c") {
      _setFieldFromXml(tag, content, _min_temp_c);
    } else if (tag == "max_temp_c") {
      _setFieldFromXml(tag, content, _max_temp_c);
    } else if (tag == "sea_surface_temp_c") {
      _setFieldFromXml(tag, content, _sea_surface_temp_c);
    } else if (tag == "dewpoint_c") {
      _setFieldFromXml(tag, content, _dewpoint_c);
    } else if (tag == "rh_percent") {
      _setFieldFromXml(tag, content, _rh_percent);
    } else if (tag == "wind_dirn_degt") {
      _setFieldFromXml(tag, content, _wind_dirn_degt);
    } else if (tag == "wind_speed_mps") {
      _setFieldFromXml(tag, content, _wind_speed_mps);
    } else if (tag == "wind_gust_mps") {
      _setFieldFromXml(tag, content, _wind_gust_mps);
    } else if (tag == "visibility_km") {
      _setFieldFromXml(tag, content, _visibility_km);
    } else if (tag == "extinction_per_km") {
      _setFieldFromXml(tag, content, _extinction_per_km);
    } else if (tag == "vert_vis_km") {
      _setFieldFromXml(tag, content, _vert_vis_km);
    } else if (tag == "ceiling_km") {
      _setFieldFromXml(tag, content, _ceiling_km);
    } else if (tag == "rvr_km") {
      _setFieldFromXml(tag, content, _rvr_km);
    } else if (tag == "pressure_mb") {
      _setFieldFromXml(tag, content, _pressure_mb);
    } else if (tag == "msl_pressure_mb") {
      _setFieldFromXml(tag, content, _msl_pressure_mb);
    } else if (tag == "press_tend_mb") {
      _setFieldFromXml(tag, content, _press_tend_mb);
    } else if (tag == "precip_liquid_mm") {
      _setFieldFromXml(tag, content, _precip_liquid_mm);
    } else if (tag == "precip_rate_mmph") {
      _setFieldFromXml(tag, content, _precip_rate_mmph);
    } else if (tag == "snow_depth_mm") {
      _setFieldFromXml(tag, content, _snow_depth_mm);
    } else if (tag == "sky_obsc") {
      _setFieldFromXml(tag, content, _sky_obsc);
    } else if (tag == "wx_type") {
      _setWxTypeFromXml(tag, content, _wx_type);
    }

  }

  return 0;

}

///////////////////////////////////////////////////////////
// Disassembles from METAR strings XML
// Returns 0 on success, -1 on failure

int WxObs::_disassembleMetarStringsXml(const char *buf, int len)

{

  _metarStringsXml = "";
  _metarStringsXml.append(buf, len - 1);

  string metarStrings;
  if (TaXml::readString(_metarStringsXml, "metar_strings", metarStrings)) {
    return -1;
  }

  // find the tags

  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(metarStrings, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = metarStrings.substr(start, len);

    if (tag == "metar_text") {
      TaXml::readString(content, tag, _metar_text);
    } else if (tag == "metar_wx") {
      TaXml::readString(content, tag, _metar_wx);
    } else if (tag == "metar_remarks") {
      TaXml::readString(content, tag, _metar_remarks);
    } else if (tag == "ceiling_is_minimum") {
      TaXml::readBoolean(content, tag, _ceiling_is_minimum);
    } else if (tag == "visibility_is_minimum") {
      TaXml::readBoolean(content, tag, _visibility_is_minimum);
    } 

  }

  return 0;

}

///////////////////////////////////////////////////////////
// Disassembles _report from buffer.
// Optionally set internal data members.
// Returns 0 on success, -1 on failure

int WxObs::_disassembleReport(const void *buf, int len, bool setMembers)
  
{
  
  if (len < (int) sizeof(station_report_t)) {
    return -1;
  }

  station_report_t report;
  memcpy(&report, buf, sizeof(station_report_t));
  station_report_from_be(&report);
  
  if (setMembers) {
    setFromStationReport(report);
  }

  return 0;

}

///////////////////////////////////////////////////////////
// disassemble to get station name and position.
// Efficient routine to get just station details.
// Fills out stationId, lat and lon (deg) and elev (meters)
// Returns 0 on success, -1 on failure

int WxObs::disassembleStationDetails(const void *buf, int len,
                                     string &stationId,
                                     double &latitude,
                                     double &longitude,
                                     double &elevationM)

{

  // check if buffer starts with station_report
  
  msg_id_t msgId;
  bool startsWithReport = checkForReport(buf, len, msgId);
  
  if (startsWithReport) {

    // decode from station report

    if (len < (int) sizeof(station_report_t)) {
      return -1;
    }

    station_report_t report;
    memcpy(&report, buf, sizeof(station_report_t));
    
    char id[ST_LABEL_SIZE + 4];
    MEM_zero(id);
    memcpy(id, report.station_label, ST_LABEL_SIZE);
    stationId = id;

    fl32 lat, lon, elev;
    memcpy(&lat, &report.lat, sizeof(fl32));
    memcpy(&lon, &report.lon, sizeof(fl32));
    memcpy(&elev, &report.alt, sizeof(fl32));

    BE_to_array_32(&lat, sizeof(fl32));
    BE_to_array_32(&lon, sizeof(fl32));
    BE_to_array_32(&elev, sizeof(fl32));

    latitude = lat;
    longitude = lon;
    elevationM = elev;

    return 0;

  }

  // decode from XML

  string xml;
  xml.append((char *) buf, len - 1);

  int iret = 0;
  if (TaXml::readString(xml, "station_id", stationId)) {
    iret = -1;
  }
  if (TaXml::readDouble(xml, "latitude", latitude)) {
    iret = -1;
  }
  if (TaXml::readDouble(xml, "longitude", longitude)) {
    iret = -1;
  }
  if (TaXml::readDouble(xml, "elevation_m", elevationM)) {
    iret = -1;
  }

  return iret;

}

///////////////////////////////////////////////////////////
// Check if buffer starts as report
//
// Sets msgId

bool WxObs::checkForReport(const void *buf, int len, msg_id_t &msgId)

{
  
  if (len < (int) sizeof(station_report_t)) {
    return false;
  }

  ui32 id;
  memcpy(&id, buf, sizeof(ui32));
  BE_to_array_32(&id, sizeof(ui32));

  if (id <= SENSOR_REPORT ||
      id == STATION_REPORT ||
      id == METAR_REPORT ||
      id == PRESSURE_STATION_REPORT ||
      id == METAR_WITH_REMARKS_REPORT ||
      id == REPORT_PLUS_METAR_XML ||
      id == REPORT_PLUS_FULL_XML) {
    msgId = (msg_id_t) id;
    return true;
  }

  return false;

}

///////////////
// print object

void WxObs::print(ostream &out, string spacer /* = ""*/ ) const

{
  
  if (_basedOnStationReport) {
    // use legacy printing
    _printReport(out, spacer);
    return;
  }

  out << spacer << "========== Weather observation ==========" << endl;

  out << spacer << "---------- OBS time and place -----------" << endl;
  out << spacer << "stationId: " << _station_id << endl;
  out << spacer << "longName: " << _long_name << endl;
  out << spacer << "time: " << DateTime::strm(_observation_time) << endl;
  out << spacer << "latitude: " << _latitude << endl;
  out << spacer << "longitude: " << _longitude << endl;
  out << spacer << "elevation_m: " << _elevation_m << endl;

  if (_metar_text.size() > 0 ||
      _metar_wx.size() > 0 ||
      _metar_remarks.size() > 0) {

    out << spacer << "------------- METAR strings -------------" << endl;
    out << spacer << "metarText: " << _metar_text << endl;
    out << spacer << "metarWx: " << _metar_wx << endl;
    out << spacer << "metarRemarks: " << _metar_remarks << endl;
    if (_visibility_is_minimum) {
      out << spacer << "visibility is a minimum" << endl;
    }
    if (_ceiling_is_minimum) {
      out << spacer << "ceiling is a minimum" << endl;
    }
  }

  if (_metar_rem_info_set) {
    out << spacer << "---------- METAR remark info ------------" << endl;
    out << spacer << "rem station indicator: "
        << _metar_rem_stn_indicator << endl;
    out << spacer << "pwi_down: " << _metar_rem_pwi_down << endl;
    out << spacer << "fzra_down: " << _metar_rem_fzra_down << endl;
    out << spacer << "ts_down: " << _metar_rem_ts_down << endl;
  }

    out << spacer << "------------ FIELD data -----------------" << endl;
  _temp_c.print(out, "temp_c", spacer);
  _min_temp_c.print(out, "min_temp_c", spacer);
  _max_temp_c.print(out, "max_temp_c", spacer);
  _sea_surface_temp_c.print(out, "sea_surface_temp_c", spacer);
  _dewpoint_c.print(out, "dewpoint_c", spacer);
  _rh_percent.print(out, "rh_percent", spacer);
  printWindDirn(out, "wind_dirn_degt", spacer);
  _wind_speed_mps.print(out, "wind_speed_mps", spacer);
  _wind_gust_mps.print(out, "wind_gust_mps", spacer);
  _visibility_km.print(out, "visibility_km", spacer);
  if (_visibility_is_minimum) {
    out << spacer << "visibility is a minimum" << endl;
  }
  _extinction_per_km.print(out, "extinction_per_km", spacer);
  _vert_vis_km.print(out, "vert_vis_km", spacer);
  _ceiling_km.print(out, "ceiling_km", spacer);
  if (_ceiling_is_minimum) {
    out << spacer << "ceiling is a minimum" << endl;
  }
  _rvr_km.print(out, "rvr_km", spacer);
  _pressure_mb.print(out, "pressure_mb", spacer);
  _msl_pressure_mb.print(out, "msl_pressure_mb", spacer);
  _press_tend_mb.print(out, "press_tend_mb", spacer);
  _precip_liquid_mm.print(out, "precip_liquid_mm", spacer);
  _precip_rate_mmph.print(out, "precip_rate_mmph", spacer);
  _snow_depth_mm.print(out, "snow_depth_mm", spacer);
  _sky_obsc.print(out, "sky_obsc", spacer);
  printWxTypes(out, "wx_type", spacer);

}

///////////////
// print as XML

void WxObs::printAsXml(ostream &out,
                       int startIndentLevel) const

{

  if (_xml.size() == 0) {
    loadXml(_xml, false, startIndentLevel);
  }
  else {
    std::string::iterator it = find_if (_xml.begin(), _xml.end(), (int(*)(int))iscntrl);
    if (it != _xml.end() ) { // found control characters, let's try to reload
      _xml = "";
      loadXml(_xml, false, startIndentLevel);
    }      
  } 
  out << _xml;
}
  
//////////////////////
// print wind dirn
    
void WxObs::printWindDirn(ostream &out,
                          const string &label,
                          string spacer) const {

  for (int ii = 0; ii < (int) _wind_dirn_degt._measurements.size(); ii++) {
    out << spacer << label;
    if ( _wind_dirn_degt._measurements.size() > 1) {
      out << "[" << (ii) << "]";
    }
    out << ": ";
    if (_wind_dirn_degt._measurements[ii]._qualifier != missing &&
        _wind_dirn_degt._measurements[ii]._qual_label == "variable") {
      out << "variable" << endl;
    } else {
      out << _wind_dirn_degt._measurements[ii]._value << endl;
    }
    if (_wind_dirn_degt._measurements[ii]._info.size() > 0) {
      out << spacer << "  Info: " << _wind_dirn_degt._measurements[ii]._info << endl;
    }
  } // ii

}

//////////////////////
// print weather types
    
void WxObs::printWxTypes(ostream &out,
                         const string &label,
                         string spacer) const {

  for (int ii = 0; ii < (int) _wx_type._measurements.size(); ii++) {
    out << spacer << label;
    if ( _wx_type._measurements.size() > 1) {
      out << "[" << (ii) << "]";
    }
    out << ": " << wxType2Str(_wx_type._measurements[ii]._value) << endl;
    if (_wx_type._measurements[ii]._info.size() > 0) {
      out << spacer << "    Info: "
          << _wx_type._measurements[ii]._info << endl;
    }
  } // ii
}

////////////////////////////////////////////////////////////////////////////
// Print the station report to the given stream.
// spacer string is prepended to each output line so
// you can indent the output, if desired.

void WxObs::_printReport(ostream &out,
                         string spacer /* = ""*/ ) const
  
{

  out << spacer << "WxObs: weather observation" << endl;
  out << spacer << "Stored as station_report_t struct" << endl;

  string str;
  char tmpStr[1024];
  time_t print_time;
  
  sprintf(tmpStr, "%smsg_id = %d\n", spacer.c_str(), _report.msg_id);
  str += tmpStr;

  print_time = _report.time;
  sprintf(tmpStr, "%stime = %s", spacer.c_str(), asctime(gmtime(&print_time)));
  str += tmpStr;
  
  print_time = _report.accum_start_time;
  sprintf(tmpStr, "%saccum_start_time = %s",
	  spacer.c_str(), asctime(gmtime(&print_time)));
  str += tmpStr;
  
  sprintf(tmpStr, "%sweather_type = %s\n",
	  spacer.c_str(), weather_type2string(_report.weather_type));
  str += tmpStr;
  
  if(_report.lat != STATION_NAN) {
      sprintf(tmpStr, "%slat = %.3f deg\n", spacer.c_str(), _report.lat);
  } else {
      sprintf(tmpStr, "%slat = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.lon != STATION_NAN) {
      sprintf(tmpStr, "%slon = %.3f deg\n", spacer.c_str(), _report.lon);
  } else {
      sprintf(tmpStr, "%slon = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.alt != STATION_NAN) {
      sprintf(tmpStr, "%salt = %.3f m\n", spacer.c_str(), _report.alt);
  } else {
      sprintf(tmpStr, "%salt = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.temp != STATION_NAN) {
      sprintf(tmpStr, "%stemp = %.3f deg C\n", spacer.c_str(), _report.temp);
  } else {
      sprintf(tmpStr, "%stemp = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.dew_point != STATION_NAN) {
      sprintf(tmpStr, "%sdew_point = %.3f deg C\n", spacer.c_str(), _report.dew_point);
  } else {
      sprintf(tmpStr, "%sdew_point = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.relhum != STATION_NAN) {
      sprintf(tmpStr, "%srelhum = %.3f %%\n", spacer.c_str(), _report.relhum);
  } else {
      sprintf(tmpStr, "%srelhum = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.windspd != STATION_NAN) {
      sprintf(tmpStr, "%swindspd = %.3f m/s\n", spacer.c_str(), _report.windspd);
  } else {
      sprintf(tmpStr, "%swindspd = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.winddir != STATION_NAN) {
    if (SignBit::isSet(_report.winddir)) { // variable
      sprintf(tmpStr, "%swinddir = variable\n", spacer.c_str());
    } else {
      sprintf(tmpStr, "%swinddir = %.3f deg\n", spacer.c_str(), _report.winddir);
    }
  } else {
      sprintf(tmpStr, "%swinddir = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.windgust != STATION_NAN) {
      sprintf(tmpStr, "%swindgust = %.3f m/s\n", spacer.c_str(), _report.windgust);
  } else {
      sprintf(tmpStr, "%swindgust = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.pres != STATION_NAN) {
      sprintf(tmpStr, "%spres = %.3f mb\n", spacer.c_str(), _report.pres);
  } else {
      sprintf(tmpStr, "%spres = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.liquid_accum != STATION_NAN) {
      sprintf(tmpStr, "%sliquid_accum = %.3f mm\n",
	  spacer.c_str(), _report.liquid_accum);
  } else {
      sprintf(tmpStr, "%sliquid_accum = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.precip_rate != STATION_NAN) {
      sprintf(tmpStr, "%sprecip_rate = %.3f mm/hr\n", 
	  spacer.c_str(), _report.precip_rate);
  } else {
      sprintf(tmpStr, "%sprecip_rate = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  if(_report.visibility != STATION_NAN) {
    if (_report.visibility >= 0) {
      sprintf(tmpStr, "%svisibility = %.3f km\n", spacer.c_str(), _report.visibility);
    } else {
      sprintf(tmpStr, "%sExtinction coeff: = %.3f /km\n", spacer.c_str(), _report.visibility * -1.0);
    }
  } else {
      sprintf(tmpStr, "%svisibility = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;
   
  if(_report.rvr != STATION_NAN) {
      sprintf(tmpStr, "%srvr = %.3f km\n", spacer.c_str(), _report.rvr);
  } else {
      sprintf(tmpStr, "%srvr = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;
   
  if(_report.ceiling != STATION_NAN) {
      sprintf(tmpStr, "%sceiling = %.3f km\n", spacer.c_str(), _report.ceiling);
  } else {
      sprintf(tmpStr, "%sceiling = NOT SET\n", spacer.c_str());
  }
  str += tmpStr;

  switch (_report.msg_id)
  {
  case SENSOR_REQUEST :
  case SENSOR_REPORT :
    /* do nothing */
    break;
    
  case STATION_REPORT :
  case STATION_REPORT_ARRAY :
    if(_report.shared.station.liquid_accum2 != STATION_NAN) {
      sprintf(tmpStr, "%sliquid_accum2 = %.3f mm\n",
	      spacer.c_str(), _report.shared.station.liquid_accum2);
    } else {
      sprintf(tmpStr, "%sliquid_accum2 = NOT SET\n", spacer.c_str());
    }
    str += tmpStr;
	   
    if(_report.shared.station.Spare1 != STATION_NAN) {
      sprintf(tmpStr, "%sSpare1 = %.3f \n",
	      spacer.c_str(), _report.shared.station.Spare1);
    } else {
      sprintf(tmpStr, "%sSpare1 = NOT SET\n", spacer.c_str());
    }
    str += tmpStr;
    
    if(_report.shared.station.Spare2 != STATION_NAN) {
      sprintf(tmpStr, "%sSpare2 = %.3f \n",
	      spacer.c_str(), _report.shared.station.Spare2);
    } else {
      sprintf(tmpStr, "%sSpare2 = NOT SET\n", spacer.c_str());
    }
    str += tmpStr;
    break;

  case METAR_REPORT :
  case REPORT_PLUS_METAR_XML:
  case REPORT_PLUS_FULL_XML:
    sprintf(tmpStr, "%sMetar weather = <%s>\n",
	    spacer.c_str(), _report.shared.metar.weather_str);
    str += tmpStr;
    break;
    
  case PRESSURE_STATION_REPORT :
    if(_report.shared.pressure_station.stn_pres != STATION_NAN) {
      sprintf(tmpStr, "%sstn_pres = %.3f mb\n",
	      spacer.c_str(), _report.shared.pressure_station.stn_pres);
    } else {
      sprintf(tmpStr, "%sstn_pres = NOT SET\n", spacer.c_str());
    }
    str += tmpStr;
	   
    if(_report.shared.pressure_station.Spare1 != STATION_NAN) {
      sprintf(tmpStr, "%sSpare1 = %.3f \n",
	      spacer.c_str(), _report.shared.pressure_station.Spare1);
    } else {
      sprintf(tmpStr, "%sSpare1 = NOT SET\n", spacer.c_str());
    }
    str += tmpStr;
    
    if(_report.shared.pressure_station.Spare2 != STATION_NAN) {
      sprintf(tmpStr, "%sSpare2 = %.3f \n",
	      spacer.c_str(), _report.shared.pressure_station.Spare2);
    } else {
      sprintf(tmpStr, "%sSpare2 = NOT SET\n", spacer.c_str());
    }
    str += tmpStr;
    break;
  case METAR_WITH_REMARKS_REPORT :
    sprintf(tmpStr, "%sStation indicator = <%s>\n",
	    spacer.c_str(), _report.shared.remark_info.stn_indicator);
    str += tmpStr;
    sprintf(tmpStr, "%spwi_down = <%d>\n",
	    spacer.c_str(), _report.shared.remark_info.pwi_no);
    str += tmpStr;
    sprintf(tmpStr, "%sfzra_down = <%d>\n",
	    spacer.c_str(), _report.shared.remark_info.fzra_no);
    str += tmpStr;
    sprintf(tmpStr, "%sts_down = <%d>\n",
	    spacer.c_str(), _report.shared.remark_info.ts_no);
    str += tmpStr;
    break;
    
    
  }
  
  sprintf(tmpStr, "%sstation_label = <%s>\n",
	  spacer.c_str(), _report.station_label);
  str += tmpStr;
  out << str;
  
  if (_report.msg_id == REPORT_PLUS_METAR_XML) {
    if (_metarStringsXml.size() > 0) {
      out << endl;
      out << _metarStringsXml << endl;
    }
  } else if (_report.msg_id == REPORT_PLUS_FULL_XML) {
    if (_xml.size() > 0) {
      out << endl;
      out << _xml;
    }
  }
  
}

////////////////////////////////
// Convert weather type to string

string WxObs::wxType2Str(wx_type_t wxType)

{

  switch (wxType) {

    case WxT_RA:
      return "RA";
    case WxT_SN:
      return "SN";
    case WxT_UP:
      return "UP";
    case WxT_FG:
      return "FG";
    case WxT_DS:
      return "DS";
    case WxT_FZFG:
      return "FZFG";
    case WxT_BR:
      return "BR";
    case WxT_HZ:
      return "HZ";
    case WxT_SQ:
      return "SQ";
    case WxT_FC:
      return "FC";
    case WxT_TS:
      return "TS";
    case WxT_GR:
      return "GR";
    case WxT_GS:
      return "GS";
    case WxT_MFZDZ:
      return "MFZDZ";
    case WxT_FZRA:
      return "FZRA";
    case WxT_VA:
      return "VA";
    case WxT_CLR:
      return "CLR";
    case WxT_FROST:
      return "FROST";
    case WxT_SCT:
      return "SCT";
    case WxT_BKN:
      return "BKN";
    case WxT_OVC:
      return "OVC";
    case WxT_FEW:
      return "FEW";
    case WxT_PE:
      return "PE";
    case WxT_BLSN:
      return "BLSN";
    case WxT_FZDZ:
      return "FZDZ";
    case WxT_DZ:
      return "DZ";
    case WxT_MRA:
      return "MRA";
    case WxT_PRA:
      return "PRA";
    case WxT_MSN:
      return "MSN";
    case WxT_PSN:
      return "PSN";
    case WxT_PTS:
      return "PTS";
    case WxT_MFZRA:
      return "MFZRA";
    case WxT_PFZRA:
      return "PFZRA";
    case WxT_SG:
      return "SG";
    case WxT_PFZDZ:
      return "PFZDZ";
    case WxT_DU:
      return "DU";
    case WxT_SA:
      return "SA";
    case WxT_SS:
      return "SS";
    default:
      return "MISSING";

  }

  return "UNKNOWN";

}

/////////////////////////////////////////
// Convert weather type vector to string

string WxObs::wxTypes2Str() const

{

  string wxTypes;
  for (int ii = 0; ii < getWeatherTypeSize(); ii++) {
    if (ii != 0) {
      wxTypes += " ";
    }
    wxTypes += wxType2Str(getWeatherType(ii));
  }
  return wxTypes;

}

/////////////////////////////////
// Convert string to weather type

wx_type_t WxObs::str2WxType(const string &wxStr)

{

  if (wxStr == "RA") {
    return WxT_RA;
  } else if (wxStr == "SN") {
    return WxT_SN;
  } else if (wxStr == "UP") {
    return WxT_UP;
  } else if (wxStr == "FG") {
    return WxT_FG;
  } else if (wxStr == "DS") {
    return WxT_DS;
  } else if (wxStr == "FZFG") {
    return WxT_FZFG;
  } else if (wxStr == "BR") {
    return WxT_BR;
  } else if (wxStr == "HZ") {
    return WxT_HZ;
  } else if (wxStr == "SQ") {
    return WxT_SQ;
  } else if (wxStr == "FC") {
    return WxT_FC;
  } else if (wxStr == "TS") {
    return WxT_TS;
  } else if (wxStr == "GR") {
    return WxT_GR;
  } else if (wxStr == "GS") {
    return WxT_GS;
  } else if (wxStr == "MFZDZ") {
    return WxT_MFZDZ;
  } else if (wxStr == "FZRA") {
    return WxT_FZRA;
  } else if (wxStr == "VA") {
    return WxT_VA;
  } else if (wxStr == "CLR") {
    return WxT_CLR;
  } else if (wxStr == "FROST") {
    return WxT_FROST;
  } else if (wxStr == "SCT") {
    return WxT_SCT;
  } else if (wxStr == "BKN") {
    return WxT_BKN;
  } else if (wxStr == "OVC") {
    return WxT_OVC;
  } else if (wxStr == "FEW") {
    return WxT_FEW;
  } else if (wxStr == "PE") {
    return WxT_PE;
  } else if (wxStr == "BLSN") {
    return WxT_BLSN;
  } else if (wxStr == "FZDZ") {
    return WxT_FZDZ;
  } else if (wxStr == "DZ") {
    return WxT_DZ;
  } else if (wxStr == "MRA") {
    return WxT_MRA;
  } else if (wxStr == "PRA") {
    return WxT_PRA;
  } else if (wxStr == "MSN") {
    return WxT_MSN;
  } else if (wxStr == "PSN") {
    return WxT_PSN;
  } else if (wxStr == "PTS") {
    return WxT_PTS;
  } else if (wxStr == "MFZRA") {
    return WxT_MFZRA;
  } else if (wxStr == "PFZRA") {
    return WxT_PFZRA;
  } else if (wxStr == "SG") {
    return WxT_SG;
  } else if (wxStr == "PFZDZ") {
    return WxT_PFZDZ;
  } else if (wxStr == "DU") {
    return WxT_DU;
  } else if (wxStr == "SA") {
    return WxT_SA;
  } else if (wxStr == "SS") {
    return WxT_SS;
  } else if (wxStr == "MISSING") {
    return WxT_MISSING;
  } else {
    return WxT_UNKNOWN;
  }

}

////////////////////////////
// add a double field as XML

void WxObs::_addFieldAsXml(const WxObsField &field,
                           const string &tag,
                           int level,
                           string &xml) const

{                                     

  for (int ii = 0; ii < (int) field._measurements.size(); ii++) {

    const WxObsMeasurement &meas = field._measurements[ii];

    // set up attributes

    vector<TaXml::attribute> attrs;
    
    // add qualifier attribute?

    if (meas._qualifier != missing) {
      string qualLabel(meas._qual_label);
      if (meas._qual_label.size() == 0) {
        qualLabel = "qualifier";
      }
      TaXml::addDoubleAttr(qualLabel, meas._qualifier, attrs);
    }
    
    // add info attribute?

    if (meas._info.size() > 0) {
      string attrName("info");
      string attrVal(meas._info);
      TaXml::addStringAttr(attrName, attrVal, attrs);
    }

    // write

    xml += TaXml::writeDouble(tag, level, attrs, meas._value);

  } // ii
  
}
  
/////////////////////////////////
// add weather type as XML

void WxObs::_addWxTypeAsXml(const WxTypeField &field,
                            const string &tag,
                            int level,
                            string &xml) const

{                                     

  for (int ii = 0; ii < (int) field._measurements.size(); ii++) {

    const WxTypeMeasurement &meas = field._measurements[ii];

    // set up attributes

    vector<TaXml::attribute> attrs;
    
    // add info attribute?

    if (meas._info.size() > 0) {
      string attrName("info");
      string attrVal(meas._info);
      TaXml::addStringAttr(attrName, attrVal, attrs);
    }

    // write

    string wx = wxType2Str(meas._value);
    xml += TaXml::writeString(tag, level, attrs, wx);

  } // ii
  
}

////////////////////////////
// set field from XML

void WxObs::_setFieldFromXml(const string &tag,
                             const string &content,
                             WxObsField &field)

{                                     
  
  vector<TaXml::attribute> attrs;
  double val;
  TaXml::readDouble(content, tag, val, attrs);
  int index = field.addValue(val);
  
  for (int ii = 0; ii < (int) attrs.size(); ii++) {
    const TaXml::attribute &attr = attrs[ii];
    if (attr.getName() == "info") {
      string info = attr.getVal();
      field.setInfo(info, index);
    } else {
      double qualVal;
      if (sscanf(attr.getVal().c_str(), "%lg", &qualVal) == 1) {
        field.setQualifier(qualVal, index);
        string qualLabel = attr.getName();
        field.setQualLabel(qualLabel, index);
      }
    }
  }

}
  
////////////////////////////
// set weather type from XML

void WxObs::_setWxTypeFromXml(const string &tag,
                              const string &content,
                              WxTypeField &field)

{                                     
  
  vector<TaXml::attribute> attrs;
  string wxStr;
  TaXml::readString(content, tag, wxStr, attrs);
  wx_type_t wxType = str2WxType(wxStr);
  int index = field.addValue(wxType);
  
  for (int ii = 0; ii < (int) attrs.size(); ii++) {
    const TaXml::attribute &attr = attrs[ii];
    if (attr.getName() == "info") {
      string info = attr.getVal();
      field.setInfo(info, index);
    }
  }

}
  
//////////////////////////////////////////
// load up WxObs object from decoded metar
//
// returns 0 on success, -1 on failure

int WxObs::setFromDecodedMetar(const string &metarText,
                               const string &stationName,
                               const Decoded_METAR &dcdMetar,
                               time_t valid_time,
                               double lat,
                               double lon,
                               double alt)
  
{

  bool valid = false;
  int i;

  reset();

  // obs time, station name and location

  char id[8];
  MEM_zero(id);
  strncpy(id, dcdMetar.stnid, 4);
  setStationId(id);

  setLongName(stationName);
  setObservationTime(valid_time);
  setLatitude(lat);
  setLongitude(lon);
  setElevationM(alt);

  // temp, dewpoint and RH

  if(dcdMetar.temp != MAXINT) {
    addTempC(dcdMetar.temp);
    valid = TRUE;
  }
  
  if(dcdMetar.dew_pt_temp != MAXINT) {
    addDewpointC(dcdMetar.dew_pt_temp);
    valid = TRUE;
  }
  
  if (dcdMetar.temp != MAXINT &&
      dcdMetar.dew_pt_temp != MAXINT) {
    double rh = _relh((double) dcdMetar.temp, (double) dcdMetar.dew_pt_temp);
    addRhPercent(rh);
  }

  // wind speed and dirn

  if(dcdMetar.winData.windSpeed != MAXINT) {
    valid = TRUE;
    if (!strcmp(dcdMetar.winData.windUnits, "MPS")) {
      // m/s
      addWindSpeedMps(dcdMetar.winData.windSpeed);
    } else if (!strcmp(dcdMetar.winData.windUnits, "KMH")) {
      // kmh
      addWindSpeedMps(dcdMetar.winData.windSpeed / MPERSEC_TO_KMPERHOUR);
    } else {
      // assume knots
      addWindSpeedMps(dcdMetar.winData.windSpeed / NMH_PER_MS);
    }
  }
  
  if(dcdMetar.winData.windGust != MAXINT) {
    valid = TRUE;
    if (!strcmp(dcdMetar.winData.windUnits, "MPS")) {
      // m/s
      addWindGustMps(dcdMetar.winData.windGust);
    } else if (!strcmp(dcdMetar.winData.windUnits, "KMH")) {
      // kmh
      addWindGustMps(dcdMetar.winData.windGust / MPERSEC_TO_KMPERHOUR);
    } else {
      // assume knots
      addWindGustMps(dcdMetar.winData.windGust / NMH_PER_MS);
    }
  }
  
  if (dcdMetar.winData.windVRB) {
    addWindDirnVariable(); // set for variable winds
  } else if (dcdMetar.winData.windDir != MAXINT) {
    valid = TRUE;
    addWindDirnDegT(dcdMetar.winData.windDir);
  }

  // pressure

  if(dcdMetar.hectoPasc_altstng != MAXINT) {
    valid = TRUE;
    addSeaLevelPressureMb(dcdMetar.hectoPasc_altstng);
  }

  if (dcdMetar.inches_altstng !=  (double) MAXINT) {
    valid = TRUE;
    addSeaLevelPressureInHg(dcdMetar.inches_altstng);
  }

  if(dcdMetar.SLP != MAXINT) {
    valid = TRUE;
    addSeaLevelPressureQFFMb(dcdMetar.SLP);
  }

  // precip

  if(dcdMetar.precip_24_amt != MAXINT) {
    valid = TRUE;
    addPrecipLiquidMm(dcdMetar.precip_24_amt, 86400);
  }
  
  if(dcdMetar.hourlyPrecip != MAXINT) {
    valid = TRUE;
    addPrecipRateMmPerHr(dcdMetar.hourlyPrecip, 3600);
  }

  // visibility

  if(dcdMetar.prevail_vsbySM != MAXINT) {
    valid = TRUE;
    addVisibilityKm(dcdMetar.prevail_vsbySM * KM_PER_MI);

    // check that visibility is a minimum
     _visibility_is_minimum = false;
    if( strcmp(dcdMetar.charPrevailVsby,"CAVOK") == 0 ) {
      _visibility_is_minimum = true;
    }
  }

  // RVR

  for(i = 0; i < 12; i++) {
    if (dcdMetar.RRVR[i].visRange != MAXINT) {
      valid = TRUE;
      addRvrKm(dcdMetar.RRVR[i].visRange);
    }
  }

  // ceiling

  _ceiling_is_minimum = false;
  if (!strcmp(dcdMetar.cldTypHgt[0].cloud_type, "CLR") ||
             !strcmp(dcdMetar.cldTypHgt[0].cloud_type, "SKC") ||
	     !strcmp(dcdMetar.cldTypHgt[0].cloud_type, "NSC") ||
             !strcmp(dcdMetar.charPrevailVsby,"CAVOK") ) {
    // MAX_CEILING_KM is defined in station_report.h
    addCeilingKm(MAX_CEILING_KM);  
    _ceiling_is_minimum = true;

  } else if (dcdMetar.Ceiling != MAXINT) {
    
    valid = TRUE;
    addCeilingKm(dcdMetar.Ceiling * 0.0003048);
    
  } else if (dcdMetar.Estimated_Ceiling != MAXINT) {
    
    valid = TRUE;
    addCeilingKm(dcdMetar.Estimated_Ceiling * 0.0003048);
    
  } else {
    
    double min_ceiling = MAX_CEILING_KM;
    _ceiling_is_minimum = true;

    for (i = 0; i < 6; i++) {
      if(dcdMetar.cldTypHgt[i].cloud_hgt_meters != MAXINT) {
        if (dcdMetar.cldTypHgt[i].cloud_type[0] == 'X' ||
            !strcmp(dcdMetar.cldTypHgt[i].cloud_type, "BKN") ||
            !strcmp(dcdMetar.cldTypHgt[i].cloud_type, "VV") ||
            !strcmp(dcdMetar.cldTypHgt[i].cloud_type, "OVC")) {
          double ceiling = dcdMetar.cldTypHgt[i].cloud_hgt_meters / 1000.0;

	  /*
	   * need to allow for a ceiling above MAX_CEILING_KM,
	   * if it is set in METAR
	   */
	  if(_ceiling_is_minimum) {
	    min_ceiling = ceiling;
	  }

	  _ceiling_is_minimum = false;
          valid = TRUE;

          if (ceiling < min_ceiling) {
            min_ceiling = ceiling;
          }

        }
      }
    }

    addCeilingKm(min_ceiling);

  }

  // weather types and wx string
  // Search through the 10 remark slots for condition indicators.

  string wxStr;
  
  for( i = 0; i < 10; i++ ) {
    
    const char *wxPtr = dcdMetar.WxObstruct[i];

    if (wxPtr != NULL && strlen(wxPtr) > 0) {
      if (i > 0) {
        wxStr += " ";
      }
      wxStr += wxPtr;
    }
    
    if(strstr(wxPtr,"RA") != NULL) {
      if(strstr(wxPtr,"+RA") != NULL) {
	addWeatherType(WxT_PRA);
      } else if(strstr(wxPtr,"-RA") != NULL) {
	addWeatherType(WxT_MRA);
      } else {
	addWeatherType(WxT_RA);
      }
    }
    
    if(strstr(wxPtr,"SN") != NULL) {
      if(strstr(wxPtr,"+SN") != NULL) {
	addWeatherType(WxT_PSN);
      } else if(strstr(wxPtr,"-SN") != NULL) {
	addWeatherType(WxT_MSN);
      } else {
	addWeatherType(WxT_SN);
      }
    }
    
    if(strstr(wxPtr,"IC") != NULL) {
      addWeatherType(WxT_SN);
    }
    
    if(strstr(wxPtr,"PE") != NULL) {
      addWeatherType(WxT_PE);
    }
    
    if(strstr(wxPtr,"SG") != NULL) { // Fudge snow grains to snow pellets.
      addWeatherType(WxT_PE);
    }
    
    if(strstr(wxPtr,"UP") != NULL) {
      addWeatherType(WxT_UP);
    }
    
    if(strstr(wxPtr,"FG") != NULL) {
      addWeatherType(WxT_FG);
    }
    
    if(strstr(wxPtr,"FZFG") != NULL) {
      addWeatherType(WxT_FZFG);
    }
    
    if(strstr(wxPtr,"BR") != NULL) {
      addWeatherType(WxT_BR);
    }
    
    if(strstr(wxPtr,"HZ") != NULL) {
      addWeatherType(WxT_HZ);
    }
    
    if(strstr(wxPtr,"SQ") != NULL) {
      addWeatherType(WxT_SQ);
    }
    
    if(strstr(wxPtr,"DS") != NULL) {
      addWeatherType(WxT_DS);
    }
    
    if(strstr(wxPtr,"SS") != NULL) { 
      addWeatherType(WxT_SS);
    }
    
    if(strstr(wxPtr,"DU") != NULL) {
      addWeatherType(WxT_DU);
    }
    
    if(strstr(wxPtr,"SA") != NULL) { 
      addWeatherType(WxT_SA);
    }
    
    if(strstr(wxPtr,"FC") != NULL) {
      addWeatherType(WxT_FC);
    }
    
    if(strstr(wxPtr,"TS") != NULL) {
      if(strstr(wxPtr,"+TS") != NULL) {
	addWeatherType(WxT_PTS);
      } else {
	addWeatherType(WxT_TS);
      }
    }
    
    if(strstr(wxPtr,"GR") != NULL) {
      addWeatherType(WxT_GR);
    }
    
    if(strstr(wxPtr,"GS") != NULL) { // Fudge Small Hail to Snow Pellets 
      addWeatherType(WxT_PE);
    }
    
    if(strstr(wxPtr,"FZRA") != NULL) {
      if(strstr(wxPtr,"+FZRA") != NULL) {
	addWeatherType(WxT_PFZRA);
      } else if(strstr(wxPtr,"-FZRA") != NULL) {
	addWeatherType(WxT_MFZRA);
      } else {
	addWeatherType(WxT_FZRA);
      }
    }
    
    if(strstr(wxPtr,"VA") != NULL) {
      addWeatherType(WxT_VA);
    }
    
    if(strstr(wxPtr,"BLSN") != NULL) {
      addWeatherType(WxT_BLSN);
    }
    
    if(strstr(wxPtr,"DZ") != NULL) {
      addWeatherType(WxT_DZ);
    }
    
    if(strstr(wxPtr,"FZDZ") != NULL) {
      
      if(strstr(wxPtr,"-FZDZ") != NULL) {
        addWeatherType(WxT_MFZDZ);
      } else if(strstr(wxPtr,"+FZDZ") != NULL) {
        addWeatherType(WxT_PFZDZ);
      } else {
        addWeatherType(WxT_FZDZ);
      }
    
    }

  }

  // sky obscuration

  // NOTE: these values do not match the defined constants (i.e. CLOUD_FEW)
  // used in this class. The defined constants are an average of the
  // fractions of cloud cover that can be assigned to the enumerated types.

  // WxObs::skyObscurationFractionToString() will take either set of values
  // and return the correct string.

  // -P. Prestopnik

  for (i = 0; i < 6; i++) {
    if(dcdMetar.cldTypHgt[i].cloud_hgt_meters != MAXINT) {
      double htKm = dcdMetar.cldTypHgt[i].cloud_hgt_meters / 1000.0;
      if (!strcmp(dcdMetar.cldTypHgt[i].cloud_type, "OVC")) {
        addSkyObscuration(1.0, htKm);
        addWeatherType(WxT_OVC);
      } else if (!strcmp(dcdMetar.cldTypHgt[i].cloud_type, "BKN")) {
        addSkyObscuration(0.75, htKm);
        addWeatherType(WxT_BKN);
      } else if (!strcmp(dcdMetar.cldTypHgt[i].cloud_type, "SCT")) {
        addSkyObscuration(0.5, htKm);
        addWeatherType(WxT_SCT);
      } else if (!strcmp(dcdMetar.cldTypHgt[i].cloud_type, "FEW")) {
        addSkyObscuration(0.25, htKm);
        addWeatherType(WxT_FEW);
      } else if (!strcmp(dcdMetar.cldTypHgt[i].cloud_type, "CLR")) {
        addSkyObscuration(0.0, 0.0);
      }
    }
  }

  if (wxStr.size() > 0) {
    setMetarWx(wxStr);
  }

  // metar text
  // remove '=' and trailing return

  string tmpStr;
  for (int ii = 0; ii < (int) metarText.size(); ii++) {
    bool ignore = false;
    if (ii > ((int) metarText.size() - 4) && metarText[ii] == '\n') {
      ignore = true;
    }
    if (metarText[ii] == '=') {
      ignore = true;
    }
    if (!ignore) {
      tmpStr += metarText[ii];
    }
  }
  setMetarText(tmpStr);

#ifdef NOTNOW
  // metar text
  // remove '=' and extra white space

  vector<string> toks;
  _tokenize(metarText, " =\n\r\t", toks);
  string tmpStr;
  for (int ii = 0; ii < (int) toks.size(); ii++) {
    if (tmpStr.size() != 0) {
      tmpStr += " ";
    }
    tmpStr += toks[ii];
  }
  setMetarText(tmpStr);
#endif

  // remarks

  if (dcdMetar.autoIndicator[0] != '\0'){
    setMetarRemStnIndicator(dcdMetar.autoIndicator);
  }

  vector<string> toks;
  _tokenize(metarText, " =\n\r\t", toks);
  string remarks;
  bool remFound = false;
  for (int ii = 0; ii < (int) toks.size(); ii++) {
    if (toks[ii] == "RMK") {
      remFound = true;
    }
    if (remFound) {
      if (remarks.size() > 0) {
        remarks += " ";
      }
      remarks += toks[ii];
    }
    if (toks[ii] == "FZRANO") {
      setMetarRemFzraDown(true);
    } else if (toks[ii] == "PWINO") {
      setMetarRemPwiDown(true);
    } else if (toks[ii] == "TSNO") {
      setMetarRemTsDown(true);
    }

  }
  setMetarRemarks(remarks);

  if (valid) {
    return(0);
  } else {
    return (-1);
  }
  
}

//////////////////////////////////////////
// Dress the raw metar text with the report type and endig character
//

void WxObs::dressRawMetarText(const string &reportType)
{

  string oldStr;
  string newStr;

  oldStr = getMetarText();

  newStr = reportType + " " + oldStr + "=";

  //cout << "Old raw text: " << oldStr << endl; 
  //cout << "New raw text: " << newStr << endl << endl;

  setMetarText(newStr);

}


///////////////////////////////////////////////////
// compute RH from temp and dewpoint

double WxObs::_relh(double t, double td)

{
  
  double vdt = 6.1121 * exp((17.502*td)/(td+240.97));
  double vt = 6.1121 * exp((17.502*t)/(t+240.97));

  double ratio = vdt / vt;
  if (ratio > 1.0) {
    ratio = 1.0;
  }
  double relh = 100.0 * ratio;
  
  return (relh);

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void WxObs::_tokenize(const string &str,
                      const string &spacer,
                      vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}


bool WxObs::skyObscurationStringToFraction(string str, double& frac)
{
 
  if(str=="SKC") {
    frac = CLOUD_SKC;
  }
  else if(str=="CLR") {
    frac = CLOUD_CLR;
  }
  else if(str=="CAVOK") {
    frac = CLOUD_CLR;
  }
  else if(str=="FEW") {
    frac = CLOUD_FEW;
  }
  else if(str=="SCT") {
    frac = CLOUD_SCT;
  }
  else if(str=="BKN") {
    frac = CLOUD_BKN;
  }
  else if(str=="OVC") {
    frac = CLOUD_OVC;
  }
  else if(str=="OVX") {
    frac = CLOUD_OVX;
  }
  else {
    return false;
  }
  return true;
}

string WxObs::skyObscurationFractionToString(double fraction)
{
  if (fraction == -1) {
    return "OVX";
  }
  if(fraction < 0 || fraction > 1) {
    return "MISS";
  } else if(fraction < (CLOUD_CLR+CLOUD_FEW)/2.0) {
    return "CLR";
  } else if(fraction < (CLOUD_FEW+CLOUD_SCT)/2.0) {
    return "FEW";
  } else if(fraction < (CLOUD_SCT+CLOUD_BKN)/2.0 ) {
    return "SCT";
  } else if(fraction < (CLOUD_BKN+CLOUD_OVC)/2.0 ) {
    return "BKN";
  } else {
    return "OVC";
  }
}
