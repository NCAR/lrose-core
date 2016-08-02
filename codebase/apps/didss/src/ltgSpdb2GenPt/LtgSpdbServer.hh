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
#ifndef SPDBSERVER_HH
#define SPDBSERVER_HH
 
#include <string>
#include <vector>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Spdb_typedefs.hh>
#include "Params.hh"
#include <rapformats/GenPt.hh>
using namespace std;

#define SUCCESS 0
#define FAILURE -1

class Params;

class LtgSpdbServer {

public:
  
  //
  // Constructor/ Destructor
  // 
  LtgSpdbServer( Params &parameters);
  ~LtgSpdbServer(){};
  
  // 
  // Read ltg data, return success or failure based on read.
  // data retrieval mode, start time, end times determined from
  // params.
  //
  int readData(); 

  
  //
  // Read ltg data in interval [timeBegin, timeEnd].
  //
  int readData(time_t timeBegin, time_t timeEnd);
  
   //
  // write data.
  //
  int writeGenPt( GenPt &genPt);

  //
  // get data functions
  //
  int                getNChunks()   { return spdb.getNChunks(); } 
  
  Spdb::chunk_ref_t *getChunkRefs() { return spdb.getChunkRefs(); } 
  
  vector<Spdb::chunk_t> getChunks() { return spdb.getChunks(); }  
  
private:
  
  DsSpdb spdb;
  Params params;
  int    dataType; 
  
};

# endif

