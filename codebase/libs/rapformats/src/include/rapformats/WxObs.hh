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
// WxObs.hh
//
// C++ class for dealing with weather observations.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Feb 2007
//////////////////////////////////////////////////////////////
//
// Underlying data store can be station_report.h or XML
//
//////////////////////////////////////////////////////////////

#ifndef _WxObs_hh
#define _WxObs_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaXml.hh>
#include <rapformats/station_reports.h>
#include <rapformats/WxObsField.hh>
#include <rapformats/WxTypeField.hh>

using namespace std;

class WxObs {

public:

  // constructor

  WxObs();

  // destructor

  ~WxObs();

  //////////////////////// set methods /////////////////////////

  // reset to default state
  
  void reset();

  // get observation location and time

  void setStationId(const string &id) { _station_id = id; }
  void setLongName(const string &name) { _long_name = name; }
  void setObservationTime(time_t time) { _observation_time = time; }
  void setLatitude(double lat) { _latitude = lat; }
  void setLongitude(double lon) { _longitude = lon; }
  void setElevationM(double lon) { _elevation_m = lon; }

  // METAR-specific members

  void setMetarText(const string &text) { _metar_text = text; }
  void setMetarWx(const string &wx) { _metar_wx = wx; }
  void setMetarRemarks(const string &rem) { _metar_remarks = rem; }

  void setMetarRemStnIndicator(const string &stn_indicator) {
    _metar_rem_stn_indicator = stn_indicator;
    _metar_rem_info_set = true;
  }
  void setMetarRemPwiDown(bool down) { _metar_rem_pwi_down = down; }
  void setMetarRemFzraDown(bool down) { _metar_rem_fzra_down = down; }
  void setMetarRemTsDown(bool down) { _metar_rem_ts_down = down; }

  // Ceiling and visibility minimum flags

  void setCeilingIsMinimum(bool minimum) {_ceiling_is_minimum = minimum; }
  void setVisibilityIsMinimum(bool minimum) {_visibility_is_minimum = minimum; }

  ///////// adding/setting field values etc ////////////////////
  //
  // add() methods return the index of the vector element added.
  // set() methods take the index as an argument, defaulting to 0.
  //       so set() with no index is the same as a single add.

  // temperature

  int addTempC(double temp) {
    return _temp_c.addValue(temp);
  }
  void setTempC(double temp, int index = 0) {
    _temp_c.setValue(temp, index);
  }
  void setTempCInfo(const string &info, int index = 0) {
    _temp_c.setInfo(info, index);
  }

  int addMinTempC(double temp, double period_secs) {
    int index = (int) _min_temp_c.getSize();
    setMinTempC(temp, period_secs, index);
    return index;
  }
  void setMinTempC(double temp, double period_secs, int index = 0) {
    _min_temp_c.setValue(temp, index);
    _min_temp_c.setQualifier(period_secs, index);
    _min_temp_c.setQualLabel("period_s", index);
  }
  void setMinTempCInfo(const string &info, int index = 0) {
    _min_temp_c.setInfo(info, index);
  }

  int addMaxTempC(double temp, double period_secs) {
    int index = (int) _max_temp_c.getSize();
    setMaxTempC(temp, period_secs, index);
    return index;
  }
  void setMaxTempC(double temp, double period_secs, int index = 0) {
    _max_temp_c.setValue(temp, index);
    _max_temp_c.setQualifier(period_secs, index);
    _max_temp_c.setQualLabel("period_s", index);
  }
  void setMaxTempCInfo(const string &info, int index = 0) {
    _max_temp_c.setInfo(info, index);
  }

  // sea surface temperature

  int addSeaSurfaceTempC(double temp) {
    return _sea_surface_temp_c.addValue(temp);
  }
  void setSeaSurfaceTempC(double temp, int index = 0) {
    _sea_surface_temp_c.setValue(temp, index);
  }
  void setSeaSurfaceTempCInfo(const string &info, int index = 0) {
    _sea_surface_temp_c.setInfo(info, index);
  }

