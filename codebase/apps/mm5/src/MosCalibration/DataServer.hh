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
///////////////////////////////////////////////////////////////
// 
// Reads data from an spdb server and creates a list of entries 
//
// $Id: DataServer.hh,v 1.14 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////
# ifndef _DATA_SERVER_INC_
# define _DATA_SERVER_INC_

#include <string>
#include <map>
#include <Spdb/DsSpdb.hh>
using namespace std;

class DataServer {
 public:

   DataServer(){};
   ~DataServer(){};

   void init( char* url, time_t start = 0, time_t end = 0,
              bool respectTypes = false );

   int readExact( time_t dataTime, char* stationId, int leadTime = 0 );
   int readData( char* stationId, int leadTime = 0 );

   int                   getNChunks() { return spdbMgr.getNChunks(); }  
   Spdb::chunk_ref_t    *getChunkRefs() { return spdbMgr.getChunkRefs(); }
   vector<Spdb::chunk_t> getChunks() { return spdbMgr.getChunks(); }

   const string& getUrl() const { return urlStr; }

   const time_t getStartTime() const { return startTime; }

   const time_t getEndTime() const { return endTime; }

 private:

   bool    respectDataTypes;
   string  urlStr;
   time_t  startTime;
   time_t  endTime;
   
   DsSpdb  spdbMgr;

};

# endif
