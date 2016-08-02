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

#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/umisc.h>
#include <stdio.h>

int main(int argc, char *argv[]){


  bool gotStart=false;
  bool gotEnd=false;
  bool gotUrl=false;
  bool gotFieldName=false;

  //
  // Go through the command line arguments and see that they
  // are all present.
  //

  date_time_t start, end;
  char *url = NULL;
  char *fieldName = NULL;

  for (int i=1; i < argc; i++){

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
    // Field ?
    //
    if (!(strcmp(argv[i],"-fieldName"))){
      i++;
      if (i < argc){
	fieldName = argv[i];
	gotFieldName = true;
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
  if ( !gotStart || !gotEnd || !gotUrl || !gotFieldName){
    fprintf(stderr,"USAGE : PrintGenPt1D -url ? -fieldName ?\n");
    fprintf(stderr,"   -start \"YYYY MM DD hh mm ss\"\n");
    fprintf(stderr,"   -end \"YYYY MM DD hh mm ss\"\n\n");
    fprintf(stderr,"Resulting printout has the following format :\n");
    fprintf(stderr,"unixTime year month day hour min sec lat lon dataValue\n");
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
   GenPt G;
    if (0 != G.disassemble(Input.getChunks()[i].data,
			   Input.getChunks()[i].len)){
      fprintf(stderr,
	      "GenPt dissassembly failed for point %d\n", i);
      exit(-1);
    }

    //
    // Get stuff from the GenPt.
    //
    time_t spdbDataTime = G.getTime();
    double lat = G.getLat();
    double lon = G.getLon();

    //
    // Get the values that were stored with the GenPt.
    //
    int fn = G.getFieldNum( fieldName );
    //
    // If the field name is -1 then that field does
    // not exist for this point. Only proceed with the printout
    // if that is not the case.
    //
    if (fn != -1){
      double dataValue = G.get1DVal(fn);
      date_time_t dataTime;
      dataTime.unix_time = spdbDataTime;
      uconvert_from_utime( &dataTime );
      //
      // Do the printing to stdout (User can re-direct to
      // a file if they want to).
      //
      // First, print the time information.
      //
      fprintf(stdout,
	      "%ld %d %d %d %d %d %d ",
	      (long)dataTime.unix_time,
	      dataTime.year,
	      dataTime.month,
	      dataTime.day,
	      dataTime.hour,
	      dataTime.min,
	      dataTime.sec);
      //
      // Then the lat and lon.
      //
      fprintf(stdout,
	      "%g %g ",
	      lat, lon);
      //
      // And finally the data value.
      //
      fprintf(stdout,"%g\n",dataValue);
      numPrinted++;
    }
  }

  if (numPrinted == 0){
    fprintf(stderr,"No data found with field name %s ",
	    fieldName);
    fprintf(stderr," at URL %s in the interval from %s to %s.\n",
	    url, utimstr(start.unix_time), utimstr(end.unix_time));
    exit(-1);
  }

  //
  // Normal termination.
  //
  return 0;

}