  // dewpoint

  int addDewpointC(double dp) {
    return _dewpoint_c.addValue(dp);
  }
  void setDewpointC(double dp, int index = 0) {
    _dewpoint_c.setValue(dp, index);
  }
  void setDewpointCInfo(const string &info, int index = 0) {
    _dewpoint_c.setInfo(info, index);
  }

  // relative humidity

  int addRhPercent(double rh) {
    return _rh_percent.addValue(rh);
  }
  void setRhPercent(double rh, int index = 0) {
    _rh_percent.setValue(rh, index);
  }
  void setRhPercentInfo(const string &info, int index = 0) {
    _rh_percent.setInfo(info, index);
  }

  // wind - speeds in meters per sec

  int addWindDirnDegT(double dirn) {
    return _wind_dirn_degt.addValue(dirn);
  }
  int addWindDirnVariable() {
    int index = _wind_dirn_degt.addValue(-0.0);
    _wind_dirn_degt.setQualifier(1, index);
    _wind_dirn_degt.setQualLabel("variable", index);
    return index;
  }
  void setWindDirnDegT(double dirn, int index = 0) {
    _wind_dirn_degt.setValue(dirn, index);
  }
  void setWindDirnVariable(int index = 0) {
    _wind_dirn_degt.setValue(-0.0, index);
    _wind_dirn_degt.setQualifier(1, index);
    _wind_dirn_degt.setQualLabel("variable", index);
  }
  void setWindDirnDegTInfo(const string &info, int index = 0) {
    _wind_dirn_degt.setInfo(info, index);
  }

  int addWindSpeedMps(double speed) {
    return _wind_speed_mps.addValue(speed);
  }
  void setWindSpeedMps(double speed, int index = 0) {
    _wind_speed_mps.setValue(speed, index);
  }
  void setWindSpeedMps(const string &info, int index = 0) {
    _wind_speed_mps.setInfo(info, index);
  }

  int addWindGustMps(double gust) {
    return _wind_gust_mps.addValue(gust);
  }
  void setWindGustMps(double gust, int index = 0) {
    _wind_gust_mps.setValue(gust, index);
  }
  void setWindGustMps(const string &info, int index = 0) {
    _wind_gust_mps.setInfo(info, index);
  }

  // visibility / ceiling / runway visual range

  int addVisibilityKm(double vis) {
    return _visibility_km.addValue(vis);
  }
  void setVisibilityKm(double vis, int index = 0) {
    _visibility_km.setValue(vis, index);
  }
  void setVisibilityKmInfo(const string &info, int index = 0) {
    _visibility_km.setInfo(info, index);
  }

  int addExtinctionPerKm(double vis) {
    return _extinction_per_km.addValue(vis);
  }
  void setExtinctionPerKm(double vis, int index = 0) {
    _extinction_per_km.setValue(vis, index);
  }
  void setExtinctionPerKmInfo(const string &info, int index = 0) {
    _extinction_per_km.setInfo(info, index);
  }

  int addVertVisKm(double vis) {
    return _vert_vis_km.addValue(vis);
  }
  void setVertVisKm(double vis, int index = 0) {
    _vert_vis_km.setValue(vis, index);
  }
  void setVertVisKmInfo(const string &info, int index = 0) {
    _vert_vis_km.setInfo(info, index);
  }

  int addCeilingKm(double vis) {
    return _ceiling_km.addValue(vis);
  }
  void setCeilingKm(double vis, int index = 0) {
    _ceiling_km.setValue(vis, index);
  }
  void setCeilingKmInfo(const string &info, int index = 0) {
    _ceiling_km.setInfo(info, index);
  }

  int addRvrKm(double rvr) {
    return _rvr_km.addValue(rvr);
  }
  void setRvrKm(double rvr, int index = 0) {
    _rvr_km.setValue(rvr, index);
  }
  void setRvrKmInfo(const string &info, int index = 0) {
    _rvr_km.setInfo(info, index);
  }

