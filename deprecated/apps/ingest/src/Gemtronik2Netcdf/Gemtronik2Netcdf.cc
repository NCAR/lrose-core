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
//////////////////////////////////////////////////////////////
//  Gemtronik2Netcdf application
//
//  Jason Craig, RAP, NCAR, Boulder, CO, 80307, USA
//  March 2012
//
//  $Id: Gemtronik2Netcdf.cc,v 1.6 2016/03/07 01:23:00 dixon Exp $
//
/////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <string>

#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
#include <didss/DsInputPath.hh>

#include "Gemtronik2Netcdf.hh"

using namespace std;


Gemtronik2Netcdf::Gemtronik2Netcdf(Params *P, const char *programName)
{
  params = P;
  progName = programName;

  //
  // Register with procmap now that we have the instance name
  //
  PMU_auto_init( progName, params->instance, PROCMAP_REGISTER_INTERVAL );
  PMU_auto_register( "starting up application" );
  
  //
  // Some general stuff
  //
  msgLog.setApplication( progName );

  ucopyright( progName );

  //
  // Set debug level messaging
  //
  if ( params->debug ) {
    msgLog.enableMsg( DEBUG, true );
  } 

  //
  // Set info level messaging
  //
  if( params->info ) {
    msgLog.enableMsg( INFO, true );
  }

  readGemtronik = new ReadGemtronik(params);

}

Gemtronik2Netcdf::~Gemtronik2Netcdf()
{
  delete readGemtronik;
}

int Gemtronik2Netcdf::run(vector<string> inputFileList, time_t startTime, time_t endTime)
{
   DsInputPath *fileTrigger;

   //
   // Setup the DsInputPath based on our mode
   //
   if( params->mode == Params::REALTIME ) {
     POSTMSG( INFO, "Starting up in Realtime Mode");

     fileTrigger = new DsInputPath( progName,
				    params->debug,
				    params->input_dir,
				    params->max_valid_age_min * 60,
				    PMU_auto_register,
				    true );
     
     string label = progName;
     label += "_";
     label += params->instance;
     
     fileTrigger->setSaveLatestReadInfo( label, true );
     
   }
   else if( params->mode == Params::FILE_LIST ) {
     
     if ( inputFileList.size() > 0 ) {
       POSTMSG( INFO, "Starting up in File List Mode, NFiles: %d", inputFileList.size());

       fileTrigger = new DsInputPath( progName,
				      params->debug,
				      inputFileList,
				      false );
     }
     else{
       POSTMSG( ERROR, "Must provide a file list on command line for file list mode" );
       return( -1 );
     }
   }
   else if( params->mode == Params::START_END ) {
     POSTMSG( INFO, "Starting up in Archive Mode");

     fileTrigger = new DsInputPath( progName,
				    params->debug,
				    params->input_dir,
				    startTime,
				    endTime );
     
   }
   else {
     
     POSTMSG( ERROR, "Must use one of the available modes in "
	      "the parameter file" );
     return( -1 );
   }
   
   bool uncompress = true;
   bool processedFile = false;
   time_t volumeTime = -1, curVolumeTime = -1;

   ReadGemtronik::VolumeEnum_t fileType;
   char *filePath;

   //
   // Loop over all the input files
   //

   while(filePath = fileTrigger->next()) {

     POSTMSG( DEBUG, "Processing file %s", filePath );
     uncompress = iscompressed( filePath);
     fileType = getFileType( filePath );
     volumeTime = getVolumeTime( filePath );
     if(volumeTime == -2) {
       POSTMSG( ERROR, "Error reading file: %s", filePath );
       readGemtronik->clearSweeps();
       delete fileTrigger;
       return (-2);
     }
     if(readGemtronik->readFile(filePath, volumeTime, uncompress, fileType) == 0) 
     {
       curVolumeTime = volumeTime;
       processedFile = true;
     } else 
       {
	 POSTMSG( ERROR, "Error reading file: %s", filePath );
	 readGemtronik->clearSweeps();
       }
   }

   delete fileTrigger;

   if(processedFile == false) {
     POSTMSG( ERROR, "No files processed within time specified" );
     return (-1);
   } else {
     readGemtronik->clearSweeps();
   }


   if( params->mode == Params::REALTIME ) {
     POSTMSG( ERROR, "Could not get next file" );
     return (-1);
   }

   POSTMSG( DEBUG, "End of file list" );

   return (0);
}

