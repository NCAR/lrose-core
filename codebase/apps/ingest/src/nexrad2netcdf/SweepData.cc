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
// $Id: SweepData.cc,v 1.42 2016/03/07 01:23:10 dixon Exp $
//
/////////////////////////////////////////////////////////
#include <toolsa/pmu.h>
#include <rapformats/ridds.h>
#include "SweepData.hh"
#include "Driver.hh"
#include "RangeTable.hh"
#include "VCP121Lookup.hh"

//
// Constants
//
const int    SweepData::MAX_RAYS        = 720;
const int    SweepData::MISSING_INDEX   = -1;
const double SweepData::M_TO_KM         = 0.001;
const double SweepData::DZ_SCALE        = 0.5;
const double SweepData::DZ_BIAS         = -32.0;
const double SweepData::VE_SCALE        = 0.5;
const double SweepData::VE_BIAS         = -63.5;
const double SweepData::DELTA_AZIMUTH   = 0.10;
const double SweepData::SPEED_OF_LIGHT  = 299792458;

//
//  Note that these constants are used for readability only.
//  If these values need to be changed, the memset calls in
//  the clear function may need to be modified. THINK CAREFULLY
//  BEFORE SETTING THESE TO ANYTHING BUT ZERO.
//  
const short  SweepData::DZ_BAD  = 0;
const short  SweepData::VE_BAD  = 0;
const short  SweepData::SNR_BAD = 0;
const short  SweepData::PR_BAD  = 0;

const char*  SweepData::HDR = " VCP Status Elev  Elev_num   Az     Az_num "
                              "Ref_Ngat Vel_Ngat Ref_gspc Vel_gspc Date"
                              "       Time";

const char*  SweepData::FMT = "%4ld   %2ld %6.2f  %4ld      %6.2f  %4ld   "
                              "%5ld   %5ld     %6.2f   %6.2f  %s";

SweepData::SweepData( Params& params ) 
{
   double indexCount = 360.0 / DELTA_AZIMUTH;
   
   //
   // Set data members that won't change over lifetime of
   // object
   //
   combineSweeps      = params.combineSweeps;
   printSummary       = params.printSummary;
   summaryInterval    = params.summaryInterval;
   maxCells           = params.maxCells;
   snrFactor          = params.snrFactor;
   snrScale           = DZ_SCALE;
   snrBias            = DZ_BIAS;
   azTolerance        = params.azimuthTolerance / DELTA_AZIMUTH;
   elevTolerance      = params.elevationTolerance;
   timeTolerance      = params.timeTolerance;
   numIndeces         = (int) indexCount;
   prScale            = DZ_SCALE;
   prBias             = DZ_BIAS;
   
   for( int ii = 0; ii < params.derivedFieldDefs_n; ii++ ) {
      switch( params._derivedFieldDefs[ii].outputField ) {
          case Params::SNR :
             snrScale = params._derivedFieldDefs[ii].scale;
             snrBias  = params._derivedFieldDefs[ii].bias;
             break;
             
          case Params::PR :
             prScale = params._derivedFieldDefs[ii].scale;
             prBias  = params._derivedFieldDefs[ii].bias;
             break;
      }
   }
   
   powerRatioDefault  = 
      (short) ((params.powerRatioDefault - prBias) / prScale);

   //
   // Set up data arrays
   //
   timeData    = new double[MAX_RAYS];
   azimuth     = new float[MAX_RAYS];
   elevation   = new float[MAX_RAYS];
   azimuthIdex = new int[numIndeces];
   dz          = new short[MAX_RAYS*maxCells];
   snr         = new short[MAX_RAYS*maxCells];
   ve          = new short[MAX_RAYS*maxCells];
   sw          = new short[MAX_RAYS*maxCells];
   pr          = new short[MAX_RAYS*maxCells];

   //
   // Set up stuff for vcp 121
   //
   vcp121 = new VCP121Lookup( elevTolerance );

   clear();
}

