
#include "FourDD.hh"
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

using namespace std;

// Assume that all data in the volumes has been scaled and bias applied
// including missingVal.  I.e. assume the data in the volumes is decoded.


//////////////////////////////////////////////////////////////////////////////
//
//  UW Radial Velocity Dealiasing Algorithm 
//  Four-Dimensional Dealiasing (4DD)
//
//  DESCRIPTION:
//     This algorithm unfolds a volume of single Doppler radial velocity data.
//  The algorithm uses a previously unfolded volume (or VAD if previous volume
//  is unavailable) and the previous elevation sweep to unfold some of gates 
//  in each sweep. Then, it spreads outward from the 'good' gates, completing
//  the unfolding using gate-to-gate continuity. Gates that still remain
//  unfolded are compared to an areal average of neighboring dealiased gates.
//  Isolated echoes that still remain uncorrected are dealiased against a VAD
//  (as a last resort).
//
//  DEVELOPER:
//	Curtis N. James     25 Jan 99
//      Modified by Sue Dettling 11 Nov 2001  
//      Modified by Brenda Javornik April 2019
// 
///////////////////////////////////////////////////////////////
//
// Constructor
//
FourDD::FourDD(bool debug,
               char *sounding_url,
               float sounding_look_back,
               float wind_alt_min,
               float wind_alt_max,
               float avg_wind_u,
               float avg_wind_v,
               bool prep,
               bool filt,
               float max_shear,
               int sign,
               int del_num_bins,
               bool dbz_rm_rv,
               float low_dbz,
               float high_dbz,
               float angle_variance,
               float comp_thresh,
               float comp_thresh2,
               float thresh,
               bool strict_first_pass,
               int max_count,
               float ck_val,
               int proximity,
               int min_good,
               float std_thresh)
{
  _debug = debug;
  _sounding_url = sounding_url;
  _sounding_look_back = sounding_look_back;
  _wind_alt_min = wind_alt_min;
  _wind_alt_max = wind_alt_max;
  _avg_wind_u = avg_wind_u;
  _avg_wind_v = avg_wind_v;
  _prep = prep;
  _filt = filt;
  _max_shear = max_shear;
  _sign = sign;
  _del_num_bins = del_num_bins;
  _dbz_rm_rv = dbz_rm_rv;
  _low_dbz = low_dbz;
  _high_dbz = high_dbz;
  _angle_variance = angle_variance;
  _comp_thresh = comp_thresh;
  _comp_thresh2 = comp_thresh2;
  _thresh = thresh;  // used in window 
  _strict_first_pass = strict_first_pass;
  _max_count = max_count;
  _ck_val = ck_val;
  _proximity = proximity;
  _min_good = min_good;
  _std_thresh = std_thresh;

  // TODO: validate paramters; 
  // del_num_bins must be >= 0 and < max_bins??

}

///////////////////////////////////////////////////////////////
//
// Destructor
//
FourDD::~FourDD()
{

}

float FourDD::getMissingValue(Volume *volume) {
  if (volume == NULL)
    throw std::invalid_argument("volume is NULL");
  return volume->h.missing; 
}

bool FourDD::_isMissing(float dataValue, float missingValue) {
  return fabs(dataValue - missingValue) < MISSING_THRESHOLD; 
}


float FourDD::getNyqVelocity(Volume *volume, int sweepIndex) {
  if (volume == NULL) 
    throw std::invalid_argument("volume is NULL");
  if ((sweepIndex < 0) || (sweepIndex >= volume->h.nsweeps)) 
    throw std::invalid_argument("sweepIndex outside range");
  return volume->sweep[sweepIndex]->ray[0]->h.nyq_vel;
}

float FourDD::getNyqInterval(float nyqVelocity) { // Volume *volume, int sweepIndex) {
  return 2.0 * nyqVelocity;
}

int FourDD::getNumBins(Volume *volume, size_t sweepIndex, size_t rayIndex) {
  if (volume == NULL) 
    throw std::invalid_argument("getNumBins: volume is NULL");
  if (sweepIndex >= volume->h.nsweeps) 
    throw std::invalid_argument("getNumBins: sweepIndex outside range");
  if (volume->sweep[sweepIndex] == NULL) 
    throw std::invalid_argument("getNumBins: volume->sweep[sweepIndex] is NULL");
  if (rayIndex >= volume->sweep[sweepIndex]->h.nrays) 
    throw std::invalid_argument("getNumBins: rayIndex outside range");
  return volume->sweep[sweepIndex]->ray[rayIndex]->h.nbins;
}

int FourDD::getNumRays(Volume *volume, int sweepIndex) {
  if (volume == NULL) 
    throw std::invalid_argument("volume is NULL");
  if ((sweepIndex < 0) || (sweepIndex >= volume->h.nsweeps)) 
    throw std::invalid_argument("sweepIndex outside range");
  return volume->sweep[sweepIndex]->h.nrays;
}

/*
defined in Rsl.hh
// return the maximum number of bins for this sweep
int FourDD::getMaxBins(Volume *volume, size_t sweepIndex) {
  // find the max number of bins for this sweep 
  if (volume == NULL) 
    throw std::invalid_argument("getMaxBins: volume is NULL");
  if (sweepIndex >= volume->h.nsweeps)
    throw std::invalid_argument("getMaxBins: sweepIndex outside range");
  return volume->sweep[sweepIndex]->h.maxBins;
}
*/
/* 

int FourDD::getMaxNumRays(Volume *volume, int sweepIndex) {
  if (volume == NULL) 
    throw std::invalid_argument("volume is NULL");
  if ((sweepIndex < 0) || (sweepIndex >= volume->h.nsweeps)) 
    throw std::invalid_argument("sweepIndex outside range");
  return volume->sweep[sweepIndex]->h.nrays;
}
*/




////////////////////////////////////////////////////////////////
//
// Dealias: This is the "main" routine for the 4DD algorithm.
//
//
int FourDD::Dealias(Volume *lastVelVol, Volume *currVelVol, Volume *currDbzVol,
		    Volume *soundVolume) 
{
  Volume* velVolCopy;
  //Volume* soundVolume;  
  unsigned short unfoldSuccess = 0;
  
  //
  // Check for reflectivity volume 
  //  
  if (_prep) {
      if(currDbzVol == NULL)
	printf("No DZ field available\n");
  }

  //
  // Check for current radial velocity data
  //
  if (currVelVol != NULL)  {
    //
    // Get a copy of the radial velocity, if unfolding fails we'll 
    // have the original data
    //
    velVolCopy = Rsl::copy_volume(currVelVol);
  } else {
    fprintf(stderr, "No Radial Velocity field available to unfold; aborting\n");
      
    exit(-1);
  }
    
  //
  // Create first guess field from VAD and put in soundVolume.
  //  
  //soundVolume = RSL_copy_volume(currVelVol);

  //firstGuess(soundVolume, velocityMissingValue, &firstGuessSuccess, volTime);

  //if (!firstGuessSuccess) {
    //
    // free memory for soundVolume
    //
    //RSL_free_volume( soundVolume);

  //    soundVolume = NULL;
  //}

  // the missing value for velocity and reflectivity have already been
  // set (and overriden as needed) when extracted from RadxVol into Volume structure. 
  float velocityMissingValue = getMissingValue(currVelVol);
  float dbzMissingValue = getMissingValue(currDbzVol);

  //
  // Unfold Volume if we have either a previous volume or VAD data
  //
  if ( soundVolume != NULL  || lastVelVol != NULL) { 
    //
    // Proceed with unfolding  
    //
    if(_debug)
      fprintf(stderr, "Dealiasing\n");

    //
    // Remove any bins where reflectivity is missing (if 
    // dbz_rm_rv == true or outside the interval 
    // _low_dbz <= dbz <=_high_dbz.
    //
    if (_prep) {

      prepVolume(currDbzVol, currVelVol, _del_num_bins, 
		 velocityMissingValue, dbzMissingValue,
                 _low_dbz, _high_dbz, _dbz_rm_rv);
    } 

    //
    // Finally, we call unfoldVolume, which unfolds the velocity field.
    // usuccess is 1 if unfolding has been  performed successfully.  
    //
      
    unfoldVolume(currVelVol, soundVolume, lastVelVol, _del_num_bins, velocityMissingValue, 
		 _filt, &unfoldSuccess);
  }
  
  if (!unfoldSuccess) {
    fprintf(stderr, "Velocity Volume was NOT unfolded!\n");
      
    //
    // In case we modified the current velocity volume, copy
    // the original data back into bins of the rays of each sweep of
    // current velocity volume.
    //
    int nSweeps = currVelVol->h.nsweeps;
  
    for (int i = 0; i < nSweeps;  i++) {
      int nRays = currVelVol->sweep[i]->h.nrays;
	 
      for (int j = 0; j < nRays ; j ++) {
	int nBins = currVelVol->sweep[i]->ray[j]->h.nbins;
	
	for (int k = 0; k < nBins  ; k++) { 
	  currVelVol->sweep[i]->ray[j]->range[k] = velVolCopy->sweep[i]->ray[j]->range[k];
	}
      }
    }
  } else {
    if(_debug)
      printf("Velocity Volume was unfolded.\n");
  }

  //
  // free memory
  //
  Rsl::free_volume( velVolCopy);

  /*
  if( firstGuessSuccess) {
    if (_output_soundVol) {
      //
      // Debug 
      //
      int nSweeps = currVelVol->h.nsweeps;
      int nRays = currVelVol->sweep[0]->h.nrays;
      int nBins = currVelVol->sweep[0]->ray[0]->h.nbins;
	  
      for (int i = 0; i < nSweeps;  i++)
	for (int j = 0; j < nRays ; j ++)
	  for (int k = 0; k < nBins  ; k++) { 
	    currVelVol->sweep[i]->ray[j]->range[k] = soundVolume->sweep[i]->ray[j]->range[k];
	  }
      fprintf(stderr, "\nREPLACED VELOCITY DATA WITH SOUNDING DATA!!\n");
    }
    RSL_free_volume( soundVolume);
      
  }
  */

  return 0;


} // FourDD  