  // pressure

  int addPressureMb(double pres) {
    return _pressure_mb.addValue(pres);
  }
  void setPressureMb(double pres, int index = 0) {
    _pressure_mb.setValue(pres, index);
  }
  void setPressureMbInfo(const string &info, int index = 0) {
    _pressure_mb.setInfo(info, index);
  }

  int addSeaLevelPressureMb(double pres) {
    return _msl_pressure_mb.addValue(pres);
  }
  void setSeaLevelPressureMb(double pres, int index = 0) {
    _msl_pressure_mb.setValue(pres, index);
  }
  void setSeaLevelPressureMbInfo(const string &info, int index = 0) {
    _msl_pressure_mb.setInfo(info, index);
  }

  int addSeaLevelPressureInHg(double pres) {
    return _msl_pressure_inHg.addValue(pres);
  }
  void setSeaLevelPressureInHg(double pres, int index = 0) {
    _msl_pressure_inHg.setValue(pres, index);
  }
  void setSeaLevelPressureInHgInfo(const string &info, int index = 0) {
    _msl_pressure_inHg.setInfo(info, index);
  }

  int addSeaLevelPressureQFFMb(double pres) {
    return _msl_pressure_QFF_mb.addValue(pres);
  }
  void setSeaLevelPressureQFFMb(double pres, int index = 0) {
    _msl_pressure_QFF_mb.setValue(pres, index);
  }
  void setSeaLevelPressureQFFMbInfo(const string &info, int index = 0) {
    _msl_pressure_QFF_mb.setInfo(info, index);
  }

  int addPressureTendencyMb(double pres, double period_secs) {
    int index = (int) _press_tend_mb.getSize();
    setPressureTendencyMb(pres, period_secs, index);
    return index;
  }
  void setPressureTendencyMb(double pres, double period_secs,
                             int index = 0) {
    _press_tend_mb.setValue(pres, index);
    _press_tend_mb.setQualifier(period_secs, index);
    _press_tend_mb.setQualLabel("period_s", index);
  }
  void setPressureTendencyMbInfo(const string &info, int index = 0) {
    _press_tend_mb.setInfo(info, index);
  }

  // precip

  int addPrecipLiquidMm(double mm, double accum_secs) {
    int index = (int) _precip_liquid_mm.getSize();
    setPrecipLiquidMm(mm, accum_secs, index);
    return index;
  }
  void setPrecipLiquidMm(double mm, double accum_secs, int index = 0) {
    _precip_liquid_mm.setValue(mm, index);
    _precip_liquid_mm.setQualifier(accum_secs, index);
    _precip_liquid_mm.setQualLabel("period_s", index);
  }
  void setPrecipLiquidMmInfo(const string &info, int index = 0) {
    _precip_liquid_mm.setInfo(info, index);
  }

  int addPrecipRateMmPerHr(double rate) {
    return _precip_rate_mmph.addValue(rate);
  }
  int addPrecipRateMmPerHr(double rate, double period_secs) {
    int index = (int) _precip_rate_mmph.getSize();
    setPrecipRateMmPerHr(rate, period_secs, index);
    return index;
  }
  void setPrecipRateMmPerHr(double rate, int index = 0) {
    _precip_rate_mmph.setValue(rate, index);
  }
  void setPrecipRateMmPerHr(double rate, double period_secs,
                            int index = 0) {
    _precip_rate_mmph.setValue(rate, index);
    _precip_rate_mmph.setQualifier(period_secs, index);
    _precip_rate_mmph.setQualLabel("period_s", index);
  }
  void setPrecipRateMmPerHrInfo(const string &info, int index = 0) {
    _precip_liquid_mm.setInfo(info, index);
  }