SweepData::~SweepData() 
{
   delete[] timeData;
   delete[] azimuth;
   delete[] elevation;
   delete[] dz;
   delete[] snr;
   delete[] pr;
   delete[] ve;
   delete[] sw;
   delete[] azimuthIdex;
   delete vcp121;
}

void SweepData::clear() 
{
   scanType           = NONE;
   sweepComplete      = false;
   maxRaysExceeded    = false;
   mergeDone          = false;
   skipSweep          = false;
   elevIndex          = -1;
   summaryCount       = 0;
   latestMillisecs    = 0;
   numRays            = 0;
   rayIdex            = 0;
   gateIdex           = 0;
   rangeToFirstGate   = 0.0;
   unambiguousRange   = 0.0;
   velScaleBiasFactor = 1.0;
   dzGateWidth        = 0.0;
   veGateWidth        = 0.0;
   vcp                = 0;
   fixedAngle         = -1.0;
   nyquistVelocity    = 0.0;
   prf                = 0.0;
   prevSweep          = NULL;

   for( int i = 0; i < numIndeces; i++ ) {
      azimuthIdex[i] = MISSING_INDEX;
   }

   //
   // Note that if for some reason, the bad values for the
   // given data needs to be something other than zero, these
   // calls should probably be changed.
   //
   memset( (void *) timeData, 0, sizeof( double ) * MAX_RAYS );
   memset( (void *) azimuth, 0, sizeof( float ) * MAX_RAYS );
   memset( (void *) elevation, 0, sizeof( float ) * MAX_RAYS );
   
   memset( (void *) dz, 0, sizeof( short ) * MAX_RAYS * maxCells );
   memset( (void *) snr, 0, sizeof( short ) * MAX_RAYS * maxCells );
   memset( (void *) ve, 0, sizeof( short ) * MAX_RAYS * maxCells );
   memset( (void *) sw, 0, sizeof( short ) * MAX_RAYS * maxCells );
   memset( (void *) pr, 0, sizeof( short ) * MAX_RAYS * maxCells );
}


Status::info_t SweepData::setInfo( RIDDS_data_hdr* nexradData, 
                                   SweepData* pSweep ) 
{
   //
   // Determine the scale and bias factor for the velocity 
   // data
   //
   if ( nexradData->velocity_resolution == 2 ) {
      //
      // 0.5 m/s resolution which is the default
      //
      velScaleBiasFactor = 1.0;
   }
   else {
      //
      // 1.0 m/s resolution which is double the default
      //
      velScaleBiasFactor = 2.0;
   }

   //
   // Set information for this tilt
   //
   vcp              = nexradData->vol_coverage_pattern;
   fixedAngle       = (nexradData->elevation / 8.) * (180. / 4096.);
   nyquistVelocity  = ((float) nexradData->nyquist_vel) / 100;
   unambiguousRange = ((float) nexradData->unamb_range_x10) / 10;
   prf              = (float)(SPEED_OF_LIGHT / (2 * unambiguousRange * 1000));

   //
   // Check for tilts without range unfolding -- don't want to 
   // use these
   //
   if( vcp == 121 && !vcp121->useUnambRng( fixedAngle, unambiguousRange ) ) {
      skipSweep     = true;
      sweepComplete = true;
      return( Status::ALL_OK );
   }

   //
   // Decide what data we have in this sweep
   //
   int numGates = maxCells;
   
   if( nexradData->ref_data_playback > 0 &&
       nexradData->vel_data_playback <= 0 ) {
      scanType         = REFL_ONLY;
      dzGateWidth      = (float) nexradData->ref_gate_width;
      rangeToFirstGate = (float) nexradData->ref_gate1;
      numGates         = nexradData->ref_num_gates;
   }
   else if( nexradData->ref_data_playback <= 0 &&
            nexradData->vel_data_playback > 0 ) {
      scanType         = VEL_ONLY;
      veGateWidth      = (float) nexradData->vel_gate_width;
      rangeToFirstGate = (float) nexradData->vel_gate1;
      numGates         = nexradData->vel_num_gates;
   }
   else {
      scanType         = BOTH;
      dzGateWidth      = (float) nexradData->ref_gate_width;
      veGateWidth      = (float) nexradData->vel_gate_width;
      rangeToFirstGate = (float) nexradData->vel_gate1;
      numGates         = max( nexradData->ref_num_gates, 
                              nexradData->vel_num_gates );
   }

   if( numGates > maxCells ) {
      POSTMSG( WARNING, "Number of cells in data exceeds expected maximum." );
      POSTMSG( WARNING, "Cells will be truncated" );
   }

   //
   // Set the elevation index
   //
   elevIndex = nexradData->elev_num;

   //
   // Retain pointer to the previous sweep.
   //   Note that most of the time this will be NULL
   //
   prevSweep = pSweep;

   //
   // If the pointer to the previous sweep is not
   // NULL, set up the tables needed to merge that
   // sweep with the current one.
   //
   if( prevSweep ) {
      prevSweep->fillIndexArray();
   }

   return( Status::ALL_OK );
   
}

