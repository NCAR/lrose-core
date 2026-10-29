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
////////////////////////////////////////////////////
// spdb buffer
//
////////////////////////////////////////////////////

#include <toolsa/MemBuf.hh>
#include <symprod/spdb.h>
#include <string>
#include <vector>
using namespace std;

//
// Forward class declarations
//
class Metar;

class SpdbBuffer  {
 public:
   
   SpdbBuffer(int spdb_id, const char *spdb_label,
	      int expSecs, bool use_urls);

   ~SpdbBuffer();

   void clearDests();
   void addDest(const char *destination);

   void reset( );
   void addDecodedMetar( Metar& metar );
   void addRawMetar( Metar& metar );
   void send();
   
 private:

   int spdbId;
   string spdbLabel;
   
   vector<string> destinations;
   int expireSecs;

   MemBuf           chunkHdrBuf;
   MemBuf           chunkBuf;
   int              nChunks;
   bool             useUrls;
};

// spdb_chunk_ref_t *chunkHdrs;
