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
//  Working class for converting a nexrad data buffer to Dsr format
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: Reformat.hh,v 1.11 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _REFORMAT_INC_
#define _REFORMAT_INC_

#include <rapformats/DsRadarMsg.hh>

#include "FieldTable.hh"
#include "ScanTable.hh"
#include "KmTilt.hh"
#include "Params.hh"
#include "Status.hh"
using namespace std;


class Reformat
{
public:
   Reformat();
  ~Reformat(){};

   //
   // Return 0 upon success, -1 upon failure
   //
   int init( Params& params );

   //
   // Reformat a nexrad data buffer to a Dsr message
   // This is where all the real work happens
   //
   Status::info_t  nexrad2dsr( ui08* nexradBuffer,
                               DsRadarMsg& dsrMsg,
                               bool volumeTitleSeen );

private:

   //
   // Steps in the nexrad2dsr conversion/reformatting process
   //
   void            setDataInStream( RIDDS_data_hdr* nexradData );

   void            setRadarParams( RIDDS_data_hdr* nexradData, 
                                   DsRadarMsg& dsrMsg );

   void            setRadarBeam( RIDDS_data_hdr* nexradData, 
                                 DsRadarMsg& dsrMsg );

   void            setRadarFlags( RIDDS_data_hdr* nexradData,
                                  DsRadarMsg& dsrMsg );

   void            setDataStored( RIDDS_data_hdr* nexradData, 
                                  DsRadarMsg& dsrMsg );

   void            setFieldParams( RIDDS_data_hdr* nexradData,
                                   DsRadarMsg& dsrMsg );

   void            setBeamData( RIDDS_data_hdr* nexradData,
                                DsRadarMsg& dsrMsg );

   //
   // Depending on the data availability with each message conversion,
   // we have to handle things a bit differently.
   //
   void            convertData( RIDDS_data_hdr* nexradData, 
                                DsRadarMsg& dsrMsg );

   void            convertDbz( RIDDS_data_hdr* nexradData, 
                               DsRadarMsg& dsrMsg );

   void            convertVel( RIDDS_data_hdr* nexradData, 
                               DsRadarMsg& dsrMsg );

   void            convertAll( RIDDS_data_hdr* nexradData );

   void            correctData( ui08* nexradData );
   void            interlaceData( ui08* nexradData, size_t fieldIndex );
   void            replicateData( ui08* kmData, size_t fieldIndex );

   //
   // Reformatting the data makes use of an intermediate data buffer
   //    nexradBuffer -> reformattedData -> dsrMsg.radarBeam
   //
   // Rather than doing a lot of new/delete/malloc/realloc/free/vector::insert
   // based on changing number of gates in the beam data, let's try the 
   // less-cpu-intensive-more-memory-intensive approach of declaring
   // the data buffers to be of maximum needed sizes for now.  
   // This approach can be revisited.
   //
   ui08            reformattedData[NEX_MAX_GATES*4];
   size_t          reformattedDataLen;

   //
   // Save a tilt's worth of data at 1000m (km) resolution for
   // synthisyzing with the subsequent tilt ofhigher resolution (250m) data
   //
   KmTilt          kmTilt;

   //
   // Field and scan tables for accessing information about
   // NEXRAD field data characteristics, e.g., scale & bias 
   // and VCP scan strategies
   //
   FieldTable      fieldTable;
   ScanTable       scanTable;

   //
   // Option for deriving signal to noise
   //
   bool            firstCall;
   bool            overrideInputTiltNumbers;
   double          noiseAt100km;
   short           noise[NEX_MAX_GATES];
   ui08            snrData[NEX_MAX_GATES];

   void            initSnrLut( double gateWidth );
   void            deriveSnr( ui08* dbzData );

   //
   // Misc. other stuff
   //
   int             prevScanType;
   int             prevTiltNum;
   int             volumeNum;
   int             numGates;
   int             numFields;
   time_t          referenceTime;
   time_t          offsetDataTime;
   bool            overrideDataTime;
   bool            volumeTitleSeen;
   int             refNumGates;
   int             sequentialTiltNumber;
   float           specifiedGateSpacing;

   //
   // Constants
   //
   static const size_t GATE_RATIO;
   static const double SPEED_OF_LIGHT;
};

#endif