float SweepData::getSurPrf() 
{
   if( prevSweep ) {
      return( prevSweep->getPrf() );
   }
   
   return( -1.0 );
}

float SweepData::getCellSpacing() 
{
   if( scanType == REFL_ONLY ) {
      return( dzGateWidth );
   }
   
   return( veGateWidth );
}


Status::info_t SweepData::copyData( RIDDS_data_hdr* nexradData, 
                                    time_t beamTime ) 
{
   //
   // Don't do anything if we are skipping this sweep
   //
   if( skipSweep ) {
      return( Status::ALL_OK );
   }
   
   //
   // Ignore any beams beyond our maximum
   //
   if( (rayIdex > 0 && beamTime <= timeData[rayIdex-1] &&
        nexradData->millisecs_past_midnight <= latestMillisecs) || 
       maxRaysExceeded ) {
      return( Status::ALL_OK );
   }
   
   //
   // Set the data that does not depend on the sweep type
   //
   timeData[rayIdex]  = (double) beamTime;
   azimuth[rayIdex]   = (nexradData->azimuth/8.0)*(180.0/4096.0);
   elevation[rayIdex] = (nexradData->elevation/8.0)*(180.0/4096.0);

   //
   // Set the data that does depend on the sweep type
   //
   switch( scanType ) {
       case REFL_ONLY :
          setDbzData( nexradData, beamTime );
          break;
          
       case VEL_ONLY:
          setVelData( nexradData, beamTime );    
          break;
          
       case BOTH:
          setData( nexradData, beamTime );    
          break;

       case NONE:
          POSTMSG( ERROR, "Could not determine sweep type" );
          return( Status::FAILURE );
          break;
   }

   //
   // Print a summary if the user asked for it
   //
   if( printSummary ) {
      writeSummary( nexradData );
   }
   
   //
   // Set the ray and gate indeces for the next pass
   //
   rayIdex++;
   gateIdex = rayIdex * maxCells;
   numRays  = rayIdex;

   //
   // Set the latest milliseconds for the next pass
   //
   latestMillisecs = nexradData->millisecs_past_midnight;

   //
   // Make sure we don't exceed the memory we have
   // allocated in our arrays
   //
   if( rayIdex >= MAX_RAYS ) {
      POSTMSG( INFO, "Maximum number of rays exceeded in this sweep" );
      POSTMSG( INFO, "All further rays for this sweep will be ignored" );
      maxRaysExceeded = true;
   }
   
   return( Status::ALL_OK );
   
}

bool SweepData::sweepLost() 
{
   if( !combineSweeps || !prevSweep || prevSweep->getElevIndex() < 0 ) {
      return false;
   }
   
   return( !mergeDone );
}