  int addSnowDepthMm(double mm, double accum_secs) {
    int index = (int) _snow_depth_mm.getSize();
    setSnowDepthMm(mm, accum_secs, index);
    return index;
  }
  void setSnowDepthMm(double mm, double accum_secs, int index = 0) {
    _snow_depth_mm.setValue(mm, index);
    _snow_depth_mm.setQualifier(accum_secs, index);
    _snow_depth_mm.setQualLabel("period_s", index);
  }
  void setSnowDepthMmInfo(const string &info, int index = 0) {
    _snow_depth_mm.setInfo(info, index);
  }

  // sky obscuration

  int addSkyObscuration(double fraction, double altitude_km) {
    int index = (int) _sky_obsc.getSize();
    setSkyObscuration(fraction, altitude_km, index);
    return index;
  }
  void setSkyObscuration(double fraction, double altitude_km,
                         int index = 0) {
    _sky_obsc.setValue(fraction, index);
    _sky_obsc.setQualifier(altitude_km, index);
    _sky_obsc.setQualLabel("altitude_km", index);
  }
  void setSkyObscurationInfo(const string &info, int index = 0) {
    _sky_obsc.setInfo(info, index);
  }

  // weather type

  int addWeatherType(wx_type_t wx) {
    return _wx_type.addValue(wx);
  }
  void setWeatherType(wx_type_t wx, int index = 0) {
    _wx_type.setValue(wx, index);
  }
  void setWeatherTypeInfo(const string &info, int index = 0) {
    _wx_type.setInfo(info, index);
  }

  //////////////////////// get methods /////////////////////////

  // get observation location and time

  inline const string &getStationId() const { return _station_id; }
  inline const string &getLongName() const { return _long_name; }
  inline time_t getObservationTime() const { return _observation_time; }
  inline double getLatitude() const { return _latitude; }
  inline double getLongitude() const { return _longitude; }
  inline double getElevationMeters() const { return _elevation_m; }
  inline double getElevationKm() const { return _elevation_m / 1000.0; }

  // METAR-specific members

  inline const string &getMetarText() const { return _metar_text; }
  inline const string &getMetarWx() const { return _metar_wx; }
  inline const string &getMetarRemarks() const { return _metar_remarks; }

  inline const string &getMetarRemStnIndicator() const { 
    return _metar_rem_stn_indicator;
  }
  inline bool getMetarRemPwiDown() const { return _metar_rem_pwi_down; }
  inline bool getMetarRemFzraDown() const { return _metar_rem_fzra_down; }
  inline bool getMetarRemTsDown() const { return _metar_rem_ts_down; }

  // Ceiling and visibility minimum flags

  inline bool getCeilingIsMinimum() const { return _ceiling_is_minimum; }
  inline bool getVisibilityIsMinimum() const { return _visibility_is_minimum; }

  // field objects
  
  inline const WxObsField &getTempCField() const { return _temp_c; }
  inline const WxObsField &getMinTempCField() const { return _min_temp_c; }
  inline const WxObsField &getMaxTempCField() const { return _max_temp_c; }
  inline const WxObsField &getSeaSurfaceTempCField() const { return _sea_surface_temp_c; }
  inline const WxObsField &getDewpointCField() const { return _dewpoint_c; }
  inline const WxObsField &getRhPercentField() const { return _rh_percent; }
  inline const WxObsField &getWindDirnDegtField() const { return _wind_dirn_degt; }
  inline const WxObsField &getWindSpeedMpsField() const { return _wind_speed_mps; }
  inline const WxObsField &getWindGustMpsField() const { return _wind_gust_mps; }
  inline const WxObsField &getVisibilityKmField() const { return _visibility_km; }
  inline const WxObsField &getExtinctionPerKmField() const { return _extinction_per_km; }
  inline const WxObsField &getVertVisKmField() const { return _vert_vis_km; }
  inline const WxObsField &getCeilingKmField() const { return _ceiling_km; }
  inline const WxObsField &getRvrKmField() const { return _rvr_km; }
  inline const WxObsField &getPressureMbField() const { return _pressure_mb; }
  inline const WxObsField &getMslPressureMbField() const { return _msl_pressure_mb; }
  inline const WxObsField &getMslPressureQFFField() const { return _msl_pressure_QFF_mb; }
  inline const WxObsField &getMslPressureInHgField() const { return _msl_pressure_inHg; }
  inline const WxObsField &getPressTendMbField() const { return _press_tend_mb; }
  inline const WxObsField &getPrecipLiquidMmField() const { return _precip_liquid_mm; }
  inline const WxObsField &getPrecipRateMmphField() const { return _precip_rate_mmph; }
  inline const WxObsField &getSnowDepthMmField() const { return _snow_depth_mm; }
  inline const WxObsField &getSkyObscField() const { return _sky_obsc; }
  inline const WxTypeField &getWeatherTypeField() const { return _wx_type; }