////////////////////////////////////////////////////////////////////////////
//
//  METHOD: findRay
//
//  DESCRIPTION:
//      This routine finds the rayindex of the nearest ray in sweepIndex2 of 
//  rvVolume2 to sweepIndex1 in rvVolume1.
//
//  DEVELOPER:
//	Curtis N. James    1 Feb 1999
//
//
//
//
int FourDD::findRay (Volume* rvVolume1, Volume* rvVolume2, int sweepIndex1, int
     sweepIndex2, int rayIndex) {

  //     int numRays,
     int  rayIndex1;
     float az0, az1, diffaz;
     float spacing;
     short direction, lastdir;

     // validate arguments ...
     if ((rvVolume1 == NULL) || (rvVolume2 == NULL))  
       throw std::invalid_argument("Volume is NULL");
     if ((sweepIndex1 > rvVolume1->h.nsweeps) || (sweepIndex2 > rvVolume2->h.nsweeps))
       throw std::invalid_argument("sweepIndex exceeds number of sweeps in volume");
     if ((sweepIndex1 < 0) || (sweepIndex2 < 0)) 
       throw std::invalid_argument("sweepIndex < 0");

     int numRays1 = rvVolume1->sweep[sweepIndex1]->h.nrays;
     int numRays2 = rvVolume2->sweep[sweepIndex2]->h.nrays;
     // Q: is this necessary?
     //     if (numRays1 != numRays2)
     //  throw std::invalid_argument("number of rays must be the same in both volumes");
     if ((rayIndex < 0) || (rayIndex >= numRays1)) 
       throw std::invalid_argument("rayIndex out of bounds");
     // end of argument validation 

     // Take care of this stinkin' degenerate case ...
     if (numRays2 == 1)
       return 0;

     float targetAz = rvVolume1->sweep[sweepIndex1]->ray[rayIndex]->h.azimuth;
     float scale = 360.0/numRays2;
     float baseAz = rvVolume2->sweep[sweepIndex2]->ray[0]->h.azimuth;
     float diff = targetAz - baseAz;
     int sign = 1.0;
     if (diff < 0) sign = -1.0;
     // if sweeps in volume2 are NOT increasing, then flip the sign
     if ((baseAz - rvVolume2->sweep[sweepIndex2]->ray[1]->h.azimuth) > 0)
       sign = sign * -1.0;
     int nsteps = fabs(diff)/scale;
     int index = sign * nsteps;  // (sign * nsteps) % numRays2;
     if (index < 0) index = numRays2 + index;
     // search from here ... like Newton's method for root finding
     float minAzDiff;
     int minIndex = index;
     float currentAz = rvVolume2->sweep[sweepIndex2]->ray[index]->h.azimuth;
     //float base360 = 0.0; // keep track of when we cross the 360 -> 0 boundary
     // Need base360 to increase in order to exit the while loop.
     if (currentAz < targetAz) {
       // move until we cross targetAz in the rays
       minAzDiff = fabs(targetAz - currentAz);
       while (currentAz < targetAz) {
         //if (_debug) cout << "currentAz = " << currentAz << " targetAz = " << targetAz << endl;
	       index = (index + sign) % numRays2;
	       if (index < 0) {
           index = numRays2 + index;
         }
         float previousAz = currentAz;
	       currentAz = rvVolume2->sweep[sweepIndex2]->ray[index]->h.azimuth;
         if (currentAz < previousAz) {
           currentAz += 360.0;
         }
         float currentDiff = fabs(targetAz - currentAz);
         if (currentDiff < minAzDiff) {
	         minAzDiff = currentDiff;
	         minIndex = index;
	       }
       }
     } else {
       // move until we cross targetAz
       // move until we cross targetAz in the rays
       minAzDiff = fabs(targetAz - currentAz);
       while (currentAz > targetAz) {
         //if (_debug) cout << "currentAz = " << currentAz << " targetAz = " << targetAz << endl;
	       index = (index - sign) % numRays2;
	       if (index < 0) { 
           index = numRays2 + index;
         }
         float previousAz = currentAz;
	       currentAz = rvVolume2->sweep[sweepIndex2]->ray[index]->h.azimuth;
         if (currentAz > previousAz) {
           currentAz -= 360.0;
         }
         float currentDiff = fabs(targetAz - currentAz);
         if (currentDiff < minAzDiff) {
	         minAzDiff = currentDiff;
	         minIndex = index;
	       }
       }
     }
     if (_debug) cout << "found minIndex=" << minIndex << " az = " << rvVolume2->sweep[sweepIndex2]->ray[minIndex]->h.azimuth << endl;
     return minIndex;
     /*
     // the closest ray will be the estimated index or estimated index +/- 1
     float diff1 = fabs(rvVolume2->sweep[sweepIndex2]->ray[index]->h.azimuth - targetAz);
     int indexPlus1 = (index + 1) % numRays2;
     float diffPlus1 = fabs(rvVolume2->sweep[sweepIndex2]->ray[indexPlus1]->h.azimuth - targetAz);
     int indexMinus1 = (index - 1 + numRays2) % numRays2;
     float diffMinus1 = fabs(rvVolume2->sweep[sweepIndex2]->ray[indexMinus1]->h.azimuth - targetAz);
     if (diff1 < diffPlus1) {
       if (diff1 < diffMinus1) return index;
       else return indexMinus1;
     } else {
       if (diffPlus1 < diffMinus1) return indexPlus1;
       else return indexMinus1;
     }
     */
     /*     
     // TODO: numRays1 or numRays2?    HERE !!!!
     az0 = rvVolume1->sweep[sweepIndex1]->ray[rayIndex]->h.azimuth;
     if (rayIndex < numRays2) rayIndex1=rayIndex;
     else rayIndex1 = numRays2 - 1;
     az1 = rvVolume2->sweep[sweepIndex2]->ray[rayIndex1]->h.azimuth;
     if (az0 == az1) {
       return rayIndex1;
     } else {
       // We just want the ray that is closest; not the closest bin, so gate size 
       // is NOT important.
       // Estimate ray spacing ...
       // abs(last az - first az)/nrays
       // NOTE: numRays2 must be > 1 otherwise, set spacing to ??
       if (numRays2 > 1) {
         spacing = fabs(rvVolume2->sweep[sweepIndex2]->ray[0]->h.azimuth -
		      rvVolume2->sweep[sweepIndex2]->ray[numRays2-1]->h.azimuth);
         printf("findRay spacing %g, numRays2 = %d\n", spacing, numRays2);
         //       if (spacing > 180) spacing= 360.0 - spacing;
         spacing = spacing/(float) (numRays2 - 1);   // <--- this is off
       } else {
         spacing = 1.0;
       }
       printf("findRay spacing %g\n", spacing);

       // Compute the difference in azimuth between the two rays:  
       diffaz = az0 - az1;
       if (diffaz >= 180.0) diffaz = diffaz - 360.0;
       else if (diffaz < -180.0) diffaz = diffaz + 360.0;
       
       // Get close to the correct index:  
       rayIndex1 = rayIndex1 + (int) (diffaz/spacing);
       if ((rayIndex1 >= numRays2) || (rayIndex1 < 0)) rayIndex1 = rayIndex1 % numRays2;
       //if (rayIndex1<0) rayIndex1 = numRays2 + rayIndex1;
       printf("rayIndex1 = %d\n", rayIndex1);
       az1=rvVolume2->sweep[sweepIndex2]->ray[rayIndex1]->h.azimuth;
       diffaz = az0 - az1;
       if (diffaz >= 180.0) diffaz = diffaz - 360.0;
       else if (diffaz < -180.0) diffaz = diffaz + 360.0;

       // Now add or subtract indices until the nearest ray is found:  
       if (diffaz >= 0) lastdir = 1;
       else lastdir = -1;
       while (fabs(diffaz) > spacing/2.0) {
	 if (diffaz >= 0) {
	   rayIndex1++;
	   direction = 1;
	 } else {
	   rayIndex1--;
	   direction = -1;
	 }
	 if (rayIndex1 >= numRays2) rayIndex1 = rayIndex1 - numRays2;
	 if (rayIndex1 < 0) rayIndex1 = numRays2 + rayIndex1;
	 az1 = rvVolume2->sweep[sweepIndex2]->ray[rayIndex1]->h.azimuth;
	 diffaz = az0 - az1;
	 if (diffaz >= 180.0) diffaz = diffaz - 360.0;
	 else if (diffaz < -180.0) diffaz = diffaz + 360.0;
	 if (direction != lastdir) break;
	 else lastdir = direction;
       }
       return rayIndex1;
     }
     */
}

// If the values are already converted to float, then this is not needed
float revertScaleAndBias(float base, float scale, float bias) {
  float value;
  value = (base - bias) / scale;
  return value;
}

float applyScaleAndBias(float base, float scale, float bias) {
  float value;
  value = base * scale + bias;
  return value;
}

     
////////////////////////////////////////////////////////////////////////
//
//  METHOD: prepVolume
//
//  DESCRIPTION:
//      This routine thresholds VR bins where reflectivity is below
//      _low_dbz or missing (if _dbz_rm_rv==true).
//
//  dbz_rm_rv: If true, all radial velocity bins with dbz values missing will be deleted
//
//  DEVELOPER:
//	Curtis N. James 20 Mar 98
//      Modified by C. James 25 Jan 99
//
//
// 
//

// TODO: force same geometry ...
//void remapToPredomGeom();
//then 
//remapRangeGeom(double startRangeKm,
//	       double gateSpacingKm,
//	       bool interp = false);

