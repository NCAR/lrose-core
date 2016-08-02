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
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
// $Id: SweepData.cc,v 1.20 2016/03/07 01:23:03 dixon Exp $
//
/////////////////////////////////////////////////////////
#include <math.h>
#include <toolsa/pmu.h>
#include <rapformats/ridds.h>
#include "SweepData.hh"
#include "Nexrad2Netcdf.hh"
#include "RangeTable.hh"
#include "Beam.hh"

//
// Constants
//
const int    SweepData::MISSING_INDEX   = -1;
const float  SweepData::BEAM_WIDTH      = 0.95;  // deg
const double SweepData::SPEED_OF_LIGHT  = 299792458;

const char*  SweepData::HDR = " VCP Status Elev  Elev_num   Az     Az_num "
                              "Ref_Ngat Vel_Ngat Ref_gspc Vel_gspc Date"
                              "       Time";

const char*  SweepData::FMT = "%4ld   %2ld %6.2f  %4ld      %6.2f  %4ld   "
                              "%5ld   %5ld     %6.2f   %6.2f  %s";

SweepData::SweepData( Params *tdrpParams )
{
  params = tdrpParams;

   //
   // Set data members that won't change over lifetime of
   // object
   //    
   maxBeamQueueSize   = params->recKernelAzimuthWidth;
   midBeamIndex       = maxBeamQueueSize / 2;
   azTolerance        = params->azimuthTolerance;
   elevTolerance      = params->elevationTolerance;
   timeTolerance      = params->timeTolerance;

   //
   // See if user requested to calc snr, pr, or rec
   //
   calcSnr = false;
   calcPr = false;
   calcRec = false;
   
   for( int ii = 0; ii < params->derivedFieldDefs_n; ii++ ) {
     switch( params->_derivedFieldDefs[ii].outputField ) {
     case Params::SNR :
       calcSnr = true;
       break;
       
     case Params::PR :
       calcPr = true;
       break;
       
     case Params::REC :
       calcRec = true;
     }
   }
   
   clear();
}

SweepData::~SweepData() 
{
   clear();
}

void SweepData::clear() 
{
   mergeDone          = false;
   skipSweep          = false;
   summaryCount       = 0;
   prevSweep          = NULL;
   applyRec           = false;
   clutterSegment     = NULL;
   bypassSegment      = NULL;
   vcpHdr             = NULL;
   vcpElev            = NULL;
   statusData         = NULL;

   beamQueue.clear();

   vector< Beam* >::iterator it;
   for( it = beams.begin(); it != beams.end(); it++ ) {
      delete (*it);
   }
   beams.erase( beams.begin(), beams.end() );

}

