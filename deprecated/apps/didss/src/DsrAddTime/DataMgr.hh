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
// $Id: DataMgr.hh,v 1.10 2016/03/06 23:53:40 dixon Exp $
//
///////////////////////////////////////////////////////////////
#ifndef _DATAMGR_
#define _DATAMGR_

#include <string>
#include <vector>
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarMsg.hh>
#include "Params.hh"
using namespace std;

class DataMgr {
  
 public:
   DataMgr();
   ~DataMgr();

   int  init( Params& params, MsgLog& msgLog );
   void processData();

   //
   // Constants
   //
   static const string TIME_FIELD_UNITS;
   static const int    MISSING_TIME_VAL;
   static const float  TIME_BIAS;
  
private:

   //
   // Input
   //
   DsRadarQueue                    inputQueue;
   DsRadarMsg                      inputMsg;
   
   //
   // Output
   //
   bool                      paramsSet;
   bool                      fieldsSet;
   DsRadarQueue              outputQueue;
   DsRadarMsg                outputMsg;
   DsRadarParams            &outputParams;
   vector< DsFieldParams* > &outputFields;
   DsRadarBeam              &outputBeam;
   DsRadarFlags             &outputFlags;
   
   //
   // Time data
   //
   bool                      setReferenceTime;
   time_t                    referenceTime;
   string                    timeFieldName;
   float                     timeScale;
   bool                      addTime( int contents );

   //
   // Printing the summary
   //
   bool                      writeSummary;
   int                       summaryInterval;
   void                      printSummary();

};

#endif