void FourDD::prepVolume(Volume* DBZVolume, Volume* rvVolume, int del_num_bins,
			float velocityMissingValue, float dbzMissingValue,
                        float low_dbz, float high_dbz, bool dbz_rm_rv) {

  int currIndex, sweepIndex, i, j, DBZIndex, numRays, numBins, numDBZRays,
     numSweepsRV, numSweepsDZ;
  float  dzval; // removing this ...  minpossDBZ = -50.0;
  int DBZGateSize = 0, DBZfloatBin1 = 0;
  // int DBZfactor;  Command decision on DBZfactor: make sure the geometry of the DBZ and velocity volumes
  // are the same, then there is no need for DBZfactor.
  int limit;
  int rvGateSize = 0, rvfloatBin1 = 0;
  char errorMessage[1024];

  if ((DBZVolume == NULL) || (rvVolume == NULL)) return;
     
  numSweepsRV = rvVolume->h.nsweeps;
  numSweepsDZ = DBZVolume->h.nsweeps;
  if (numSweepsRV!=numSweepsDZ) {
    return;
  } 


  for (sweepIndex=0; sweepIndex<numSweepsRV; sweepIndex++) {
    numRays = rvVolume->sweep[sweepIndex]->h.nrays;
    numDBZRays = DBZVolume->sweep[sweepIndex]->h.nrays;
    //numDBZBins = DBZVolume->sweep[sweepIndex]->ray[0]->h.nbins; // <---- NOT all rays have the same number of bins!
     
    // Get the bin geometry for both DBZ and rv:  

    DBZGateSize=DBZVolume->sweep[sweepIndex]->ray[0]->h.gate_size;
    DBZfloatBin1=DBZVolume->sweep[sweepIndex]->ray[0]->h.range_bin1;
    rvGateSize=rvVolume->sweep[sweepIndex]->ray[0]->h.gate_size;
    rvfloatBin1=rvVolume->sweep[sweepIndex]->ray[0]->h.range_bin1;

    // Terminate routine if the locations of the first range bins
    // in DBZ and rv do not coincide.  
       
    if ( (DBZfloatBin1!=rvfloatBin1 || numDBZRays!=numRays) )
      return;

    // TODO: the gate size is a double in RadxRangeGeom.hh, but is int in Volume data model
    if (DBZGateSize != rvGateSize) {
      sprintf(errorMessage, "prepVolume: gate size DBZ %d must equal velocity gates size %d",
	      DBZGateSize, rvGateSize);
      throw std::invalid_argument(errorMessage);
    }
    
    // TODO: this is also done in InitialDealiasing,  Is it necessary here?
    // Erase the first del_num_bins bins of DBZ and radial velocity, as per
    //  communication with Peter Hildebrand.  
    // missing values may differ for DBZ and velocity
    // float dbzMissingValue = getMissingValue(DBZVolume);
    // float velocityMissingValue = getMissingValue(rvVolume);
    if (0) { /// del_num_bins > 0) {
      for (currIndex=0;currIndex<numRays;currIndex++) {
	numBins = rvVolume->sweep[sweepIndex]->ray[currIndex]->h.nbins; // <---- NOT all rays have the same number of bins!
	size_t last_bin = del_num_bins;
	if (del_num_bins > numBins) {
	  last_bin = numBins;
	}
        //printf("last_bin = %d\n", last_bin);
	for (i = 0; i < last_bin; i++) {	     
	  rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = velocityMissingValue;
	  DBZVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = dbzMissingValue;
	}
        printf("after zeroing ray %d ...\n", currIndex);
	Rsl::print_volume(rvVolume);
      }
    }
    // if  dbz_rm_rv is true ...	 
    // Now that we know the bin geometry, we assign missingVal to any
    // velocity bins where the reflectivity is missing 
    // or outside _low_dbz and _high_dbz.  
    //    limit = numBins;
    for (currIndex=0; currIndex<numRays; currIndex++) {
	numBins = rvVolume->sweep[sweepIndex]->ray[currIndex]->h.nbins; // <---- NOT all rays have the same number of bins!
	//size_t last_bin = del_num_bins;
	//if (del_num_bins > numBins) {
	//  last_bin = numBins;
	//}

	for (i=0; i < numBins; i++) {
          if (i < del_num_bins) {
	    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = velocityMissingValue;
	    DBZVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = dbzMissingValue;
	  } else {
	    float dbzValue = DBZVolume->sweep[sweepIndex]->ray[currIndex]->range[i]; 
	    if ((dbz_rm_rv) && _isMissing(dbzValue, dbzMissingValue)) {
	      rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = velocityMissingValue;
	    } else {
	      if ((dbzValue < low_dbz) || (dbzValue > high_dbz)) {
		rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = velocityMissingValue;
	      }
	    }
	  }
	} //end for i=del_num_bins
    } // end for each ray (currIndex=0) ...
  } //for each sweep
}


// return true if the value is missing

bool FourDD::_missing(Volume *original, size_t sweepIndex, size_t rayIndex, size_t rangeIndex,
		       float missingValue) {
  if (original == NULL)
    return true; // missingValue;
  if (sweepIndex >= original->h.nsweeps)
    return true; // missingValue;
  if (original->sweep[sweepIndex] == NULL)
    return true; // missingValue;
  if (rayIndex >= original->sweep[sweepIndex]->h.nrays)
    return true; // missingValue;
  if (original->sweep[sweepIndex]->ray == NULL)
    return true; // missingValue;
  if (rangeIndex >= original->sweep[sweepIndex]->ray[rayIndex]->h.nbins)
    return true; // missingValue;
  if (original->sweep[sweepIndex]->ray[rayIndex]->range == NULL)
    return true; // missingValue;
  return _isMissing(original->sweep[sweepIndex]->ray[rayIndex]->range[rangeIndex], missingValue);
}

//
// c. Filtering
//
// currIndex is ray index
// i is bin index
// 
short FourDD::Filter3x3(Volume *original, int i, int currIndex, int sweepIndex,
                        int del_num_bins) {

  //Perform a 3x3 filter, as proposed by Bergen & Albers 1988 
  int numberOfNonMissingNeighbors = 0;
  int left, right;
  int prev, next;

  if (original == NULL) 
    throw std::invalid_argument("original is NULL");

  if ((sweepIndex < 0) || (sweepIndex >= original->h.nsweeps)) 
    throw std::invalid_argument("sweepIndex outside range");
 
  int numRays = original->sweep[sweepIndex]->h.nrays;
    if ((currIndex < 0) || (currIndex >= numRays))
    throw std::invalid_argument("currIndex outside range");

  int numBins = original->sweep[sweepIndex]->ray[currIndex]->h.nbins;
  if ((i < 0) || (i >= numBins))
    throw std::invalid_argument("bin index, i,  outside range");


  if (currIndex==0) left=numRays-1;
  else left=currIndex-1;

  if (currIndex==numRays-1) right=0;
  else right=currIndex+1;

  next=i+1;
  prev=i-1;

  float missingVal = getMissingValue(original);

  // Look at all bins adjacent to current bin in question:  
  if (i > del_num_bins) {
    // TODO: RED FLAG!!! here, missingVal as float is compared with data value as short? or unscaled?
    // This is ok; we are assuming all range values are scaled as well as missingVal
    
    //if (!_isMissing(original->sweep[sweepIndex]->ray[left]->range[prev], missingVal)) {
    if (!_missing(original, sweepIndex, left, prev, missingVal)) {
      numberOfNonMissingNeighbors += 1;
    }
    //if (!_isMissing(original->sweep[sweepIndex]->ray[currIndex]->range[prev], missingVal)) {
    if (!_missing(original, sweepIndex, currIndex, prev, missingVal)) {
      numberOfNonMissingNeighbors += 1;
    }
    //    if (!_isMissing(original->sweep[sweepIndex]->ray[right]->range[prev], missingVal)) {
    if (!_missing(original, sweepIndex, right, prev, missingVal)) {
      numberOfNonMissingNeighbors += 1;
    }
  }

  //if (!_isMissing(original->sweep[sweepIndex]->ray[left]->range[i], missingVal)) {
  if (!_missing(original, sweepIndex, left, i, missingVal)) {
    numberOfNonMissingNeighbors += 1;
  }
  //if (!_isMissing(original->sweep[sweepIndex]->ray[right]->range[i], missingVal)) {
  if (!_missing(original, sweepIndex, right, i, missingVal)) {
    numberOfNonMissingNeighbors += 1;
  }

  if (i<numBins-1) {  
    // if (!_isMissing(original->sweep[sweepIndex]->ray[left]->range[next], missingVal)) {
    if (!_missing(original, sweepIndex, left, next, missingVal)) {
      numberOfNonMissingNeighbors += 1;
    }
    // if (!_isMissing(original->sweep[sweepIndex]->ray[currIndex]->range[next], missingVal)) {
    if (!_missing(original, sweepIndex, currIndex, next, missingVal)) {
      numberOfNonMissingNeighbors += 1;
    }
    // if (!_isMissing( original->sweep[sweepIndex]->ray[right]->range[next], missingVal)) {
    if (!_missing(original, sweepIndex, right, next, missingVal)) {
      numberOfNonMissingNeighbors += 1;
    }
  }

  if ( ((i == numBins-1 || i == del_num_bins) && numberOfNonMissingNeighbors >= 3)
        || (numberOfNonMissingNeighbors >= 5) ) {
    // Save the bin for dealiasing:  
    // STATE[i][currIndex]=TBD;
    return TBD;  
  } else {
    // Assign missing value to the current bin.  
    return MISSING;
  }
}

//
// d. Initial Dealiasing
//

