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
// $Id: SweepData.hh,v 1.23 2016/03/07 01:23:10 dixon Exp $
//
/////////////////////////////////////////////////////////
#ifndef _SWEEP_DATA_
#define _SWEEP_DATA_

#include <netcdf.hh>
#include <rapformats/ridds.h>
#include "Status.hh"
#include "Params.hh"

//
// Forward class declarations
//
class VCP121Lookup;

class SweepData 
{
public:

   enum scan_t { NONE, REFL_ONLY, VEL_ONLY, BOTH };

   //
   // Constructor
   //   params = reference to tdrp parameters
   //
   SweepData( Params& params );

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
   // Set info related to this sweep
   //   nexradData = ridds data header
   //   pSweep      = previous sweep 
   //                Note that this parameter is not
   //                necessary
   //
   //   Certain data values need to be set at the
   //   beginning of each sweep.  Set these values
   //   
   //   See Status.hh for meaning of return value
   //
   Status::info_t setInfo( RIDDS_data_hdr* nexradData, 
                           SweepData* pSweep = NULL ) ;

   //
   // Copy the data from the current message into the 
   // sweep
   //   nexradData = ridds data header and byte data
   //                that follows
   //   beamTime   = time associated with current message
   //
   //   See Status.hh for meaning of return value
   //
   Status::info_t copyData( RIDDS_data_hdr* nexradData, time_t beamTime );

   //
   // Is the sweep complete yet?
   //   Note that this does not indicate whether or not
   //   we have all the beams for the given sweep.  
   //
   //   Return of true indicates that we do have at least
   //   some of the data for each data field expected for
   //   this type of sweep.  
   //
   bool sweepIsComplete(){ return sweepComplete; }

   //
   // Was there a sweep lost because we couldn't merge?
   //
   bool sweepLost();

   //
   // Was this tilt merged with the previous one?
   //
   bool merged(){ return mergeDone; }

   //
   // Are we skipping this sweep?
   //
   bool sweepSkipped(){ return skipSweep; }

   //
   // Return data values from this sweep
   //
   scan_t         getScanType(){ return scanType; }

   int            getVCP(){ return vcp; }
   int            getNumRays(){ return numRays; }
   int            getElevIndex(){ return elevIndex; }

   float          getFixedAngle(){ return fixedAngle; }
   float          getVelScaleBiasFactor(){ return velScaleBiasFactor; }
   float          getNyquistVelocity(){ return nyquistVelocity; }
   float          getCellSpacing();
   float          getDzGateWidth(){ return dzGateWidth; }
   float          getVeGateWidth(){ return veGateWidth; }
   float          getRangeToFirstGate(){ return rangeToFirstGate; }
   float          getUnambiguousRange(){ return unambiguousRange; }
   float          getPrf(){ return prf; }
   float          getSurPrf();

   short*         getDz(){ return dz; }
   short*         getSnr(){ return snr; }
   short*         getVe(){ return ve; }
   short*         getSw(){ return sw; }
   short*         getPr(){ return pr; }

   float*         getAzimuth(){ return azimuth; }
   float*         getElevation(){ return elevation; }

   double*        getTimeData(){ return timeData; }

   
   //
   // Constants
   //  MAX_RAYS        = maximum number of rays we would expect
   //  MISSING_INDEX   = missing values for azimuth indeces
   //  M_TO_KM         = multiplier for conversion from meters
   //                    to kilometers
   //  DZ_SCALE        = scale for dz data
   //  DZ_BIAS         = bias for dz data
   //  VE_SCALE        = scale for ve data
   //  VE_BIAS         = bias for ve data
   //  DELTA_AZIMUTH   = used to define data structure used
   //                    when combining scans
   //  SPEED_OF_LIGHT  = speed of light in m/s
   //  DZ_BAD          = bad or missing value for output dz data
   //  VE_BAD          = bad or missing value for output ve data
   //  SNR_BAD         = bad or missing value for output snr data
   //  PR_BAD          = bad or missing value for output pr data
   //  HDR             = header for summary print out
   //  FMT             = format for summary print out
   //
   static const int     MAX_RAYS;
   static const int     MISSING_INDEX;
   static const double  M_TO_KM;
   static const double  DZ_SCALE;
   static const double  DZ_BIAS;
   static const double  VE_SCALE;
   static const double  VE_BIAS;
   static const double  DELTA_AZIMUTH;
   static const double  SPEED_OF_LIGHT;
   static const short   DZ_BAD;
   static const short   VE_BAD;
   static const short   SNR_BAD;
   static const short   PR_BAD;
   static const char   *HDR;
   static const char   *FMT;

private:

