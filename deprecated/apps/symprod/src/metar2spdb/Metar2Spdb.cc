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
///////////////////////////////////////////////////////////////////////////
//  metar2spdb top-level application class
//
//////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <string.h>
#include <ctime>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/MsgLog.hh>
#include <didss/LdataInfo.hh>
#include "Metar2Spdb.hh"
#include "DataMgr.hh"
using namespace std;

Metar2Spdb::Metar2Spdb()
{
   msgLog        = new MsgLog();
   dataMgr       = NULL;
   inputFilePrfx = NULL;
   SingleFile    = NULL;
}

Metar2Spdb::~Metar2Spdb() 
{
   if( dataMgr )
      delete dataMgr;
   if( msgLog )
     delete msgLog;
   if( NULL != inputFilePrfx ){
     STRfree( inputFilePrfx );
   }
   if( NULL != SingleFile ){
      STRfree( SingleFile );
   }
}

int
Metar2Spdb::init( int argc, char **argv )
{

   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog->setApplication( getProgramName() );
   ucopyright( (char*)PROGRAM_NAME );
   
   // read command line args
   
   tdrp_override_t override;
   TDRP_init_override( &override );   
   if ( processArgs( argc, argv, override ) != SUCCESS ) {
     return( FAILURE );              
   }

   // get TDRP params
   
   char *paramsPath = (char *) "not-set";
   if (_params.loadFromArgs(argc, argv,
			    override.list,
			    &paramsPath)) {
     cerr << "ERROR: " << getProgramName() << endl;
     cerr << "  Problem with TDRP parameters" << endl;
     cerr << "  Param file: " << paramsPath << endl;
     return ( FAILURE );
   }
   TDRP_free_override( &override );                     

   //
   // Register with procmap now that we have the instance name
   //
   PMU_auto_init( (char*)PROGRAM_NAME, _params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   
   //
   // Initialize the messaging
   //
   initLog();

   //
   // Set the execution parameters
   //
   setPrfx( _params.file_prefix );
   
   // set up data manager

   dataMgr = new DataMgr(_params);
   if( dataMgr->init() ) {
     return ( FAILURE );
   }

   return( SUCCESS );
}

void Metar2Spdb::setPrfx( char* prefix ) 
{
  inputFilePrfx = STRdup( prefix );
   if( prefix[strlen(prefix)-1] != '.' )
    STRconcat( inputFilePrfx, ".", strlen(prefix)+1 );
}

void
Metar2Spdb::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -debug ]               produce debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -f METAR_File ]        run on a single file.\n"
        << endl;

   Params::usage(cerr);
   dieGracefully( -1 );
}

int
Metar2Spdb::processArgs( int argc, char **argv,
                         tdrp_override_t& override )
{
   int          i;
   char         tmpStr[BUFSIZ];

   //
   // Process each argument
   //
   for( i=1; i < argc; i++ ) {
      //
      // request for usage information
      //
      if ( !strcmp(argv[i], "--" ) ||
           !strcmp(argv[i], "-h" ) ||
           !strcmp(argv[i], "-help" ) ||
           !strcmp(argv[i], "-man" )) {
         usage();
      }

      //
      // request for verbose debug messaging
      //
      if ( !strcmp(argv[i], "-debug" )) {
         sprintf(tmpStr, "debug = TRUE;");
         TDRP_add_override(&override, tmpStr);
      }

      else if (!strcmp( argv[i], "-f") ){
	if (i == argc -1) {
	  fprintf(stderr,"Specify filename with -f option\n");
	  exit(-1);
	}
	i++; SingleFile = STRdup(argv[i]); i++; 
	// Decode the time, if possible.
	GetTimeFromFileName(SingleFile, &SingleTime);
	fprintf(stdout,"METAR time is : %s\n",
		utimstr(SingleTime));
      }
   }

   return( SUCCESS );
}



int AllDigits(char *s, int len){
  for (int i=0;i<len;i++){
    if (!(isdigit(s[i]))) return 0;
    if (s[i] == '.') return 0;
  }
  return 1;
}


