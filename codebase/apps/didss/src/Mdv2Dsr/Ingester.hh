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
// Ingester
//
// $Id: Ingester.hh,v 1.16 2016/03/06 23:53:41 dixon Exp $
//
///////////////////////////////////////////////////////////////
#ifndef _INGESTER_
#define _INGESTER_

#include <string>
#include <vector>
#include <dataport/port_types.h>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxRadar.hh>
#include <rapformats/DsRadarMsg.hh>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class DsRadarQueue;
class DsRadarParams;
class DsFieldParams;

class Ingester {
  
 public:
   Ingester( DsRadarQueue& queue, Params& params );
   ~Ingester();

   int processFile( char* fileName, int startT = -1, int endT = -1 );
  
private:

   const Params &_params;

   bool                      useCurrent;
   bool                      useFieldHeaders;
   int                       volumeNum;
   DsRadarQueue             *radarQueue;
   DsRadarMsg                radarMsg;
   DsRadarParams            &radarParams;
   DsRadarCalib             &radarCalib;
   vector< DsFieldParams* > &fieldParams;
  DsRadarBeam              &radarBeam;
   int                      nElevations;
   fl32                     *elevations;
   void                      clearFields();

   Mdvx                      mdvx;
   MdvxRadar                 mdvxRadar;
   int                       readPlanes( char* fileName );

   //
   // Note that this class does not own the memory
   // associated with the radar queue
   //
   int                       beamWait;
   int                       volumeWait;

   bool                      summaryOn;
   int                       summaryInterval;
   int                       summaryCount;
   void                      printSummary();

   //
   // Defaults
   //
   Ingester();
   Ingester( Ingester& source );
   Ingester& operator=( Ingester& source );

};

#endif