status_t SweepData::setInfo(RIDDS_data_31_hdr* nexradData, 
			    SweepData* pSweep ) 
{
  prevSweep = pSweep;

  //
  // Get pointers to each header section

  RIDDS_volume_31_hdr *volData = (RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->volume_ptr);
  
  RIDDS_elev_31_hdr *elevData = (RIDDS_elev_31_hdr *)((ui08*)nexradData + nexradData->elevation_ptr);
  
  RIDDS_radial_31_hdr *radialData = (RIDDS_radial_31_hdr *)((ui08*)nexradData + nexradData->radial_ptr);
  

  //
  // Set information for this tilt
  //
  elevIndex        = nexradData->elev_num;
  vcp              = volData->vol_coverage_pattern;
  fixedAngle       = nexradData->elevation;
  nyquistVelocity  = ((float) radialData->nyquist_vel) / 100;
  unambiguousRange = ((float) radialData->unamb_range_x10) / 10;
  prf              = (float)(SPEED_OF_LIGHT / (2 * unambiguousRange * 1000));
  velScaleBiasFactor = 1.0;
  dbz0             = elevData->dbz0;
  horiz_noise      = radialData->horiz_noise;
  vert_noise       = radialData->vert_noise;

  if(nexradData->ref_ptr != 0) {
    RIDDS_data_31 *reflData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->ref_ptr);
    cellSpacingRefl  = (float) reflData->gate_width;
    rangeToFirstReflGate = (float) reflData->gate1;
    numGatesRefl     = reflData->num_gates;
  } else {
    cellSpacingRefl  = 0.0;
    rangeToFirstReflGate = 0.0;
    numGatesRefl     = 0;
  }

  if(nexradData->vel_ptr != 0) {
    RIDDS_data_31 *velData = (RIDDS_data_31 *)((ui08*)nexradData + nexradData->vel_ptr);
    cellSpacingVel  = (float) velData->gate_width;
    rangeToFirstVelGate = (float) velData->gate1;
    numGatesVel      = velData->num_gates;
  } else {
    cellSpacingVel  = 0.0;
    rangeToFirstVelGate = 0.0;
    numGatesVel      = 0;
  }

  // Decide what data we have in this sweep
  if( nexradData->ref_ptr > 0 && nexradData->vel_ptr <= 0) {
    scanType         = REFL_ONLY;
    POSTMSG( DEBUG, "Sweep has Reflectivity only data");
   }
   else if( nexradData->ref_ptr <= 0 && nexradData->vel_ptr > 0 ) {
      scanType         = VEL_ONLY;
      POSTMSG( DEBUG, "Sweep has Velocity only data");
   }
   else if( params->combineSweeps && vcpElev && vcpElev->waveform_type == 1 && 
	    nexradData->ref_ptr > 0 && nexradData->vel_ptr > 0) {
    scanType         = REFL_ONLY;
    POSTMSG( DEBUG, "Sweep has both Reflectivity and Velocity but is a Contiguous Surveillance Cut");
   }
   else if( params->combineSweeps && vcpElev && vcpElev->waveform_type == 2 && 
	    nexradData->ref_ptr > 0 && nexradData->vel_ptr > 0) {
    scanType         = VEL_ONLY;
    POSTMSG( DEBUG, "Sweep has both Reflectivity and Velocity but is a Contiguous Doppler Cut");
   }
   else if( nexradData->ref_ptr > 0 && nexradData->vel_ptr > 0 ) {
      scanType         = BOTH;
      // Hack for when Velocity is coming in on the reflectivity data block
      if( ((RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->ref_ptr))->block_name[0] == 'V' &&
	  ((RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->vel_ptr))->block_name[0] == 'S' &&
	  nexradData->sw_ptr == 0) {
	scanType       = VEL_ONLY;
	nexradData->sw_ptr = nexradData->vel_ptr;
	nexradData->vel_ptr = nexradData->ref_ptr;
	nexradData->ref_ptr = 0;
	POSTMSG( DEBUG, "Sweep has Velocity only data");
      } else {
	POSTMSG( DEBUG, "Sweep has both Reflectivity and Velocity data");
      }
   }
   else {
      POSTMSG( ERROR, "No data present" );
      skipSweep = true;
   }

  if(params->combineSweeps && scanType == VEL_ONLY && prevSweep == NULL)
    skipSweep = true;

  // Decide if we should apply the rec to this particular sweep
  if(calcRec && (scanType == BOTH || params->combineSweeps))
    applyRec = true;

  return( ALL_OK );
}

status_t SweepData::setInfo( RIDDS_data_hdr* nexradData, 
                                   SweepData* pSweep ) 
{
  prevSweep = pSweep;
  
  // Determine the scale and bias factor for the velocity 
  // data
  if ( nexradData->velocity_resolution == 2 ) {
    // 0.5 m/s resolution which is the default
    velScaleBiasFactor = 1.0;
  }
  else {
    // 1.0 m/s resolution which is double the default
    velScaleBiasFactor = 2.0;
  }
  

  // Set information for this tilt
  elevIndex        = nexradData->elev_num;
  vcp              = nexradData->vol_coverage_pattern;
  fixedAngle       = (nexradData->elevation / 8.) * (180. / 4096.);
  nyquistVelocity  = ((float) nexradData->nyquist_vel) / 100;
  unambiguousRange = ((float) nexradData->unamb_range_x10) / 10;
  prf              = (float)(SPEED_OF_LIGHT / (2 * unambiguousRange * 1000));
  cellSpacingRefl  = (float) nexradData->ref_gate_width;
  cellSpacingVel   = (float) nexradData->vel_gate_width;
  rangeToFirstReflGate = (float) nexradData->ref_gate1;
  rangeToFirstVelGate  = (float) nexradData->vel_gate1;
  numGatesRefl     = nexradData->ref_num_gates;
  numGatesVel      = nexradData->vel_num_gates;
  dbz0             = -999.0;
  horiz_noise      = -999.0;
  vert_noise       = -999.0;
  
  // Decide what data we have in this sweep
  if( nexradData->ref_ptr > 0 &&
      (nexradData->vel_ptr <= 0 || nexradData->vel_num_gates <= 0)) {
    scanType         = REFL_ONLY;
   }
   else if( nexradData->ref_ptr <= 0 &&
            nexradData->vel_ptr > 0 ) {
      scanType         = VEL_ONLY;
   }
   else if( nexradData->ref_ptr > 0 &&
            nexradData->vel_ptr > 0 ) {
      scanType         = BOTH;
   }
   else {
      POSTMSG( ERROR, "No data present" );
      skipSweep = true;
   }

  if(params->combineSweeps && scanType == VEL_ONLY && prevSweep == NULL)
    skipSweep = true;

   // Decide if we should apply the rec to this particular sweep
  if(calcRec && (scanType == BOTH || params->combineSweeps))
   {
      applyRec = true;
   }

  return( ALL_OK );
   
}

