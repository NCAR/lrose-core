/*
 *   Module: FourDD.hh
 *
 *   Author: Curtis James 
 *           Packaged into a C++ class by Sue Dettling with a few mods.
 *   
 *   Date:   11/13/2001
 *
 * Copyright Information: This software was developed by the 
 * Mesoscale Group; Department of Atmospheric
 * Sciences; University of Washington, Seattle, WA, USA.
 * Access and use of this software shall impose the following obligations on
 * the user.  The user is granted the right; without any fee or cost, to use,
 * copy; modify, alter, enhance, and distribute this software, and any
 * derivative works thereof; and its supporting documentation for any purpose
 * whatsoever; except commercial sales, provided that this entire notice appears
 * in all copies of the software; derivative works, and supporting 
 * documentation. Further; the user agrees to credit the University 
 * of Washington in any publications that result from the use of this 
 * software or in any software package that includes this software.  
 * The name University of Washington; however, may not be used in 
 * any advertising or publicity to endorse or promote any products or 
 * commercial entity unless specific written permission
 * is obtained from the University of Washington.  The user also understands
 * that the University of Washington is not obligated to provide the user with
 * any support; consulting, training, or assistance of any kind with regard to
 * the use; operation, and performance of this software nor to provide the user
 * with any updates; revisions, new versions, or "bug fixes."
 *  
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY OF WASHINGTON "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES; INCLUDING BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MECHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR
 * ANY SPECIAL; INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER,
 * INCLUDING BUT NOT LIMITED TO CLAIMS ASSOCIATED WITH THE LOSS OF DATA OR
 * PROFITS; WHICH MAY RESULT FROM AN ACTION IN CONTRACT, NEGLIGENCE, OR OTHER
 * TORTIOUS CLAIM THAT ARISES OUT OF OR IN CONNECTION WITH THE ACCESS; USE, OR
 * PREFORMANCE OF THIS SOFTWARE.
 *
 * 
 *   UW Radial Velocity Unfolding Algorithm
 *   Four-Dimensional Dealiasing (4DD)
 *
 *   DESCRIPTION:
 *     This algorithm unfolds a volume of single Doppler radial velocity data.
 *   The algorithm uses a previously unfolded volume (or VAD if previous volume
 *   is unavailable) and a higher elevation sweep to unfold some of the bins
 *   in each sweep. Then; it completes the unfolding, assuming spatial
 *   continuity around each bin.
 *
 */


#ifndef FOURDD_HH
#define FOURDD_HH

//
// Radius of earth in kilometers. 
// 
#define A 6372.0 

#define PI 3.1415927 

#include <cstdlib>
#include <math.h>
//#include <Spdb/SoundingGet.hh>
#include "Rsl.hh"
//#include <toolsa/DateTime.hh>
//#include <toolsa/mem.h>
//#include "Params.hh"
using namespace std;

class FourDD 
{
public:
  
  FourDD(bool debug = true,
	 char *sounding_url = NULL,
	 float sounding_look_back = 30,
	 float wind_alt_min = 0.0,
	 float wind_alt_max = 5.0,
	 float avg_wind_u = 0.0,
	 float avg_wind_v = 0.0,
	 bool prep = false,
	 bool filt = false,
	 bool output_soundVol = false,
	 float max_shear = .05,
	 int sign = -1,
	 int del_num_bins = 0,
	 bool no_dbz_rm_rv = false,
	 float low_dbz = 0.0,
	 float high_dbz = 80.0,
	 float angle_variance = 0.10,
	 float comp_thresh = .25,
	 float comp_thresh2 = .49,
         float thresh = .4,
	 bool strict_first_pass = false,
	 int max_count = 10,
	 float ck_val = 1.0,
	 int proximity = 6,
	 int min_good = 5,
	 float std_thresh = .8);

  ~FourDD();
  
  int Dealias(Volume *lastVelVol, Volume *currVelVol, Volume *currDbzVol, Volume *soundVolume);
	      //      time_t volTime);
 
  //  void firstGuess(Volume* soundVolume, float missingVal,short unsigned int *,time_t volTime);

  void unfoldVolume(Volume* rvVolume, Volume* soundVolume, Volume* lastVolume,
		    int del_num_bins, float missingVal, unsigned short rm,
                    unsigned short* success);

