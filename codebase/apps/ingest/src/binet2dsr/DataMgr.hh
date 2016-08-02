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
//  Working class for application
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  September 2001
//
//  $Id: DataMgr.hh,v 1.6 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _BINET2DSR_DATAMGR_INC_
#define _BINET2DSR_DATAMGR_INC_

#include <Fmq/DsRadarQueue.hh>
#include "BinetNetCDF.hh"
using namespace std;

//
// Forward class declarations
//
class Params;
class DsInputPath;


class DataMgr
{
public:
   DataMgr();
  ~DataMgr(){};

   //
   // Processing steps invoked by application Driver
   // Return 0 upon success, -1 upon failure
   //
   int   init( Params& params, DsInputPath* trigger );
   int   processData();

private:

   //
   // Radar input stream
   //
   BinetNetCDF   inputStream;

   //
   // Output to radar queue
   //
   DsRadarQueue   outputRadarQueue;
   int            paramCount;

   int            writeRadarMsg();

   //
   // Diagnostics
   //
   size_t         summaryInterval;
   size_t         summaryCount;

   void           printSummary();

   static const size_t RADAR_PARAM_INTERVAL;
   static const char*  HDR;
   static const char*  FMT;
};

#endif