float SweepData::getSurPrf() 
{
   if( prevSweep ) {
      return( prevSweep->getPrf() );
   }
   
   return( -1.0 );
}

float SweepData::getSurPulseCount()
{
   if( prevSweep ) {
      return( prevSweep->getPulseCount() );
   }
   
   return( -1.0 );
}

status_t SweepData::copyData( RIDDS_data_hdr* nexradData, time_t beamTime ) 
{
   if( skipSweep ) {
      return( ALL_OK );
   }
   
   Beam *newBeam = new Beam(params,  nexradData,  beamTime, 
                            scanType, prevSweep );
   
   _copyData(newBeam);

   if( params->printSummary ) {
      writeSummary( nexradData );
   }

   return( ALL_OK );
   
}

status_t SweepData::copyData(RIDDS_data_31_hdr* nexradData, time_t beamTime ) 
{
   if( skipSweep ) {
      return( ALL_OK );
   }
   
   Beam *newBeam = new Beam(params, nexradData, beamTime, 
                            scanType, prevSweep );
   _copyData(newBeam);
   
   return( ALL_OK );
   
}

void SweepData::_copyData(Beam *newBeam)
{
   beams.push_back( newBeam );

   if( applyRec ) {
      addBeamToQueue( newBeam );

      if( (int) beamQueue.size() == maxBeamQueueSize ) {
         beamQueue[midBeamIndex]->computeRec( beamQueue, midBeamIndex );
      }
   }

   if( newBeam->beamMerged() ) {
      mergeDone = true;
   }
   
}

void SweepData::calculatePulseCount()
{
  if(vcpHdr == NULL)
  {
    int bsize = beams.size();
    float startaz = (beams[0])->getAzimuth();
    double startT = (beams[0])->getTime();
    float endaz = (beams[bsize-1])->getAzimuth();
    double endT = (beams[bsize-1])->getTime();
    
    if(endT == startT)
      return;
    
    if(endaz < startaz)
      endaz += 360.0;
    
    float scanrate = (endaz - startaz) / (endT - startT);
    float degrees = scanrate / prf;
    float pulse_count = 1.0 / degrees;
    pulseCount = pulse_count;
    // if number of range gates differs between surv/dop (and not merged), in batch mode
    // use prf * .75
    if(params->combineSweeps && prevSweep != NULL && prevSweep->getNBeams() > 0 && prevSweep->getScanType() == SweepData::REFL_ONLY)
    {
	prevSweep->calculatePulseCount();
    }
  } else {
    pulseCount = -1.0;
  }

}

void SweepData::fillRec() 
{
   //
   // If we are not computing the rec, don't do anything
   //
   if( !applyRec ) {
      return;
   }
   
   //
   // Find out if we had overlap
   //
   int n = (int) beams.size();

   float prevAz     = (beams[0])->getAzimuth();
   float sumOfDiffs = 0.0;

   for( int i = 1; i < n; i++ ) {
      float currentAz = (beams[i])->getAzimuth();

      //
      // Perform "special" mod - fmodf is rounding towards
      // zero, rather than flooring
      //
      float quot = floorf( (currentAz-prevAz) / 360.0 );
      float currentDiff = (currentAz-prevAz) - (quot * 360.0);

      sumOfDiffs += currentDiff;

      prevAz = currentAz;
   }

   //fprintf( stderr, "sumOfDiffs = %f\n", sumOfDiffs );
   
   
   //
   // If we did not have overlap in the beam azimuths...
   //
   if( sumOfDiffs <= 359.5 ) {

      float firstAzimuth = (beams[0])->getAzimuth();
      float lastAzimuth  = (beams[n-1])->getAzimuth();

      //
      // If there was not too large of a gap between the first
      // and last beam azimuths...
      //
      if( fabs( lastAzimuth - firstAzimuth) < 1.5 * BEAM_WIDTH ) {
         
         //
         // Compute the rec for the azimuths that we were not
         // able to before (because of the lack of azimuth overlap)
         // by wrapping around to the start of the list of beams.
         //   Note: This assumes the beamQueue was not cleared out
         //   after the last beam was done, but before the sweep
         //   was written out.
         //
         int nMissing = 2 * ((int) (maxBeamQueueSize / 2));

         for( int i = 0; i < nMissing; i++ ) {
            addBeamToQueue( beams[i] );

            if( (int) beamQueue.size() == maxBeamQueueSize ) {
               beamQueue[midBeamIndex]->computeRec( beamQueue, midBeamIndex );
            }
         }
      }
   }
   
}