// rvVolume (in/out) working volume; initial dealiased values on output
// rvVolume->h.missing  (in)
// rvVolume->sweep[sweepIndex]->ray[0]->h.nyq_vel;      (in)
// original     (in)  original values
// STATE    (out)    the state for each bin {TBD, MISSING, etc.}
void FourDD::InitialDealiasing(Volume *rvVolume, Volume *lastVolume, Volume *soundVolume,
			       Volume *original, float velocityMissingValue,
			       int sweepIndex, int del_num_bins, short **STATE,
			       bool filt, float fraction, float ck_val,
                               bool strict_first_pass, int max_count) {

  //  int flag=1;
  
  if (rvVolume == NULL) 
    throw std::invalid_argument("rvVolume is NULL");

  if ((sweepIndex < 0) || (sweepIndex >= rvVolume->h.nsweeps)) 
    throw std::invalid_argument("sweepIndex outside range");
 
  int numSweeps = rvVolume->h.nsweeps;
  int numRays = rvVolume->sweep[sweepIndex]->h.nrays;
  /*
    if ((currIndex < 0) || (currIndex >= numRays))
    throw std::invalid_argument("currIndex outside range");
  */
  
  int prevIndex = 0;
  int aboveIndex = 0;

  if (del_num_bins < 0)
    del_num_bins = 0;

  // for each ray ...
  for (int rayIndex=0; rayIndex<numRays; rayIndex++) {
    if (lastVolume!=NULL)
      prevIndex=findRay(rvVolume, lastVolume, sweepIndex, sweepIndex, rayIndex);

    // find the closest ray in the next sweep of the same volume
    if (sweepIndex < numSweeps-1)
      aboveIndex=findRay(rvVolume, rvVolume, sweepIndex, sweepIndex+1, rayIndex);

    size_t numBins = rvVolume->sweep[sweepIndex]->ray[rayIndex]->h.nbins;

    for (size_t i=del_num_bins; i < numBins; i++) {
      // Doesn't this overwrite any missingVal set in prepVolume? TODO: redundant code?
      // TODO: is prepVolume needed at all?
      // Initialize Output Sweep with missing values:              
      rvVolume->sweep[sweepIndex]->ray[rayIndex]->range[i] = velocityMissingValue;

      float startingValue = original->sweep[sweepIndex]->ray[rayIndex]->range[i];

      if (_isMissing(startingValue, velocityMissingValue)) {
	      STATE[i][rayIndex] = MISSING;
      } else {  // check the neighborhood for information
	      if (filt) {
	        STATE[i][rayIndex] = Filter3x3(original, i, rayIndex, sweepIndex,
                                         del_num_bins);
	    } else {
	      // If no filter is being applied save bin for dealiasing:
	      STATE[i][rayIndex] = TBD;
	    }
      
        if ((STATE[i][rayIndex] == TBD) &&  (fabs(startingValue) > ck_val)) {
            float unfoldedValue;
            bool successful;
	   
            float prevVal = velocityMissingValue;
            if (lastVolume != NULL) {
              // TODO: what if prevIndex is out of bounds?
              prevVal = lastVolume->sweep[sweepIndex]->ray[prevIndex]->range[i];
            }
            float soundVal = velocityMissingValue;
            if (soundVolume!=NULL && lastVolume == NULL) {
              soundVal = soundVolume->sweep[sweepIndex]->ray[rayIndex]->range[i];
            }
            // aboveIndex is the closest ray in the next higher sweep
            float aboveVal = velocityMissingValue;
            if (sweepIndex<numSweeps-1) {
              aboveVal = rvVolume->sweep[sweepIndex+1]->ray[aboveIndex]->range[i];
              //if (lastVolume != NULL && !_isMissing(aboveVal, velocityMissingValue))
	      //std::cerr << "aboveVal is NOT missing: sweep[" << sweepIndex << "]->ray[" << aboveIndex << "]->range[" << i <<
	      //    "]= " << aboveVal << std::endl;
            }
	    // NOTE: Special case to seed the subsequent data files ...
	    // if there is a previously dealiased volume, AND this is the top sweep, 
	    // use the t-1 velocity value as the above velocity value
	          if ((sweepIndex == numSweeps-1) && (lastVolume != NULL)) {
	            aboveVal = prevVal;
	          }
            // TODO: where do we want to get the NyqVelocity? from original? or rvVolume?
            float NyqVelocity = getNyqVelocity(rvVolume, sweepIndex);
            //float fractionNyqVelocity = fraction * NyqVelocity;
            TryToDealiasUsingVerticalAndTemporalContinuity(velocityMissingValue,
                                                           aboveVal, soundVal,
                                                           startingValue, prevVal, lastVolume==NULL,
                                                           fraction, NyqVelocity,
                                                           strict_first_pass,
                                                           max_count,
                                                           &unfoldedValue, &successful);
            if (successful) {
              rvVolume->sweep[sweepIndex]->ray[rayIndex]->range[i] = unfoldedValue;
              STATE[i][rayIndex] = DEALIASED;
              if (_debug) cout << "Dealiased sweep " << sweepIndex << 
                " ray az " << rvVolume->sweep[sweepIndex]->ray[rayIndex]->h.azimuth << " range " << i << " to " << unfoldedValue << endl;
            }
      
	      } //end if ( STATE[i][rayIndex]==TBD) && (startingValue > _ck_val)
      } // check neighborhood for information
    } // end for (i=del_num_bins;i<numBins;i++)
                                                                                  
  } //end for (rayIndex=0;rayIndex<numRays;rayIndex++)          
}


// NOTE: precondition startingValue != missingValue
// Send currentValue, previousValue, abValue, and soundValue
// then, no need for all the indexes, and Volumes?
void FourDD::TryToDealiasUsingVerticalAndTemporalContinuity(
							    float missingValue,
							    float abValue, float soundValue,
							    float startingValue, float prevValue,
							    bool lastVolumeIsNull,
							    float fraction, float NyqVelocity,
                                                            bool first_pass_only,
                                                            int max_count,
							    float *unfoldedValue, bool *successful) {

  //  float valcheck = startingValue;
  float cval;
  int dcase = 0;

  *unfoldedValue = missingValue;
  *successful = false;
		     
  //
  // Try to dealias the bin using vertical and temporal 
  //   continuity (this is initial dealiasing).  
  //

  // determine case ... 
  if (lastVolumeIsNull && 
      !_isMissing(soundValue, missingValue) && _isMissing(abValue, missingValue)) {
    cval = soundValue;
    dcase = 1;
  } else if (lastVolumeIsNull && 
	     !_isMissing(soundValue, missingValue) && !_isMissing(abValue, missingValue)) {
    cval = abValue;
    dcase = 2;
  } else if (!_isMissing(prevValue, missingValue) && 
	     !_isMissing(abValue, missingValue) && !first_pass_only) {	       
    cval = prevValue;
    dcase=3;
  } else if (first_pass_only && 
	     !_isMissing(prevValue ,missingValue) && !_isMissing(abValue, missingValue) &&
	     !_isMissing(soundValue, missingValue)) {
    cval = prevValue;
    dcase = 4;
  } else { 
    dcase = 0;
    cval = 0.0;
  }

  if (dcase>0) {
 
    float potentialUnfoldedValue = Unfold(startingValue, cval, max_count, NyqVelocity);
    //printf("case: %d potentialUnfoldedValue = %g\n", dcase, potentialUnfoldedValue);

    float diff = cval - potentialUnfoldedValue;
    float fractionNyqVelocity = fraction * NyqVelocity;

    float v1 = fabs(abValue-potentialUnfoldedValue); // < fractionNyqVelocity
	  float v2 = fabs(soundValue-potentialUnfoldedValue); //  < fractionNyqVelocity) {
        //printf("v1=%g v2=%g\n", v1, v2);


    bool good = false;
    if (diff < fractionNyqVelocity) { //  && fabs(valcheck)>_ck_val) { 
      switch(dcase) {
      case 1: 
	      good = true;
	      break;
      case 2: if (fabs(soundValue-potentialUnfoldedValue) < fractionNyqVelocity) {
	        good = true;
	      }
	      break;
      case 3: if (fabs(abValue-potentialUnfoldedValue) < fractionNyqVelocity) {
	        good = true;
	      }
	      break;
      case 4: 
        if (fabs(abValue-potentialUnfoldedValue) < fractionNyqVelocity
		     && fabs(soundValue-potentialUnfoldedValue) < fractionNyqVelocity) {
          // case:  strict_first_pass 
	        good = true;
	      }
	      break;
      default:
	      good = false;
      }
    }
    if (good) {
      *unfoldedValue = potentialUnfoldedValue;
      *successful = true;
    }
  } // end if (dcase > 0)
}

// check the neighborhood of this bin.
//
// returns:
//  numberOfDealiasedNeighbors - number of neighbors that have been dealiased successfully
//  numberOfTbdNeighbors       - number of neighbors that need to be dealiased
//  rayindex                   - list of neighboring rays that have been dealiased successfully
//  binindex                   - list of neighboring bins that have been dealiased successfully
//  flag                       - 0 no neighbors are dealiased; 1 if found a dealiased neighbor 
// Note: flag is only set to 1, if a DEALIASED neighbor is found, so just
// use the number of dealiased neighbors to determine the flag
/*
void FourDD::AssessNeighborhood(short **STATE, int currIndex, int i, int numRays,
     int numBins, 
     int *numberOfDealiasedNeighbors, int *numberOfTbdNeighbors,
     int *binindex, int *rayindex) {

  int nTbd = 0;
  int nDealiased = 0;

  int left, right;
  int prev, next;
		 
  if (currIndex==0) 
    left=numRays-1;
  else 
    left=currIndex-1;

  if (currIndex==numRays-1) 
    right=0;
  else 
    right=currIndex+1;

  next=i+1;
  prev=i-1;

  // Look at all bins adjacent to current bin in question:  
  if (i>_del_num_bins) {
    if (STATE[prev][left] == DEALIASED) {
      binindex[nDealiased]=prev;
      rayindex[nDealiased]=left;
      nDealiased=nDealiased+1;
      //if (_debug) printf("pl ");
    }
    if (STATE[prev][left] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[prev][currIndex] == DEALIASED) {
      binindex[nDealiased]=prev;
      rayindex[nDealiased]=currIndex;
      nDealiased=nDealiased+1;
      //if (_debug) printf("pc ");
    }
    if (STATE[prev][currIndex] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[prev][right] == DEALIASED) {
      binindex[nDealiased]=prev;
      rayindex[nDealiased]=right;
      nDealiased=nDealiased+1;
      //if (_debug) printf("pr ");
    }
    if (STATE[prev][right] == TBD) {
      nTbd=nTbd+1;
    }
  }
  if (STATE[i][left] == DEALIASED) {
    binindex[nDealiased]=i;
    rayindex[nDealiased]=left;
    nDealiased=nDealiased+1;
    //if (_debug) printf("il ");
  }
  if (STATE[i][left] == TBD) {
    nTbd=nTbd+1;
  }
  if (STATE[i][right] == DEALIASED) {
    binindex[nDealiased]=i;
    rayindex[nDealiased]=right;
    nDealiased=nDealiased+1;
    //if (_debug) printf("ir "); 	   
  }
  if (STATE[i][right] == TBD) {
    nTbd=nTbd+1;
  }
  if (i<numBins-1) {  
    if (STATE[next][left] == DEALIASED) {
      binindex[nDealiased]=next;
      rayindex[nDealiased]=left;
      nDealiased=nDealiased+1;
      //if (_debug) printf("nl "); 
    }
    if (STATE[next][left] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[next][currIndex] == DEALIASED) {
      binindex[nDealiased]=next;
      rayindex[nDealiased]=currIndex;
      nDealiased=nDealiased+1;
      //if (_debug) printf("nc ");
    }
    if (STATE[next][currIndex] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[next][right] == DEALIASED) {
      binindex[nDealiased]=next;
      rayindex[nDealiased]=right;
      nDealiased=nDealiased+1;
      //if (_debug) printf("nr ");
    }
    if (STATE[next][right] == TBD) {
      nTbd=nTbd+1;
    }
  }


  *numberOfDealiasedNeighbors = nDealiased;
  *numberOfTbdNeighbors = nTbd;
}

*/

