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
///////////////////////////////////////////////////////
// Ingester - class that manages data ingest and 
//            writing data to output
//
// Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
// September 2004
//
// $Id: Ingester.hh,v 1.9 2016/03/07 01:23:03 dixon Exp $
//
//////////////////////////////////////////////////////
#ifndef _INGESTER_INC_
#define _INGESTER_INC_

#include <netcdf.hh>

#include "Nexrad2Netcdf.hh"
#include "NcOutput.hh"

//
// Forward class declarations
//
class Params;
class SweepData;

class Ingester 
{
public:
  
  //
  // Constructor
  //
  Ingester(Params *P);
  
  //
  // Destructor
  //
  ~Ingester();
  
  //
  // Add a beam for processing
  // Overloaded to handle msg31 format and msg1 format
  //   volumeTitleSeen = true if we have seen a start of volume
  //                     indicator
  status_t addMsg1Beam(RIDDS_data_hdr* nexradData, bool volumeTitleSeen);
  status_t addMsg31Beam(RIDDS_data_31_hdr *header, bool volumeTitleSeen);

  void addAdaptation(RIDDS_adaptation_data *adapt_data);
  void addVCP(VCP_data_t *vcp_data);
  RIDDS_elevation_angle *getElevationAngle();
  void addStatus(RIDDS_status_data *status_data);
  RIDDS_status_data *getStatus();


  void addBypassMap(BypassMap_t *bypass_Map);
  void deleteBypassMap(BypassMap_t *map);
  BypassSegment_t *getBypassSegment();

  void addClutterMap(ClutterMap_t *clutter_Map);
  void deleteClutterMap(ClutterMap_t *map);
  ClutterSegment_t *getClutterSegment();

  // 
  // Indicate that we are at the end of the data stream.
  // Write out any remaining data
  //
  status_t endOfData();

private:

  void _setFirstBeamInfo(time_t beamTime, int msec_past_midnight, int elev_num, float elevation);
  void _setFirstBeamInfo(time_t beamTime, int msec_past_midnight, int elev_num, float elevation,
			 char *radarName, int radarHeight, float radarLat, float radarLon);
  void _setNewElevationInfo(time_t beamTime, int msec_past_midnight, int elev_num,
			    float elevation, bool volumeTitleSeen, int radial_status);

  Params *params;

  //
  // Keeps track of whether this is the first beam 
  // or not
  //
  bool firstBeam;
  
  //
  // Keep track of how long it takes to create a file
  //
  time_t fileStart;
  time_t fileEnd;
  
  //
  // Sweeps
  //
  int        curElevNumber;
  int        prevElevNumber;
  float      curElevation;
  float      prevElevation;
  SweepData *currentSweep;
  SweepData *prevSweep;

  RIDDS_adaptation_data *adaptData;
  RIDDS_adaptation_data *newAdaptData;
  BypassMap_t *bypassMap;
  BypassMap_t *newBypassMap;
  ClutterMap_t *clutterMap;
  ClutterMap_t *newClutterMap;
  VCP_data_t *vcpData;
  VCP_data_t *newVcpData;
  RIDDS_status_data *statusData;
  RIDDS_status_data *newStatusData;

  //
  // NetCDF output
  //
  NcOutput  *ncOutput;

   
};

#endif
   
   