   //
   // Does this sweep have reflectivity data only,
   // velocity and spectrum width data only or 
   // both reflectivity and velocity
   //
   scan_t  scanType;

   //
   // Tells us whether we have at least part of all
   // of the types of data we expect.  It does NOT
   // tell us if we have a complete sweep in the sense
   // of a full 360 degrees of data.
   //
   bool sweepComplete;

   //
   // Tells us if we want to combine reflectivity only
   // sweep with the velocity and spectrum width sweep
   //
   bool combineSweeps;

   //
   // Tells us if we have exceeded our maximum number
   // of rays for this sweep
   //
   bool maxRaysExceeded;

   //
   // Tells us if a merge was done with the previous
   // sweep
   //
   bool mergeDone;

   //
   // Tells us if we should skip writing this sweep out
   //
   bool skipSweep;

   //
   // Elevation index within the volume
   //
   int elevIndex;
   
   //
   // Print a beam summary?  How often?
   //
   bool printSummary;
   int  summaryInterval;
   int  summaryCount;

   //
   // Keep track of the latest milliseconds past
   // midnight.  We need this information because
   // we need greater precision than one second to
   // "fix" the data repeat problem from ldm data -
   // sometimes we get an exact repeat of 100 beams
   // or so in a given ldm file...
   // 
   int latestMillisecs;

   //
   // Dimensions
   //
   int maxCells;
   int numRays;

   //
   // One dimensional arrays by time
   //
   int     rayIdex;
   double *timeData;
   float  *azimuth;
   float  *elevation;

   //
   // Two dimensional arrays by time
   // and cell number
   //
   int     gateIdex;
   short  *dz;
   short  *snr;
   short  *ve;
   short  *sw;
   short  *pr;

   //
   // Variables used to compute snr
   //
   float   snrFactor;
   float   rangeToFirstGate;
   double  snrScale;
   double  snrBias;

   //
   // Variables used to compute power ratio
   //
   short   powerRatioDefault;
   float   unambiguousRange;
   double  prScale;
   double  prBias;

   //
   // Scale and bias factor -
   //   If the velocity resolution is set to 2 in 
   //   nexrad header, the scale and bias should
   //   remain as they are in the cdl file.  Otherwise
   //   they should be changed by a factor of two
   //
   float velScaleBiasFactor;

   //
   // Gate widths
   //
   float dzGateWidth;
   float veGateWidth;

   //
   // Other info from ridds data
   //
   int   vcp;
   float fixedAngle;
   float nyquistVelocity;
   float prf;

   //
   // Previous sweep
   //   Note that this class does not own the memory
   //   associated with this object.  It is not responsible
   //   for allocating or freeing its memory.
   //
   SweepData *prevSweep;

   //
   // Azimuth, elevation and time tolerances
   //   
   double azTolerance;
   double elevTolerance;
   double timeTolerance;
   
   //
   // Azimuth index data
   //   numIndeces  = length of the azimuthIdex array
   //   azimuthIdex = array which tells which index in the
   //                 azimuth data array corresponds to the
   //                 given azimuth as defined by i * DELTA_AZIMUTH
   //                 where i is the index into the azimuthIdex
   //                 array and DELTA_AZIMUTH is the resolution
   //                 in azimuths
   //
   int  numIndeces;
   int *azimuthIdex;

