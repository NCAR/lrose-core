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
#include <strings.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/DateTime.hh>
#include <rapformats/ComboPt.hh>
#include <dsserver/DsLdataInfo.hh>

#include "Params.hh"
#include "MosFcastRaw.hh"
using namespace std;

//
// Constants
//
const int MosFcastRaw::HOURS_TO_SEC = 60 * 60;
const int MosFcastRaw::DAYS_TO_SEC = 24 * 60 * 60;

//
// Constructor
//
MosFcastRaw::MosFcastRaw(){

  startTime = 0;
  endTime = 0; // Default to REALTIME mode.

  TDRP_init_override(&override);

}

//////////////////////////////////////////////  
//
// Destructor
//

MosFcastRaw::~MosFcastRaw(){

  TDRP_free_override(&override);

}

//////////////////////////////////////////////////
// Main method - run.
//
int MosFcastRaw::init( int argc, char **argv ){

  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (ParseArgs(argc,argv)) return -1;

  //
  // Get TDRP args and check in to PMU.
  //

  
  if (P.loadFromArgs(argc,argv,override.list,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("MosFcastRaw", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  if ((startTime != 0) || (endTime != 0)){
    P.Mode = Params::ARCHIVE;
  }

  //
  // Initialize ApplyRegression object
  //
  applyRegression.init( P.ClearVisibility, P.ClearCeiling, P.RegressionUrl,
                        P.InUrl, P.RegressionLookBack * DAYS_TO_SEC, 
                        P.ModelDeltaT * HOURS_TO_SEC,
                        P.CheckForecast ? true : false,
			P.Debug >= Params::DEBUG_VERBOSE,
                        (P.Mode == Params::REALTIME) ? true : false,
						P.debug_output_dir);

  return 0;

}

///////////////////////////////////////////////
// Print usage message
//

void MosFcastRaw::usage(ostream &out)
  
{

  out << "Usage: MosFcastRaw [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -instance ? ] set instance\n"
      << "       [ -interval YYYYMMDDhhmmss YYYYMMDDhhmmss]\n"
      << "         data interval - forces ARCHIVE mode\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;
  
  Params::usage(out);

}
    
///////////////////////////////////////////////
// Parse command line args.
//
int MosFcastRaw::ParseArgs(int argc,char *argv[]){

  int iret = 0;
  char tmp_str[BUFSIZ];

  for (int i=1; i<argc; i++){
    
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "-help")) ||
         (!strcmp(argv[i], "-man")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                

      usage(cout);
      exit (0);

    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "Debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "Debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "Mode = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "Instance = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-interval")){

      sprintf(tmp_str, "Mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);
      
      i++;
      if (i == argc) {
	cerr << "Must specify start and end times with -interval" << endl;
	iret = -1;
      }
      
      date_time_t T;
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for start time is YYYYMMDDhhmmss" << endl;
        iret = -1;
      }
      uconvert_to_utime( &T );
      startTime = T.unix_time;
    
      i++;
      if (i == argc) {
	cerr << "Must specify end time with -interval" << endl;
	iret = -1;
      }
      
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for end time is YYYYMMDDhhmmss" << endl;
	iret = -1;
      }
      uconvert_to_utime( &T );
      endTime = T.unix_time;
      
    } // if

  } // i

  if (iret) {
    usage(cerr);
  }

  return (iret);

}

//////////////////////////////////////////////////////
//
// Principal run method
//

int MosFcastRaw::run()

{

  //
  // Set up for input.
  // Depends on what mode we're in.
  //

  if (P.Mode == Params::ARCHIVE){

    return _runArchive();

  } else {

    return _runRealtime();

  }

}

////////////////////////////////////////////////////////
// ARCHIVE mode run method

int MosFcastRaw::_runArchive()