void SweepData::setDbzData( RIDDS_data_hdr* nexradData, time_t beamTime ) 
{
   ui08 *dbzData = ((ui08*)nexradData + nexradData->ref_data_playback);

   correctData( dbzData, nexradData->ref_num_gates );
   storeData( dbzData, dz, nexradData->ref_num_gates );

   if( !combineSweeps ) {
      calcSnr( nexradData->ref_num_gates, dzGateWidth );
      sweepComplete = true;
   }

}


void SweepData::setVelData( RIDDS_data_hdr* nexradData, time_t beamTime ) 
{
   ui08 *velData = ((ui08*)nexradData + nexradData->vel_data_playback);
   ui08 *spwData = ((ui08*)nexradData + nexradData->sw_data_playback);
   
   correctData( velData, nexradData->vel_num_gates );
   correctData( spwData, nexradData->vel_num_gates );

   storeData( velData, ve, nexradData->vel_num_gates );
   storeData( spwData, sw, nexradData->vel_num_gates );
   
   if( combineSweeps && prevSweep ) {
      mergeDbz();
      calcSnr( maxCells, veGateWidth );
   }
   sweepComplete = true;
}


void SweepData::setData( RIDDS_data_hdr* nexradData, time_t beamTime ) 
{
   ui08 *dbzData = ((ui08*)nexradData + nexradData->ref_data_playback);
   ui08 *velData = ((ui08*)nexradData + nexradData->vel_data_playback);
   ui08 *spwData = ((ui08*)nexradData + nexradData->sw_data_playback);

   correctData( dbzData, nexradData->ref_num_gates );
   correctData( velData, nexradData->vel_num_gates );
   correctData( spwData, nexradData->vel_num_gates );

   duplicateDbz( dbzData, dzGateWidth, veGateWidth, 
                 nexradData->ref_num_gates );

   storeData( velData, ve, nexradData->vel_num_gates );
   storeData( spwData, sw, nexradData->vel_num_gates );

   calcSnr( maxCells, veGateWidth );
   calcPR( maxCells, veGateWidth );
   
   sweepComplete = true;
}


void SweepData::correctData( ui08* nexradData, int numGatesIn ) 
{
   int nGates = min( numGatesIn, maxCells );

   //
   // Correct the incoming nexrad data in place.
   // That is change the values in the given array
   // itself. This is why we use actual values here
   // of 0, 1 and 2. We do not use the constant
   // 'BAD' values above.
   //
   for( int i = 0; i < nGates; i++ ) {

      //
      // 0: below SNR, 1: ambiguous range
      //
      ui08 value = nexradData[i];
      if ( value == 0  ||  value == 1 ) {
         nexradData[i] = 0;
      }
      else {
         //
         // Subtract 2 to get the unscaled data value
         // since 0 and 1 were used for data quality indicators
         //
         nexradData[i] = (short) (value - 2);
      }
   }

}


void SweepData::storeData( ui08* nexradData, short* ncfData, int numGatesIn ) 
{
   int nGates = min( numGatesIn, maxCells );
   
   for( int i = 0; i < nGates; i++ ) {
      ncfData[gateIdex + i] = (short) nexradData[i];
   }
}

///////////////////////////////////////////////////////
// NOTE:  The code in this function is replicated in
// mergeDbz(), except that in mergeDbz() we are copying
// data from an array of type short rather than an
// array of type ui08 and that the input array consists
// of a sweep worth of rays rather than one ray.  In
// mergeDbz() we find the correct offset into the array
// and treat it like a single ray from then on out.
// Therefore, if changes need to be made to the 
// functionality of duplicateDbz(), it is most likely
// that corresponding changes will need to be made
// to mergeDbz()
///////////////////////////////////////////////////////
void SweepData::duplicateDbz( ui08* dbzData, float inGateWidth, 
                              float outGateWidth, int numGatesIn ) 
{
   int dupCount = (int) ( inGateWidth / outGateWidth );

   //
   // Loop through all the values in our input data buffer
   //
   int outIndex, inIndex;
   
   for( outIndex = gateIdex, inIndex = 0; 
        outIndex < gateIdex + maxCells  &&  inIndex < numGatesIn; 
        inIndex++ ) {

      for( int dupIndex = 0; dupIndex < dupCount; dupIndex++ ) {
         dz[outIndex] = (short) dbzData[inIndex];
         outIndex++;
      }
   }
}


