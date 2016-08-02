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
#include <physics/thermo.h>
#include <toolsa/pmu.h>

#include "Sams2Spdb.hh"
using namespace std;

//
// Destructor - frees up the station descriptions.
//
Sams2Spdb::~Sams2Spdb(){
  for(int k=0; k < MaxStations; k++){
    if (_StationDesc[k]!=NULL) free(_StationDesc[k]);
  }
}
///////////////////////////////
//
// Constructor. Reads the station location file and
// stores that information in memory.
//
Sams2Spdb::Sams2Spdb(char *LocationFile,
		     char *Url){

  //
  // Set station descriptions to NULL.
  //
  for(int k=0; k < MaxStations; k++){
    _StationDesc[k]=NULL;
  }

  //
  // Set up the output URL in the DsSpdb object.
  //
  _OutputDsSpdbObject.clearUrls();  
  _OutputDsSpdbObject.addUrl(Url);

  //
  // Read the station location file.
  //
  _NumStations = 0;
  FILE *LocFile;
  const int MaxLineLen = 1024;

  LocFile=fopen(LocationFile,"rt");
  if (LocFile == NULL) return;

  char Line[MaxLineLen];

  int i=0;
  while ((i < MaxStations) && (NULL!=fgets(Line, MaxLineLen, LocFile))){ 
    //
    // Skip commented lines that start with the '#' sign.
    //
    if (Line[0] == '#') continue;
    char Desc[1024];
    if (5 == sscanf(Line,"%d %lf %lf %lf %s",
		    _StationID + i, _StationLat + i, 
		    _StationLon + i, _StationAlt + i,
		    Desc)){
      _StationDesc[i]=STRdup(Desc);
      i++;
    }
  }
  fclose(LocFile);
  _NumStations = i;
  //
  // OK - station location information is now resident in memory.
  //
  if (_NumStations == MaxStations)
    fprintf(stderr,
	    "WARNING! Maximum number of stations (%d) possibly exceeded.\n",
	    MaxStations);

  return;

}

////////////////////////////
//
// Small method to locate a station from the data read in
// from the station location file. Returns 0 on success, -1 otherwise.
//
int Sams2Spdb::_FindStation(int ID, double *Lat, double *Lon, 
			   double *Alt, char **Desc)
{

  for (int i=0; i< _NumStations; i++){
    if (ID == _StationID[i]){
      *Lat = _StationLat[i];
      *Lon = _StationLon[i];
      *Alt = _StationAlt[i];
      *Desc = _StationDesc[i];
      return 0;
    }
  }

  return -1;

}