  // field objects sizes
  
  inline int getTempCSize() const { return _temp_c.getSize(); }
  inline int getMinTempCSize() const { return _min_temp_c.getSize(); }
  inline int getMaxTempCSize() const { return _max_temp_c.getSize(); }
  inline int getSeaSurfaceTempCSize() const { return _sea_surface_temp_c.getSize(); }
  inline int getDewpointCSize() const { return _dewpoint_c.getSize(); }
  inline int getRhPercentSize() const { return _rh_percent.getSize(); }
  inline int getWindDirnDegtSize() const { return _wind_dirn_degt.getSize(); }
  inline int getWindSpeedMpsSize() const { return _wind_speed_mps.getSize(); }
  inline int getWindGustMpsSize() const { return _wind_gust_mps.getSize(); }
  inline int getVisibilityKmSize() const { return _visibility_km.getSize(); }
  inline int getExtinctionPerKmSize() const { return _extinction_per_km.getSize(); }
  inline int getVertVisKmSize() const { return _vert_vis_km.getSize(); }
  inline int getCeilingKmSize() const { return _ceiling_km.getSize(); }
  inline int getRvrKmSize() const { return _rvr_km.getSize(); }
  inline int getPressureMbSize() const { return _pressure_mb.getSize(); }
  inline int getMslPressureMbSize() const { return _msl_pressure_mb.getSize(); }
  inline int getMslPressureInHgbSize() const { return _msl_pressure_inHg.getSize(); }
  inline int getMslPressureQFFMbSize() const { return _msl_pressure_QFF_mb.getSize(); }
  inline int getPressTendMbSize() const { return _press_tend_mb.getSize(); }
  inline int getPrecipLiquidMmSize() const { return _precip_liquid_mm.getSize(); }
  inline int getPrecipRateMmphSize() const { return _precip_rate_mmph.getSize(); }
  inline int getSnowDepthMmSize() const { return _snow_depth_mm.getSize(); }
  inline int getSkyObscSize() const { return _sky_obsc.getSize(); }
  inline int getWeatherTypeSize() const { return _wx_type.getSize(); }

  /////////////////////////////////////////////////////////////
  // get() field value, qualifier and info methods
  //
  // These return missing (-9999) if the index is out of range.
  // The getInfo() methods return an empty string if the index
  // is out of range.

  // temperature

  inline double getTempC(int index = 0) const {
    return _temp_c.getValue(index);
  }
  inline string getTempCInfo(int index = 0) const {
    return _temp_c.getInfo(index);
  }

  inline double getMinTempC(int index = 0) const {
    return _min_temp_c.getValue(index);
  }
  inline double getMinTempCPeriodSecs(int index = 0) const {
    return _min_temp_c.getQualifier(index);
  }
  inline string getMinTempCInfo(int index = 0) const {
    return _min_temp_c.getInfo(index);
  }

  inline double getMaxTempC(int index = 0) const {
    return _max_temp_c.getValue(index);
  }
  inline double getMaxTempCPeriodSecs(int index = 0) const {
    return _max_temp_c.getQualifier(index);
  }
  inline string getMaxTempCInfo(int index = 0) const {
    return _max_temp_c.getInfo(index);
  }