void SweepData::calcSnr( int nGates, float gateWidth ) 
{
   //
   // Calculate snr for all gates for which
   // we have reflectivity
   //
   for( int i = 0; i < nGates; i++ ) {

      float radialDistance = (i * gateWidth + rangeToFirstGate) * M_TO_KM;

      if( dz[gateIdex+i] == DZ_BAD || radialDistance <= 0 ) {
        snr[gateIdex+i] = SNR_BAD;
        continue;
      }

      double dzVal  = dz[gateIdex+i] * DZ_SCALE + DZ_BIAS;
      double snrVal = dzVal + 20.0*log10(snrFactor/radialDistance);

      snr[gateIdex+i] = (short) ( (snrVal - snrBias) / snrScale + 0.5 );
   }

}

////////////////////////////////////////////////////////////
// Note: The power ratio for the tilts that will be combined
// is not caluclated using this method. The code for that
// calculation is in mergeDz. Beware that changes here may need
// to be replicated in mergeDz, although the computation is
// different, since snr is not available in that method.
//////////////////////////////////////////////////////////////
void SweepData::calcPR( int nGates, float gateWidth ) 
{

   int unambigRngOffset = 
      (int) ( unambiguousRange / ( veGateWidth * M_TO_KM ) + 0.5 );
   
   for( int i = 0; i < nGates; i++ ) {
      if( snr[gateIdex+i] == SNR_BAD ) {
         pr[gateIdex+i] = PR_BAD;
         continue;
      }
      
      int multBack    = -(i / unambigRngOffset);
      int multForward = (maxCells - i) / unambigRngOffset;

      double linSnrSum = 0.0;

      for( int mi = multBack; mi <= multForward; mi++ ) {

         if( mi == 0 ) 
            continue;

         int newIdex = i + mi * unambigRngOffset;

         if( snr[gateIdex+newIdex] == SNR_BAD ) {
            continue;
         }
               
         linSnrSum += pow(10.0, (double) (snr[gateIdex+newIdex] * 
                                       snrScale + snrBias) / 10.0 );
         
      }

      if( linSnrSum != 0 ) {
         double prVal = (double) (snr[gateIdex+i] * snrScale + snrBias) - 
            10.0*log10(linSnrSum);
      
         pr[gateIdex+i] = (short) (( prVal - prBias ) / prScale );
      }
      else {
         pr[gateIdex+i] = powerRatioDefault;
      }
      
   }
}

