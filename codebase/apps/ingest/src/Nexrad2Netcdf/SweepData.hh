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
//////////////////////////////////////////////////////////
// SweepData - class that contains and manages data
//            specific to a single tilt or sweep
//
// $Id: SweepData.hh,v 1.18 2018/04/26 21:37:48 jcraig Exp $
//
/////////////////////////////////////////////////////////
#ifndef _SWEEP_DATA_
#define _SWEEP_DATA_

#include <vector>
#include <deque>

#include <rapformats/ridds.h>
#include "Nexrad2Netcdf.hh"
#include "Params.hh"

//
// Forward class declarations
//
class Beam;

class SweepData 
{
public:

   enum scan_t { NONE, REFL_ONLY, VEL_ONLY, BOTH };

   //
   // Constructor
   //   tdrpParams = reference to tdrp parameters
   //
   SweepData( Params *tdrpParams );

   //
   // Destructor
   //
   ~SweepData();

   //
   // Clear out data values that will change with the
   // next sweep
   //
   void clear();



  //
  // Set functions
  //

  void setClutterSegment(ClutterSegment_t *cS) { clutterSegment = cS; };
  void setBypassSegment(BypassSegment_t *bS) { bypassSegment = bS; };
  void setVcpInfo(RIDDS_VCP_hdr *hdr, RIDDS_elevation_angle *eA) {vcpHdr = hdr; vcpElev = eA; };
  void setStatusData(RIDDS_status_data *sD) { statusData = sD; };

   //
   // Set info related to this sweep
   //   nexradData = ridds data header
   //   pSweep     = previous sweep 
   //                Note that this parameter is not
   //                necessary
   //
   //   Certain data values need to be set at the
   //   beginning of each sweep.  Set these values
   //   
   //   See Status.hh for meaning of return value
   //
  status_t setInfo(RIDDS_data_hdr* nexradData, SweepData* pSweep = NULL );
  status_t setInfo(RIDDS_data_31_hdr* nexradData, SweepData* pSweep = NULL );

   //
   // Copy the data from the current message into the 
   // sweep
   //   nexradData = ridds data header and byte data
   //                that follows
   //   beamTime   = time associated with current message
   //
   //   See Status.hh for meaning of return value
   //
   status_t copyData(RIDDS_data_hdr* nexradData, time_t beamTime );
   status_t copyData(RIDDS_data_31_hdr* nexradData, time_t beamTime );

   //
   // Do we have at least one beam with all the expected fields
   // present? 
   //
   bool allFieldsPresent();

   //
   // Was there a sweep lost because we couldn't merge?
   //
   bool sweepLost();

   //
   // Was this tilt merged with another?
   //
   bool merged(){ return mergeDone; }

   //
   // Are we skipping this sweep?
   //
   bool sweepSkipped(){ return skipSweep; }

   //
   // Return the beam that is closest to the beam
   // in the velocity data with the given azimuth, elevation
   // and time
   //
   //   currentAz   = azimuth of the beam to which we need to find the 
   //                 closest in the given sweep
   //   currentElev = elevation of the beam to which we need to find the
   //                 closest in the given sweep
   //   currentTime = time of the beam to which we need to find the 
   //                 closest in the given sweep
   //
   //   Note that this function should only be used when scanType
   //   is REFL_ONLY
   //
   //   returns a pointer to the closest beam on success and NULL
   //   on failure
   //
   Beam* getClosestBeam( float currentAz, float currentElev, 
                         double currentTime );

   //
   // If there is no overlap in the beam azimuths in this
   // sweep, fill in the missing rec values around the first
   // and last beams by wrapping around that line if appropriate
   //
   void fillRec();

  void calculatePulseCount();

  //
  // Get functions
  //
  float          getSurPrf();
  float          getSurPulseCount();
  vector<Beam*>& getBeams(){ return beams; }
  scan_t         getScanType(){ return scanType; }

  //int            getNumGates(){ return numGates; }
  int            getVCP(){ return vcp; }
  int            getElevIndex(){ return elevIndex; }
  
  float          getFixedAngle(){ return fixedAngle; }
  float          getVelScaleBiasFactor(){ return velScaleBiasFactor; }
  float          getNyquistVelocity();
  float          getUnambiguousRange();
  float          getPrf();
  float          getDbz0() { return dbz0; };
  float          getHorizNoise();
  float          getVertNoise();
  float          getPulseCount() { return pulseCount; };

  bool           getCalcSnr(){ return calcSnr; }
  bool           getCalcPr(){ return calcPr; }
  bool           getCalcRec(){ return calcRec; }

