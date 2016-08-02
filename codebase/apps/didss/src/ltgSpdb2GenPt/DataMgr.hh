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
////////////////////////////////////////////////////////////////////////////////
//
//  Server, ingest, and mdv/grid data management
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC
#define _DATAMGR_INC

#include <map>
#include <euclid/TypeGrid.hh>

#include <rapformats/GenPtArray.hh>
#include "LtgSpdbServer.hh"
#include "Params.hh"
using namespace std;


//
// Forward class declarations
//
class Params;

class DataMgr
{
public:
  
  DataMgr();
  
  ~DataMgr();
  
  int init( Params &parameters );
  
  int processData();

  int writeGenPtSpdb();

private:
  
  LtgSpdbServer           *ltgSpdbServer;

  int                     nChunks;

  vector<Spdb::chunk_t>   chunks;

  Params                  params;

  Params::run_mode_t      runMode;

  int                     sleepTime;
};

#endif