bool Gemtronik2Netcdf::iscompressed( char *fileName )
{
  bool compress = false;
   char *s = strrchr( fileName, '/' );
   if(!s)
     return compress;
   char *p = strchr( s, '.' );
   if(!p)
     return compress;
   
   if( (p[1] == 'v' && p[2] == 'o' && p[3] == 'l') || (p[1] == 'a' && p[2] == 'z' && p[3] == 'i') )
     if(p[4] == 'u')
       compress = false;
     else
       compress = true;
   return compress;
}

//
// Get Volume file type
// dBZ (), W (), or V () 
ReadGemtronik::VolumeEnum_t Gemtronik2Netcdf::getFileType( char *fileName )
{
   ReadGemtronik::VolumeEnum_t type = ReadGemtronik::VOLUMEUNKNOWN;

   char *s = strrchr( fileName, '/' );
   if(!s)
     return type;
   
   if(s[17] == 'W')
     type = ReadGemtronik::SPECTRUMWIDTH;
   if(s[17] == 'V')
     type = ReadGemtronik::VELOCITY;
   if(s[17] == 'd')
     type = ReadGemtronik::REFLECTIVITY;

   return type;
}

struct conversion_failure { };

template <typename T>
T from_string (const std::string & s)
{
  T result;
  std::istringstream stream (s);
  if (stream >> result) return result;
  throw conversion_failure ();
}

time_t Gemtronik2Netcdf::getVolumeTime( char *fileName )
{
  date_time_t timeStruct;
  size_t lastSlash, secondLastSlash;
  string cpyFileName(fileName);


  lastSlash = cpyFileName.find_last_of('/') +1;

  if(lastSlash == string::npos) {
    POSTMSG( ERROR, "Unable to determine volume time from file name" );
    return( -2 );
  }


  string yearS = cpyFileName.substr(lastSlash, 4);
  string monthS = cpyFileName.substr(lastSlash+4, 2);
  string dayS = cpyFileName.substr(lastSlash+6, 2);
  string hourS = cpyFileName.substr(lastSlash+8, 2);
  string minS = cpyFileName.substr(lastSlash+10, 2);
  string secS = cpyFileName.substr(lastSlash+12, 2);

  try {
    timeStruct.year = from_string<int>(yearS);
    timeStruct.month = from_string<int>(monthS);
    timeStruct.day = from_string<int>(dayS);
    timeStruct.hour = from_string<int>(hourS);
    timeStruct.min = from_string<int>(minS);
    timeStruct.sec = from_string<int>(secS);
  }
  catch( conversion_failure E ) {
    POSTMSG( ERROR, "Unable to determine volume time from file name" );
    return( -2 );
  }
  
  time_t rtime = uunix_time(&timeStruct);
  /* These check for unix time conversion error but are not neccessary 
  if(rtime < 1190000000) {
    POSTMSG( ERROR, "Small unix time %d detected, %d, %d, %d, %d, %d, %d, file %s, chars: '%s' '%s' '%s' '%s' '%s' '%s'",
	     rtime, timeStruct.year, timeStruct.month, timeStruct.day, timeStruct.hour, timeStruct.min, timeStruct.sec, fileName,
	     yearS.c_str(), monthS.c_str(), dayS.c_str(), hourS.c_str(), minS.c_str(), secS.c_str());
  }
  if(rtime > 1200000000) {
    POSTMSG( ERROR, "Large unix time %d detected, %d, %d, %d, %d, %d, %d, file %s, chars: '%s' '%s' '%s' '%s' '%s' '%s'",
	     rtime, timeStruct.year, timeStruct.month, timeStruct.day, timeStruct.hour, timeStruct.min, timeStruct.sec, fileName,
	     yearS.c_str(), monthS.c_str(), dayS.c_str(), hourS.c_str(), minS.c_str(), secS.c_str());
  }
  */
  return rtime;
}
