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
//  Class for Dsr output stream
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: DsrOutput.hh,v 1.11 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DSR_OUTPUT_INC_
#define _DSR_OUTPUT_INC_

#include <Fmq/DsRadarQueue.hh>

#include "Status.hh"
#include "Params.hh"
using namespace std;


class DsrOutput
{
public:
   DsrOutput();
  ~DsrOutput();

   //
   // Processing steps invoked by application Driver
   // Return 0 upon success, -1 upon failure
   //
   int   init( Params& params );

   //
   // Output to radar queue with I/O status return
   //
   Status::info_t  writeDsrMsg( DsRadarMsg& radarMsg, 
                                bool flagsChanged, bool paramsChanged );

private:

   //
   // Output to radar queue
   //
   DsRadarQueue   outputRadarQueue;
   int            paramCount;
   int            waitMsec;
   bool           filter1KmDbz;
   bool           separateFlags;
   bool           oneFilePerVolume;
   bool           paramsOnFilteredTilt;

   bool           firstRotationOnly;
   double         azTol;
   int            numBeamsPostRotation;

   double         currentElevation;
   bool           movedAlittle;
   double         startAzimuth;
   bool           firstRotationDone;
   int            icount;
   int            currentNumGates;
   double         currentGateSpacing;
   //
   // Diagnostics
   //
   size_t         summaryInterval;
   size_t         summaryCount;

   void           printSummary( DsRadarMsg& radarMsg );

   static const size_t RADAR_PARAM_INTERVAL;
   static const char*  HDR;
   static const char*  FMT;
};

#endif