  int            getNBeams(){ return beams.size(); }
  int            getNVelGates();
  int            getNReflGates();
  float          getCellSpacingVel();
  float          getCellSpacingRefl();
  float          getRangeToFirstVelGate();
  float          getRangeToFirstReflGate();
  float          getDbzScale();
  float          getDbzBias();
  short          getDbzBad();
  float          getVelScale();
  float          getVelBias();
  short          getVelBad();
  float          getSwScale();
  float          getSwBias();
  short          getSwBad();
  float          getZdrScale();
  float          getZdrBias();
  short          getZdrBad();
  float          getPhiScale();
  float          getPhiBias();
  short          getPhiBad();
  float          getRhoScale();
  float          getRhoBias();
  short          getRhoBad();
  float          getSnrScale();
  float          getSnrBias();
  short          getSnrBad();
  float          getPrScale();
  float          getPrBias();
  short          getPrBad();
  float          getRecScale();
  float          getRecBias();
  short          getRecBad();

  ClutterSegment_t *getClutterSegment() { return clutterSegment; };
  BypassSegment_t  *getBypassSegment() { return bypassSegment; };
  RIDDS_VCP_hdr    *getVcpHdr() { return vcpHdr; };
  RIDDS_elevation_angle *getVcpElev() { return vcpElev; };
  RIDDS_status_data  *getStatusData() { return statusData; };

  //
  // Constants
  //  MISSING_INDEX   = missing values for azimuth indeces
  //  BEAM_WIDTH      = assumed beam width in degrees
  //  DELTA_AZIMUTH   = used to define data structure used
  //                    when combining scans
  //  HDR             = header for summary print out
  //  FMT             = format for summary print out
  //  SPEED_OF_LIGHT  = speed of light in m/s
  //
  static const int     MISSING_INDEX;
  static const float   BEAM_WIDTH;
  static const double  DELTA_AZIMUTH;
  static const char   *HDR;
  static const char   *FMT;
  static const double SPEED_OF_LIGHT;

private:

  void _copyData(Beam *newBeam);

  //
  // Reference to tdrp parameters
  //
  Params *params;
  
  //
  // Was this tilt merged with another?
  //
  bool mergeDone;
  
  //
  // Do we want to skip this sweep?
  //
  bool skipSweep;
  
  //
  // Count on how long it has been since we last printed
  // a summary
  //
  int summaryCount;
  
  //
  // Does this sweep have reflectivity data only,
  // velocity and spectrum width data only or 
  // both reflectivity and velocity
  //
  scan_t scanType;
  
  //
  // Scale and bias factor -
  //   If the velocity resolution is set to 2 in 
  //   nexrad header, the scale and bias should
  //   remain as they are in the cdl file.  Otherwise
  //   they should be changed by a factor of two
  //
  float velScaleBiasFactor;
  
  int   elevIndex;
  //   int   numGates;
  int   vcp;
  float rangeToFirstVelGate;
  float rangeToFirstReflGate;
  //float unambiguousRange;
  float fixedAngle;
  float nyquistVelocity;
  //float prf;
  float cellSpacingVel;
  float cellSpacingRefl;
  float dbz0;
  //float horiz_noise;
  //float vert_noise;
  float pulseCount;
  int   numGatesRefl;
  int   numGatesVel;

  //
  // User request to calc Rec, Snr, and Pr
  bool calcRec;
  bool calcSnr;
  bool calcPr;

  //
  // Vector of beams
  //
  vector< Beam* > beams;

  //
  // Rec computation 
  //   applyRec         = compute the rec or not?
  //   maxBeamQueueSize = maximum number of beams allowd in the queue
  //   beamQueue        = list of beams needed for rec computation
  //
  bool          applyRec;
  int           midBeamIndex;
  int           maxBeamQueueSize;
  deque<Beam *> beamQueue;
  
  //
  // Previous sweep
  //   Note that this class does not own the memory
  //   associated with this object.  It is not responsible
  //   for allocating or freeing its memory.
  //
  SweepData *prevSweep;

  //
  // Clutter Segment pointer  
  ClutterSegment_t *clutterSegment;

  //
  // Bypass Segment pointer
  BypassSegment_t *bypassSegment;

  //
  // VCP header and elevation pointer
  RIDDS_VCP_hdr *vcpHdr;
  RIDDS_elevation_angle *vcpElev;

  //
  // Status Data pointer
  RIDDS_status_data *statusData;

  //
  // Azimuth, elevation and time tolerances
  //   
  double azTolerance;
  double elevTolerance;
  double timeTolerance;
  
  //
  // Fill in the index array using the information from this file
  //   Note that this function should only be used when scanType
  //   is REFL_ONLY
  //
  void fillIndexArray();
  
  //
  // Figure out which index from the azimuth data array
  // should be used at a given point in the azimuthIdex array
  //   i = index into azimuth data array - the ith ray in the
  //       data arrays
  //   j = index into azimuthIdex array
  //
  //   Note that this function should only be used when scanType
  //   is REFL_ONLY
  //
  void setIndex( int i, int j );
  
  //
  // Add the beam to the list of beams for the rec computation
  //   beam = beam object to be added
  //
  void addBeamToQueue( Beam* beam );
  
  //
  // Write the beam summary
  //   nexradData = ridds data header
  //
  void writeSummary( RIDDS_data_hdr* nexradData );
  
};

#endif







