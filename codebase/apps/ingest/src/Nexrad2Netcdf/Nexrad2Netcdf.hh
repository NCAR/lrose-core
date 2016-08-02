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
/////////////////////////////////////////////////////////////
//  Nexrad2Netcdf application
//
//  Refactored July 2007, Jason Craig
//
//  Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
//  July 2004
//
//  Adapted from nexrad2dsr application by Terri Betancourt 
//  RAP, NCAR, Boulder, CO, 80307, USA
//
//  $Id: Nexrad2Netcdf.hh,v 1.7 2016/03/07 01:23:03 dixon Exp $
//
/////////////////////////////////////////////////////////////
#ifndef _NEXRAD2NETCDF_HH_
#define _NEXRAD2NETCDF_HH_

#include <string>
#include <vector>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>
#include <rapformats/ridds.h>

#include "Params.hh"
#include "NexradInput.hh"

using namespace std;

class ReadNexrad;
//
// Generic processing status id's
//
enum status_t { FAILURE = -1,
		ALL_OK,
		END_OF_FILE, 
		END_OF_DATA, 
		BAD_DATA,
		BAD_INPUT_STREAM,
		BAD_OUTPUT_STREAM,
		SKIP_DATA,
		NO_FILES
};

extern MsgLog msgLog;

// 
// Macros for message logging 
// 
#define POSTMSG          msgLog.postMsg
#define DEBUG_ENABLED    msgLog.isEnabled( DEBUG )
#define INFO_ENABLED     msgLog.isEnabled( INFO )


//
// Stuct defs for internal storage of clutter and bypass maps
//
typedef struct  {
  ui16 op_code;
  ui16 end_range;
} ClutterRange_t;

typedef struct {
  ClutterRange_t ranges[360*20]; // 360 = azimuth segments
} ClutterSegment_t;              // 20 = possible ranges, end_range of 511 is always last

typedef struct {
  ui16 julian_date;              // Days since jan 1, 1970
  ui16 minutes_past_midnight;    // Minutes since midnight on julian_date
  ui16 num_message_segs;         // Number of segments

  ClutterSegment_t *segment[5];  // Up to 5 possible segments
} ClutterMap_t;

typedef struct {
  ui16 ranges[360*32];           // 360 = azimuth segments
} BypassSegment_t;               // 32 = range bins

typedef struct {
  ui16 julian_date;              // Days since jan 1, 1970
  ui16 minutes_past_midnight;    // Minutes since midnight on julian_date
  ui16 num_message_segs;         // Number of segments

  BypassSegment_t *segment[5];   // Up to 5 possible segments
} BypassMap_t;

typedef struct {
  RIDDS_VCP_hdr hdr;
  RIDDS_elevation_angle angle[25];
} VCP_data_t;

class Nexrad2Netcdf
{
public:
   
   //
   // Constructor
   //
  Nexrad2Netcdf(Params *P, const char *programeName);

   //
   // Destructor
   //
  ~Nexrad2Netcdf();

   //
   // Initialization
   //
   static const string version;

   const string& getVersion(){ return version; }

   //
   // Execution
   //
   int run(vector<string> inputFileList, time_t startTime, time_t endTime);

  typedef enum {
    VOLUMESTART,
    VOLUMEINTERMEDIATE,
    VOLUMEEND,
    VOLUMEUNKNOWN
  } VolumeEnum_t;

private:

  int processFile( char *filePath, bool uncompress);

  int getSequenceNum( char *fileName );
  int getVersionNum( char *fileName );
  time_t getVolumeTime( char *fileName );
  VolumeEnum_t getFileType( char *fileName );
  char *replaceSequenceNum( char *fileName, int newNumber, bool setEnd, int sequencingOffset );



  Params            *params;
  
  NexradInput        nexradInput;
  ReadNexrad        *readNexrad;

  const char *progName;

};

#endif