/////////////////////////////////////////////////////////
// NOTE: The power ratio computation done here is 
// different than that done for the higher tilts. Here
// The computation is performed before the duplication 
// occurs. For the higher tilts, the duplication is
// performed and then the power ratio computation is
// done
////////////////////////////////////////////////////////
void SweepData::mergeDbz() 
{
   //
   // If there is no previous sweep, we're done
   //
   if( !prevSweep ) 
     return;

   //
   // Make sure this is the correct sweep to
   // merge with
   //
   if( prevSweep->getElevIndex() != elevIndex - 1 ) {
      return;
   }

   //
   // Get the azimuth, elevation and time for the
   // current beam
   //
   float  currentAz   = azimuth[rayIdex];
   float  currentElev = elevation[rayIdex];
   double currentTime = timeData[rayIdex];
   
   //
   // Find the beam in the reflectivity data that is closest
   //
   int nearestRay = prevSweep->findNearest( currentAz, currentElev, 
                                            currentTime );

   //
   // Calculate the offset into the reflectivity and snr data
   // that corresponds to that beam
   //
   int offset = nearestRay * maxCells;

   //
   // Get the data from the previous sweep
   //
   float  prevGateWidth = prevSweep->getDzGateWidth();
   short *prevDz        = prevSweep->getDz();

   //
   // Calculate the unambiguous range offset
   //
   int unambigRngOffset = 
      (int) ( unambiguousRange / (prevGateWidth * M_TO_KM) + 0.5 );

   if( offset >= 0 ) {

      int dupCount = (int) ( prevGateWidth / veGateWidth );

      //
      // For each gate in the beam of the previous sweep...
      //
      int outIndex, inIndex, cellIndex;
      
      for( outIndex = gateIdex, inIndex = offset, cellIndex = 0; 
           outIndex < gateIdex + maxCells  &&  inIndex < offset + maxCells; 
           inIndex++, cellIndex++ ) {

         short prVal = 0;

         //
         // Do the power ratio calculation in this special case where
         // we are merging different tilts
         //
         if( prevDz[inIndex] != DZ_BAD ) {

            int multBack    = -(cellIndex / unambigRngOffset);
            int multForward = 
               (maxCells - 1 - cellIndex) / unambigRngOffset;

            double snrVal    = 0.0;
            double linSnrSum = 0.0;

            for( int mi = multBack; mi <= multForward; mi++ ) {

               int newIdex     = inIndex + mi * unambigRngOffset;
               int newCellIdex = cellIndex + mi * unambigRngOffset;

               double radialDistance = (newCellIdex * prevGateWidth + 
                                       rangeToFirstGate) * M_TO_KM;

               if( prevDz[newIdex] != DZ_BAD ) {
                  double dzVal = prevDz[newIdex] * DZ_SCALE + DZ_BIAS;

                  if( mi == 0 ) {
                     if( radialDistance <= 0 ) {
                        break;
                     }
                     
                     snrVal = dzVal + 20.0*log10(snrFactor/radialDistance);
                     continue;
                  }

                  if( radialDistance <= 0 ) {
                     continue;
                  }
               
                  float linSnrVal = pow(10.0, dzVal/10) * 
                     (snrFactor/radialDistance) * (snrFactor/radialDistance);

                  linSnrSum += linSnrVal;
               }
            }

            if( linSnrSum == 0 ) {
               prVal = powerRatioDefault;
            }
            else {
               prVal = (short) ( (snrVal - 10.0*log10(linSnrSum) - prBias ) / 
                                 prScale );
            }
            
         }

         //
         // Duplicate the data and put it into the reflectivity
         // and snr data of the current sweep
         //
         for( int dupIndex = 0; dupIndex < dupCount; dupIndex++ ) {
            dz[outIndex] = prevDz[inIndex];
            pr[outIndex] = prVal;
	    outIndex++;
         }
      }

   }

   mergeDone = true;
   
}


