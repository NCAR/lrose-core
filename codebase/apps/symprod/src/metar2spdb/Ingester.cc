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
/////////////////////////////////////////////////////////////////
// Ingester
//
//////////////////////////////////////////////////////////////////

#include <string>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/MsgLog.hh>

#include "Ingester.hh"
#include "Metar.hh"
#include "MetarLocation.hh"
#include "Metar2Spdb.hh"
using namespace std;

Ingester::Ingester( char *inputDir, const Params &params ) : _params(params)
{
   fileYear            = 1970;
   fileMonth           = 1;
   fileDay             = 1;
   fileHour            = 0;
   fileMin             = 0;
   inputPath       = STRdup(inputDir);
}

Ingester::~Ingester()
{

   if( inputPath )
      STRfree(inputPath);

   map< string, MetarLocation*, less<string> >::iterator item;
   
   for(item = locations.begin(); item != locations.end(); item++) {
      delete( (*item).second );
   }

   locations.erase(locations.begin(), locations.end());
}

int
Ingester::setTime(char* fileName)
{

   char *fileDate;

   //
   // Find year, month, day, hour, and min (if avail) from the file name
   //
   fileDate = strstr(fileName, _params.file_prefix);
   fileDate += strlen(_params.file_prefix);

   //
   // First look for the longer file format, with minutes: YYYYMMDDHHMM
   //
   if( sscanf(fileDate, ".%4d%2d%2d%2d%2d", 
                        &fileYear, &fileMonth, &fileDay, &fileHour, &fileMin) != 5 ) {
      //
      // Try again, look for the shorter file format, without min: YYYYMMDDHH
      //
      if( sscanf(fileDate, ".%4d%2d%2d%2d", 
                           &fileYear, &fileMonth, &fileDay, &fileHour) != 4 ) {
         POSTMSG( ERROR, "Cannot parse file name %s", fileName );
         return(FAILURE);
      } 

      //
      // Clear out minutes
      //
      fileMin = 0;
   }

   // clear out secs
   fileSec = 0;

   return (SUCCESS);

}

void
Ingester::setTime(time_t fileTime)

{

  date_time_t dataTime;
  dataTime.unix_time = fileTime;
  uconvert_from_utime(&dataTime);

  fileYear = dataTime.year;
  fileMonth = dataTime.month;
  fileDay = dataTime.day;
  fileHour = dataTime.hour;
  fileMin = dataTime.min;
  fileSec = dataTime.sec;

}


int Ingester::processFile(char* fileName, 
			  vector< Metar* >& metars,
			  int ExtendFilename /* = 1*/ )

{
   FILE *fp;
   char filePath[FILE_NAME_LENGTH];
   string metarMessage;
   char line[MAX_LINE];

   //
   // Get the full path name for the file - unless we are in SINGLE_FILE
   // mode, in which case ExtendFilename will be 0.
   //
   if (ExtendFilename)
     sprintf( filePath, "%s%s%s", inputPath, PATH_DELIM, fileName );
   else
     sprintf( filePath, "%s", fileName);
   //
   // Open the file
   //
   if((fp = ta_fopen_uncompress(filePath, "r")) == NULL) {
      POSTMSG( WARNING, "Couldn't open file %s", fileName );
      return(FAILURE);
   }
   
   //
   // Concatenate the lines in the file until we reach a blank line,
   // or until the line added contains an '='.
   // This means that we have reached the end of a metar message, so
   // create a metar object and store this information.  Then add this
   // metar object to our list of metars.
   //

   // we have to handle METAR's in two forms
   // (a) A lines starts with METAR and is followed by the data record
   // (b) A line with only the word METAR in it signifies the beginning of a 
   //     block of metars, which is terminated by a blank line or ^C.
   // We also have to deal with SPECIs, in the same way as METARs.

   metarMessage = "";
   bool inBlock = false;
   blockHour = -1;
   blockMin = -1;
   
   while( fgets(line, MAX_LINE, fp) != NULL ) {

     // if start of SA Block, start block metar
     
     if (startOfSABlock(line)) {
       metarMessage = "";
       inBlock = true;
       continue;
     }
     
     // if METAR or SPECI is present, start new metar
     
     char *startChar = line;
     
     if (strstr(line, "METAR") || strstr(line, "SPECI")) {

       metarMessage = "";
       inBlock = true;
       
       if (strlen(line) < 10) {
	 // only METAR or SPECI on line, must be start of block
	 continue;
       }
       
       //
       // Jump ahead of METAR or SPECI prefix, if it exists
       //
       
       if (strlen(line) > 7 && !strncmp(line, " METAR ", 7)) {
	 startChar = line + 7;
       } else if (strlen(line) > 6 && !strncmp(line, "METAR ", 6)) { 
	 startChar = line + 6;
       } else if (strlen(line) > 7 && !strncmp(line, " SPECI ", 7)) {
	 startChar = line + 7;
       } else if (strlen(line) > 6 && !strncmp(line, "SPECI ", 6)) { 
	 startChar = line + 6;
       } else {
	 startChar = line;
       }
       
     }

     // if we are not in a METAR block, continue
     
     if (!inBlock) {
       continue;
     }

     // add line to metar string if not blank
     
     if( !STRequal(startChar, "\n")) {
       metarMessage += startChar;
     }
     
     // check if complete
     
     bool complete = false;
     if (STRequal(startChar, "\n") || strchr(startChar, '=')) {
       complete = true;
     }

     // check for end of block - blank line or Ctrl-C
     
     if (strstr(startChar, "NNNN") || strchr(startChar, 0x03)) {
       inBlock = false;
       blockHour = -1;
       blockMin = -1;
     }
     
     if (complete) {
       storeMetar(fileName, metars, metarMessage);
       metarMessage = "";
     } 
     
   } // while (fgets ...
   
   fclose(fp);
   return(SUCCESS);
   
}