void
Metar2Spdb::GetTimeFromFileName(char *FileName, time_t *time){

  date_time_t t;

  t.sec = 0;
  //
  // First, try for the YYYYMMDDHHMM string (length 12).
  // Search backwards so that the filname take precedance over
  // the directory name.
  //
  int len = strlen(FileName);
  if (len >= 12){
    for (int i = len - 12; i > -1; i--){

      if (AllDigits(FileName + i, 12) && 
	     (5 == sscanf(FileName + i,"%4d%2d%2d%2d%2d.",
		      &t.year, &t.month, &t.day,
		      &t.hour, &t.min))){
	uconvert_to_utime( &t);
	*time = t.unix_time;
	return;
      }
    }
  }


  t.min = 0;
  //
  // Next, try for the YYYYMMDDHH string (length 10).
  //
  len = strlen(FileName);
  if (len >= 10){
    for (int i= len - 10; i > -1; i--){

      if (AllDigits(FileName + i, 10) && 
	  (4 == sscanf(FileName + i,"%4d%2d%2d%2d.",
		      &t.year, &t.month, &t.day,
		      &t.hour))){
	uconvert_to_utime( &t);
	*time = t.unix_time;
	return;
      }
    }
  }

  //
  // Next, try for the YYYYMMDD/HHMMSS string (length 15).
  //
  len = strlen(FileName);
  char *lastSlash = strrchr(FileName, '/');
  if (lastSlash != NULL &&
      lastSlash - 8 > FileName) {
    if (6 == sscanf(lastSlash - 8,"%4d%2d%2d/%2d%2d%2d.",
		    &t.year, &t.month, &t.day,
		    &t.hour, &t.min, &t.sec)) {
      uconvert_to_utime( &t);
      *time = t.unix_time;
      return;
    }
  }

  // Can't do it - fatal.
  fprintf(stderr,"Failed to decode time from %s\n",FileName);
  exit(-1);

}

void
Metar2Spdb::initLog()
{
   //
   // Enable debug level messaging
   //
   if ( _params.debug == TRUE )
      msgLog->enableMsg( DEBUG, true );
   else 
      msgLog->enableMsg( DEBUG, false );

   //
   // Enable info level messaging
   //
   if ( _params.info == TRUE )
      msgLog->enableMsg( INFO, true );
   else
      msgLog->enableMsg( INFO, false );

   //
   // Direct message logging to a file
   //
   if ( msgLog->setOutputDir( _params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        _params.log_dir );
   }

}
 
int
Metar2Spdb::run()
{
  //
  // Single file mode
  //

  if( _params.mode == Params::SINGLE_FILE ) { // mode

    dataMgr->processFile(SingleFile, SingleTime, 0);
    exit(0);

    //
    // Archive mode
    //
    
  } else if( _params.mode == Params::ARCHIVE ) { // mode

    if( dataMgr->processFiles() != SUCCESS ) {
      return( FAILURE);
    }
    return(SUCCESS);
    
  } else if (_params.mode == Params::REALTIME ) { // mode
    
    //
    // Realtime mode
    //
    bool forever = true;
    time_t nextCheckTime = 0;

    while(forever) {
      
      time_t now = time(0);
      
      if( now >= nextCheckTime ) {
	nextCheckTime = now + _params.check_seconds;
	
	if( dataMgr->processFiles() != SUCCESS ) {
	  return( FAILURE );
	}
	
	PMU_auto_register("Zzzzz ... ");
	if( _params.debug ) {
	  POSTMSG( DEBUG, "Next Scan of data base scheduled for %s",
		   ctime(&nextCheckTime));
	}
      } else {
	PMU_auto_register("Zzzzz ... ");
      }
      
      PMU_auto_register("Zzzzz ... ");
      sleep(1);
      
    }
    
  } else if (_params.mode == Params::REALTIME_WITH_LDATA) { // mode
    
    //
    // Realtime mode with latest data info
    //
    
    LdataInfo ldata(_params.input_dir, _params.debug);
    bool forever = true;
    
    while(forever) {
      
      ldata.readBlocking(_params.max_realtime_valid_age,
			 5000,
			 PMU_auto_register);
      
      const date_time_t &ltime = ldata.getLatestTimeStruct();
      char fileName[1024];
      sprintf(fileName, "%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	      ltime.year, ltime.month, ltime.day,
	      PATH_DELIM,
	      ltime.hour, ltime.min, ltime.sec,
	      _params.file_ext);
      
      if( dataMgr->processFile(fileName, ltime.unix_time) != SUCCESS ) {
	return( FAILURE );
      }

    } // while(forever) 
    
  } // mode
  
  return(SUCCESS);

}