void SweepData::fillIndexArray() 
{
   PMU_auto_register( "Filling index array" );

   //
   // For each ray...
   //
   for( int i = 0; i < numRays; i++ ) {
      
      //
      // Find the 'index' into the index array for this
      // azimuth
      //   Note:  i            = index into input azimuth data array
      //          idex         = index into the array of azimuth indexes
      //          azimuthIdex  = array filled with indexes into
      //                         original azimuth data array
      //          azimuth      = array of original azimuth data
      //                         from radar stream
      //
      int idex = (int) ( azimuth[i] / DELTA_AZIMUTH + 0.5 );

      azimuthIdex[idex] = i;
      
      //
      // Fill in around this point in the index array
      //
      int startIdex = idex - (int) (azTolerance + 0.5);
      int endIdex   = idex + (int) (azTolerance + 0.5);

      //
      // If we are in the negative range for the start index,
      // that means that we should wrap around to 360 to check
      // for azimuths there.  Then we can go on from azimuth 0
      //
      if( startIdex < 0 ) {
         for( int j = numIndeces + startIdex; j < numIndeces; j++ ) {
            setIndex( i, j );
         }

         startIdex = 0;
      }
      
      //
      // If we are past the end of the array for the end index,
      // that means that we should wrap around to 0 to check for
      // azimuths there.  Then we can stop at the end of the array
      // for the next set of checking.
      //
      if( endIdex >= numIndeces ) {
         for( int j = 0; j <= endIdex - numIndeces; j++ ) {
            setIndex( i, j );
         }
         
         endIdex = numIndeces - 1;
      }
      
      //
      // Fill in all the values between the start and end index
      //
      for( int j = startIdex; j <= endIdex; j++ ) {
         setIndex( i, j );
      }
      
   }

}


void SweepData::setIndex( int i, int j ) 
{
   //
   // Get the index value at this place in the array
   //
   int currentAzIndex = azimuthIdex[j];

   //
   // If there isn't anything there already, fill it
   // with the current information
   //         
   if( currentAzIndex == MISSING_INDEX ) {
      azimuthIdex[j] = i;
   }

   //
   // If there is something already there, decide
   // which beam to use - use the closest to the
   // current azimuth
   //
   else {

      //
      // What is the azimuth value here?
      //
      float azValue = j * DELTA_AZIMUTH;
            
      //
      // Is it closer to the value that is already there
      // or the one coming in?  If it is closer to the one
      // coming in, replace the azimuthIdex value here.
      // Otherwise do nothing - it is fine the way it is.
      //
      if( fabs( azValue - azimuth[i] ) <
          fabs( azValue - azimuth[currentAzIndex] ) ) {
         azimuthIdex[j] = i;
      }
   }
   
}


int SweepData::findNearest( float currentAz, float currentElev,
                            double currentTime ) 
{
   //
   // Find the index of the azimuth
   //
   int currentAzIndex = (int) ( currentAz / DELTA_AZIMUTH + 0.5 );

   //
   // Find the index of the beam to use from the previous sweep
   //
   int nearIndex = azimuthIdex[currentAzIndex];

   //
   // Check index
   //
   if( nearIndex < 0 || nearIndex > numRays ) {
      return( -1 );
   }

   //
   // Check elevation
   //
   if( fabs( currentElev - elevation[nearIndex] ) > elevTolerance ) {
      return( -1 );
   }
      
   //
   // Check time
   //
   if( fabs( currentTime - timeData[nearIndex] ) > timeTolerance ) {
      return( -1 );
   }
      
   return( nearIndex );
    
}


void SweepData::writeSummary( RIDDS_data_hdr* nexradData )
{
   if ( summaryCount == 0 ) {

      fprintf( stderr, HDR );
      fprintf( stderr, "\n" );

      //
      // Parse the time of the beam 
      //  
      time_t beamTime = ((nexradData->julian_date - 1) * 86400 +
                         nexradData->millisecs_past_midnight / 1000);
      DateTime when( beamTime );

      //
      // Print info
      //
      fprintf( stderr, FMT,
               (long)   vcp,
               (long)   nexradData->radial_status,
               (double) elevation[rayIdex],
               (long)   nexradData->elev_num,
               (double) azimuth[rayIdex],
               (long)   nexradData->radial_num,
               (long)   nexradData->ref_num_gates,
               (long)   nexradData->vel_num_gates,
               (double)   nexradData->ref_gate_width,
               (double)   nexradData->vel_gate_width,
                        when.dtime() );

      fprintf(stderr, "\n");

      //
      // Reset the counter
      //
      summaryCount = summaryInterval;
   }

   //
   // Decrement the counter
   //
   summaryCount--;
}  
   









