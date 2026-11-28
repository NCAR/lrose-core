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

//////////////////////////////////////////////
//
// Small program to read netcdf mesonet data from ATD and put it
// into SPDB format. Niles Oien July 2002.
//

#include <iostream>
#include <cstdio>
#include <string.h>

#include <netcdf.h>
#include <cstdlib>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>

#include "Params.hh"
using namespace std;

int getDimLen(int ncID, char *dimName);
float *ReadFloat(int ncID, char *varName, int length,
		 int getMissing, float *missingVal);

void  ReplaceMissing(float *data, int len, 
		     float NCmissingVal, float badVal);

int main(int argc, char *argv[]){

  //
  // Process args to get the filename. Required.
  //
  bool gotName = false;
  char fileName[MAX_PATH_LEN];
  for (int i=1; i < argc; i++){
    if (
	(!(strcmp(argv[i],"-h"))) ||
	(!(strcmp(argv[i],"-?")))
	){
      cerr << "Try the -print_params option for help." << endl;
      exit(0);
    }

    if (!(strcmp(argv[i],"-f"))){
      i++;
      if (i > argc-1){
	cerr << "Error in input file specification." << endl;
	exit(-1);
      }
      sprintf(fileName,"%s",argv[i]);
      gotName = true;
    }
  }

  //
  // Get the Tdrp parameters into the wrapper structure.
  //
  Params TrdWrap;
  if (TrdWrap.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }
  //
  // Check that we have the filename.
  //
  if (!(gotName)){
    cerr << "Must specify input filename with -f <name> option." << endl;
    exit(-1);
  }
  //
  // Check that the specified output names are all 4 characters.
  //
  for (int j=0; j < TrdWrap.NameRemap_n; j++){
    if (4 != strlen(TrdWrap._NameRemap[j].output_name)){

      cerr << "ERROR IN PARAMETER FILE - output station name ";
      cerr << TrdWrap._NameRemap[j].output_name << " is not 4 characters long. Please correct." << endl;
      exit(-1);
     
    }
  }


  //
  // Open the netCDF file.
  //
  int status, ncID;
  status = nc_open(fileName, NC_NOWRITE, &ncID);
  if (status != NC_NOERR){
    cerr << "Failed to open " << fileName << endl;
    exit(-1);
  }
  //
  // Get the dimension lengths.
  //
  int timeDimLen = getDimLen(ncID, "time");
  int numStations = getDimLen(ncID, "station");
  int idLen = getDimLen(ncID, "idLen");

  if (TrdWrap.Debug){
    cerr << "Time dimension length is : " << timeDimLen << endl;
    cerr << "Number of stations : " << numStations << endl;
    cerr << "Station name length : " << idLen << endl;
  }

  //
  // Get the variables that have dimension numStations -
  // latitude, longitude, altitude
  //
  float *lat = ReadFloat(ncID, "latitude", numStations, 0, NULL);
  float *lon = ReadFloat(ncID, "longitude", numStations, 0, NULL);
  float *alt = ReadFloat(ncID, "altitude", numStations, 0, NULL);
  //
  // Get the station name strings together.
  //
  char StationName[numStations][idLen];
  int varID;
  status = nc_inq_varid(ncID, "station", &varID);
  if (status != NC_NOERR){
    cerr << "Failed to get ID for variable station." << endl;
    exit(-1);
  }

  status = nc_get_var_text(ncID, varID, StationName[0]);
  if (status != NC_NOERR){
    cerr << "Failed to get name of station numbers." << endl;
    exit(-1);
  }
  

  if (TrdWrap.Debug){
    for (int i=0; i < numStations; i++){
      cerr << "Station : " << i+1;
      cerr << " Lat : " << lat[i] << " Lon : " << lon[i] << " Alt : " << alt[i] << endl;
    }
  }

  //
  // Get the times. This is the only time we read doubles so I do it directly.
  //
  status = nc_inq_varid(ncID, "time", &varID);
  if (status != NC_NOERR){
    cerr << "Failed to get ID for variable time." << endl;
    exit(-1);
  }
  //
  // Allocate space for the variable.
  //
  double *dataTimes = (double *) malloc(sizeof(double)*timeDimLen);
  if (dataTimes == NULL){
    cerr <<"Malloc failed for variable time." << endl;
    exit(-1);
  }
  //
  // Perpetrate the read.
  //
  status = nc_get_var_double(ncID, varID, dataTimes);
  if (status != NC_NOERR){
    cerr << "Failed to read variable time." << endl;
    exit(-1);
  }
  
  if (TrdWrap.Debug){
    for(int i=0; i < timeDimLen; i++){
      cerr << "Data at time " << utimstr((long)dataTimes[i]) << endl;
    }
  }
  //
  // See if we can read the temperature. Stored in
  // netCDF as temperature(time, station) so station
  // changes fastest in memory. Replace their missing value
  // with ours.
  //
  float NCmissingVal;
  const float badVal =-9999.0;
  //
  // Temperature.
  //
  float *temp =  ReadFloat(ncID, "TAIR", 
			   numStations*timeDimLen, 1, &NCmissingVal);
  ReplaceMissing(temp, numStations*timeDimLen, NCmissingVal, badVal);
  //
  // Pressure.
  //
  float *pres =  ReadFloat(ncID, "PRES", 
			   numStations*timeDimLen, 1, &NCmissingVal);
  ReplaceMissing(pres, numStations*timeDimLen, NCmissingVal, badVal);
  //
  // rh.
  //
  float *rh =  ReadFloat(ncID, "RELH", 
			   numStations*timeDimLen, 1, &NCmissingVal);
  ReplaceMissing(rh, numStations*timeDimLen, NCmissingVal, badVal);
  //
  // Wind dir.
  //
  float *wdir =  ReadFloat(ncID, "WDIR", 
			   numStations*timeDimLen, 1, &NCmissingVal);
  ReplaceMissing(wdir, numStations*timeDimLen, NCmissingVal, badVal);
  //
  // Wind speed.
  //
  float *wspd =  ReadFloat(ncID, "WSPD", 
			   numStations*timeDimLen, 1, &NCmissingVal);
  ReplaceMissing(wspd, numStations*timeDimLen, NCmissingVal, badVal);

  //
  // Set up output object, and
  // save out all the stations that have a shred of data.
  //
  DsSpdb Output;
  Output.clearPutChunks();

  char OutStationName[5];
  for (int is =0; is < numStations; is++){

    bool gotStationName = false;

    if (strlen(StationName[is]) != 4){
      //
      // See if we can look up the output station name
      // given the input name.
      //
      for (int j=0; j < TrdWrap.NameRemap_n; j++){
	if (!(strcmp(StationName[is], TrdWrap._NameRemap[j].input_name))){
	  sprintf(OutStationName, "%s", TrdWrap._NameRemap[j].output_name);
	  gotStationName = true;
	  if (TrdWrap.Debug){
	    cerr << "Using output name " << TrdWrap._NameRemap[j].output_name;
	    cerr << " for input name " << TrdWrap._NameRemap[j].input_name << endl;
	  }
	  break;
	}
      }
    } else { 
      sprintf(OutStationName, "%s", StationName[is]);
      gotStationName = true;
    }

    if (!(gotStationName)){
      cerr << "Failed to get ouput name for input station name " << StationName[is]  << endl;
      cerr << "Skipping this station." << endl;
    } else {
      for (int it=0; it < timeDimLen; it++){
	int index = is + it * numStations;

	if (
	    (temp[index] != badVal) ||
	    (pres[index] != badVal) ||
	    (rh[index] != badVal) ||
	    (wdir[index] != badVal) ||
	    (wspd[index] != badVal)
	    ){
	  
	  if (TrdWrap.Debug){
	    cerr << "Station : " << OutStationName << " ";
	    cerr << dataTimes[it]-dataTimes[0] << "  ";
	    cerr << temp[index] << " ";
	    cerr << pres[index] << " ";
	    cerr << rh[index] << " ";
	    cerr << wdir[index] << " ";
	    cerr << wspd[index] << " ";
	    cerr << endl;
	  }
	  //
	  // Write SPDB output.
	  //
	  station_report_t ModelReport;
	  memset(&ModelReport, 0, sizeof( station_report_t ));

	  ModelReport.time = (long) dataTimes[it];
	  ModelReport.lat = lat[is]; 
	  ModelReport.lon = lon[is]; 
	  ModelReport.alt = alt[is]; 
	  
	  if (temp[index] != badVal) ModelReport.temp = temp[index]; else ModelReport.temp = STATION_NAN;
	  if (rh[index] != badVal)   ModelReport.relhum = rh[index]; else ModelReport.relhum = STATION_NAN;
	  
	  if ( (ModelReport.temp == STATION_NAN) || (ModelReport.relhum == STATION_NAN)){
	    ModelReport.dew_point = STATION_NAN;
	  } else {
	    ModelReport.dew_point =  PHYrhdp(ModelReport.temp, ModelReport.relhum);
	  }

	  if (wdir[index] != badVal) ModelReport.winddir = wdir[index]; else ModelReport.winddir = STATION_NAN;
	  if (wspd[index] != badVal) ModelReport.windspd = wspd[index]; else ModelReport.windspd = STATION_NAN;
	  if (pres[index] != badVal) ModelReport.pres = pres[index];    else ModelReport.pres = STATION_NAN;

	  ModelReport.windgust  = STATION_NAN; 
	  ModelReport.liquid_accum  = STATION_NAN; 
	  ModelReport.precip_rate  = STATION_NAN; 
	  ModelReport.visibility  = STATION_NAN; 
	  ModelReport.rvr  = STATION_NAN; 
	  ModelReport.ceiling  = STATION_NAN; 

	  int dataType = Spdb::hash4CharsToInt32( OutStationName );
	  long dataTime = (long) dataTimes[it];

	  station_report_to_be( &ModelReport ); 

	  Output.addPutChunk(dataType,
			     dataTime,
			     dataTime + TrdWrap.Expiry,
			     sizeof( station_report_t ),
			     &ModelReport,
			     0);
	}
      }
    }
  }
  //
  // Free up, do the write, and exit.
  //
  free(lat); free(lon); free(alt); free(dataTimes);
  free(temp); free(pres); free(rh); free(wspd); free(wdir);

  nc_close(ncID);

  Output.put(TrdWrap.OutUrl,
	     SPDB_STATION_REPORT_ID,
	     SPDB_STATION_REPORT_LABEL);


  return 0;

}