  // set surface temp

  inline double getSeaSurfaceTempC(int index = 0) const {
    return _sea_surface_temp_c.getValue(index);
  }
  inline string getSeaSurfaceTempCInfo(int index = 0) const {
    return _sea_surface_temp_c.getInfo(index);
  }

  // dewpoint

  inline double getDewpointC(int index = 0) const {
    return _dewpoint_c.getValue(index);
  }
  inline string getDewpointCInfo(int index = 0) const {
    return _dewpoint_c.getInfo(index);
  }

  // relative humidity

  inline double getRhPercent(int index = 0) const {
    return _rh_percent.getValue(index);
  }
  inline string getRhPercentInfo(int index = 0) const {
    return _rh_percent.getInfo(index);
  }

  // wind

  inline double getWindDirnDegt(int index = 0) const {
    return _wind_dirn_degt.getValue(index);
  }
  inline string getWindDirnDegtInfo(int index = 0) const {
    return _wind_dirn_degt.getInfo(index);
  }
  inline bool getWindDirnVariable(int index = 0) const {
    double var = _wind_dirn_degt.getQualifier(index);
    if (var > 0.9) {
      return true;
    } else {
      return false;
    }
  }

  inline double getWindSpeedMps(int index = 0) const {
    return _wind_speed_mps.getValue(index);
  }
  inline string getWindSpeedMpsInfo(int index = 0) const {
    return _wind_speed_mps.getInfo(index);
  }

  inline double getWindGustMps(int index = 0) const {
    return _wind_gust_mps.getValue(index);
  }
  inline string getWindGustMpsInfo(int index = 0) const {
    return _wind_gust_mps.getInfo(index);
  }

  // visibility / ceiling / runway visual range

  inline double getVisibilityKm(int index = 0) const {
    return _visibility_km.getValue(index);
  }
  inline string getVisibilityKmInfo(int index = 0) const {
    return _visibility_km.getInfo(index);
  }

  inline double getExtinctionPerKm(int index = 0) const {
    return _extinction_per_km.getValue(index);
  }
  inline string getExtinctionPerKmInfo(int index = 0) const {
    return _extinction_per_km.getInfo(index);
  }

  inline double getVertVisKm(int index = 0) const {
    return _vert_vis_km.getValue(index);
  }
  inline string getVertVisKmInfo(int index = 0) const {
    return _vert_vis_km.getInfo(index);
  }

  inline double getCeilingKm(int index = 0) const {
    return _ceiling_km.getValue(index);
  }
  inline string getCeilingKmInfo(int index = 0) const {
    return _ceiling_km.getInfo(index);
  }

  inline double getRvrKm(int index = 0) const {
    return _rvr_km.getValue(index);
  }
  inline string getRvrKmInfo(int index = 0) const {
    return _rvr_km.getInfo(index);
  }

  // pressure

  inline double getPressureMb(int index = 0) const {
    return _pressure_mb.getValue(index);
  }
  inline string getPressureMbInfo(int index = 0) const {
    return _pressure_mb.getInfo(index);
  }

  inline double getSeaLevelPressureMb(int index = 0) const {
    return _msl_pressure_mb.getValue(index);
  }
  inline string getSeaLevelPressureMbInfo(int index = 0) const {
    return _msl_pressure_mb.getInfo(index);
  }

  inline double getSeaLevelPressureQFFMb(int index = 0) const {
    return _msl_pressure_QFF_mb.getValue(index);
  }
  inline string getSeaLevelPressureQFFMbInfo(int index = 0) const {
    return _msl_pressure_QFF_mb.getInfo(index);
  }

  inline double getSeaLevelPressureInHg(int index = 0) const {
    return _msl_pressure_inHg.getValue(index);
  }
  inline string getSeaLevelPressureInHgInfo(int index = 0) const {
    return _msl_pressure_inHg.getInfo(index);
  }

