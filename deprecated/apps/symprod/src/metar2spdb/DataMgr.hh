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
////////////////////////////////////////////////
// Data Manager
//
////////////////////////////////////////////////

#include <vector>
#include <didss/LdataInfo.hh>
#include <toolsa/utim.h>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class Metar;
class Ingester;
class SpdbBuffer;

class DataMgr {
 public:
   
   DataMgr(const Params &params);
   ~DataMgr();
   
  int init();
  int processFiles(); // process multiple files
  int processFile(char *fileName,
		  time_t fileTime = -1,
		  int ExtendFilename = 1); // process one file

 private:

   const Params &_params;
   char       latestFileName[256];
   char       *inputDir;
   int        maxRealtimeValidAge;
   int        nDestinations;
   time_t     latestFileTime;
   time_t     currentFileTime;
   UTIMstruct currentTime;
   tdrp_bool_t store_raw_metars;

   
   vector< Metar* > metars;

   Ingester         *ingester;
   SpdbBuffer       *decodedSpdbBuf;
   SpdbBuffer       *rawSpdbBuf;
   LdataInfo        *tstamp;

   void clearMetars();
   int  writeIndex();
   void setCurrentTime(int year, int month, int day, int hour, int min);
   void updateLatestTime();
   
};