  float window(Volume* rvVolume, int sweepIndex, int startray, int endray,
               int firstbin, int lastbin, float std_thresh,
               bool* success);

  void prepVolume (Volume* DBZVolume, Volume* rvVolume, int del_num_bins, float missingVal);

  int findRay (Volume* rvVolume1, Volume* rvVolume2, int sweepIndex1, int 
	       sweepIndex2, int currIndex1);

  float previousVal (Volume* rvVolume, Volume* lastVolume, int sweepIndex, int
		     currIndex, int rangeIndex, float missingVal);

  int loadSoundingData(time_t issueTime);

  float getMissingValue(Volume *volume);

  float getNyqVelocity(Volume *volume, int sweepIndex);

  float getNyqInterval(float nyqVelocity);

  int getNumBins(Volume *volume, int sweepIndex);
  int getNumRays(Volume *volume, int sweepIndex);

  short Filter3x3(Volume *VALS, int i, int currIndex, int sweepIndex,
                  int del_num_bins);

  void InitialDealiasing(Volume *rvVolume, Volume *lastVolume, Volume *soundVolume, Volume *original,
			 int sweepIndex, int del_num_bins, short **STATE, bool filt, float fraction);

  void TryToDealiasUsingVerticalAndTemporalContinuity(float missingVal,
						      float abVal, float soundVal, 
						      float startingVal, float prevVal, bool lastVolumeIsNull,
						      float fraction, float NyqVelocity,
						     float *unfoldedValue, bool *successful);

  /*  void AssessNeighborhood(short **STATE, int currIndex, int i, int numRays, int numBins,
			  int *numberOfDealiasedNeighbors, int *numberOfTbdNeighbors,
			  int *binindex, int *rayindex);
  */

  void AssessNeighborhood2(short **STATE, Volume *rvVolume, int sweepIndex, int currIndex, int i,
                           int del_num_bins, float foldedValue,
                           float pfraction, float NyqVelocity,
                           int *nWithinNyquist, int *nOutsideNyquist,
                           int *nPositiveFolds, int *nNegativeFolds,
                           bool *noHope);

  void UnfoldTbdBinsAssumingSpatialContinuity(short **STATE,
					      Volume *original, Volume *rvVolume,
					      int sweepIndex, int del_num_bins, float pfraction); 
  // , float missingVal);

  void UnfoldRemoteBinsOrUnsuccessfulBinsUsingWindow(short **STATE, Volume *rvVolume, Volume *original,
                                                     int sweepIndex, int del_num_bins,
                                                     float pfraction, int proximity,
                                                     float std_thresh, float NyqVelocity,
                                                     bool soundVolumeNull, bool lastVolumeNull);

  void SecondPassUsingSoundVolumeOnly(short **STATE, Volume *soundVolume, Volume *original, Volume *rvVolume,
                                      int sweepIndex, int del_num_bins,
                                      float fraction2, float pfraction,
                                      float NyqVelocity, int max_count, float ck_val);

  float Unfold(float foldedValue, float referenceValue,
	       int max_count, float NyqVelocity);

  short **CreateSTATE(Volume *rvVolume, short initialValue = TBD);
  void  DestroySTATE(short **STATE, int nbins);

  const static short UNSUCCESSFUL = -2;
  const static short MISSING      = -1;
  const static short TBD          =  0; // To Be Dealiased
  const static short DEALIASED    =  1;


private:


  //  SoundingGet       sounding;

  bool   _debug;
  char*  _sounding_url;
  float  _sounding_look_back;
  float  _wind_alt_min;
  float  _wind_alt_max;
  float  _avg_wind_u;
  float  _avg_wind_v;
  bool   _prep;
  bool   _filt;
  bool   _output_soundVol;
  float  _max_shear;
  int    _sign;
  int    _del_num_bins;
  bool   _no_dbz_rm_rv;
  float  _low_dbz;
  float  _high_dbz;
  float  _angle_variance;
  float  _comp_thresh;
  float  _comp_thresh2;
  float  _thresh;
  bool   _strict_first_pass;
  int    _max_count;
  float  _ck_val;
  int    _proximity;
  int    _min_good;
  float  _std_thresh;

  float  _epsilon;
  float  _missingVal;

  bool _isMissing(float value);

};

#endif // FOURDD_HH
