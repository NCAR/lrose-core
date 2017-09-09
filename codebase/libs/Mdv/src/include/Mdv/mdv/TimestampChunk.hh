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

#ifndef _TIME_STAMP_INC_
#define _TIME_STAMP_INC_

#include <string>
#include <cstdio>
#include <Mdv/mdv/mdv_file.h>
#include <toolsa/DateTime.hh>
using namespace std;

//
// Forward class declarations
//
class MdvChunk;


class TimestampChunk
{
public:
   TimestampChunk();
   TimestampChunk( string &name, time_t when );
   TimestampChunk( const char *name, time_t when );
   TimestampChunk( void *chunkData, si32 chunkSize );
  ~TimestampChunk();

   void setName( const string &newName );
   void setTime( time_t newTime );
   void setPostMark( const string &newName, time_t newTime );

   const string& getName(){ return name; }
   time_t  getTime(){ return when.utime(); }

   //
   // Mdv chunk support
   //
   MdvChunk& getChunk(){ return *chunk; }
   void print( FILE *outfile );

private:
   string    name;
   DateTime  when;

   //
   // byte-swapped structure for writing to the chunk
   //
   MdvChunk *chunk;
   struct{
      si32   when;
      char   who[MDV_LONG_FIELD_LEN];
   } chunkData;

   void      init();

};

#endif