// check the neighborhood of this bin.
//
// STATE[bin][ray]
//
// returns:
//  numberOfDealiasedNeighbors - number of neighbors that have been dealiased successfully
//  numberOfTbdNeighbors       - number of neighbors that need to be dealiased
//  rayindex                   - list of neighboring rays that have been dealiased successfully
//  binindex                   - list of neighboring bins that have been dealiased successfully
//  flag                       - 0 no neighbors are dealiased; 1 if found a dealiased neighbor 
// Note: flag is only set to 1, if a DEALIASED neighbor is found, so just
// use the number of dealiased neighbors to determine the flag
void FourDD::AssessNeighborhood2(short **STATE, Volume *rvVolume, int sweepIndex, int currRayIndex,
				 int currBinIndex,
                                 int del_num_bins, 
				 float foldedValue,
				 float pfraction, float NyqVelocity,
				 int *nWithinNyquist, int *nOutsideNyquist,
                                 int *nPositiveFolds, int *nNegativeFolds,
                                 bool *noHope) {

  int numberOfDealiasedNeighbors = 0;
  int numberOfTbdNeighbors = 0;
  int binindex[8];
  int rayindex[8];
  //  int numBins = getNumBins(rvVolume, sweepIndex);
  int numRays = getNumRays(rvVolume, sweepIndex);


  int nTbd = 0;
  int nDealiased = 0;

  int left, right;
  int prev, next;
		 
  if (currRayIndex==0) 
    left=numRays-1;
  else 
    left=currRayIndex-1;

  if (currRayIndex==numRays-1) 
    right=0;
  else 
    right=currRayIndex+1;

  next=currBinIndex+1;
  prev=currBinIndex-1;

  // Look at all bins adjacent to current bin in question:  
  if (currBinIndex > del_num_bins) { // if del_num_bins = 0, then currBinIndex has to be 1, then prev is in bounds
    if (STATE[prev][left] == DEALIASED) {
      binindex[nDealiased]=prev;
      rayindex[nDealiased]=left;
      nDealiased=nDealiased+1;
      //if (_debug) printf("pl ");
    }
    if (STATE[prev][left] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[prev][currRayIndex] == DEALIASED) {
      binindex[nDealiased]=prev;
      rayindex[nDealiased]=currRayIndex;
      nDealiased=nDealiased+1;
      //if (_debug) printf("pc ");
    }
    if (STATE[prev][currRayIndex] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[prev][right] == DEALIASED) {
      binindex[nDealiased]=prev;
      rayindex[nDealiased]=right;
      nDealiased=nDealiased+1;
      //if (_debug) printf("pr ");
    }
    if (STATE[prev][right] == TBD) {
      nTbd=nTbd+1;
    }
  }
  if (STATE[currBinIndex][left] == DEALIASED) {
    binindex[nDealiased]=currBinIndex;
    rayindex[nDealiased]=left;
    nDealiased=nDealiased+1;
    //if (_debug) printf("il ");
  }
  if (STATE[currBinIndex][left] == TBD) {
    nTbd=nTbd+1;
  }
  if (STATE[currBinIndex][right] == DEALIASED) {
    binindex[nDealiased]=currBinIndex;
    rayindex[nDealiased]=right;
    nDealiased=nDealiased+1;
    //if (_debug) printf("ir "); 	   
  }
  if (STATE[currBinIndex][right] == TBD) {
    nTbd=nTbd+1;
  }
  int numBins = getNumBins(rvVolume, sweepIndex, currRayIndex);
  if (currBinIndex < numBins-1) { // then next is within bounds  
    if (STATE[next][left] == DEALIASED) {
      binindex[nDealiased]=next;
      rayindex[nDealiased]=left;
      nDealiased=nDealiased+1;
      //if (_debug) printf("nl "); 
    }
    if (STATE[next][left] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[next][currRayIndex] == DEALIASED) {
      binindex[nDealiased]=next;
      rayindex[nDealiased]=currRayIndex;
      nDealiased=nDealiased+1;
      //if (_debug) printf("nc ");
    }
    if (STATE[next][currRayIndex] == TBD) {
      nTbd=nTbd+1;
    }
    if (STATE[next][right] == DEALIASED) {
      binindex[nDealiased]=next;
      rayindex[nDealiased]=right;
      nDealiased=nDealiased+1;
      //if (_debug) printf("nr ");
    }
    if (STATE[next][right] == TBD) {
      nTbd=nTbd+1;
    }
  }

  numberOfDealiasedNeighbors = nDealiased;
  numberOfTbdNeighbors = nTbd;

  // printf("numberTBD=%d  numberDEALIASED=%d\n", nTbd, nDealiased);

  int in=0;
  int out=0;
  int numneg=0;
  int numpos=0;
  float diffs[8];

  // now go through the neighbors that have been dealiased
  // and get their velocities
  for (int l=0; l<numberOfDealiasedNeighbors; l++) {
    int goodNeighborBin = binindex[l];
    int goodNeighborRay = rayindex[l];
    float neighborVal= rvVolume->sweep[sweepIndex]->ray[goodNeighborRay]->range[goodNeighborBin];
    diffs[l] = neighborVal - foldedValue;
    if ( fabs(diffs[l]) < pfraction*NyqVelocity) {
      in+=1;
    } else {
      out+=1;
      if (diffs[l] > NyqVelocity) {
        numpos=numpos+1;
      } else if (diffs[l] < -NyqVelocity) {
        numneg=numneg+1;
      }
    } // end else                                                                                               
  } // end  for (l=0;l<numberOfDealiasedNeighbors;l++)                                                          

  *nWithinNyquist = in;
  *nOutsideNyquist = out;
  *nPositiveFolds = numpos;
  *nNegativeFolds = numneg;
  if (numberOfTbdNeighbors + numberOfDealiasedNeighbors < 1)
    *noHope = true;
  else 
    *noHope = false;
}

/*

// Return tryAgainLater if ??? numpos+numneg < in+out - (numpos+numneg) && loopcount <=2
							       Volume *rvVolume,
							       int sweepIndex,
							       int *binindex;
							       int *rayindex;
							       int numberOfDealiasedNeighbors;
							       float pfraction,
							       float NyqVelocity,
							       float foldedVal,
							       short **STATE,
							       int loopcount,
							       bool relaxedTest
) {

  bool tryAgainLater = false;

  // Unfold against all adjacent values where STATE == DEALIASED
  int in=0;
  int out=0;
  int numneg=0;
  int numpos=0;
  int diffs[8];

  for (int l=0; l<numberOfDealiasedNeighbors; l++) {
    int goodNeighborBin = binindex[l];
    int goodNeighborRay = rayindex[l];
    float neighborVal= rvVolume->sweep[sweepIndex]->ray[goodNeighborRay]->range[goodNeighborBin];
    diffs[l] = neighborVal - foldedVal;
    if ( fabs(diffs[l]) < pfraction*NyqVelocity) {
      in+=1;
    } else {
      out+=1;
      if (diffs[l] > NyqVelocity) {   
	numpos=numpos+1;
      } else if (diffs[l] < -NyqVelocity) {
	numneg=numneg+1;
      }
    } // end else 
  } // end  for (l=0;l<numberOfDealiasedNeighbors;l++)

  float val = foldedVal;
  bool withinNyqVelocity = false;
  if (relaxedTest) 
    withinNyqVelocity = (in > out);
  else 
    withinNyqVelocity = (in > 0) && (out == 0);

  if (withinNyqVelocity) {
    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;	     
    STATE[i][currIndex] = DEALIASED;
    // we are done
  } else { // in == 0 || out != 0
    if ((numpos+numneg)<(in+out-(numpos+numneg))) {
      if (loopcount>2) {
	// Keep the value after two passes through data.  
	rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;
	STATE[i][currIndex] = DEALIASED;
      } else {
	tryAgainLater = true;
      }				     
      // end  ((numpos+numneg)<(in+out-(numpos+numneg))) 
    } else if (numpos>numneg) {
      val=val+NyqInterval;  // TODO: HERE !!!! val must get changed on each iteration through loop?
    } else if (numneg>numpos) {
      val=val-NyqInterval;
    } else {
      // Save bin for windowing if unsuccessful after
      // four passes:  
      if (loopcount>4) STATE[i][currIndex] = UNSUCCESSFUL; // TODO: or MISSING
    }
  } // end else (in == 0 || out != 0)
  return tryAgainLater;
}
*/