void SweepData::addBeamToQueue( Beam* beam )
{
   //
   // Manage the size of the beam queue, popping off the back
   //
   if ( (int) beamQueue.size() == maxBeamQueueSize ) {
      beamQueue.pop_back();
   }
  
   //
   // Push beam onto front of queue
   //
   beamQueue.push_front( beam );

}

bool SweepData::allFieldsPresent() 
{
   for( int i = 0; i < (int) beams.size(); i++ ) {
      if( beams[i]->beamComplete() ) {
         return( true );
      }
   }
   
   return( false );
}


bool SweepData::sweepLost() 
{
   if( !params->combineSweeps || 
       scanType == BOTH ||
       !prevSweep || 
       prevSweep->getElevIndex() < 0 ) {
      return false;
   }

   return( !mergeDone );

}


Beam* SweepData::getClosestBeam(float currentAz, float currentElev, double currentTime) 
{
  int closestIndex = -1;
  float closestAzimuthDiff = 999.99;
  float azimuthDiff;
  double closestTimeDiff;
  float closestElevDiff;

  for(int a = 0; a < beams.size(); a++) {
    azimuthDiff = fabs(currentAz - beams[a]->getAzimuth());
    if(azimuthDiff < closestAzimuthDiff) {
      closestIndex = a;
      closestAzimuthDiff = azimuthDiff;
      closestTimeDiff = fabs(currentTime - beams[a]->getTime());
      closestElevDiff = fabs(currentElev - beams[a]->getElevation());
    }

  }

  if(closestIndex == -1) {
    POSTMSG( WARNING, "No beams with which to merge" );
    return( NULL );
  }

  if(closestAzimuthDiff > azTolerance ) {
    POSTMSG( WARNING, "No beams within azimuth tolerance to merge with" );
    return( NULL );
  }

  if(closestElevDiff > elevTolerance ) {
    POSTMSG( WARNING, "No beams within elevation tolerance to merge with" );
    return( NULL );
  }
  
  if(closestTimeDiff > timeTolerance ) {
    POSTMSG( WARNING, "No beams within time tolerance to merge with" );
    return( NULL );
  }

   return( beams[closestIndex] );
    
}


void SweepData::writeSummary( RIDDS_data_hdr* nexradData )
{
   if ( summaryCount == 0 ) {

      fprintf( stderr, HDR );
      fprintf( stderr, "\n" );
      
      summaryCount = params->summaryInterval;     
   }

   //
   // Parse the time of the beam 
   //  
   time_t beamTime = ((nexradData->julian_date - 1) * 86400 +
		      nexradData->millisecs_past_midnight / 1000);
   DateTime when( beamTime );
   
   float elevation = (nexradData->elevation/8.0)*(180.0/4096.0);
   float azimuth   = (nexradData->azimuth/8.0)*(180.0/4096.0);
   
   //
   // Print info
   //
   fprintf( stderr, FMT,
	    (long)   vcp,
	    (long)   nexradData->radial_status,
	    (double) elevation,
	    (long)   nexradData->elev_num,
	    (double) azimuth,
	    (long)   nexradData->radial_num,
	    (long)   nexradData->ref_num_gates,
	    (long)   nexradData->vel_num_gates,
	    (double)   nexradData->ref_gate_width,
	    (double)   nexradData->vel_gate_width,
	    when.dtime() );
   
   fprintf(stderr, "\n");

   
   //
   // Decrement the counter
   //
   summaryCount--;
}  