///////////////////////////////////////////////////
//
// returns the length of a dimension, or dies in the attempt.
//

int getDimLen(int ncID, char dimName[]){

  int status;
  int DimID;
  status = nc_inq_dimid(ncID, dimName, &DimID);
  if (status != NC_NOERR){
    cerr << "Failed to get dimension ID for " << dimName << endl;
    exit(-1);
  }

  size_t DimLen;

  status = nc_inq_dimlen(ncID, DimID, &DimLen);
  if (status != NC_NOERR){
    cerr << "Failed to get dimension length for " << dimName << endl;
    exit(-1);
  }

  return int( DimLen );

}

///////////////////////////////////////////////////
//
// returns the requested array of floats (and allocates space) or dies.
//
float *ReadFloat(int ncID, char *varName, int length,
		 int getMissing, float *missingVal){
  //
  // Get the variable ID.
  //
  int status, varID;
  status = nc_inq_varid(ncID, varName, &varID);
  if (status != NC_NOERR){
    cerr << "Failed to get ID for variable " << varName << endl;
    exit(-1);
  }
  //
  // Allocate space for the variable.
  //
  float *p = (float *) malloc(sizeof(float)*length);
  if (p==NULL){
    cerr <<"Malloc failed for variable " << varName << endl;
    exit(-1);
  }
  //
  // Perpetrate the read.
  //
  status = nc_get_var_float(ncID, varID, p);
  if (status != NC_NOERR){
    cerr << "Failed to read variable " << varName << endl;
    exit(-1);
  }
  //
  // Get the missing value, if the caller asked for it.
  //
  if (getMissing){
    status = nc_get_att_float(ncID, varID, "missing_value", missingVal);
    if (status != NC_NOERR){
      cerr << "Failed to get missing_value for " << varName << endl;
      exit(-1);
    }
  }

  return p;

}

////////////////////////////////////////////////////////////
//
// Fix up missing values.
//
void  ReplaceMissing(float *data, int len, float NCmissingVal, float badVal){
  
  for (int i=0; i < len; i++){
    if (data[i] == NCmissingVal) data[i] = badVal; 
  }

}