//
// e. Spatial dealiasing
//
// Now, unfold STATE=TBD bins assuming spatial continuity:  

void FourDD::UnfoldTbdBinsAssumingSpatialContinuity(short **STATE,
						    Volume *original, Volume *rvVolume,
						    size_t sweepIndex, int del_num_bins, 
                                                    float NyqVelocity, float pfraction,
                                                    int max_count) {

  if ((original == NULL) || (rvVolume == NULL))
    throw std::invalid_argument("original volume or rvVolume is NULL");

  if ((sweepIndex > original->h.nsweeps) || (sweepIndex > rvVolume->h.nsweeps))
    throw std::invalid_argument("sweepIndex out of bounds");

  //  sanity check the Nyquist velocity
  if (fabs(NyqVelocity*pfraction) < 1)
    throw std::invalid_argument("Nyquist velocity and pfraction too small");

  float missingVal = getMissingValue(original);
  //float NyqVelocity = getNyqVelocity(original, sweepIndex);
  float NyqInterval = getNyqInterval(NyqVelocity);

  int loopcount = 0;
  int flag = 1;
  int step = -1;
  int startindex, endindex;

  if (del_num_bins < 0)
    del_num_bins = 0;

  size_t maxBinsForSweep = Rsl::getMaxBinsInSweep(original, sweepIndex);
  //int numBins = getNumBins(original, sweepIndex);
  int numRays = getNumRays(original, sweepIndex);

  // exit the loop when no change has been made; or max number of loops?
  while (flag==1) {
    //printf("loopcount = %d ", loopcount);
    loopcount=loopcount+1;
    flag=0;
    if (step==1) {
      step=-1;
      startindex=numRays-1;
      endindex=-1;
    } else {
      step=1;
      startindex=0;
      endindex=numRays;
    }

    // Q: the number of bins varies for each ray; maybe just fill them with missing?
    //  and make the number of bins the same for each ray in a sweep?
    // can we reverse the order of this loop?
    for (size_t i=del_num_bins; i<maxBinsForSweep; i++) {
      for (int currIndex=startindex; currIndex!=endindex; currIndex=currIndex+step) {
		
	float val = Rsl::getValue(original, sweepIndex, currIndex, i);
      //float val = original->sweep[sweepIndex]->ray[currIndex]->range[i];
        // printf("working val = %g ... \n", val);
        if (!_isMissing(val, missingVal)) {
          int numtimes = 0;          // <<======
          while (STATE[i][currIndex] == TBD  && numtimes <= max_count) {
            numtimes = numtimes + 1; // <<====

            int in, out;
            int numpos, numneg;
            bool noHope = false;
            AssessNeighborhood2(STATE, rvVolume, sweepIndex, currIndex, i, 
                                del_num_bins, val, 
                                pfraction, NyqVelocity,
                                &in, &out, &numpos, &numneg, &noHope);

            int numberOfDealiasedNeighbors = in + out;
            //printf("number of dealiased neighbors = %d \n", numberOfDealiasedNeighbors);
            // Perform last step of Bergen and Albers filter:  
            //	  if (loopcount == 1 && numberOfTbdNeighbors+numberOfDealiasedNeighbors < 1)
            if (loopcount == 1 && noHope)
              STATE[i][currIndex] = MISSING; 

            // otherwise, try to unfold this value
            if (numberOfDealiasedNeighbors >= 1) {
              bool withinNyqVelocity = (in > 0) && (out == 0);
              if (withinNyqVelocity) {
                rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;
                if (_debug) cout << "Dealiased sweep " << sweepIndex << 
                  " ray az " <<  rvVolume->sweep[sweepIndex]->ray[currIndex]->h.azimuth << " range " << i << " to " << val << endl;
                STATE[i][currIndex] = DEALIASED;
                flag = 1;
              } else { // in == 0 || out != 0
                if ((numpos+numneg)<(in+out-(numpos+numneg))) {
                  if (loopcount>2) {
                    // Keep the value after two passes through data.
                    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;
                    STATE[i][currIndex] = DEALIASED;
                    if (_debug) cout << "Dealiased sweep " << sweepIndex << 
                      " ray " <<  rvVolume->sweep[sweepIndex]->ray[currIndex]->h.azimuth << " range " << i << " to " << val << endl;

                    flag = 1;
                  } 
                  // end  ((numpos+numneg)<(in+out-(numpos+numneg)))
                  // TODO: what use are these? as we get a new val each time through the loop??
                } else if (numpos>numneg) {
                  val=val+NyqInterval; 
                } else if (numneg>numpos) {
                  val=val-NyqInterval;
                } else {
                  // Save bin for windowing if unsuccessful after four passes:
                  if (loopcount>4) STATE[i][currIndex] = UNSUCCESSFUL;
                }
              } // end else (in == 0 || out != 0)
              //printf("numtimes = %d val = %g\n", numtimes, val);
              //} // end while still unfolding (STATE == TBD)
            } // end if (numberOfDealiasedNeighbors>=1)

          } // while (STATE[i][currIndex]== TBD) && numtime <= max_count
          if (numtimes > max_count) {
            // Remove bin:
            STATE[i][currIndex] = UNSUCCESSFUL;
          }
        } // end if val != missing

        // printf("flag = %d STATE[%1d][%1d] = %d\n", flag, i, currIndex, STATE[i][currIndex]);
      }// end for (currIndex= ...
    } // end for (i=del_num_bins;i<numBins;i++)
  } // while flag == 1
}

//
// f. Window dealiasing
//
void FourDD::UnfoldRemoteBinsOrUnsuccessfulBinsUsingWindow(short **STATE, Volume *rvVolume, Volume *original,
                                                           int sweepIndex, int del_num_bins,
							   float pfraction, int proximity,
							   int min_good,
                                                           float std_thresh, float NyqVelocity,
                                                           bool soundVolumeNull, bool lastVolumeNull) {

  
  // TODO: Q: Are these checks made when validating params?
  if (STATE == NULL)
    throw std::invalid_argument("STATE is NULL");
  if (rvVolume == NULL)
    throw std::invalid_argument("rvVolume is NULL");
  if (original == NULL)
    throw std::invalid_argument("original volume is NULL");
  if ((sweepIndex < 0) || (sweepIndex >= rvVolume->h.nsweeps) || (sweepIndex >= original->h.nsweeps))
    throw std::invalid_argument("sweepIndex must be >= 0 and less than number of sweeps");
  if (del_num_bins < 0)
    throw std::invalid_argument("del_num_bins must be >= 0");
  if (pfraction < 0)
    throw std::invalid_argument("pfraction must be >= 0");
  if (proximity < 0)
    throw std::invalid_argument("proximity must be >= 0");
  if (min_good < 0)
    throw std::invalid_argument("min_good must be >= 0");
  if (std_thresh < 0)
    throw std::invalid_argument("std_thresh  must be >= 0");
  
  size_t maxBinsForSweep = Rsl::getMaxBinsInSweep(original, sweepIndex);

  int numRays = getNumRays(rvVolume, sweepIndex);
  float missingVal = getMissingValue(rvVolume);
  
  for (size_t i=del_num_bins;i<maxBinsForSweep;i++) {
    for (int currIndex=0;currIndex<numRays;currIndex++) { 

      if (STATE[i][currIndex]==TBD || STATE[i][currIndex]==UNSUCCESSFUL) {
	float originalValue = original->sweep[sweepIndex]->ray[currIndex]->range[i];

	int startray = currIndex - proximity;
	int endray = currIndex + proximity;
	int firstbin = i - proximity;
	int lastbin = i + proximity;
	if (startray<0) startray=numRays+startray;
	if (endray>numRays-1) endray=endray-numRays;
	if (firstbin<0) firstbin=0;
	size_t numBins = getNumBins(rvVolume, sweepIndex, currIndex);
	if (lastbin>numBins-1) lastbin=numBins-1;

	bool success = false;
	float averageVelocity = window(rvVolume, sweepIndex, startray, endray, 
				     firstbin, lastbin, min_good, 
				     std_thresh, NyqVelocity,
				     missingVal, &success);

	// if the averageVelocity == missingVal, then expand the window
	// NOTE:  Here, should probably be && success ==> too many missing values found
	if (_isMissing(averageVelocity, missingVal) && success) {   
	  startray=currIndex-2 * proximity;
	  endray=currIndex+2 * proximity;
	  firstbin=i-2 * proximity;
	  lastbin=i+2 * proximity;
	  if (startray<0) startray=numRays+startray;
	  if (endray>numRays-1) endray=endray-numRays;
	  if (firstbin<0) firstbin=0;
	  if (lastbin>numBins-1) lastbin=numBins-1;
	  averageVelocity = window(rvVolume, sweepIndex, startray, endray, 
			       firstbin, lastbin, min_good,
			       std_thresh, NyqVelocity, 
			       missingVal, &success);
	}

	if (!_isMissing(averageVelocity, missingVal)) {
	  float winval = averageVelocity;			
          float unfoldedVal = Unfold(originalValue, winval, _max_count, NyqVelocity);
          float diff = fabs(winval - unfoldedVal);

	  if (diff<pfraction*NyqVelocity) {
	    // Return the value.  
	    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = unfoldedVal;
	    STATE[i][currIndex]=DEALIASED;
	  } else if (diff<(1.0 - (1.0 - pfraction)/2.0)*NyqVelocity) {
	    // If within relaxed threshold, then return value, but
	    //   do not use to dealias other bins.			     
	    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = unfoldedVal;
	    STATE[i][currIndex]=MISSING;		   
	  } else {
	    // Remove bin  
	    STATE[i][currIndex]=MISSING;
	  }
	} //end  if (winval!=missingVal) 
	else {// winval == missingVal) 
	    if (!success) {
	      // Remove bin; could not examine entire window
	      STATE[i][currIndex]=MISSING;
	    } else if (soundVolumeNull || lastVolumeNull) {
	      if (STATE[i][currIndex]==TBD ) {
		// Leave bin untouched.  
		float val = original->sweep[sweepIndex]->ray[currIndex]->range[i];				 
		rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;
		STATE[i][currIndex]=MISSING; // Don't use to unfold other bins 
	      } else {
		// Remove bin  
		STATE[i][currIndex]=MISSING;
	      }
	    } else if (STATE[i][currIndex]==TBD && 
		       !soundVolumeNull && !lastVolumeNull) {
	      // Leave STATE[i][currIndex]=TBD bins for a second pass.
	      // In the second pass, we repeat unfolding, except this
	      // time we use soundVolume for comparison instead of
	      // lastVolume.  
	    } else {
	      // Remove bin  
	      STATE[i][currIndex]=MISSING;
	    }
	  }// end else ( ie.winval ==missingVal) 
      } // end (STATE[i][currIndex]==TBD || STATE[i][currIndex]==UNSUCCESSFUL)
    } // for (currIndex=0;currIndex<numRays;currIndex++)
  } // end for (i=del_num_bins;i<numBins;i++)
}

