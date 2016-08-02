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
//
// Simple program to reformat ASCII terrain data into MDV.
// Niles Oien June 2005.
//
#include <cstdio>
#include <iostream>
#include <toolsa/pjg_flat.h>
#include "Params.hh"
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

using namespace std;

//
// File scope.
//
void _processForecast(Params *params, time_t dataTime, time_t offset);
void _processNonForecast(Params *params, time_t genTime, time_t offset);


int main( int argc, char *argv[] )
{
  //
  // Get params and check in.
  //  
  Params params;

  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  PMU_auto_init("MdvRepeatDay", params.instance,
                PROCMAP_REGISTER_INTERVAL);

  //
  // Get the start and end times and form a vector
  // of available input times.
  //
  date_time_t T;
  T.year = params.date.year; T.month = params.date.month; T.day = params.date.day; 
  //
  T.hour=0; T.min=0; T.sec=0;
  uconvert_to_utime( &T );
  time_t startTime = T.unix_time;
  //
  T.hour=23; T.min=59; T.sec=59;
  uconvert_to_utime( &T );
  time_t endTime = T.unix_time;
  //

  DsMdvx Tlist;
  if (params.forecastMode){
    Tlist.setTimeListModeGen(params.InUrl, startTime, endTime);
  } else {
    Tlist.setTimeListModeValid( params.InUrl, startTime, endTime);
  }

  Tlist.compileTimeList();

  vector<time_t> inTimes = Tlist.getTimeList();

  if (inTimes.size() == 0){
    cerr << "No data for specified date found." << endl;
    return -1;
  }

  if (params.debug){
    cerr << inTimes.size() << " data times found." << endl;
    if (params.debug >= Params::DEBUG_VERBOSE){
      for (unsigned i=0; i < inTimes.size(); i++){
	cerr << "Time " << i << " : " << utimstr( inTimes[i] ) << endl;
      }
    }
  }


  int lastDay = -1;
  time_t offset = 0L;
  vector <time_t> availableTimes;

  do {
    time_t now;
    now = time(NULL) - params.realtimeDelay;

    time_t processTime = (time_t)(floor(double(now)/double(params.Delay)))*params.Delay;

    if (params.debug){
      cerr << "Processing at current time " << utimstr(processTime) << endl;
    }

    T.unix_time = processTime;    uconvert_from_utime( &T );

    if (T.day != lastDay){
      //
      // Calculate the time offset that needs to be added for this day.
      //
      T.hour = 0; T.min = 0; T.sec = 0;
      uconvert_to_utime( &T );
      offset = T.unix_time - startTime;

      //
      // Time to re-populate
      // the vector of available times to process.
      // Skip the ones that are too early if we have just started.
      //
      availableTimes.clear();
      for (unsigned i=0; i < inTimes.size(); i++){
	if (
	    (lastDay == -1) &&
	    (!(params.catchUpOnStartup)) &&
	    ((inTimes[i] + offset) < (time(NULL) - params.realtimeDelay) )
	    ){
	  continue;
	}
	availableTimes.push_back( inTimes[i] );
      }
      lastDay = T.day;
    }

    //
    // Loop through the available times - if they are
    // ready to be processed, process them and then
    // remove them from the list of what's available.
    //
    vector <time_t>::iterator it;
    for (it = availableTimes.begin(); it != availableTimes.end(); it++){
      if (*it <= processTime - offset ){
	//
	// Ready to process.
	//
	if (params.forecastMode){
	  _processForecast(&params, *it, offset);
	} else {
	  _processNonForecast(&params, *it, offset);
	}

	if (params.debug >= Params::DEBUG_VERBOSE){
	  cerr << "Erasing time " << utimstr( *it );
	  cerr << " from input list - it's done." << endl;
	}
	availableTimes.erase( it ); it--; // Need to decrement iterator here.
      }
    }
    
    if (params.debug >= Params::DEBUG_VERBOSE){
      cerr << availableTimes.size() << " times remain : " << endl;
      for (unsigned i=0; i < availableTimes.size(); i++){
	cerr << "  Still have to get to time " << i << " : " << utimstr( availableTimes[i] ) << endl;
      }
    }
    //
    // Delay.
    //
    for (int i=0; i < params.Delay; i++){
      PMU_auto_register("working...");
      sleep(1);
    }

  } while( 1 );

  return 0;

}

