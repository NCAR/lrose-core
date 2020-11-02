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

#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <cstdio>
#include <unistd.h>

#include <physics/physics.h>
#include <physics/stn_pressure.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <rapmath/math_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/TaArray.hh>

#include "Params.hh"
#include "madis2Spdb.hh"

//
// Constructor.
//
madis2Spdb::madis2Spdb(Params *TDRP_params) :
  _QCR(0),
  _arrayAlloc(0)
{
  _params = TDRP_params;
}

//
// Destructor.
//
madis2Spdb::~madis2Spdb()
{
  delete [] _QCR;
}
//
// Main method.
//
bool madis2Spdb::processFile(char *fileName){

  PMU_auto_register("Processing data");

  // Use toolsa uncompress routine and then we won't need the
  // inputDataCompressed parameter anymore

  if (_params->inputDataCompressed){
    char com[256];
    sprintf(com,"gunzip -f %s", fileName);
    system(com); sleep(1);
    int index = strlen(fileName);
    index -= 3;
    fileName[index]=char(0);
  }


  int netID;
  int status = nc_open(fileName, NC_NOWRITE, &netID);
  if (!_checkStatus(status, "Failed to open input file."))
    return false;
  
  int dimID;

  //////////////////////////////////////////////////////

  size_t recNum; // Number of records.
  status = nc_inq_dimid(netID, "recNum", &dimID);
  if (!_checkStatus(status, "Failed to get recNum dimension ID"))
    return false;

  status = nc_inq_dimlen(netID, dimID, &recNum);
  if (!_checkStatus(status, "Failed to read recNum integer"))
    return false;
  

  if (_params->debug)
    cerr << recNum << " records found." << endl;

  // Allocate the data arrays

  _allocateArrays(recNum);
  
  //
  // Lat.-- required for processing
  //
  float *lat_data;
  
  if ((lat_data =
       _readFloatVar(netID, recNum, _params->latitude_var.var_name,
		     _params->latitude_var.fill_value_name,
		     _params->latitude_var.missing_value_name)) == 0){
    cerr << "Failed to read latitude data" << endl;
    return false;
  }
  
  //
  // Lon.-- required for processing
  //
  float *lon_data;
  
  if ((lon_data =
       _readFloatVar(netID, recNum, _params->longitude_var.var_name,
		     _params->longitude_var.fill_value_name,
		     _params->longitude_var.missing_value_name)) == 0)
  {
    cerr << "Failed to read longitude data" << endl;
    delete [] lat_data;
    return false;
  }
  
  //
  // ObsTime.-- required for processing
  //
  double *obs_time_data;
  
  if ((obs_time_data =
       _readDoubleVar(netID, recNum, _params->obs_time_var.var_name,
		      _params->obs_time_var.fill_value_name,
		      _params->obs_time_var.missing_value_name)) == 0)
  {
    cerr << "Failed to read time data" << endl;
    delete [] lat_data;
    delete [] lon_data;
    return false;
  }
  

  if (_params->takeFilenameDate){
    //
    // Try to use filename year, month and day with hour, min and sec from data.
    //
    if (strlen(fileName) > strlen("YYYYMMDD_HHMM")){
      char *p = fileName + strlen(fileName)-strlen("YYYYMMDD_HHMM");
       date_time_t tFname;
       tFname.sec=0; tFname.min=0; tFname.hour=0;
       if (5==sscanf(p,"%4d%2d%2d_%2d%2d",
		     &tFname.year, &tFname.month, &tFname.day,
		     &tFname.hour, &tFname.min)){
	 
	 uconvert_to_utime( &tFname );
	 
	 for(unsigned i=0; i < recNum; i++){

	   date_time_t tAct;
	   tAct.unix_time= (time_t)obs_time_data[i];
	   uconvert_from_utime( &tAct );

	   date_time_t tUse;
	   tUse.year = tFname.year; tUse.month = tFname.month; tUse.day = tFname.day;
	   tUse.hour = tAct.hour; tUse.day = tAct.day; tUse.sec = tAct.sec;     
	   uconvert_to_utime( &tUse );
	   //
	   // It is possible that we may have, say, data from the end of 20100910
	   // in the 20109111 file. If that's the case then tUse will be significantly
	   // greater than tFname. If that's the case, try subtracting a day, and
	   // if that does not work, use tFname.
	   //
	   if (tUse.unix_time - tFname.unix_time < 7200){
	     obs_time_data[i] = (double) tUse.unix_time;
	   } else {
	     if (tUse.unix_time-86400 - tFname.unix_time < 7200){
	       obs_time_data[i] = (double) tUse.unix_time-86400;
	     } else {
	       obs_time_data[i] = (double) tFname.unix_time;
	     }
	   }
	 }
       }
    }
  }

  //
  // Elev.
  //
  float *elev_data = 0;

  if (string(_params->elevation_var.var_name) != "")
  {
    elev_data =
      _readFloatVar(netID, recNum, _params->elevation_var.var_name,
		    _params->elevation_var.fill_value_name,
		    _params->elevation_var.missing_value_name);
  }
  
  //
  // Temperature, in K.
  //
  float *temp_data = 0;
  
  if (string(_params->temperature_var.var_name) != "")
  {
    temp_data =
      _readFloatVar(netID, recNum, _params->temperature_var.var_name,
		    _params->temperature_var.qc_var_name,
		    _params->temperature_var.fill_value_name,
		    _params->temperature_var.missing_value_name);
  }
  
  if (temp_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (temp_data[i] != STATION_NAN)
	temp_data[i] = TEMP_K_TO_C(temp_data[i]);
  }
  
  //
  // dewpoint, in K.
  //
  float *dewpoint_data = 0;

  if (string(_params->dewpoint_var.var_name) != "")
  {
    dewpoint_data =
      _readFloatVar(netID, recNum, _params->dewpoint_var.var_name,
		    _params->dewpoint_var.qc_var_name,
		    _params->dewpoint_var.fill_value_name,
		    _params->dewpoint_var.missing_value_name);
  }

  if (dewpoint_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (dewpoint_data[i] != STATION_NAN)
	dewpoint_data[i] = TEMP_K_TO_C(dewpoint_data[i]);
  }
  
  //
  // relHumidity
  //
  float *rh_data = 0;
  
  if (string(_params->rh_var.var_name) != "")
  {
    rh_data =
      _readFloatVar(netID, recNum, _params->rh_var.var_name,
		    _params->rh_var.qc_var_name,
		    _params->rh_var.fill_value_name,
		    _params->rh_var.missing_value_name);
  }
  
  //
  // stationPressure
  //
  float *station_press_data = 0;

  if (string(_params->station_pressure_var.var_name) != "")
  {
    station_press_data =
      _readFloatVar(netID, recNum, _params->station_pressure_var.var_name,
		    _params->station_pressure_var.qc_var_name,
		    _params->station_pressure_var.fill_value_name,
		    _params->station_pressure_var.missing_value_name);
  }

  if (station_press_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (station_press_data[i] != STATION_NAN)
	station_press_data[i] /= 100.0;
  }
  
  //
  // seaLevelPressure
  //
  float *sl_press_data = 0;

  if (string(_params->sea_level_pressure_var.var_name) != "")
  {
    sl_press_data =
      _readFloatVar(netID, recNum, _params->sea_level_pressure_var.var_name,
		    _params->sea_level_pressure_var.qc_var_name,
		    _params->sea_level_pressure_var.fill_value_name,
		    _params->sea_level_pressure_var.missing_value_name);
  }

  if (sl_press_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (sl_press_data[i] != STATION_NAN)
	sl_press_data[i] /= 100.0;
  }
  
  //
  // altimeter
  //
  float *altimeter_data = 0;

  if (string(_params->altimeter_var.var_name) != "")
  {
    altimeter_data =
      _readFloatVar(netID, recNum, _params->altimeter_var.var_name,
		    _params->altimeter_var.qc_var_name,
		    _params->altimeter_var.fill_value_name,
		    _params->altimeter_var.missing_value_name);
  }

  if (altimeter_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (altimeter_data[i] != STATION_NAN)
	altimeter_data[i] /= 100.0;
  }
  
  //
  // windDir
  //
  float *wind_dir_data = 0;

  if (string(_params->wind_dir_var.var_name) != "")
  {
    wind_dir_data = 
      _readFloatVar(netID, recNum, _params->wind_dir_var.var_name,
		    _params->wind_dir_var.qc_var_name,
		    _params->wind_dir_var.fill_value_name,
		    _params->wind_dir_var.missing_value_name);
  }
  
  //
  // windSpeed
  //
  float *wind_speed_data = 0;
  
  if (string(_params->wind_speed_var.var_name) != "")
  {
    wind_speed_data =
      _readFloatVar(netID, recNum, _params->wind_speed_var.var_name,
		    _params->wind_speed_var.qc_var_name,
		    _params->wind_speed_var.fill_value_name,
		    _params->wind_speed_var.missing_value_name);
  }
  
  //
  // windGust
  //
  float *wind_gust_data = 0;
  
  if (string(_params->wind_gust_var.var_name) != "")
  {
    wind_gust_data =
      _readFloatVar(netID, recNum, _params->wind_gust_var.var_name,
		    _params->wind_gust_var.fill_value_name,
		    _params->wind_gust_var.missing_value_name);
  }
  
  //
  // visibility
  //
  float *vis_data = 0;
  
  if (string(_params->visibility_var.var_name) != "")
  {
    vis_data = 
      _readFloatVar(netID, recNum, _params->visibility_var.var_name,
		    _params->visibility_var.qc_var_name,
		    _params->visibility_var.fill_value_name,
		    _params->visibility_var.missing_value_name);
  }

  if (vis_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (vis_data[i] != STATION_NAN)
	vis_data[i] /= 1000.0;
  }
  
  //
  // precipRate
  //
  float *precip_rate_data = 0;
  
  if (string(_params->precip_rate_var.var_name) != "")
  {
    precip_rate_data =
      _readFloatVar(netID, recNum, _params->precip_rate_var.var_name,
		    _params->precip_rate_var.qc_var_name,
		    _params->precip_rate_var.fill_value_name,
		    _params->precip_rate_var.missing_value_name);
  }

  if (precip_rate_data)
  {
    for (unsigned int i = 0; i < recNum; ++i)
      if (precip_rate_data[i] != STATION_NAN)
	precip_rate_data[i] *= 1000.0 * 3600.0;    // meters/sec to mm/hr
  }
  
  // Create the fields that can be calculated from other fields

  if (!rh_data && temp_data && dewpoint_data)
  {
    rh_data = new float[recNum];
    for (unsigned int i = 0; i < recNum; ++i)
      rh_data[i] = STATION_NAN;
  }
  
  if (!dewpoint_data && temp_data && rh_data)
  {
    dewpoint_data = new float[recNum];
    for (unsigned int i = 0; i < recNum; ++i)
      dewpoint_data[i] = STATION_NAN;
  }
  
  // Create the output records

  DsSpdb Out;
  int nStations = 0;

  size_t maxStaIdLen; 
  status = nc_inq_dimid(netID, "maxStaIdLen", &dimID);
  if (!_checkStatus(status, "Failed to get maxStaIdLen dimension ID, trying maxStaNamLen", _params->debug)){
    status = nc_inq_dimid(netID, "maxStaNamLen", &dimID);
    if (!_checkStatus(status, "Failed to get maxStaNamLen dimension ID")){
      return false;
    }
  }

  status = nc_inq_dimlen(netID, dimID, &maxStaIdLen);
  if (!_checkStatus(status, "Failed to read maxStaIdLen integer"))
    return false;



  bool haveStnTypes = true;
  size_t maxStaTypeLen; 
  status = nc_inq_dimid(netID, "maxStaTypeLen", &dimID);
  if (!_checkStatus(status, "Failed to get maxStaTypeLen dimension ID, setting types to UNKOWN"), _params->debug)
    haveStnTypes = false;

  if ( haveStnTypes ){
    status = nc_inq_dimlen(netID, dimID, &maxStaTypeLen);
    if (!_checkStatus(status, "Failed to read maxStaTypeLen integer"))
      haveStnTypes = false;
  }

  if (!(haveStnTypes)) maxStaTypeLen = strlen("UNKNOWN");


  TaArray<char> PrevType_;
  char *PrevType = PrevType_.alloc(maxStaTypeLen+1);
  PrevType[0] = '\0';

  for(unsigned int i=0; i < recNum; i++)
  {
    TaArray<char> ID_;
    char *ID = ID_.alloc(maxStaIdLen+1);
    if (!_readString(netID, i, maxStaIdLen, "stationId", ID, _params->debug)){
      if (!_readString(netID, i, maxStaIdLen, "stationName", ID)){
	cerr << "Failed to read station ID/name" << endl;
	return false;
      }
    }
    
    int dataType = _Hash(ID);

    TaArray<char> Type_;
    char *Type = Type_.alloc(maxStaTypeLen+1);
    
    if (!(haveStnTypes)){
      sprintf(Type,"UNKOWN");
    } else {
      if (!_readString(netID, i, maxStaTypeLen, "dataProvider", Type))
	sprintf(Type,"UNKOWN");
    }
    //
    // Calculate missing dew point or RH, if possible.
    //
    if (rh_data && rh_data[i] == STATION_NAN)
    {
      if (temp_data && dewpoint_data &&
	  temp_data[i] != STATION_NAN && dewpoint_data[i] != STATION_NAN)
	rh_data[i] = PHYrelh(temp_data[i], dewpoint_data[i]);
    }

    if (dewpoint_data && dewpoint_data[i] == STATION_NAN)
    {
      if (temp_data && rh_data &&
	  temp_data[i] != STATION_NAN &&
	  rh_data[i] != STATION_NAN)
	dewpoint_data[i] = PHYrhdp(temp_data[i], rh_data[i]);
    }
    //
    // Do some debugging.
    //
    if (_params->verbose)
    {
      cerr << i << " : ID=" << ID << " : " << dataType << " : ";
      cerr << "lat=" << lat_data[i] << " lon=" << lon_data[i];
      if (elev_data)
      {
	if (elev_data[i] == STATION_NAN)
	  cerr << " elev=NOT SET";
	else
	  cerr << " elev=" << elev_data[i];
      }
      else
      {
	cerr << " elev=MISSING";
      }
      cerr << " t=" << (time_t) obs_time_data[i] << endl;

      cerr << "Data provider : " << Type << endl;

      if (temp_data)
      {
	if (temp_data[i] == STATION_NAN)
	  cerr << "Temp NOT SET" << endl;
	else
	  cerr << "Temp " << temp_data[i] << endl;
      }
      else
      {
	cerr << "Temp MISSING" << endl;
      }
      
      if (dewpoint_data)
      {
	if (dewpoint_data[i] == STATION_NAN)
	  cerr << "Dew point NOT SET" << endl;
	else
	  cerr << "Dew point " << dewpoint_data[i] << endl;
      }
      else
      {
	cerr << "Dew point MISSING" << endl;
      }
      
      if (rh_data)
      {
	if (rh_data[i] == STATION_NAN)
	  cerr << "Relative humidity NOT SET" << endl;
	else
	  cerr << "Relative humidity " << rh_data[i] << endl;
      }
      else
      {
	cerr << "Relative humidity MISSING" << endl;
      }
      
      if (station_press_data)
      {
	if (station_press_data[i] == STATION_NAN)
	  cerr << "Station pressure NOT SET" << endl;
	else
	  cerr << "Station pressure " << station_press_data[i] << endl;
      }
      else
      {
	cerr << "Station pressure MISSING" << endl;
      }
      
      if (sl_press_data)
      {
	if (sl_press_data[i] == STATION_NAN)
	  cerr << "Sea level pressure NOT SET" << endl;
	else
	  cerr << "Sea level pressure " << sl_press_data[i] << endl;
      }
      else
      {
	cerr << "Sea level pressure MISSING" << endl;
      }
      
      if (altimeter_data)
      {
	if (altimeter_data[i] == STATION_NAN)
	  cerr << "Altimeter NOT SET" << endl;
	else
	  cerr << "Altimeter " << altimeter_data[i] << endl;
      }
      else
      {
	cerr << "Altimeter MISSING" << endl;
      }

      if (wind_dir_data)
      {
	if (wind_dir_data[i] == STATION_NAN)
	  cerr << "Wind direction NOT SET" << endl;
	else
	  cerr << "Wind direction " << wind_dir_data[i] << endl;
      }
      else
      {
	cerr << "Wind direction MISSING" << endl;
      }
      
      if (wind_speed_data)
      {
	if (wind_speed_data[i] == STATION_NAN)
	  cerr << "Wind speed NOT SET" << endl;
	else
	  cerr << "Wind speed " << wind_speed_data[i] << endl;
      }
      else
      {
	cerr << "Wind speed MISSING" << endl;
      }
      
      if (wind_gust_data)
      {
	if (wind_gust_data[i] == STATION_NAN)
	  cerr << "Wind gust NOT SET" << endl;
	else
	  cerr << "Wind gust " << wind_gust_data[i] << endl;
      }
      else
      {
	cerr << "Wind gust MISSING" << endl;
      }
      
      if (vis_data)
      {
	if (vis_data[i] == STATION_NAN)
	  cerr << "Visibility NOT SET" << endl;
	else
	  cerr << "Visibility " << vis_data[i] << endl;
      }
      else
      {
	cerr << "Visibility MISSING" << endl;
      }
      
      if (precip_rate_data)
      {
	if (precip_rate_data[i] == STATION_NAN)
	  cerr << "Precip rate NOT SET" << endl;
	else
	  cerr << "Precip rate " << precip_rate_data[i] << endl;
      }
      else
      {
	cerr << "Precip rate MISSING" << endl;
      }
      
      cerr << endl;
    }

    //
    // See if we want to accept this station.
    //
    if (_params->applyLatLonLimits){
      if (
	  (lat_data[i] < _params->latLonLimits.minLat) ||
	  (lat_data[i] > _params->latLonLimits.maxLat) ||
	  (lon_data[i] < _params->latLonLimits.minLon) ||
	  (lon_data[i] > _params->latLonLimits.maxLon)
	  ){
	if (_params->verbose){
	  cerr << "Station rejected as outside of lat/lon limits." << endl << endl;
	}
	continue;
      }
    }


    if (_params->applyDataProviderLimits){
      int ok = 0;
      for(int k = 0; k < _params->dataProviders_n; k++){
	if (!(strcmp(Type, _params->_dataProviders[k]))){
	  ok = 1;
	}
      }
      if (ok==0){
	if (_params->verbose){
	  cerr << "Station rejected as not from approved source." << endl << endl;
	}
	continue;
      }
    }
    if (_params->verbose){
      cerr << "Station accepted." << endl << endl;
    }
    //
    // We do, so set up the output object.
    //
    station_report_t S;
    memset(&S, 0, sizeof(station_report_t));

    S.msg_id = PRESSURE_STATION_REPORT;
    S.time = (time_t) obs_time_data[i];
    S.accum_start_time = 0;
    S.weather_type = 0;
    S.lat = lat_data[i];
    S.lon = lon_data[i];
    S.alt = elev_data[i];

    if (temp_data)
      S.temp = temp_data[i];
    else
      S.temp = STATION_NAN;

    if (dewpoint_data)
      S.dew_point = dewpoint_data[i];
    else
      S.dew_point = STATION_NAN;
    
    if (rh_data)
      S.relhum = rh_data[i];
    else
      S.relhum = STATION_NAN;

    if (wind_dir_data)
      S.winddir = wind_dir_data[i];
    else
      S.winddir = STATION_NAN;

    if (wind_speed_data)
      S.windspd = wind_speed_data[i];
    else
      S.windspd = STATION_NAN;

    if (wind_gust_data)
      S.windgust = wind_gust_data[i];
    else
      S.windgust = STATION_NAN;

    if (altimeter_data)
    {
      if (altimeter_data[i] == STATION_NAN && sl_press_data)
	S.pres = sl_press_data[i];
      else
	S.pres = altimeter_data[i];
    }
    else if (sl_press_data)
    {
      S.pres = sl_press_data[i];
    }
    else
    {
      S.pres = STATION_NAN;
    }
    
    S.liquid_accum = STATION_NAN;

    if (precip_rate_data)
      S.precip_rate = precip_rate_data[i];
    else
      S.precip_rate = STATION_NAN;

    if (vis_data)
      S.visibility = vis_data[i];
    else
      S.visibility = STATION_NAN;

    S.rvr = STATION_NAN;
    S.ceiling = STATION_NAN;

    if (station_press_data)
      S.shared.pressure_station.stn_pres = station_press_data[i];
    else
      S.shared.pressure_station.stn_pres = STATION_NAN;

    if (S.shared.pressure_station.stn_pres == STATION_NAN &&
	S.pres != STATION_NAN &&
	S.alt != STATION_NAN)
      S.shared.pressure_station.stn_pres =
	SL2StnPressure(S.pres, S.alt);
    S.shared.pressure_station.Spare1 = STATION_NAN;
    S.shared.pressure_station.Spare2 = STATION_NAN;
    
    sprintf(S.station_label,"%s", ID);

    if (_params->verbose)
      print_station_report(stderr, "", &S);
    
    if (_params->output_to_provider_subdirs &&
	PrevType[0] != '\0' &&
	!STRequal_exact(PrevType, Type))
    {
      string output_url = string(_params->output_url) + "/" + PrevType;
      if (Out.put(output_url,
		  SPDB_STATION_REPORT_ID,
		  SPDB_STATION_REPORT_LABEL) != 0)
	cerr << "SPDB write failed to URL " << output_url << endl;

      Out.clearPutChunks();
    }

    STRcopy(PrevType, Type, maxStaTypeLen+1);

    station_report_to_be(&S);
    Out.addPutChunk(dataType,
		    (time_t) obs_time_data[i],
		    _params->Expiry + (time_t) obs_time_data[i],
		    sizeof(station_report_t),
		    &S);
    nStations++;

  } /* endfor - i */
  
			    // Clean up arrays

  delete [] lat_data;
  delete [] lon_data;
  delete [] obs_time_data;
  delete [] elev_data;
  delete [] temp_data;
  delete [] dewpoint_data;
  delete [] rh_data;
  delete [] station_press_data;
  delete [] sl_press_data;
  delete [] altimeter_data;
  delete [] wind_dir_data;
  delete [] wind_speed_data;
  delete [] wind_gust_data;
  delete [] vis_data;
  delete [] precip_rate_data;
  
  // Write what's left in the output buffer to the database

  string output_url = _params->output_url;
  if (_params->output_to_provider_subdirs)
  {
    output_url = string(_params->output_url) + "/" + PrevType;
  }
  
  if (Out.put(output_url,
              SPDB_STATION_REPORT_ID,
              SPDB_STATION_REPORT_LABEL) == 0) {
    cerr << "Processing nStations: " << nStations << endl;
    cerr << "Wrote SPDB to URL " << output_url << endl;
  } else {
    cerr << "SPDB write failed to URL " << output_url << endl;
    cerr << Out.getErrStr() << endl;
  }
  
  // Close the input file

  nc_close(netID);

  PMU_auto_register("Done processing data");

  if (_params->outputDataCompressed){
    char com[256];
    sprintf(com,"gzip -f %s", fileName);
    system(com); sleep(1);
  }

  return true;

}

