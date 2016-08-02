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
// DataMgr - Data Manager
//
// $Id: DataMgr.hh,v 1.11 2016/03/06 23:53:41 dixon Exp $
//
///////////////////////////////////////////////////////////////
#ifndef _Mdv2Dsr_DATAMGR_HH
#define _Mdv2Dsr_DATAMGR_HH

#include <string>
#include <vector>
#include <Fmq/DsRadarQueue.hh>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class MsgLog;
class Ingester;
class DsInputPath;

class DataMgr {
  
 public:
   DataMgr();
   ~DataMgr();

   int init( Params& params, MsgLog* msgLog,
             vector< string > &fileList );
   int processData();
   void resetInputDataQueue();
  
private:

   DsRadarQueue                      radarQueue;
   DsInputPath                      *inputPath;
   Ingester                         *ingester;

   bool                              useSimulatedTimes;
   vector< pair< time_t, time_t >* > simulatedTimes;

  time_t _stringToTime(const char *timeStr);

};

#endif
