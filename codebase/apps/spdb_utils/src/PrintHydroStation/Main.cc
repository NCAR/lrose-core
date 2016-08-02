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

#include <rapformats/HydroStation.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include <stdio.h>
#include <string>

void printData( double value );

int main(int argc, char *argv[]){


  bool printingNames = false;
  bool gotStart=false;
  bool gotEnd=false;
  bool gotUrl=false;
  bool stationNameSpecified = false;
  char *desiredStationName = NULL;
  //
  // Go through the command line arguments and see that they
  // are all present.
  //

  date_time_t start, end;
  char *url = NULL;

  for (int i=1; i < argc; i++){
    //
    // Printing Station Names only ?
    //
    if (!(strcmp(argv[i],"-namesOnly"))){
      printingNames = true;
    }

    //
    // Station Name ?
    //
    if (!(strcmp(argv[i],"-station"))){
      i++;
      if (i < argc){
	desiredStationName = argv[i];
	stationNameSpecified = true;
      }
    }
    //
    // URL ?
    //
    if (!(strcmp(argv[i],"-url"))){
      i++;
      if (i < argc){
	url = argv[i];
	gotUrl = true;
      }
    }
    //
    // Start time?
    //
    if (!(strcmp(argv[i],"-start"))){
      i++;
      if (i < argc){
	if (6 == sscanf(argv[i],
			"%d %d %d %d %d %d",
			&start.year, &start.month, &start.day,
			&start.hour, &start.min, &start.sec)){
	  uconvert_to_utime( &start );
	  gotStart = true;
	}
      }
    }
    //
    // End time?
    //
    if (!(strcmp(argv[i],"-end"))){
      i++;
      if (i < argc){
	if (6 == sscanf(argv[i],
			"%d %d %d %d %d %d",
			&end.year, &end.month, &end.day,
			&end.hour, &end.min, &end.sec)){
	  uconvert_to_utime( &end );
	  gotEnd = true;
	}
      }
    }
  }
  //
  // Make sure we got all the required arguments.
  // Print help if not.
  //
  if ( !gotStart || !gotEnd || !gotUrl){
    fprintf(stderr,"USAGE : PrintHydroStation -url\n");
    fprintf(stderr,"   -start \"YYYY MM DD hh mm ss\"\n");
    fprintf(stderr,"   -end \"YYYY MM DD hh mm ss\"\n\n");
    fprintf(stderr,"   -station stationName [optional, by default does all stations.]\n");
    fprintf(stderr,"   -namesOnly - causes printout of station names only.\n");
    fprintf(stderr,"                This is useful for selectin a name to specify\n");
    fprintf(stderr,"                with the -station option. Also prints lat, lon.\n");
    fprintf(stderr,"Resulting printout has the following format :\n\n");
    fprintf(stderr,"unixTime year month day hour min sec lat lon alt\n");
    fprintf(stderr,"wind_speed wind_direction temperature rel_hum\n" );
    fprintf(stderr,"rainfall solar_rad pressure soil_moist1 soil_moist2\n" );
    fprintf(stderr,"soil_moist3 soil_moist4 soil_temp\n");
    exit(-1);
  }
  //
  // Read the data.
  //

  DsSpdb Input;
  if (Input.getInterval(url, start.unix_time, end.unix_time)){
    fprintf(stderr,"GetInterval call failed - problem with URL?\n");
    exit(-1);
  }
  
  int numPoints = Input.getNChunks();

  if (numPoints == 0){
    fprintf(stderr,"No data found");
    fprintf(stderr," at URL %s in the interval from %s to %s.\n",
	    url, utimstr(start.unix_time), utimstr(end.unix_time));
    exit(-1);
  }


  int numPrinted = 0;
  for (int i=0; i < numPoints; i++){
    HydroStation H;
    if ( !(H.disassemble(Input.getChunks()[i].data,
			 Input.getChunks()[i].len))){
      fprintf(stderr,
	      "HydroStation dissassembly failed for number %d\n", i);
      exit(-1);
    }
    //
    // Check if this is the station we want (if specified).
    //
    bool toPrint = true;
    //
    if (printingNames){
      fprintf(stderr,"Name \"%s\" at %g, %g\n", 
	      H.getStationName().c_str(),
	      H.getLatitude(), H.getLongitude());
      numPrinted++;
    }
    //
    if (
	(printingNames) ||
	(stationNameSpecified && strcmp(desiredStationName, H.getStationName().c_str() ))
	){
      toPrint = false;
    }
    //
    if (toPrint){
      //
      // Do the printing to stdout (User can re-direct to
      // a file if they want to).
      //
      // First, print the time information.
      //
      DateTime  dataTime = H.getReportTime();
      
      fprintf(stdout,
	      "%ld %d %d %d %d %d %d ",
	      (long)dataTime.utime(),
	      dataTime.getYear(),
	      dataTime.getMonth(),
	      dataTime.getDay(),
	      dataTime.getHour(),
	      dataTime.getMin(),
	      dataTime.getSec() );
      //
      // Then the lat, lon and alt.
      //
      fprintf(stdout,
	      "%g %g %g ",
	      H.getLatitude(), H.getLongitude(), H.getAltitude());
      
      //
      // And the data values.
      //
      printData( H.getWindSpeed() );
      printData( H.getWindDirection() );
      printData( H.getTemperature() );
      printData( H.getRelativeHumidity() );
      printData( H.getRainfall() );
      printData( H.getSolarRadiation() );
      printData( H.getPressure() );
      printData( H.getSoilMoisture1() );
      printData( H.getSoilMoisture2() );
      printData( H.getSoilMoisture3() );
      printData( H.getSoilMoisture4() );
      printData( H.getSoilTemperature() );
      //
      // End the line.
      //
      fprintf(stdout,"\n");
      numPrinted++;
    }
  }
  if (numPrinted == 0){
    fprintf(stderr,"No data found ");
    fprintf(stderr," at URL %s in the interval from %s to %s.\n",
	    url, utimstr(start.unix_time), utimstr(end.unix_time));
    if (stationNameSpecified){
      fprintf(stderr,"For station %s\n", desiredStationName);
    }
    exit(-1);
  }

  //
  // Normal termination.
  //
  return 0;

}

//
// Small routine to check for a missing value before printing.
//

void printData( double value ){

  if (value == HydroStation::DATA_NOT_AVAILABLE){
    fprintf(stdout,"%g ", -9999.0);
  } else {
    fprintf(stdout,"%g ", value);
  }

}