  inline double getPressureTendencyMb(int index = 0) const {
    return _press_tend_mb.getValue(index);
  }
  inline double getPressureTendencyMbPeriodSecs(int index = 0) const {
    return _press_tend_mb.getQualifier(index);
  }
  inline string getPressureTendencyMbInfo(int index = 0) const {
    return _press_tend_mb.getInfo(index);
  }

  // precip

  inline double getPrecipLiquidMm(int index = 0) const {
    return _precip_liquid_mm.getValue(index);
  }
  inline double getPrecipLiquidMmAccumSecs(int index = 0) const {
    return _precip_liquid_mm.getQualifier(index);
  }
  inline string getPrecipLiquidMmInfo(int index = 0) const {
    return _precip_liquid_mm.getInfo(index);
  }

  inline double getPrecipRateMmPerHr(int index = 0) const {
    return _precip_rate_mmph.getValue(index);
  }
  inline string getPrecipRateMmPerHrInfo(int index = 0) const {
    return _precip_rate_mmph.getInfo(index);
  }

  inline double getSnowDepthMm(int index = 0) const {
    return _snow_depth_mm.getValue(index);
  }
  inline double getSnowDepthMmAccumSecs(int index = 0) const {
    return _snow_depth_mm.getQualifier(index);
  }
  inline string getSnowDepthMmInfo(int index = 0) const {
    return _snow_depth_mm.getInfo(index);
  }

  // sky obscuration

  inline double getSkyObscuration(int index = 0) const {
    return _sky_obsc.getValue(index);
  }
  inline double getSkyObscurationAltitudeKm(int index = 0) const {
    return _sky_obsc.getQualifier(index);
  }
  inline string getSkyObscurationInfo(int index = 0) const {
    return _sky_obsc.getInfo(index);
  }
  
  // weather type

  inline wx_type_t getWeatherType(int index = 0) const {
    return _wx_type.getValue(index);
  }
  inline string getWeatherTypeInfo(int index = 0) const {
    return _wx_type.getInfo(index);
  }

  ///////////////////////////////////////////
  // assemble as XML
  // Load up an XML buffer from the object.

  void assemble();
  void assembleAsXml();

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

  void assembleAsReport(msg_id_t msgId = STATION_REPORT);

  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ///////////////////////////////////////////
  // load XML
  
  void loadXml(string &xml, bool includeWxStr = false,
               int startIndentLevel = 0) const;
  
  ///////////////////////////////////////////
  // load METAR strings XML
  
  void loadMetarStringsXml(string &xml) const;
  
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
  
  void loadStationReport(station_report_t &report,
                         msg_id_t msgId = SENSOR_REPORT) const;
                              
  ///////////////////////////////////////////////
  // set this object from a station report

  void setFromStationReport(const station_report_t &report);
  
  ///////////////////////////////////////////////
  // set this object from a decoded METAR
  
  int setFromDecodedMetar(const string &metarText,
                          const string &stationName,
                          const Decoded_METAR &dcdMetar,
                          time_t valid_time,
                          double lat,
                          double lon,
                          double alt);

  
  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  // disassemble to get station name and position.
  // Efficient routine to get just station details.
  // Fills out stationId, lat and lon (deg) and elev (meters)
  // Returns 0 on success, -1 on failure
  
  static int disassembleStationDetails(const void *buf, int len,
                                       string &stationId,
                                       double &latitude,
                                       double &longitude,
                                       double &elevationM);
  
  // Check if buffer starts as report
  // Sets msgId

  static bool checkForReport(const void *buf, int len, msg_id_t &msgId);

  /////////////////////////
  // print
  
  void print(ostream &out, string spacer = "") const;

  // Print an XML representation of the object.
  
  void printAsXml(ostream &out, int startIndentLevel = 0) const;

  // print wind dirn field

  void printWindDirn(ostream &out,
                     const string &label,
                     string spacer) const;

  // print weather types
  