//
// g. Auxiliary dealiasing
//
// NOTE: this method is untested by unit tests
void FourDD::SecondPassUsingSoundVolumeOnly(short **STATE, Volume *soundVolume, Volume *original, Volume *rvVolume, 
                                            int sweepIndex, int del_num_bins,
                                            float fraction2, float pfraction,
                                            float NyqVelocity, int max_count, float ck_val) {

  /*
         NyqVelocity = rvVolume->sweep[sweepIndex]->ray[0]->
           h.nyq_vel;
         NyqInterval = 2.0 * NyqVelocity;
         numRays = rvVolume->sweep[sweepIndex]->h.nrays;
         numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;
  */

  int numRays = rvVolume->sweep[sweepIndex]->h.nrays;
  //int numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;
  float missingVal = getMissingValue(rvVolume);
  float NyqInterval = getNyqInterval(NyqVelocity);

  // simply unfold each bin value ...
  // This is a repeat of previous method, only using soundVolume instead of previous Volume.

  for (int currIndex=0;currIndex<numRays;currIndex++) {		 
    int numBins = rvVolume->sweep[sweepIndex]->ray[currIndex]->h.nbins;
    for (int i=del_num_bins;i<numBins;i++) {
  
      if (STATE[i][currIndex]==TBD) {
	float val = original->sweep[sweepIndex]->ray[currIndex]->range[i];
	float valcheck=val;
	float soundVal = soundVolume->sweep[sweepIndex]->ray[currIndex]->range[i];
	        
	if (!_isMissing(soundVal, missingVal) && !_isMissing(val, missingVal)) {
	  float unfoldedVal = Unfold(val, soundVal, max_count, NyqVelocity);
	  float diff = soundVal - unfoldedVal;
	  if (diff < fraction2*NyqVelocity && fabs(valcheck) > ck_val) {
	    // Return the good value.
	    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = unfoldedVal;
	    STATE[i][currIndex]=DEALIASED;
	  }
	} // end if (soundval != missingVal && val != missingVal)
      } // end if (STATE[i][currIndex]==TBD) 

    } // end for (i=del_num_bins;i<numBins;i++) 
  } // end for (currIndex=0;currIndex<numRays;currIndex++) 
		   
  //
  // Now, try to unfold the rest of the STATE=TBD bins, assuming spatial
  //   continuity; i.e. consult the neighbors
  //

  int loopcount=0;
  int flag=1;
  int step;
  int startindex, endindex;
  // continue while progress is made; i.e. at least one new DEALIAS
  while (flag==1) {       // WHILE ...
    loopcount=loopcount+1;
    flag=0;

    // switch directions each time through loop
    if (step==1) {
      step=-1;
      startindex=numRays-1;
      endindex=-1;
    } else {
      step=1;
      startindex=0;
      endindex=numRays;
    }
    // TODO: rename currIndex ==> rayIndex
    // TODO: rename i ==> binIdx
    for (int currIndex=startindex;currIndex!=endindex;currIndex=currIndex+step) {
      int numBins = original->sweep[sweepIndex]->ray[currIndex]->h.nbins;

      for (int i=del_num_bins;i<numBins;i++) {
	if (STATE[i][currIndex]==TBD) {
	  float val = original->sweep[sweepIndex]->ray[currIndex]->range[i];

          if (!_isMissing(val, missingVal)) {
	    // valcheck=val;

            int in, out;
            int numpos, numneg;
            bool noHope = false;
            AssessNeighborhood2(STATE, rvVolume, sweepIndex, currIndex, i,
                                del_num_bins, val, 
                                pfraction, NyqVelocity,
                                &in, &out, &numpos, &numneg, &noHope);

            int numberOfDealiasedNeighbors = in + out;

            if (numberOfDealiasedNeighbors > 0) {
	      int attempts = 0;
	      bool tryAgainLater = false;
	      while ((STATE[i][currIndex]==TBD) && (attempts < max_count) && (!tryAgainLater)) {
		attempts += 1;
		//bool relaxedTest = true;
		// inside this loop, val is changed by NyqVelocity until close to neighbors
		// STATE of current val is also changed to MISSING/UNSUCCESSFUL, or DEALIASED

                if ((STATE[i][currIndex] == TBD) && (numberOfDealiasedNeighbors >= 1)) {
                  bool withinNyqVelocity = (in > 0) && (out == 0);
                  if (withinNyqVelocity) {
                    rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;
                    STATE[i][currIndex] = DEALIASED;
                    if (_debug) cout << "Dealiased sweep " << sweepIndex << 
                      " ray " << rvVolume->sweep[sweepIndex]->ray[currIndex]->h.azimuth << " range " << i << " to " << val << endl;
                  } else { // in == 0 || out != 0
                    // otherwise, try to unfold one NyqInterval
                    if ((numpos+numneg)<(in+out-(numpos+numneg))) {
                      if (loopcount>2) {
                        // Keep the value after two passes through data.
                        rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = val;
                        STATE[i][currIndex] = DEALIASED;
                        if (_debug) cout << "Dealiased sweep " << sweepIndex << 
                          " ray " << rvVolume->sweep[sweepIndex]->ray[currIndex]->h.azimuth << " range " << i << " to " << val << endl;
                      } else {
                        tryAgainLater = true;
                      }
                      // end  ((numpos+numneg)<(in+out-(numpos+numneg)))
                    } else if (numpos>numneg) {
                      val=val+NyqInterval; 
                    } else if (numneg>numpos) {
                      val=val-NyqInterval;
                    } else {
                      if (loopcount>4) STATE[i][currIndex] = MISSING;
                    }
                  } // end else (in == 0 || out != 0)       
                } // end if (STATE[i][currIndex] == TBD) &&  (numberOfDealiasedNeighbors>=1)

	      } //end while (STATE[i][currIndex]==TBD) && (attempts < _max_count)
	      if (STATE[i][currIndex] == DEALIASED) {
                flag = 1;
	      } 
	      if ((STATE[i][currIndex] == TBD) && !tryAgainLater) {
		// Remove bin: 
		STATE[i][currIndex] = MISSING;
	      } 
	    } // end if (numberOfDealiasedNeighbors>=1)
	  } // end if (val != missingVal)

	}// end  if (STATE[i][currIndex]==TBD)
      }// end for (i=del_num_bins;i<numBins;i++)
    } // end for (currIndex=0;currIndex<numRays;currIndex++) 
  } // end while (flag == 1)
}


//////////////////////////////////////////////////////////////////////////////
//
//  METHOD: unfoldVolume
//
//  DESCRIPTION:
//     This algorithm unfolds a volume of single Doppler radial velocity data.
//  The algorithm uses a previously unfolded volume (or VAD if previous volume
//  is unavailable) and the previous elevation sweep to unfold some of gates 
//  in each sweep. Then, it spreads outward from the 'good' gates, completing
//  the unfolding using gate-to-gate continuity. Gates that still remain
//  unfolded are compared to an areal average of neighboring dealiased gates.
//  Isolated echoes that still remain uncorrected are dealiased against a VAD
//  (as a last resort).
//
//  DEVELOPER:
//	Curtis N. James     25 Jan 99
//  This routine performs a preliminary unfold on the data using the bin in the
// next highest sweep (already unfolded), and the last unfolded volume. If the 
// last unfolded volume is not available, the VAD is used (or sounding if VAD
// not available). If this is successful, the bin is considered STATE. Every 
// other bin is set to STATE=0 or STATE=-1 (if the bin is missing or bad).
//
// Then, the algorithm scans azimuthally and radially. If the majority of the
// STATE=1 bins (up to 8) adjacent to a particular STATE=0 bin are within 
// _thresh*Vnyq it is saved and considered good as well. Otherwise, Nyquist 
// intervals are added/subtracted until the condition is met. If unsuccessful, 
// then STATE is set to -2 for that bin.
//
// When all STATE=0 bins that are next to STATE=1 bins have been examined, then
// STATE=0 and STATE=-2 bins are unfolded against a window of STATE=1 values. If
// there are too few STATE=1 values inside the window, then the data are
// unfolded against the VAD, if available.
// 
// PARAMETERS:
// rvVolume (in/out parameter): The radial velocity field to be unfolded.
// soundVolume: A first guess radial velocity field using VAD (or sounding) 
// lastVolume: The last radial velocity field unfolded (time of lastVolume 
//    should be within 10 minutes prior to rvVolume)
// missingVal: The value for missing radial velocity data.
// filt: A flag that specifies whether or not to use Bergen/Albers filter.
// success: flag indicating whether or not unfolding is possible.
// 