////////////////////////////////////////////////////////////////
//
// Process forecast data. In this case the data time is a generation
// time. We get all the valid times resulting from this generation time and
// write them out.
//
void _processForecast(Params *params, time_t genTime, time_t offset){

  if (params->debug){
    cerr << "Processing forecast input generated at " << utimstr(genTime);
    cerr << " with offset " << double(offset)/86400.0 << " days" << endl;
  }

  DsMdvx inTim;
  inTim.setTimeListModeForecast( params->InUrl, genTime);
  inTim.compileTimeList();

  vector <time_t> vtimes = inTim.getValidTimes();

  if (params->debug){
    cerr << vtimes.size() << " valid times found for gentime " << utimstr(genTime) << endl;
    if (params->debug >= Params::DEBUG_VERBOSE){
      for (unsigned ik=0; ik < vtimes.size(); ik++){
	cerr << "  Valid time " << ik << " is " << utimstr(vtimes[ik]) << endl;
      }
    }
  }
  //
  // OK - have list of valid times for this gen time. Loop through them and
  // process the data, similar to what we do in the non-forecast case.
  //
  for (unsigned ik=0; ik < vtimes.size(); ik++){
    //
    if (params->debug >= Params::DEBUG_VERBOSE){
      cerr << endl << "    Processing valid time " << utimstr(vtimes[ik]);
      cerr << " with gen time " << utimstr(genTime) << endl;
    }
    //
    // Read the data in, add the time offset, write it out.
    //

    DsMdvx inMdvMgr;

    int leadTime = int(vtimes[ik] - genTime);

    inMdvMgr.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, params->InUrl, 86400, genTime, leadTime );

    if (inMdvMgr.readVolume()){
      cerr << "Read failed for genTime " << utimstr(genTime);
      cerr << " valid time " << utimstr(vtimes[ik]) << " from ";
      cerr << params->InUrl  << endl;
      continue;
    }
    if (params->debug >= Params::DEBUG_VERBOSE){
      cerr << "    Read file " << inMdvMgr.getPathInUse() << endl;
      cerr << "    Valid=" << utimstr(vtimes[ik]) << endl;
      cerr << "    gen = " << utimstr(genTime) << endl;
      cerr << "    Lead =" << leadTime << endl;
    }

    Mdvx::master_header_t Mhdr = inMdvMgr.getMasterHeader();
    //
    // Only add the offset to the time if it is set (non-zero) in
    // the master header.
    //

    if (params->addTimeOffset.addTimeOffset){
      offset -= params->addTimeOffset.timeOffset;
    }

    if ( Mhdr.time_gen ) Mhdr.time_gen += offset;
    if ( Mhdr.time_begin ) Mhdr.time_begin  += offset;
    if ( Mhdr.time_end ) Mhdr.time_end += offset;
    if ( Mhdr.time_centroid ) Mhdr.time_centroid += offset;
    if ( Mhdr.time_expire ) Mhdr.time_expire += offset;
    //
    // Change the name so it's clear that these are replayed data.
    //
    sprintf(Mhdr.data_set_info, "MdvRepeatDay");
    sprintf(Mhdr.data_set_name, "MdvRepeatDay");
    sprintf(Mhdr.data_set_source, "MdvRepeatDay");

    DsMdvx Out;
    Out.setMasterHeader(Mhdr);
    Out.clearFields();
    //
    // Loop throught the fields, adjust the time in the
    // field header (only one entry) and add it to the output
    // object.
    //
    for (int ifld=0; ifld < Mhdr.n_fields; ifld++){
      MdvxField *InField = inMdvMgr.getFieldByNum( ifld );

      if (InField == NULL){
	cerr << "Field " << ifld << " not found." << endl;
	continue;
      }

      Mdvx::field_header_t Fhdr = InField->getFieldHeader();

      if (params->moveOrigin.moveOrigin){
	Fhdr.proj_origin_lat = params->moveOrigin.newLat;
	Fhdr.proj_origin_lon = params->moveOrigin.newLon;
      }

      if (Fhdr.forecast_time) Fhdr.forecast_time += offset;
      if (params->debug >= Params::DEBUG_VERBOSE){
	cerr << "      Field " << ifld << " " << Fhdr.field_name << endl;
      }

      Mdvx::vlevel_header_t Vhdr = InField->getVlevelHeader();
    
      MdvxField *fld = new MdvxField(Fhdr, Vhdr, InField->getVol() );

      Out.addField(fld);

    }
    //
    // Add any chunks from the input.
    //
    for (int ic=0; ic < inMdvMgr.getNChunks(); ic++){
      MdvxChunk *inChunk = inMdvMgr.getChunkByNum( ic );
      //
      // The chunk must be created using 'new'.
      // The chunk object becomes 'owned' by the Mdvx object, and will be
      // freed by it.
      //
      MdvxChunk *outChunk = new MdvxChunk( *inChunk );
      Out.addChunk( outChunk );
    }
    //
    // Write it out.
    //
    Out.setWriteAsForecast();
    //
    if (Out.writeToDir(params->OutUrl)) {
      cerr << "Failed to wite to " << params->OutUrl << endl;
      continue;
    }
    //
    // Delay after writing file.
    //
    for (int id=0; id < params->fileWriteDelay; id++){
      sleep(1);
      PMU_auto_register("Delaying...");
    }    
  } // End of loop through valid times.

  return;

}