   //
   // Object that decides how to handle the different tilts
   // of vcp 121 
   //   Note that this is a temporary fix, until we can figure
   //   out how to use all the tilts
   //
   VCP121Lookup *vcp121;

   //
   // Set the dbz data from the nexrad data struct and
   // calculate snr
   //   nexradData = ridds data header and byte data
   //                that follows
   //   beamTime   = time associated with current beam
   //
   void setDbzData( RIDDS_data_hdr* nexradData, time_t beamTime );

   //
   // Set the velocity and spectrum width data from the
   // nexrad data struct 
   //   nexradData = ridds data header and byte data that
   //                follows
   //   beamTime   = time associated with current beam
   // 
   void setVelData( RIDDS_data_hdr* nexradData, time_t beamTime );

   //
   // Set the reflectivity, velocity and spectrum width
   // data from the nexrad data struct
   //   nexradData = ridds data header and byte data that
   //                follows
   //   beamTime   = time associated with current beam
   //
   void setData( RIDDS_data_hdr* nexradData, time_t beamTime );

   //
   // Set the data that is flagged as low snr or ambiguous
   // range to the missing value and adjust the rest of
   // the data to take out those flag values.  Applies to
   // all fields, except snr, since that is computed from
   // the corrected reflectivity.  
   //   nexradData = byte data
   //   numGatesIn = number of input gates, i.e. number
   //                of gates coming in nexradData array
   //
   void correctData( ui08* nexradData, int numGatesIn );

   //
   // Store the nexrad data in an array that can later
   // be used to write to a netcdf file 
   //   nexradData = byte data
   //   ncfData    = data array to write into
   //   numGatesIn = number of input gates, i.e. number
   //                of gates coming in nexradData array
   //   
   void storeData( ui08* nexradData, short* ncfData, int numGatesIn );

   //
   // Duplicate the reflectivity data so that the
   // gate spacing matches that of the velocity data
   //   nexradData   = reflectivity byte data from message
   //   inGateWidth  = input gate spacing
   //   outGateWidth = output gate spacing, or the gate
   //                  spacing the reflectivity data
   //                  will have after duplication
   //   numGatesIn   = number of gates in input data
   //   
   void duplicateDbz( ui08* nexradData, float inGateWidth, 
                      float outGateWidth, int numGatesIn );

   //
   // Calculate snr
   //   nGates    = number of input gates
   //   gateWidth = gate spacing
   //   
   void calcSnr( int nGates, float gateWidth );

   //
   // Calculate power ratio
   //   nGates    = number of input gates
   //   gateWidth = gate spacing
   //
   void calcPR( int nGates, float gateWidth );
   
   //
   // Merge the reflectivity data in with the velocity and
   // spectrum width data.  Note that at this point, the
   // reflectivity data is corrected, but not duplicated.
   // So the duplication functionality is repeated here,
   // but the correction is not.  
   //
   void mergeDbz();

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
   // Find the index of the beam that is closest to the beam
   // in the velocity data with the given azimuth, elevation
   // and time
   //   currentAz = azimuth of velocity beam we are matching
   //               in the reflectivity only sweep
   //   currentElev = elevation of the velocity beam we are
   //                 matching in the reflectivity only sweep
   //   currentTime = time of the velocity only beam we are
   //                 matching in the reflectivity only sweep
   //
   //   Note that this function should only be used when scanType
   //   is REFL_ONLY
   //
   //   returns the index of the closest beam in the 
   //   reflectivity only sweep
   //
   int  findNearest( float currentAz, float currentElev, double currentTime );
   
   //
   // Write the beam summary
   //   nexradData = ridds data header
   //
   void writeSummary( RIDDS_data_hdr* nexradData );
   
};

#endif