void FourDD::unfoldVolume(Volume* rvVolume, Volume* soundVolume, Volume* lastVolume,
                          int del_num_bins, float velocityMissingValue, unsigned short filt,
                          unsigned short* success) {

  int sweepIndex;
  size_t numSweeps;
  float NyqVelocity, NyqInterval, fraction;
  float fraction2;
  float pfraction; 
  Volume* original;

  int max_bins, max_rays;
  bool debug = false;
  Rsl::findMaxNBins(rvVolume, &max_bins, &max_rays, debug);
  short **STATE = new short*[max_bins];
  for(int i=0; i < max_bins; i++) 
    STATE[i] = new short[max_rays];

  numSweeps = (size_t) rvVolume->h.nsweeps;

  // VALS is supposed to be the encoded form of the Volume data; with separate scale & bias
  // original is now the untouched velocity values with scale and bias applied
  original=Rsl::copy_volume(rvVolume);

  // fraction = _comp_thresh is used in the first spatial & temporal dealiasing
  if (_comp_thresh>1.0 || _comp_thresh<=0.0 ) fraction=0.25;
  else fraction=_comp_thresh;

  // faction2 = _comp_thresh2 is used for dealiasing against VAD data
  if (_comp_thresh2>1.0 || _comp_thresh2<=0.0 ) fraction2=0.49; // changed to correspond with paramdef default
  else fraction2=_comp_thresh2;

  // pfraction = _thresh used for dealiasing against neighbors, or an averaged value of neighbors
  if (_thresh >1.0 || _thresh<=0.0 ) pfraction=0.4; // changed to correspond with paramdef default
  else pfraction=_thresh;

  if (soundVolume!=NULL || lastVolume!=NULL) {
    for (sweepIndex=numSweeps-1;sweepIndex>=0;sweepIndex--) {

      printf("Sweep: %d\n", sweepIndex);

      NyqVelocity = rvVolume->sweep[sweepIndex]->ray[0]->h.nyq_vel;
      // TODO: validate NyqVelocity is NOT TOO SMALL!

      NyqInterval = 2.0 * NyqVelocity;
      //numRays = rvVolume->sweep[sweepIndex]->h.nrays;
      //numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;

      // First, unfold bins where vertical and temporal continuity
      // produces the same value (i.e., bins where the radial velocity
      // agrees with both the previous sweep and previous volume within
      // a _comp_thresh of the Nyquist velocity). This produces a number
      // of good data points from which to begin unfolding assuming spatial
      // continuity.  
      printf("Initial Dealiasing ...\n");
      InitialDealiasing(rvVolume, lastVolume, soundVolume, original,
			velocityMissingValue,
                        sweepIndex, del_num_bins, STATE, filt, fraction,
                        _ck_val, _strict_first_pass, _max_count);
      printf("Unfold Assuming Spatial Continuity ...\n");
      // Now, unfold STATE=TBD bins assuming spatial continuity:  
      UnfoldTbdBinsAssumingSpatialContinuity(STATE, original, rvVolume, sweepIndex, 
                                             del_num_bins, 
                                             NyqVelocity, pfraction,
                                             _max_count);

      //
      // Unfold remote bins or those that were previously unsuccessful
      //   using a window with dimensions 2(_proximity)+1 x 2(_proximity)+1:
      //   if still no luck delete data (or unfold against VAD if _pass2).
      //
      printf("Unfold Using Window ...\n");
      UnfoldRemoteBinsOrUnsuccessfulBinsUsingWindow(STATE, rvVolume, original,
                                                    sweepIndex, del_num_bins,
                                                    pfraction, _proximity, _min_good, _std_thresh,
                                                    NyqVelocity,
                                                    soundVolume==NULL, lastVolume==NULL);
	 
      //
      // Beginning second pass through the data, this time using 
      //  soundVolume only:  
      //
      printf("Second Pass Using Sound Volume Only ...\n");
      if (lastVolume!=NULL && soundVolume!=NULL) {
        SecondPassUsingSoundVolumeOnly(STATE, soundVolume, original, rvVolume,
                                       sweepIndex, del_num_bins,
                                       fraction2, pfraction,
                                       NyqVelocity, _max_count, _ck_val);
      }

    } //end for (sweepIndex=numSweeps-1;sweepIndex>=0;sweepIndex--) 
 
    *success=1;

  } // end if (soundVolume!=NULL || lastVolume!=NULL)
  else  {
    printf("First guess not available.\n");
    *success=0;
  }

  //
  // free memory 
  //
  Rsl::free_volume(original);

  DestroySTATE(STATE, max_bins);

  return;
}

//////////////////////////////////////////////////////////////////////////
//
//  METHOD: window
//
//  DESCRIPTION:
//      This routine averages the values in a range and azimuth window of a
//      sweep and computes the standard deviation.
//      If the number of non-missing values >= min_good,
//         Returns the average. 
//         Returns success = true, if standard deviation is <= std_thresh * NyqVelocity
//      otherwise,
//         Returns missing value and success = true
//  DEVELOPER:
//	Curtis N. James    26 Jan 1999
//      Brenda Javornik    03 Mar 2020
//
//
//
float FourDD::window(Volume* rvVolume, int sweepIndex, int startray, 
		     int endray, size_t firstbin, size_t lastbin,
		     int min_good, float std_thresh, float NyqVelocity,
		     float missingVal, bool* success) {

     int num, currIndex, rangeIndex, numRays;
     float val, sum, sumsq, ave; // , NyqVelocity;
     
     float encodedVal, retVal;

     *success=false;
     //NyqVelocity = rvVolume->sweep[sweepIndex]->ray[0]->
     //	 h.nyq_vel;
     numRays = rvVolume->sweep[sweepIndex]->h.nrays;
     // TODO: here! numBins depends on which ray we are accessing
     //int numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;
     // float missingVal = getMissingValue(rvVolume);

     // Now, sum the data in the window region between startray, 
     //  endray, firstbin, lastbin.  
     float std=0.0;
     ave=0.0;
     num=0;
     sum=0.0;
     sumsq=0.0;
       
     // We don't know the numBins ahead of time since they can vary for each ray
     // just check the lower bound on the bin indexes
     //     if (firstbin>=numBins || lastbin>=numBins || firstbin<0 || lastbin<0)
     if (firstbin<0 || lastbin<0)
       return missingVal;

     if (startray>endray) {
	 for (currIndex=startray; currIndex<numRays; currIndex++) {
	   size_t nBinsInRay = rvVolume->sweep[sweepIndex]->ray[currIndex]->h.nbins;
           size_t endBin = min(lastbin, nBinsInRay-1);

	     for (rangeIndex=firstbin; rangeIndex<=endBin; rangeIndex++) {
	       
	       encodedVal = rvVolume->sweep[sweepIndex]->ray[currIndex]->range[rangeIndex];
	       
	       if (!_isMissing(encodedVal, missingVal)) {
		 val = encodedVal;
		   num=num+1;
		   sum=sum+val;
		   sumsq=sumsq+val*val;
	       }
	     }// end for rangeIndex
	 }// end for currIndex
 
	 for (currIndex=0; currIndex<=endray; currIndex++) {
	   size_t nBinsInRay = rvVolume->sweep[sweepIndex]->ray[currIndex]->h.nbins;
           size_t endBin = min(lastbin, nBinsInRay-1);

	     for (rangeIndex=firstbin; rangeIndex<=endBin; rangeIndex++) {
		 encodedVal = rvVolume->sweep[sweepIndex]->ray[currIndex]->range[rangeIndex];
		 
		 if (!_isMissing(encodedVal, missingVal)) {
		     val = encodedVal; 
		     num=num+1;
		     sum=sum+val;
		     sumsq=sumsq+val*val;
		 }
	     } // end  for (rangeIndex ...
	 } // end for (currIndex ...)
	 
     } else {
	 for (currIndex=startray; currIndex<=endray; currIndex++) { 
	   size_t nBinsInRay = rvVolume->sweep[sweepIndex]->ray[currIndex]->h.nbins;
           size_t endBin = min(lastbin, nBinsInRay-1);
	   for (rangeIndex=firstbin; rangeIndex<=endBin; rangeIndex++) {
		 encodedVal = rvVolume->sweep[sweepIndex]->ray[currIndex]->range[rangeIndex];

		 if (!_isMissing(encodedVal, missingVal))  {
		     val = encodedVal;
		     num=num+1;
		     sum=sum+val;
		     sumsq=sumsq+val*val;
		 }
	     }
	 }
     }

     if (num >= min_good) {
	 ave=sum/num;
	 retVal = ave;
	 
	 std=sqrt(fabs((sumsq-(sum*sum)/num)/(num-1)));
	 if (std <= std_thresh*NyqVelocity) 
	   *success = true;
     } else {
	 retVal = missingVal;
	 std=0.0;
	 *success = true;
     }

     return retVal; 
}

//
// Unfold value by +/- at most max_count NyqIntervals
//

float FourDD::Unfold(float foldedValue, float referenceValue,
 int max_count,  float NyqVelocity) {

  float cval = referenceValue;
  float val = foldedValue;

  float NyqInterval = getNyqInterval(NyqVelocity); 
  int direction = 1;
  float diff = cval-val;

  if (diff < 0.0) {
    diff = -diff;
    direction = -1;
  } //  else direction=1;

  int numtimes = 0;
  while (diff > 0.99999*NyqVelocity && numtimes < max_count) {
    val = val + NyqInterval*direction;
    numtimes = numtimes + 1;
    diff = cval-val;
    //printf("%d: val=%g diff=%g\n",  numtimes, val, diff);
    if (diff<0.0) {
      diff = -diff;
      direction = -1;
    } else direction = 1;
  }
  //printf("%g unfolded to %g\n", foldedValue, val);
  return val;
}

short **FourDD::CreateSTATE(Volume *rvVolume, short initialValue) {


  int max_bins, max_rays;
  bool debug = false;
  Rsl::findMaxNBins(rvVolume, &max_bins, &max_rays, debug);
  short **STATE = new short*[max_bins];
  for(int i=0; i < max_bins; i++) {
    STATE[i] = new short[max_rays];
    for (int j=0; j < max_rays; j++)
      STATE[i][j] = initialValue;
  }

  return STATE;
}

void FourDD::DestroySTATE(short **STATE, int nbins) {
  for(int i=0; i < nbins; i++) 
    delete[] STATE[i];
  delete[] STATE;
}