bool  madis2Spdb::_checkStatus(const int status, const string &err, const bool printMsg)
{
  if (status != NC_NOERR)
  {
    if (printMsg) cerr << err << endl;
    return false;
  }

  return true;
}

void madis2Spdb::_allocateArrays(const int num_records)
{
  if (num_records > _arrayAlloc)
  {
    delete [] _QCR;
    
    _QCR = new int[num_records];
    
    _arrayAlloc = num_records;
  }
}

////////////////////////////////////////////////////////
//
// Small routine to hash a label into an int.
//
int   madis2Spdb::_Hash(char *label){

  int hash=0;

  for (unsigned int i=0; i < strlen(label); i++){
    int c = (int) label[i];

    int val=-1;
    if (isupper(c)){
      val = c - (int)char('A'); // 0..25
    }

    if (isdigit(c)){
      val = c - (int)char('0') + 26; // 26..37
    }

    if (val != -1){
      hash += val * (int)rint(pow(38.0,(double)i));
    }

  }

  return hash;

}


  // Read variables from the netCDF file

float *madis2Spdb::_readFloatVar(const int nc_id,
				 const unsigned int num_records,
				 const string &nc_var_name,
				 const string &nc_fill_value_name,
				 const string &nc_missing_value_name)
{
  int var_id;
  int status;
    
  status = nc_inq_varid(nc_id, nc_var_name.c_str(), &var_id);
  if (!_checkStatus(status,
		    string("Failed to get variable ID for ") + nc_var_name))
    return 0;

  float fillVal;
  status =  nc_get_att_float(nc_id, var_id,
			     nc_fill_value_name.c_str(), &fillVal);
  if (!_checkStatus(status,
		    string("Failed to get the fill value for ") + nc_var_name + string(" named ") + nc_fill_value_name ))
    return 0;
 
  float missing_value;
  status =  nc_get_att_float(nc_id, var_id,
			     nc_missing_value_name.c_str(), &missing_value);
  if (!_checkStatus(status,
		    string("Failed to get the missing value for ") + nc_var_name + string(" - using fill value")),
      _params->debug)
    missing_value = fillVal;

  float *data = new float[num_records];
    
  status = nc_get_var_float(nc_id, var_id, data);
  if (!_checkStatus(status,
		    string("Failed to read variable for ") + nc_var_name))
  {
    delete [] data;
    return 0;
  }
    
  for (unsigned int i = 0; i < num_records; ++i)
  {
    if (data[i] == fillVal || data[i] == missing_value)
      data[i] = STATION_NAN;
  }

  return data;
}
  