bool Ingester::startOfSABlock(char *line)
  
{

  // check for SA

  char *sa = strstr(line, "SA");
  if (sa == NULL) {
    return false;
  }

  if (strlen(line) > 128) {
    return false;
  }

  // check for "SA* stid ddhhmm"

  char sagroup[128];
  char stid[128];
  char timestr[128];

  if (sscanf(line, "%s %s %s", sagroup, stid, timestr) != 3) {
    return false;
  }

  if (strlen(stid) != 4) {
    return false;
  }

  int day, hour, min;
  if (sscanf(timestr, "%2d%2d%2d", &day, &hour, &min) != 3) {
    return false;
  }
  blockHour = hour;
  blockMin = min;

  return true;

}
  
int Ingester::storeMetar(char* fileName, 
			 vector< Metar* >& metars,
			 const string &metarMessage)
  
{
  
  //
  // Ignore blank messages - need at least 4 chars for station name
  //
  
  if ( metarMessage.size() < 4 ) {
    return -1;
  }
  
  // check if in the list
       
  string stationName(metarMessage, 0, 4);
  if (locations.find(stationName) == locations.end()) {
    return -1;
  }
    
  // in the list, so process
  
  Metar *currentMetar =
    new Metar(_params,
	      fileYear, fileMonth, fileDay, fileHour, fileMin,
	      blockHour, blockMin);
    
  POSTMSG( DEBUG, "Processing METAR message:" );
  POSTMSG( DEBUG, "  %s", metarMessage.c_str() );
  
  if( currentMetar->setMetar((char *) metarMessage.c_str(), 
			     fileName, 
			     locations) != SUCCESS ) {
    delete currentMetar;
    return -1;
  }

  metars.push_back(currentMetar);
  
  return 0;
      
}
 
	 
int
Ingester::setLocations(char* locationFileName)
{
   FILE *fp;
   char line[MAX_LINE];
   char station[MAX_LINE];
   double lat, lon, alt;

   string stationId;
   MetarLocation *metarLoc;
   
   if((fp = ta_fopen_uncompress(locationFileName, "r")) == NULL) {
      POSTMSG( ERROR, "Couldn't open file %s", locationFileName );
      return(FAILURE);
   }
   
   int count = 0;

   while( fgets(line, MAX_LINE, fp) != NULL ) {

      //
      // If the line is not a comment, process it
      //
      if( line[0] != '#' ) {

         //
         // Read in the line - try different formats
         //
	 if( sscanf(line, "%4s, %lg, %lg, %lg", 
		    station, &lat, &lon, &alt) != 4 ) {
	   if( sscanf(line, "%4s,%lg,%lg,%lg", 
		      station, &lat, &lon, &alt) != 4 ) {
	     if( sscanf(line, "%3s,%lg,%lg,%lg", 
			station, &lat, &lon, &alt) != 4 ) {
	       POSTMSG( WARNING, "In file %s", locationFileName );
	       POSTMSG( WARNING, "  Couldn't read line %s", line );
	       continue;
	     }
	   }
	 }

	 count++;

         //
         // Convert station to a string
         //
         stationId = station;

         //
         // Convert altitude to meters
         //
         if( alt == MISSING_ALT ) {
	    alt = STATION_NAN;
	 } else {
	    alt *= FT_TO_M;
	 }

         //
         // Create new metar location and add it to the list
         //
	 metarLoc = new MetarLocation(stationId, lat, lon, alt);
	 locations[stationId] = metarLoc;
      }
   }

   if (count == 0) {
     POSTMSG( ERROR, "No suitable entries in file %s", locationFileName );
     return(FAILURE);
   }


   fclose(fp);
   return(SUCCESS);
	 
}