{

  DsSpdb OutSpdb;
  OutSpdb.addUrl(P.OutUrl);
  
  DsSpdb InSpdb;
  if (InSpdb.getInterval(P.InUrl, startTime, endTime + P.MaxLeadSecs)) {
    cerr << "ERROR - MosFcastRaw::_runArchive()" << endl;
    cerr << "  Failed to get spdb data from " << P.InUrl << endl;
    return -1;
  }
  
  if (P.Debug){
    cerr << "Archive mode : ";
    cerr << "  "
	 << InSpdb.getNChunks() << " chunks between "
	 << utimstr(startTime) << " and " << utimstr(endTime)
	 << endl;
  }
    
  // process the chunks returned

  OutSpdb.clearPutChunks();
  const vector <Spdb::chunk_t> &chunks = InSpdb.getChunks();

  for (size_t ii = 0; ii < chunks.size(); ii++) {

    int issue_time = chunks[ii].valid_time - chunks[ii].data_type2;
    if (issue_time >= startTime && issue_time < endTime){
		      
      _processChunk(chunks[ii], OutSpdb);
    }

  } // ii
  
  if (OutSpdb.put(SPDB_STATION_REPORT_ID,
		  SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - MosFcastRaw::_runArchive()" << endl;
    cerr << "  Cannot put output to url: " << P.OutUrl << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////
// REALTIME mode run method

int MosFcastRaw::_runRealtime()

{

  DsSpdb OutSpdb;
  OutSpdb.addUrl(P.OutUrl);
  OutSpdb.setLeadTimeStorage(Spdb::LEAD_TIME_IN_DATA_TYPE2);
  DsLdataInfo ldata(P.InUrl);
  
  while (true) {

    if (P.Debug) {
      cerr << "  Waiting for data ..." << endl;
    }

    // wait for new data

    ldata.readBlocking(P.MaxValidAge, 5000,
		       PMU_auto_register);
    time_t validTime = ldata.getLatestValidTime();

    // check if we have a lead time

    bool hasLeadTime = false;
    int leadTime = 0;
    if (ldata.isFcast()) {
      hasLeadTime = true;
      leadTime = ldata.getLeadTime();
    }

    if (P.Debug) {
      cerr << "  Got data" << endl;
      cerr << "  validTime: " << DateTime::str(validTime) << endl;
      if (hasLeadTime) {
	cerr << "  leadTime(s): " << leadTime << endl;
      }
    }

    // get the mm5 spdb data for this valid time

    DsSpdb InSpdb;
    if (InSpdb.getExact(P.InUrl, validTime)) {
      cerr << "ERROR - MosFcastRaw::_runRealtime()" << endl;
      cerr << "  Failed to get spdb data from " << P.InUrl << endl;
      cerr << "  Valid time: " << DateTime::str(validTime) << endl;
      continue;
    }

    // if we have a lead time, we only process chunks with this lead
    // time stored in data_type2

    // process the chunks returned
    
    OutSpdb.clearPutChunks();
    const vector <Spdb::chunk_t> &chunks = InSpdb.getChunks();

    if (P.Debug) {
      cerr << "  Input data has nchunks: " << chunks.size() << endl;
    }
    
    for (size_t ii = 0; ii < chunks.size(); ii++) {
      
      if (!hasLeadTime ||
	  chunks[ii].data_type2 == leadTime) {
	_processChunk(chunks[ii], OutSpdb);
      }
      
    } // ii
    
    if (P.Debug) {
      cerr << "  Writing out data." << endl;
      cerr << "  N output chunks: " << OutSpdb.nPutChunks() << endl;
    }
    
    if (OutSpdb.put(SPDB_STATION_REPORT_ID,
		    SPDB_STATION_REPORT_LABEL)) {
      cerr << "ERROR - MosFcastRaw::_runRealtime()" << endl;
      cerr << "  Cannot put output to url: " << P.OutUrl << endl;
      continue;
    }
    
  } // while true

  return 0;

}
  
//////////////////
// process a chunk

int MosFcastRaw::_processChunk(const Spdb::chunk_t &chunk,
			       DsSpdb &OutSpdb)

{
    
  // Disassemble chunk into a combo point.
  
  ComboPt C;
  C.disassemble(chunk.data, chunk.len);
  
  // apply regression and put results into station_report

  station_report_t S;
  MEM_zero(S);

  if(applyRegression.process(C, S, chunk.data_type2, chunk.data_type) != 0) {
    return -1;
  }
  
  // Make sure that if one of the wind entries is missing,
  // the other one is too - Niles.

  if (S.windspd == STATION_NAN) S.winddir = STATION_NAN;
  if (S.winddir == STATION_NAN) S.windspd = STATION_NAN;

  // byte swap
  
  station_report_to_be( &S );

  // add output chunk to buffer

  OutSpdb.addPutChunk(chunk.data_type,
		      chunk.valid_time,
		      chunk.valid_time + P.Longevity,
		      sizeof(S),
		      (void *) &S,
		      chunk.data_type2);

  return 0;

}
     