int SweepData::getNVelGates()
{
  if(beams.size() > 0)
    return beams[0]->getNVelGates();
  return 0;
}
int SweepData::getNReflGates()
{
  if(beams.size() > 0)
    return beams[0]->getNReflGates();
  return 0;
}
float SweepData::getCellSpacingVel()
{
  if(beams.size() > 0)
    return beams[0]->getVelGateWidth();
  return -1.0;
}
float SweepData::getCellSpacingRefl()
{
  if(beams.size() > 0)
    return beams[0]->getReflGateWidth();
  return -1.0;
}
float SweepData::getRangeToFirstVelGate()
{
  if(beams.size() > 0)
    return beams[0]->getRangeToFirstVelGate();
  return 0.0;
}
float SweepData::getRangeToFirstReflGate()
{
  if(beams.size() > 0)
    return beams[0]->getRangeToFirstReflGate();
  return 0.0;
}
float SweepData::getDbzScale()
{
  if(beams.size() > 0)
    return beams[0]->getDbzScale();
  return 1.0;
}
float SweepData::getDbzBias()
{
  if(beams.size() > 0)
    return beams[0]->getDbzBias();
  return 0.0;
}
short SweepData::getDbzBad()
{
  if(beams.size() > 0)
    return beams[0]->getDbzBad();
  return -1;
}
float SweepData::getVelScale()
{
  if(beams.size() > 0)
    return beams[0]->getVelScale();
  return 1.0;
}
float SweepData::getVelBias()
{
  if(beams.size() > 0)
    return beams[0]->getVelBias();
  return 0.0;
}
short SweepData::getVelBad()
{
  if(beams.size() > 0)
    return beams[0]->getVelBad();
  return -1;
}
float SweepData::getSwScale()
{
  if(beams.size() > 0)
    return beams[0]->getSwScale();
  return 1.0;
}
float SweepData::getSwBias()
{
  if(beams.size() > 0)
    return beams[0]->getSwBias();
  return 0.0;
}
short SweepData::getSwBad()
{
  if(beams.size() > 0)
    return beams[0]->getSwBad();
  return -1;
}
float SweepData::getZdrScale()
{
  if(beams.size() > 0)
    return beams[0]->getZdrScale();
  return 1.0;
}
float SweepData::getZdrBias()
{
  if(beams.size() > 0)
    return beams[0]->getZdrBias();
  return 0.0;
}
short SweepData::getZdrBad()
{
  if(beams.size() > 0)
    return beams[0]->getZdrBad();
  return -1;
}
float SweepData::getPhiScale()
{
  if(beams.size() > 0)
    return beams[0]->getPhiScale();
  return 1.0;
}
float SweepData::getPhiBias()
{
  if(beams.size() > 0)
    return beams[0]->getPhiBias();
  return 0.0;
}
short SweepData::getPhiBad()
{
  if(beams.size() > 0)
    return beams[0]->getPhiBad();
  return -1;
}
float SweepData::getRhoScale()
{
  if(beams.size() > 0)
    return beams[0]->getRhoScale();
  return 1.0;
}
float SweepData::getRhoBias()
{
  if(beams.size() > 0)
    return beams[0]->getRhoBias();
  return 0.0;
}
short SweepData::getRhoBad()
{
  if(beams.size() > 0)
    return beams[0]->getRhoBad();
  return -1;
}
float SweepData::getSnrScale()
{
  if(beams.size() > 0)
    return beams[0]->getSnrScale();
  return 1.0;
}
float SweepData::getSnrBias()
{
  if(beams.size() > 0)
    return beams[0]->getSnrBias();
  return 0.0;
}
short SweepData::getSnrBad()
{
  if(beams.size() > 0)
    return beams[0]->getSnrBad();
  return -1;
}
float SweepData::getPrScale()
{
  if(beams.size() > 0)
    return beams[0]->getPrScale();
  return 1.0;
}
float SweepData::getPrBias()
{
  if(beams.size() > 0)
    return beams[0]->getPrBias();
  return 0.0;
}
short SweepData::getPrBad()
{
  if(beams.size() > 0)
    return beams[0]->getPrBad();
  return -1;
}
float SweepData::getRecScale()
{
  if(beams.size() > 0)
    return beams[0]->getRecScale();
  return 1.0;
}
float SweepData::getRecBias()
{
  if(beams.size() > 0)
    return beams[0]->getRecBias();
  return 0.0;
}
short SweepData::getRecBad()
{
  if(beams.size() > 0)
    return beams[0]->getRecBad();
  return -1;
}