float *madis2Spdb::_readFloatVar(const int nc_id,
				 const unsigned int num_records,
				 const string &nc_var_name,
				 const string &nc_qcr_var_name,
				 const string &nc_fill_value_name,
				 const string &nc_missing_value_name)
{
  float *data;
    
  if ((data = _readFloatVar(nc_id, num_records, nc_var_name,
			    nc_fill_value_name, nc_missing_value_name)) == 0)
    return 0;
    
  int var_id;
  int status;
    
  status = nc_inq_varid(nc_id, nc_qcr_var_name.c_str(), &var_id);
  if (!_checkStatus(status,
		    string("Failed to get variable ID for ") + nc_qcr_var_name))
  {
    delete [] data;
    return 0;
  }
	
  status = nc_get_var_int(nc_id, var_id, _QCR);
  if (!_checkStatus(status,
		    string("Failed to read QCR for ") + nc_var_name))
  {
    delete [] data;
    return 0;
  }
	
  for (unsigned int i = 0; i < num_records; ++i)
  {
    if (_QCR[i] != 0)
      data[i] = STATION_NAN;
  }

  return data;
}
  
double *madis2Spdb::_readDoubleVar(const int nc_id,
				   const unsigned int num_records,
				   const string &nc_var_name,
				   const string &nc_fill_value_name,
				   const string &nc_missing_value_name)
{
  int var_id;
  int status;
    
  status = nc_inq_varid(nc_id, nc_var_name.c_str(), &var_id);
  if (!_checkStatus(status,
		    string("Failed to get variable ID for ") + nc_var_name))
    return 0;

  double fillVal;
  status =  nc_get_att_double(nc_id, var_id,
			      nc_fill_value_name.c_str(), &fillVal);
  if (!_checkStatus(status,
		    string("Failed to get the fill value for ") + nc_var_name + string(" named ") + nc_fill_value_name ))
    return 0;
 
  double missing_value;
  status =  nc_get_att_double(nc_id, var_id,
			      nc_missing_value_name.c_str(), &missing_value);

  if (!_checkStatus(status,
		    string("Failed to get the missing value for ") + nc_var_name + string(" - using fill value")),
      _params->debug)
    missing_value = fillVal;

  double *data = new double[num_records];
    
  status = nc_get_var_double(nc_id, var_id, data);
  if (!_checkStatus(status,
		    string("Failed to read variable for ") + nc_var_name))
  {
    delete [] data;
    return 0;
  }
  
  for (unsigned int i = 0; i < num_records; ++i)
  {
    if (data[i] == fillVal || data[i] == missing_value)
      data[i] = STATION_NAN;
  }

  return data;
}
  
bool madis2Spdb::_readString(const int nc_id,
			     const int string_index,
			     const unsigned int str_len,
			     const string &nc_var_name,
			     char *nc_string,
			     const bool printMsg)
{
  int status;
  int var_id;
    
  status = nc_inq_varid(nc_id, nc_var_name.c_str(), &var_id);
  if (!_checkStatus(status,
		    string("Failed to get variable ID for ") + nc_var_name,
		    printMsg))
    return false;

  size_t index[2];
  index[0] = string_index;
  for (unsigned int j = 0; j < str_len; ++j)
  {
    index[1] = j;
    status = nc_get_var1_text(nc_id, var_id, index, &nc_string[j]);
    if (!_checkStatus(status,
		      string("Failed to get text variable ") + nc_var_name))
      return false;
  }
  nc_string[str_len] = char(0);

  return true;
}
