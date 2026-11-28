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

#include <cstdio>
#include <math.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <rapformats/station_reports.h>
#include <physics/physics.h>
#include <toolsa/pmu.h>
#include <netcdf.h>


#include "euroSfc2Spdb.hh"
using namespace std;

//
//
euroSfc2Spdb::~euroSfc2Spdb(){
  return;
}
///////////////////////////////
//
//
euroSfc2Spdb::euroSfc2Spdb(){
  return;
}


////////////////////////////////////////
//
// Main method. Reads an input netcdf file and
// outputs the results to an SPDB database.
//
int euroSfc2Spdb::ReadSamFile(char *FileName, Params *TDRP){

  int netID;
  int status = nc_open(FileName, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  // Get the number of time entries.

  int dimID;
  status = nc_inq_dimid(netID, "t", &dimID);
  _checkStatus(status, "Failed to get time dimension ID");

  size_t numTimes;
  status = nc_inq_dimlen(netID, dimID, &numTimes);
  _checkStatus(status, "Failed to get number of times");

  double lat;
  char buf[128];

  memset(buf, 0, 128);
  status =  nc_get_att_text(netID, NC_GLOBAL, "latitude", buf);
  _checkStatus(status, "Failed to get lat");
  lat = atof( buf );

  double lon;
  memset(buf, 0, 128);
  status =  nc_get_att_text(netID, NC_GLOBAL, "longitude", buf);
  _checkStatus(status, "Failed to get lon");
  lon = atof( buf );

  if (
      ( lat < TDRP->boundingBox.minLat ) ||
      ( lon < TDRP->boundingBox.minLon ) ||
      ( lat > TDRP->boundingBox.maxLat ) ||
      ( lon > TDRP->boundingBox.maxLon )
      ){
    nc_close(netID);
    return 0;
  }


  double alt;
  memset(buf, 0, 128);
  status =  nc_get_att_text(netID, NC_GLOBAL, "elevation", buf);
  _checkStatus(status, "Failed to get alt");
  alt = atof(buf);

  long wmoId;
  memset(buf, 0, 128);
  status =  nc_get_att_text(netID, NC_GLOBAL, "locationWMO", buf);
  _checkStatus(status, "Failed to get ID");
  wmoId = atoi(buf);

  if (wmoId == 0){
    wmoId = Spdb::hash4CharsToInt32(buf);
  }

  if (TDRP->debug){
    cerr << numTimes << " entries in " << FileName;
    cerr << " at " << lat << ", " << lon;
    cerr << " height " << alt << "m";
    cerr << " WMO ID : " << wmoId << endl;
  }


  //
  // Allocate space for the data and read them in.
  //
  int *dataTimes = (int *)malloc(sizeof(int)*numTimes);
  int *spd = (int *)malloc(sizeof(int)*numTimes);
  int *dir = (int *)malloc(sizeof(int)*numTimes);
  int *temp = (int *)malloc(sizeof(int)*numTimes);
  int *dp = (int *)malloc(sizeof(int)*numTimes);
  int *pres = (int *)malloc(sizeof(int)*numTimes);
  int *gust = (int *)malloc(sizeof(int)*numTimes);
  int *rh = (int *)malloc(sizeof(int)*numTimes);


  int varID;

  status = nc_inq_varid(netID, "Time", &varID);
  _checkStatus(status, "Failed to get Time variable ID");
  status = nc_get_var_int(netID, varID, dataTimes);
  _checkStatus(status, "Failed to read data times.");

  status = nc_inq_varid(netID, "ff", &varID);
  _checkStatus(status, "Failed to get speed variable ID");
  status = nc_get_var_int(netID, varID, spd);
  _checkStatus(status, "Failed to read speeds.");
  double spdSF = 1.0;
  nc_get_att_double(netID, varID, "scale_factor", &spdSF);

  status = nc_inq_varid(netID, "dd", &varID);
  _checkStatus(status, "Failed to get direction variable ID");
  status = nc_get_var_int(netID, varID, dir);
  _checkStatus(status, "Failed to read dir.");
  double dirSF = 1.0;
  nc_get_att_double(netID, varID, "scale_factor", &dirSF);

  status = nc_inq_varid(netID, "T", &varID);
  _checkStatus(status, "Failed to get temperature variable ID");
  status = nc_get_var_int(netID, varID, temp);
  _checkStatus(status, "Failed to read temps.");
  double tempSF = 1.0;
  nc_get_att_double(netID, varID, "scale_factor", &tempSF);

  status = nc_inq_varid(netID, "Td", &varID);
  _checkStatus(status, "Failed to get dew point variable ID");
  status = nc_get_var_int(netID, varID, dp);
  _checkStatus(status, "Failed to read dew point.");
  double dpSF = 1.0;
  nc_get_att_double(netID, varID, "scale_factor", &dpSF);

  status = nc_inq_varid(netID, "Psl", &varID);
  _checkStatus(status, "Failed to get pressure variable ID");
  status = nc_get_var_int(netID, varID, pres);
  _checkStatus(status, "Failed to read pressure.");
  double presSF = 1.0;
  nc_get_att_double(netID, varID, "scale_factor", &presSF);

  status = nc_inq_varid(netID, "Fxx", &varID);
  _checkStatus(status, "Failed to get gust variable ID");
  status = nc_get_var_int(netID, varID, gust);
  _checkStatus(status, "Failed to read gust data.");
  double gustSF = 1.0;
  nc_get_att_double(netID, varID, "scale_factor", &gustSF);

  nc_close(netID);

  _OutputDsSpdbObject.clearUrls();
  _OutputDsSpdbObject.addUrl(TDRP->outputUrl);

  for (unsigned it=0; it < numTimes; it++){

    PMU_auto_register("Processing");

    if (dataTimes[it] <= 0) continue;

    station_report_t Metar;
    memset(&Metar, 0, sizeof(Metar));

    Metar.lat = lat; Metar.lon = lon; Metar.alt = alt;
    Metar.msg_id = METAR_REPORT;

    Metar.time = dataTimes[it];
    Metar.temp = STATION_NAN;
    Metar.dew_point = STATION_NAN;
    Metar.relhum = STATION_NAN;
    Metar.windspd = STATION_NAN;
    Metar.winddir = STATION_NAN;
    Metar.windgust = STATION_NAN;
    Metar.pres = STATION_NAN;
    Metar.precip_rate = STATION_NAN;
    Metar.visibility = STATION_NAN;
    Metar.rvr = STATION_NAN;
    Metar.ceiling = STATION_NAN;
    Metar.liquid_accum = STATION_NAN;
    
    if (gust[it] > -500) Metar.windgust = double(gust[it])*gustSF;
    if (pres[it] > -500) Metar.pres = double(pres[it])*presSF/100.0;
    if (dp[it] > -500)   Metar.dew_point = (double(dp[it])*dpSF)-273.1;
    if (temp[it] > -500) Metar.temp = (double(temp[it])*tempSF)-273.1;
    if (dir[it] > -500)  Metar.winddir = double(dir[it])*dirSF;
    if (spd[it] > -500)  Metar.windspd = double(spd[it])*spdSF;

    if ((Metar.temp != STATION_NAN) && (Metar.dew_point != STATION_NAN)){
      Metar.relhum = 100.0*PHYhumidity(Metar.temp+273.1, Metar.dew_point+273.1);
    }

    station_report_to_be( &Metar ); 

    _OutputDsSpdbObject.addPutChunk(wmoId,
				    dataTimes[it],
				    dataTimes[it] + TDRP->expireDelta,
				    sizeof (Metar),
				    &Metar);

  }



  free(rh); free(gust); free(pres); free(dp); free(temp);
  free(dir); free(spd); free(dataTimes);

  //
  // Write the SPDB data that we have on our list to save.
  //
  _OutputDsSpdbObject.put(SPDB_STATION_REPORT_ID, SPDB_STATION_REPORT_LABEL );
  _OutputDsSpdbObject.clearPutChunks(); // Just to free up the buffer.


  return 0;

}

void euroSfc2Spdb::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}