////////////////////////////////////////
//
// Main method. Reads an input ASCII file and
// outputs the results to an SPDB database.
//
int Sams2Spdb::ReadSamFile(char *FileName, Params *TDRP){

  //
  // Decode the time from the file name.
  // Files are assumed to be named
  // YYYYMMDDhhmm.sams indicating the UTC time.
  //
  date_time_t NameTime;
  int FormatLen = strlen("YYYYMMDDhhmm.sams");
  
  if (strlen(FileName) < (unsigned)FormatLen){
    fprintf(stderr,"File name format is not YYYYMMDDhhmm.sams\n");
    return -1;
  }
  char *TimeStr = FileName + strlen(FileName) - FormatLen;
  if (5 != sscanf(TimeStr,"%04d%02d%02d%02d%02d",
		  &NameTime.year,&NameTime.month, &NameTime.day,
		  &NameTime.hour, &NameTime.min)){
    fprintf(stderr,"File name format is not YYYYMMDDhhmm.sams\n");
    return -1;
  }
  NameTime.sec=0;

  PMU_auto_register(FileName);
  //
  // Call uconvert_to_utime() so that the NameTime.unix_time
  // field gets filled out.
  //
  uconvert_to_utime( &NameTime );
  //
  // Open the input SAMS file.
  //
  FILE *SamsFile;


  SamsFile=fopen(FileName,"rt");
  if (SamsFile == NULL) return -1;

  //
  // Delcare an array of chars so we can read this file line by line.
  //
  const int MaxLineLen = 1024;
  char Line[MaxLineLen];

  //
  // Clear the SPDB chunks - this is done before and after the
  // SPDB write.
  //
  _OutputDsSpdbObject.clearPutChunks();

  //
  // Read the file line by line.
  //
  while (NULL!=fgets(Line, MaxLineLen, SamsFile)){

    PMU_auto_register(FileName);
    //
    // Each line of the input SAMS file has the following information :
    //
    //
    //      istatid,
    //      julian_day,
    //      ltime,
    //      wspd_mean,
    //      vwspd_mean,
    //      wdir,
    //      dir_std,
    //      wspd_max,
    //      pres_mean,
    //      pres_max,
    //      pres_min,
    //      dtemp_avg,
    //      soiltemp_avg,
    //      tempc_avg,
    //      rh_avg,
    //      solrad_avg,
    //      visib_avg,
    //      bgtemp_avg,
    //      wbtemp_avg,
    //      dtemp_max,
    //      soiltemp_max,
    //      tempc_max,
    //      rh_max,
    //      solrad_max,
    //      visib_max,
    //      bgtemp_max,
    //      wbtemp_max,
    //      dtemp_min,
    //      soiltemp_min,
    //      tempc_min,
    //      rh_min,
    //      solrad_min,
    //      visib_min,
    //      bgtemp_min,
    //      wbtemp_min,
    //      solrad_tot,
    //      bvol,
    //      panel_tempc,
    //      precip15min,
    //      preciptot,
    //      wspd_mean2,
    //      vwspd_mean2,
    //      wdir2,
    //      dir_std2,
    //      wspd_mean3,
    //      vwspd_mean3,
    //      wdir3,
    //      dir_std3,
    //      wspd_max2,
    //      wspd_max3  
    //
    //      -- I realise that is a long winded comment but it is worth
    //      listing them all. We will only pick off those items that can be
    //      used in the RAP metar struct, and which seem to be filled out
    //      in the dataset I have.
    //
    // Only a fraction of this information is actually used
    // in the SPDB database.
    //
    int StatID, Jday, Time;
    float WndSpd, VWndSpd, Wdir;
    float DirStd, WndMax;
    float Pavg, Pmax, Pmin;
    float Tavg, Tsoil, TC;
    float rh;
    if (15 == sscanf(Line,
		     "%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
		     &StatID,&Jday,&Time,
		     &WndSpd, &VWndSpd, &Wdir,
		     &DirStd, &WndMax,
		     &Pavg, &Pmax, &Pmin,
		     &Tavg, &Tsoil, &TC,
		     &rh)){

      //
      // Find the stations' lat, lon and alt.
      //
      double Lat,Lon,Alt;
      char *Desc;
      if (_FindStation(StatID, &Lat, &Lon, &Alt, &Desc)){
	if (TDRP->debug) fprintf(stderr,"Station ID %d not locatable.\n",StatID);
	continue;
      }
      //
      // Convert from either Knots or MPH to m/s, depending on the
      // settings in the param file.
      //
      const double Knots2metersPerSec = 0.51407;
      const double MPH2metersPerSec = 0.44694444;
      const double KPH2metersPerSec = 0.27777778;
      //
      // Set this to something to avoid compiler warnings.
      //
      double windSpeedConversionFactor = MPH2metersPerSec;
      //
      // Set it properly according to the user spec.
      //
      switch (TDRP->windUnits){

      case Params::WIND_SPEED_MPH :
	windSpeedConversionFactor = MPH2metersPerSec;
	break;

      case Params::WIND_SPEED_KPH :
	windSpeedConversionFactor = KPH2metersPerSec;
	break;

      case Params::WIND_SPEED_KTS :
	windSpeedConversionFactor = Knots2metersPerSec;
	break;

      default :
	cerr << "ERROR - unrecognised option for windUnits." << endl;
	cerr << "Please edit the param file and re-run." << endl;
	exit(-1);
	break;

      }

      //
      // Perform the unit conversion.
      //
      WndSpd = WndSpd * windSpeedConversionFactor;
      VWndSpd = VWndSpd * windSpeedConversionFactor;
      WndMax = WndMax * windSpeedConversionFactor;

      //
      // At this point there is quite a bit of work
      // with the timestamp - the reason being that the file name
      // has the UTC date and time encoded in it, while within the
      // file for each line the LOCAL time (but no date)
      // given. Therefore, we have to take the UTC time we got
      // from the filename, adjust it to local time, and see if it
      // nearly matches the local time from the file. If it
      // does then take the loacl date from the filename and the local time
      // from the file as being the time the data pertain to,
      // and adjust it back to UTC.
      //

      //
      // Decode the time from the file.
      //

      int FileHour = (int)floor((double)Time/100.0);
      int FileMinute = Time - 100*FileHour;

      //
      // Adjust the NameTime (which we got from the
      // file name) to the local time.
      //

      date_time_t NameTimeLocal;
      NameTimeLocal.unix_time = NameTime.unix_time - TDRP->adjustUTC;
      uconvert_from_utime( &NameTimeLocal );

      //
      // Take the date from this local time and the
      // time from the file time.
      //

      date_time_t FileTime;
      FileTime.sec=0;
      FileTime.year = NameTimeLocal.year;
      FileTime.month = NameTimeLocal.month;
      FileTime.day = NameTimeLocal.day;

      FileTime.hour = FileHour; FileTime.min = FileMinute;

      //
      // Use the uconvert_to_utime() routine to fill
      // out the FileTime.unix_time field.
      //

      uconvert_to_utime( &FileTime );

      //
      // If the file time is more than 10 hours ahead of the
      // expected local time, assume a day wrap has occurred and
      // subtract a day off FileTime.
      //
      
      if (FileTime.unix_time - NameTimeLocal.unix_time > 10*3600){
	FileTime.unix_time = FileTime.unix_time - 24*3600;
	uconvert_from_utime( &FileTime );
      }
      
      //
      // Check that the time specified by the file name
      // and the time specified in the file match within
      // tolerance.
      //
      
      if (fabs((double) (FileTime.unix_time - NameTimeLocal.unix_time)) 
	  > TDRP->timeTolerance){
	if (TDRP->debug) fprintf(stderr,
			      "Time mismatch : %s versus %s local time\n",
			      utimstr(FileTime.unix_time),
			      utimstr(NameTimeLocal.unix_time));
	continue;
      }

      //
      // Convert file time back to UTC from local. This
      // is the time this station reports.
      //
      FileTime.unix_time = FileTime.unix_time + TDRP->adjustUTC;
      uconvert_from_utime( &FileTime );

      //
      // Print some debugging messages, if requested.
      //
      if (TDRP->debug){
	fprintf(stderr,"Station ID :\t %d (%s)\n",StatID,Desc);
	fprintf(stderr,"Lat, Lon, Alt :\t %g %g %g\n",Lat,Lon,Alt);
	fprintf(stderr,"Julian Day :\t %d\n",Jday);

	fprintf(stderr,"Time    :\t %d (%02d:%02d) expected %02d:%02d\n",
		Time,FileHour,FileMinute, NameTimeLocal.hour,
		NameTimeLocal.min);

	fprintf(stderr,"Stored under time %s UTC\n",
		utimstr(FileTime.unix_time));

	fprintf(stderr,"Wind Speed, VWndSpd, Direction : %f %f %f\n",
		WndSpd, VWndSpd, Wdir);

	fprintf(stderr,"DirStd, WndMax : \t %f %f\n",DirStd,WndMax);
	fprintf(stderr,"Pressure avg, min, max : %f %f %f\n",
		Pavg, Pmin, Pmax);

	fprintf(stderr,"Temperature avg, soil, C : %f %f %f\n",
		Tavg, Tsoil, TC);

	fprintf(stderr,"Relative humidity : \t %f\n",rh);


	fprintf(stderr,"\n");

      }

      //
      // Load this up into a station_report_t structue - RAL's standard
      // format for surface station reports.
      //
      station_report_t Metar;
      
      Metar.lat = Lat; Metar.lon = Lon; Metar.alt = Alt;
      Metar.msg_id = METAR_REPORT;
      Metar.weather_type = 0; // Reset all bitwise flags.

      const int missingValue = -6999;

      double dewPoint;
      if ((rh == missingValue) || (TC == missingValue)){
	dewPoint = missingValue;
      } else {
	 dewPoint = (float)PHYrhdp(TC,rh);
      }

      Metar.time = FileTime.unix_time;
      Metar.temp = TC == missingValue ? STATION_NAN : TC;
      Metar.dew_point = dewPoint == missingValue ? STATION_NAN : dewPoint;
      Metar.relhum = rh == missingValue ? STATION_NAN : rh;
      Metar.windspd = WndSpd == missingValue ? STATION_NAN : WndSpd;
      Metar.winddir = Wdir == missingValue ? STATION_NAN : Wdir;
      Metar.windgust = WndMax == missingValue ? STATION_NAN : WndMax;
      Metar.pres = Pavg == missingValue ? STATION_NAN : Pavg;

      Metar.precip_rate = STATION_NAN;
      Metar.visibility = STATION_NAN;
      Metar.rvr = STATION_NAN;
      Metar.ceiling = STATION_NAN;
      Metar.liquid_accum = STATION_NAN;
      Metar.accum_start_time = 0;

      sprintf(Metar.shared.metar.weather_str,"%s","");

      STRncopy(Metar.station_label,Desc,ST_LABEL_SIZE);

      //
      // Prepare the structure for SPDB storage by byte-swapping it
      // so that the fields are in big-Endian order.
      //
      station_report_to_be( &Metar ); 

      //
      // Get the station ID as a 4 character string and
      // the use the hash function to set the data type.
      //
      char IDstr[8];
      sprintf(IDstr, "%04d", StatID);
      int dataType = Spdb::hash4CharsToInt32( IDstr );

      //
      // Add this chunk of byte-swapped data to the list
      // of things to save out. We save this out indexed
      // by the data time and the data type. 
      //
      _OutputDsSpdbObject.addPutChunk(dataType,
		    FileTime.unix_time,
		    FileTime.unix_time + TDRP->expireDelta,
		    sizeof (Metar),
		    (void *) &Metar);

    } // if we got a valid SAMS line
  } // Loop through SAMS file.

  //
  // Close the input file.
  //
  fclose(SamsFile);
  //
  // Write the SPDB data that we have on our list to save.
  //
  _OutputDsSpdbObject.put(SPDB_STATION_REPORT_ID, "SAMS");
  _OutputDsSpdbObject.clearPutChunks(); // Just to free up the buffer.

  PMU_auto_register("Done with file.");

  return 0;

}