  void printWxTypes(ostream &out,
                    const string &label,
                    string spacer = "") const;

  // Convert weather type to string
  
  static string wxType2Str(wx_type_t wxType);

  // Convert weather type vector to string
  
  string wxTypes2Str() const;

  // Convert string to weather type
  
  static wx_type_t str2WxType(const string &wxStr);

  // convert sky obscuration between fraction and string values

  static bool skyObscurationStringToFraction(string str, double& frac);

  static string skyObscurationFractionToString(double fraction);


  // Dress the raw metar text with report type and ending character
  
  void dressRawMetarText(const string &reportType);



  static const double CLOUD_SKC;
  static const double CLOUD_CLR;
  static const double CLOUD_FEW;
  static const double CLOUD_SCT;
  static const double CLOUD_BKN;
  static const double CLOUD_OVC;
  static const double CLOUD_OVX;


  // missing data value

  static const double missing;

protected:
private:

  // observation time and location

  string _station_id;
  string _long_name;
  time_t _observation_time;
  double _latitude;
  double _longitude;
  double _elevation_m;

  // METAR-specific strings

  string _metar_text;
  string _metar_wx;
  string _metar_remarks;

  bool _metar_rem_info_set;
  string _metar_rem_stn_indicator;
  bool _metar_rem_pwi_down;
  bool _metar_rem_fzra_down;
  bool _metar_rem_ts_down;


  // ceiling and visibility minimum identifiers
  // 
  // Set to true if the ceiling is set by the tokens
  // CAVOK, CLR, NSC, or SKC. Otherwise value is false.

  bool _ceiling_is_minimum;

  // Set to true the visibility is set by the token CAVOK 

  bool _visibility_is_minimum;

  // fields
  
  WxObsField _temp_c;
  WxObsField _min_temp_c;
  WxObsField _max_temp_c;
  WxObsField _sea_surface_temp_c;
  WxObsField _dewpoint_c;
  WxObsField _rh_percent;
  WxObsField _wind_dirn_degt;
  WxObsField _wind_speed_mps;
  WxObsField _wind_gust_mps;
  WxObsField _visibility_km;
  WxObsField _extinction_per_km;
  WxObsField _vert_vis_km;
  WxObsField _ceiling_km;
  WxObsField _rvr_km;
  WxObsField _pressure_mb;
  WxObsField _msl_pressure_mb;
  WxObsField _msl_pressure_inHg;  //pressure from 'A' field in a METAR
  WxObsField _msl_pressure_QFF_mb; //pressure from SLP in a metar remark
  WxObsField _press_tend_mb;
  WxObsField _precip_liquid_mm;
  WxObsField _precip_rate_mmph;
  WxObsField _snow_depth_mm;
  WxObsField _sky_obsc;
  WxTypeField _wx_type;

  // station_report buffer

  bool _basedOnStationReport;
  station_report_t _report;

  // xml

  mutable string _xml;
  mutable string _metarStringsXml;

  // buffer for assemble / disassemble

  MemBuf _memBuf;

  // functions

  void _printReport(ostream &out, string spacer = "") const;
  
  void _addFieldAsXml(const WxObsField &field,
                      const string &tag,
                      int level,
                      string &xml) const;


  void _addWxTypeAsXml(const WxTypeField &field,
                       const string &tag,
                       int level,
                       string &xml) const;
  
  int _disassembleXml(const char *buf, int len);
  int _disassembleMetarStringsXml(const char *buf, int len);
  int _disassembleReport(const void *buf, int len, bool setMembers);

  void _setFieldFromXml(const string &tag,
                        const string &content,
                        WxObsField &field);
  
  void _setWxTypeFromXml(const string &tag,
                         const string &content,
                         WxTypeField &field);

  int _removeWeatherType(wx_type_t wx);

  double _relh(double t, double td);

  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);
  
};

// set static const values

#ifdef _in_wx_obs_cc
const double WxObs::missing = WxObsField::missing;
#endif

#endif