////////////////////////////////////////////////////////////////
//
// Process non-forecast data - add the offset to the times
// in the header and then write it out.
//
void _processNonForecast(Params *params, time_t dataTime, time_t offset){

  if (params->debug){
    cerr << "Processing non-forecast input at " << utimstr(dataTime);
    cerr << " with offset " << double(offset)/86400.0 << " days" << endl;
  }
  //
  // Read the data in, add the time offset, write it out.
  //

  DsMdvx inMdvMgr;

  inMdvMgr.setReadTime(Mdvx::READ_FIRST_BEFORE, params->InUrl, 0, dataTime);

  if (inMdvMgr.readVolume()){
    cerr << "Read failed at " << utimstr(dataTime) << " from ";
    cerr << params->InUrl  << endl;
    return;
  }

  Mdvx::master_header_t Mhdr = inMdvMgr.getMasterHeader();
  //
  // Only add the offset to the time if it is set (non-zero) in
  // the master header.
  //

  if (params->addTimeOffset.addTimeOffset){
    offset -= params->addTimeOffset.timeOffset;
  }

  if ( Mhdr.time_gen )  Mhdr.time_gen += offset;
  if ( Mhdr.time_begin ) Mhdr.time_begin  += offset;
  if ( Mhdr.time_end ) Mhdr.time_end += offset;
  if ( Mhdr.time_centroid ) Mhdr.time_centroid += offset;
  if ( Mhdr.time_expire ) Mhdr.time_expire += offset;
  //
  // Change the name so it's clear that these are replayed data.
  //
  sprintf(Mhdr.data_set_info, "MdvRepeatDay");
  sprintf(Mhdr.data_set_name, "MdvRepeatDay");
  sprintf(Mhdr.data_set_source, "MdvRepeatDay");

  DsMdvx Out;
  Out.setMasterHeader(Mhdr);
  Out.clearFields();
  //
  // Loop throught the fields, adjust the time in the
  // field header (only one entry) and add it to the output
  // object.
  //
  for (int ifld=0; ifld < Mhdr.n_fields; ifld++){
    MdvxField *InField = inMdvMgr.getFieldByNum( ifld );

    if (InField == NULL){
      cerr << "Field " << ifld << " not found." << endl;
      continue;
    }

    Mdvx::field_header_t Fhdr = InField->getFieldHeader();

    if (params->moveOrigin.moveOrigin){
      Fhdr.proj_origin_lat = params->moveOrigin.newLat;
      Fhdr.proj_origin_lon = params->moveOrigin.newLon;
    }
    
    if (Fhdr.forecast_time) Fhdr.forecast_time += offset;
    Mdvx::vlevel_header_t Vhdr = InField->getVlevelHeader();
    
    MdvxField *fld = new MdvxField(Fhdr, Vhdr, InField->getVol() );

    Out.addField(fld);

  }

  //
  // Add any chunks from the input.
  //
  for (int ic=0; ic < inMdvMgr.getNChunks(); ic++){
    MdvxChunk *inChunk = inMdvMgr.getChunkByNum( ic );
    //
    // The chunk must be created using 'new'.
    // The chunk object becomes 'owned' by the Mdvx object, and will be
    // freed by it.
    //
    MdvxChunk *outChunk = new MdvxChunk( *inChunk );
    Out.addChunk( outChunk );
  }
  //
  // Write it out.
  //
  if (Out.writeToDir(params->OutUrl)) {
    cerr << "Failed to wite to " << params->OutUrl << endl;
    return;
  }
  //
  // Delay after writing file.
  //
  for (int id=0; id < params->fileWriteDelay; id++){
    sleep(1);
    PMU_auto_register("Delaying...");
  }

  return;

}









