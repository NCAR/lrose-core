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
/*
 *Module: FourDD.cc
 *
 * Author: Curtis James' Dealiaser functions
 *
 * Date:   11/12/01
 *
 * Description:
 *     
 */

// Include files 

#include "FourDD.hh"
using namespace std;

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
// 
///////////////////////////////////////////////////////////////
//
// Constructor
//
FourDD::FourDD(Params &parameters)
{
  params = parameters;

  //
  // Initialize the sounding
  // We do this even if the url is unspecified, so that we can fall back
  // on the avg sounding wind if all other wind input fails
  //
  if (params.debug)
    fprintf(stderr, "Initializing sounding at %s\n", params.sounding_url );
  
  sounding.init( params.sounding_url,
                  (time_t)params.sounding_look_back*60,
                  params.wind_alt_min*1000,
                  params.wind_alt_max*1000,
                  params.avg_wind_u,
                  params.avg_wind_v );

}

///////////////////////////////////////////////////////////////
//
// Destructor
//
FourDD::~FourDD()
{

}

////////////////////////////////////////////////////////////////
//
// Dealias: This is the "main" routine for the 4DD algorithm.
//
//
int FourDD::Dealias(Volume *lastVelVol, Volume *currVelVol, Volume *currDbzVol,
		    time_t volTime) 
{
  Volume* velVolCopy;
  Volume* soundVolume;  
  unsigned short nolast = 0, firstGuessSuccess = 0, unfoldSuccess = 0, prep;
  int numSweepsLast;
  int numSweepsCurrent;

  
  //
  // Check for reflectivity volume 
  //  
  if (params.prep)
    {
      if(currDbzVol == NULL)
	printf("No DZ field available\n");
      prep = 0;
    }

  //
  // Check for current radial velocity data
  //
  if (currVelVol != NULL)  
    {
      numSweepsCurrent = currVelVol->h.nsweeps;

      //
      // Get a copy of the radial velocity, if unfolding fails we'll 
      // have the original data
      //
      velVolCopy = RSL_copy_volume(currVelVol);
    } 
  else 
    {
      fprintf(stderr, "No Radial Velocity field available to unfold; aborting\n");
      
      exit(-1);
    }
  
  //
  // Check for previous radial velocity volume
  //
  if (lastVelVol != NULL) 
    { 
      numSweepsLast = lastVelVol->h.nsweeps;
    } 
  else
    {
      nolast = 1;
    }
  
  //
  // Create first guess field from VAD and put in soundVolume.
  //  
  soundVolume = RSL_copy_volume(currVelVol);

  firstGuess(soundVolume, params.missing_vel, &firstGuessSuccess, volTime);

  if (!firstGuessSuccess) 
    {
      //
      // free memory for soundVolume
      //
      RSL_free_volume( soundVolume);

      soundVolume = NULL;
    }

  //
  // Unfold Volume if we have either a previous volume or VAD data
  //
  if ( firstGuessSuccess  || lastVelVol != NULL) 
    { 
      //
      // Proceed with unfolding  
      //
      if(params.debug)
	fprintf(stderr, "Dealiasing\n");

      //
      // Remove any bins where reflectivity is missing (if 
      // params.no_dbz_rm_rv == 1 or ouside the interval 
      // params.low_dbz <= dbz <=params.high_dbz.
      //
      if (params.prep) 
	//prepVolume(currDbzVol, currVelVol, params.missing_vel);
	prepVolume(currDbzVol, currVelVol,currVelVol->sweep[0]->ray[0]->h.bias);
      //
      // Finally, we call unfoldVolume, which unfolds the velocity field.
      // usuccess is 1 if unfolding has been  performed successfully.  
      //
      
      unfoldVolume(currVelVol, soundVolume, lastVelVol, currVelVol->sweep[0]->ray[0]->h.bias, 
		   params.filt, &unfoldSuccess);
    }
  
  if (!unfoldSuccess) 
    {
      fprintf(stderr, "Velocity Volume was NOT unfolded!\n");
      
      //
      // In case we modified the current velocity volume, copy
      // the original data back into bins of the rays of each sweep of
      // current velocity volume.
      //
      int nSweeps = currVelVol->h.nsweeps;
      int nRays = currVelVol->sweep[0]->h.nrays;
      int nBins = currVelVol->sweep[0]->ray[0]->h.nbins;

      for (int i = 0; i < nSweeps;  i++)
	for (int j = 0; j < nRays ; j ++)
	  for (int k = 0; k < nBins  ; k++)
	    { 
	      currVelVol->sweep[i]->ray[j]->range[k] = velVolCopy->sweep[i]->ray[j]->range[k];
	    }
    }
  else 
    {
      if(params.debug)
	printf("Velocity Volume was unfolded.\n");
    }

  //
  // free memory
  //
  RSL_free_volume( velVolCopy);

  if( firstGuessSuccess)
    RSL_free_volume( soundVolume);
		  
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
     sweepIndex2, int currIndex, float missingVal) {

     int numRays, rayIndex1;
     float az0, az1, diffaz;
     float spacing;
     short direction, lastdir;
     
     numRays = rvVolume2->sweep[sweepIndex2]->h.nrays;

     az0 = rvVolume1->sweep[sweepIndex1]->ray[currIndex]->h.azimuth;
     if (currIndex<numRays) rayIndex1=currIndex;
     else rayIndex1=numRays-1;
     az1 = rvVolume2->sweep[sweepIndex2]->ray[rayIndex1]->h.azimuth;
     if (az0==az1) {
       return rayIndex1;
     } else {
       // Since the beamwidth is not necessarily the spacing between rays:  
       spacing = fabs(rvVolume2->sweep[sweepIndex2]->ray[0]->h.azimuth-
		      rvVolume2->sweep[sweepIndex2]->ray[50]->h.azimuth); 
       if (spacing>180) spacing=360.0-spacing;
       spacing=spacing/50.0;

       // Compute the difference in azimuth between the two rays:  
       diffaz=az0-az1;
       if (diffaz>=180.0) diffaz=diffaz-360.0;
       else if (diffaz<-180.0) diffaz=diffaz+360.0;
       
       // Get close to the correct index:  
       rayIndex1=rayIndex1+(int) (diffaz/spacing);
       if (rayIndex1>=numRays) rayIndex1=rayIndex1-numRays;
       if (rayIndex1<0) rayIndex1=numRays+rayIndex1;
       az1=rvVolume2->sweep[sweepIndex2]->ray[rayIndex1]->h.azimuth;
       diffaz=az0-az1;
       if (diffaz>=180.0) diffaz=diffaz-360.0;
       else if (diffaz<-180.0) diffaz=diffaz+360.0;

       // Now add or subtract indices until the nearest ray is found:  
       if (diffaz>=0) lastdir=1;
       else lastdir=-1;
       while (fabs(diffaz)>spacing/2.0) {
	 if (diffaz>=0) {
	   rayIndex1++;
	   direction=1;
	 } else {
	   rayIndex1--;
	   direction=-1;
	 }
	 if (rayIndex1>=numRays) rayIndex1=rayIndex1-numRays;
	 if (rayIndex1<0) rayIndex1=numRays+rayIndex1;
	 az1=rvVolume2->sweep[sweepIndex2]->ray[rayIndex1]->h.azimuth;
	 diffaz=az0-az1;
	 if (diffaz>=180.0) diffaz=diffaz-360.0;
	 else if (diffaz<-180.0) diffaz=diffaz+360.0;
	 if (direction!=lastdir) break;
	 else lastdir=direction;
       }
       return rayIndex1;
     }
}

/////////////////////////////////////////////////////////////////////////
//
//  METHOD: firstGuess
//
//  DESCRIPTION:
//      This routine creates a firstGuess radial velocity field given a
//      sounding or VAD. Assumes standard atmosphere refraction (4Rearth/3) 
//      and extrapolates sounding data to all radar bins (assuming the wind
//      is horizontally uniform).
//
//  DEVELOPER:
//	Curtis N. James    08 Dec 98
//      Modified by Sue Dettling Nov. 15, 2001
//
//
//
//  HISTORY:
//	An elaboration of the NSSL-Eilts algorithm.
//
void FourDD::firstGuess(Volume* soundVolume, float missingVal,
			short unsigned int *success,time_t soundingTime) 
{

  int numLevs;

  float meanShearU = 0.0, meanShearV = 0.0;

  int  alt, i, sweepIndex, currIndex, index, numBins, numRays, 
       numSweeps;
  unsigned short flag = 0;
  float ke,dRdz,height,rnge,elev,az,start_range,h_range,gate_size,val
    ,wind, wind_val_rv,dir,offset,ang, U, V;

  //
  // Load and retrieve spdb sounding data
  //
  int ret =  loadSoundingData(soundingTime);
  
  if( ret <= 0 )
    {
      *success = 0;
      return ;
    }

  double *soundingU = sounding.getU();

  double *soundingV = sounding.getV();
  
  double *soundingAlt = sounding.getAlts();

  if( (soundingU == NULL) || (soundingV || NULL) || (soundingAlt == NULL) )
    {
      *success = 0;
      return ;
    }

  //
  // Allocate space for matrix to a hold sounding data and derived data
  //
  int numPoints = sounding.getNumPoints();

  float** ua_data = new (float*)[numPoints + 1];
  
  for (int i = 0; i <numPoints ; i++)
    ua_data[i] = new float[5];

  ua_data[0][0]=0.0;
  ua_data[0][1]=0.0;
  ua_data[0][2]=0.0;
  ua_data[0][3]=0.0;
  ua_data[0][4]=0.0;
  

  int k = 1;
  for( int i = 0 ; i < numPoints; i++)
    {
      //
      // Get U,V and Alt
      //
      ua_data[k][0] = soundingAlt[i];
      
      ua_data[k][1] = soundingU[i];
      
      ua_data[k][2] = soundingV[i];
      
      //
      // calculate shear U
      //
      ua_data[k][3]=(ua_data[k][1]-ua_data[k-1][1])/
                     (ua_data[k][0]-ua_data[k-1][0]);

      //
      // calculate shear V
      //
      ua_data[i][4]=(ua_data[k][2]-ua_data[k-1][2])/
                     (ua_data[k][0]-ua_data[k-1][0]);
      
      if (fabs( ua_data[k][3] ) <= params.max_shear && 
	  fabs( ua_data[k][4] ) <= params.max_shear )
	  {
	    k++;
	    
	    //
	    // Keep running sum of shears sowe can calculate the means.
	    //
	    meanShearU = meanShearU + ua_data[k][3];
	    
	    meanShearV = meanShearV + ua_data[k][4];
		     
	  }
    }

  
  numLevs = k - 1;

  //
  // Force wind at ground to be same as that of first level: */
  //
  ua_data[0][1] = ua_data[1][1];
  ua_data[0][2] = ua_data[1][2];
  ua_data[1][3] = 0.0;
  ua_data[1][4] = 0.0;

 
  //
  // Calculate mean shear
  //
  if (numLevs > 1) 
    {
      meanShearU = meanShearU/(ua_data[numLevs-1][0]-ua_data[1][0]);
      meanShearV = meanShearV/(ua_data[numLevs-1][0]-ua_data[1][0]);
    }
  else
    {
      meanShearU = 0.0;
      meanShearV = 0.0;
    }

  if(params.debug)
    fprintf(stderr, "Number of sounding levels used: %d\n\n", numLevs);


  numSweeps = soundVolume->h.nsweeps;


  //
  // Standard Atmosphere refractivity gradient in km^-1
  //
  dRdz=-39.2464; 

  alt = soundVolume->sweep[0]->ray[0]->h.alt;
  
  if(params.debug)
    fprintf(stderr,"Radar altitude: %d\n",alt);


  for(sweepIndex = 0; sweepIndex < numSweeps; sweepIndex++) 
    {

      numRays = soundVolume->sweep[sweepIndex]->h.nrays;

      numBins = soundVolume->sweep[sweepIndex]->ray[0]->h.nbins;

      start_range = soundVolume->sweep[sweepIndex]->ray[0]->h.range_bin1;

      gate_size = soundVolume->sweep[sweepIndex]->ray[0]->h.gate_size;

      elev = PI*(soundVolume->sweep[sweepIndex]->ray[0]->h.elev)/180.0;
      
      index=0;

      //
      // Now we create a first-guess velocity field using the sounding
      // or VAD profile. (We assume standard atmospheric refraction).*/
      //
      for(i = 0; i < numBins; i++) 
	{
	  //
	  //To print out a range circle of radial velocity values: */
	  //
	  rnge = start_range + i*gate_size + gate_size/2.0;

	  //
	  // Doviak and Zrnic, 1993
	  //
	  ke = 1/(1+A*dRdz*pow(10.0,-6.0)); 

	  height = sqrt(pow(rnge,2) + pow(ke*A*1000,2) + 2*rnge*ke*A*1000
		      *sin(elev))-ke*A*1000+alt;

	  h_range = ke*A*1000*asin(rnge*cos(elev)/(ke*A*1000+height-alt));
	  
	  //
	  //  atan (dh/ds)
	  //
	  ang = atan(cos(elev)*sin(elev+h_range/ke/A/1000) * 
		     pow(cos( elev + h_range/ke/A/1000),-2)); 
	  
	 
	  if (height >= ua_data[0][0] && height <= ua_data[numLevs-1][0]) 
	    {
	      while(1) 
		{
		  if (height >= ua_data[index][0] && height < ua_data[index+1][0]) 
		    {
		      U = ua_data[index][1] + ua_data[index+1][3] * 
			(height-ua_data[index][0]);
		    
		      V = ua_data[index][2] + ua_data[index+1][4] * 
			(height-ua_data[index][0]);
		    
		      wind=sqrt(pow(U,2)+pow(V,2));

		      if (params.sign<0) 
			offset=0.0;
		      else 
			offset=PI;
		    
		      if ( U >= 0) 
			dir = (acos(V/wind)+offset) * 180/PI;
		      else 
			dir = (offset-acos(V/wind)) * 180/PI;
		      
		      break;
		    } 		  
		  else 
		    if ( height < ua_data[index][0]) 
		      index--;
		    else 
		      index++;

		}// end while

	    } 
	  else 
	    if ( height > ua_data[numLevs-1][0]) 
	      {
		U = ua_data[numLevs-1][1] + meanShearU*
		  (height-ua_data[numLevs-1][0]);

		V = ua_data[numLevs-1][2]+ meanShearV*
		  (height-ua_data[numLevs-1][0]);
		
		wind = sqrt(pow(U,2)+pow(V,2));

		if (params.sign<0) 
		  offset = 0.0;
		else 
		  offset = PI;
		
		if (U >= 0) 
		  dir = (acos(V/wind)+offset) * 180/PI;
		else 
		  dir = (offset-acos(V/wind)) * 180/PI;
	      }
	  
	  for(currIndex = 0; currIndex < numRays; currIndex++) 
	    {
	      val =(float) soundVolume->sweep[sweepIndex]->ray[currIndex]
	      ->h.scale * (soundVolume->sweep[sweepIndex]->ray[currIndex]->
	            range[i]) + soundVolume->sweep[sweepIndex]->ray[currIndex]->h.bias;
	      
	      if (wind>=0.0 && dir>=0.0) 
		{
		  az = PI * (soundVolume->sweep[sweepIndex]->ray[currIndex]->
			     h.azimuth)/180.0;

		  wind_val_rv = wind*cos(ang);

                  wind_val_rv = wind_val_rv*cos(PI*dir/180.0-az);

		  //val = (float)soundVolume->sweep[sweepIndex]->ray[currIndex]
		  //->h.invf(wind_val_rv);
		  
		  val = (wind_val_rv - soundVolume->sweep[sweepIndex]->ray[currIndex]->h.bias)/
		    soundVolume->sweep[sweepIndex]->ray[currIndex]->h.scale;
		  
                 
		 soundVolume->sweep[sweepIndex]->ray[currIndex]->range[i]=
                   (Range) (val);
		 
                 
                 if (flag==0) 
		   flag=1;
		} 
	      else 
		{
		  wind_val_rv = missingVal;
		  
		  //val = (float)soundVolume->sweep[sweepIndex]->ray[currIndex]
		  //  ->h.invf(wind_val_rv);
		  
		  //
		  // CHANGE
		  //
		  val = (wind_val_rv - soundVolume->sweep[sweepIndex]->ray[currIndex]->h.bias)/
		  soundVolume->sweep[sweepIndex]->ray[currIndex]->h.scale;
		  
		  soundVolume->sweep[sweepIndex]->ray[currIndex]->range[i]=
		    (Range) (val);
		} 
	    } // end for curr index
	} // end for i < num_bins
    }// end for sweepindex < numsweeps

  if (numLevs==0) 
    {
      flag = 0;
    }
  if (flag) 
    *success = 1;

  //
  // free memory 
  //
  for (int i = 0; i < numPoints + 1; i++)
    delete[](ua_data[i]);

  delete[](ua_data);
     
  return;

}
     
////////////////////////////////////////////////////////////////////////
//
//  METHOD: prepVolume
//
//  DESCRIPTION:
//      This routine thresholds VR bins where reflectivity is below
//      params.low_dbz or missing (if params.no_dbz_rm_rv==1).
//
//  DEVELOPER:
//	Curtis N. James 20 Mar 98
//      Modified by C. James 25 Jan 99
//
//
// 
//
void FourDD::prepVolume(Volume* DBZVolume, Volume* rvVolume, float missingVal) {

     int currIndex, sweepIndex, i, j, DBZIndex, numRays, numBins, numDBZRays,
       numDBZBins, numSweepsRV, numSweepsDZ;
     float val, dzval, minpossDBZ = -50.0;
     int DBZGateSize = 0, DBZRangeBin1 = 0, DBZfactor,limit;
     int rvGateSize = 0, rvRangeBin1 = 0, remainder;
     div_t ratioGateSize;
     
     numSweepsRV = rvVolume->h.nsweeps;
     numSweepsDZ = DBZVolume->h.nsweeps;
     if (numSweepsRV!=numSweepsDZ) {
       return;
     } else {
       for (sweepIndex=0;sweepIndex<numSweepsRV;sweepIndex++) {
	 numRays = rvVolume->sweep[sweepIndex]->h.nrays;
	 numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;
	 if (DBZVolume!=NULL) {
	   numDBZRays = DBZVolume->sweep[sweepIndex]->h.nrays;
	   numDBZBins = DBZVolume->sweep[sweepIndex]->ray[0]->h.nbins;
	 }
     
	 // Get the bin geometry for both DBZ and rv:  

	 if (params.debug) printf(" ...preparing sweep...\n");
	 if (DBZVolume!=NULL) {
	   DBZGateSize=DBZVolume->sweep[sweepIndex]->ray[0]->
	     h.gate_size;
	   DBZRangeBin1=DBZVolume->sweep[sweepIndex]->ray[0]->
	     h.range_bin1;
	 }
	 rvGateSize=rvVolume->sweep[sweepIndex]->ray[0]->
	   h.gate_size;
	 rvRangeBin1=rvVolume->sweep[sweepIndex]->ray[0]->
	   h.range_bin1;

	 // Terminate routine if the locations of the first range bins
	 // in DBZ and rv do not coincide.  
       
	 if ((DBZRangeBin1!=rvRangeBin1 || numDBZRays!=numRays) &&
             DBZVolume!=NULL) {
	   return;
	 }
	 // Compare the bin geometry between DBZ and rv to know which
	 // DBZ bin(s) corresponds spatially to the rv bin(s) 
       
	 if (DBZVolume!=NULL) {
	   ratioGateSize=div(rvGateSize,DBZGateSize);
	   remainder=ratioGateSize.rem;
	   if (remainder!=0) {
	     return;
	   } else if (remainder==0 && rvGateSize!=0 && DBZGateSize !=0) {
	     DBZfactor=ratioGateSize.quot;
	   } else {
	     return;
	   }
	 }

	 // Erase the first params.del_num_bins bins of DBZ and radial velocity, as per
	 //  communication with Peter Hildebrand.  
	 for (currIndex=0;currIndex<numRays;currIndex++) {
	   for (i = 0; i < params.del_num_bins; i++) {
	     
	     //
	     // Shouldnt missingVal  be encoded?
	     //
	     //rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i]=
	     //(Range) (missingVal);
	     //
	     rvVolume->sweep[sweepIndex]->ray[currIndex]->range[i]= (Range)
	       (missingVal - rvVolume->sweep[sweepIndex]->ray[currIndex]->h.bias) /
	       rvVolume->sweep[sweepIndex]->ray[currIndex]->h.scale;
	     

	     if (DBZVolume!=NULL) {
	       for (j=0; j<DBZfactor; j++) {	       
		 DBZIndex=i*DBZfactor+j;

		 //
		 // Shouldnt missingVal  be encoded?
		 //
		 //DBZVolume->sweep[sweepIndex]->ray[currIndex]->range[DBZIndex]=
		 //  (Range) (missingVal);
		 DBZVolume->sweep[sweepIndex]->ray[currIndex]->range[DBZIndex]= (Range)
		   (  missingVal - DBZVolume->sweep[sweepIndex]->ray[currIndex]->h.bias) /
		   DBZVolume->sweep[sweepIndex]->ray[currIndex]->h.scale;
	       }
	     }
	   }
	 }
	 
	 // Now that we know the bin geometry, we assign missingVal to any
	 // velocity bins where the reflectivity is missing (if params.no_dbz_rm_rv==1)
	 // or outside params.low_dbz and params.high_dbz.  
	 

	 limit=numBins;
	 for (currIndex=0; currIndex<numRays; currIndex++) {
	   for (i=params.del_num_bins; i<limit; i++) {
 	     if (DBZVolume!=NULL) {
	       
	       val=(float) rvVolume->sweep[sweepIndex]->ray[currIndex]
	       ->h.scale * (rvVolume->sweep[sweepIndex]->ray[currIndex]->
	            range[i]) + rvVolume->sweep[sweepIndex]->ray[currIndex]->h.bias;
	       
	       for (j=0; j<DBZfactor; j++) {	       
		 DBZIndex=i*DBZfactor+j;
		 if (DBZIndex>=numDBZBins) {
		   printf("Invalid bin geometry!\n");
		   return;
		 }
		
		 dzval=(float) DBZVolume->sweep[sweepIndex]->ray[currIndex]
		   ->h.scale * (DBZVolume->sweep[sweepIndex]->ray[currIndex]->
		  range[DBZIndex]) + DBZVolume->sweep[sweepIndex]->ray[currIndex]->h.bias;
		 
		 if ((dzval>=minpossDBZ && dzval<params.low_dbz)||(((dzval<
		     minpossDBZ)||(dzval>params.high_dbz))&&params.no_dbz_rm_rv==1)) {
		    
		   //
		   // Shouldnt missingVal  be encoded?
		   //
		   //rvVolume->sweep[sweepIndex]->ray[currIndex]->
		   // range[i]=(Range) (missingVal);
		   rvVolume->sweep[sweepIndex]->ray[currIndex]-> range[i]=(Range)
		     (missingVal - rvVolume->sweep[sweepIndex]->ray[currIndex]->h.bias) /
		     rvVolume->sweep[sweepIndex]->ray[currIndex]->h.scale;
		   
		   val=missingVal;
		 } 
	       }
	     }
	   }
	 }
       }
     }
     if (params.debug) printf(" ...volume prepped.\n");      
     return;
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
// not available). If this is successful, the bin is considered GOOD. Every 
// other bin is set to GOOD=0 or GOOD=-1 (if the bin is missing or bad).
//
// Then, the algorithm scans azimuthally and radially. If the majority of the
// GOOD=1 bins (up to 8) adjacent to a particular GOOD=0 bin are within 
// params.thresh*Vnyq it is saved and considered GOOD as well. Otherwise, Nyquist 
// intervals are added/subtracted until the condition is met. If unsuccessful, 
// then GOOD is set to -2 for that bin.
//
// When all GOOD=0 bins that are next to GOOD=1 bins have been examined, then
// GOOD=0 and GOOD=-2 bins are unfolded against a window of GOOD=1 values. If
// there are too few GOOD=1 values inside the window, then the data are
// unfolded against the VAD, if available.
// 
// PARAMETERS:
// rvVolume: The radial velocity field to be unfolded.
// soundVolume: A first guess radial velocity field using VAD (or sounding) 
// lastVolume: The last radial velocity field unfolded (time of lastVolume 
//    should be within 10 minutes prior to rvVolume)
// missingVal: The value for missing radial velocity data.
// filt: A flag that specifies whether or not to use Bergen/Albers filter.
// success: flag indicating whether or not unfolding is possible.
 


void FourDD::unfoldVolume(Volume* rvVolume, Volume* soundVolume, Volume* lastVolume,
      float missingVal, unsigned short filt, unsigned short* success) {

     int sweepIndex, currIndex, i, l, direction, numSweeps, numRays,
       numBins,  left, right, next, prev, rayindex[8], binindex[8],
       countindex, numneg, numpos, in, out, startray, endray, firstbin, 
       lastbin, step = -1, startindex, endindex, prevIndex, abIndex, loopcount,
       countbins;
    
     unsigned short numtimes, dcase, flag=1, wsuccess;
     short GOOD[params.max_bins][params.max_rays];
     float NyqVelocity, NyqInterval, val, diff, fraction, finalval, initval, 
       valcheck, goodval, winval,  diffs[8], fraction2;
     float prevval, abval, pfraction,  cval, soundval, std;
     Volume* VALS;

     numSweeps = rvVolume->h.nsweeps;
     VALS=RSL_copy_volume(rvVolume);
     if (params.comp_thresh>1.0 || params.comp_thresh<=0.0 ) fraction=0.25;
     else fraction=params.comp_thresh;
     if (params.comp_thresh2>1.0 || params.comp_thresh2<=0.0 ) fraction2=0.25;
     else fraction2=params.comp_thresh2;
     if (params.thresh >1.0 || params.thresh<=0.0 ) pfraction=0.5;
     else pfraction=params.thresh;

     if (soundVolume!=NULL || lastVolume!=NULL) {
       for (sweepIndex=numSweeps-1;sweepIndex>=0;sweepIndex--) {
	 NyqVelocity = rvVolume->sweep[sweepIndex]->ray[0]->
	   h.nyq_vel;
	 NyqInterval = 2.0 * NyqVelocity;
	 numRays = rvVolume->sweep[sweepIndex]->h.nrays;
	 numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;

	 // First, unfold bins where vertical and temporal continuity
	 // produces the same value (i.e., bins where the radial velocity
	 // agrees with both the previous sweep and previous volume within
	 // a params.comp_thresh of the Nyquist velocity). This produces a number
	 // of good data points from which to begin unfolding assuming spatial
         // continuity.  
	 printf("Sweep: %d\n", sweepIndex);
	 if (params.debug) printf("NyqVelocity: %f, missingVal: %f\n",
			     NyqVelocity, missingVal);
	 flag=1;

	 for (currIndex=0;currIndex<numRays;currIndex++) {
	   if (lastVolume!=NULL) prevIndex=findRay(rvVolume, lastVolume,
	      sweepIndex, sweepIndex,currIndex, missingVal);
	   if (sweepIndex<numSweeps-1) abIndex=findRay(rvVolume, rvVolume,
	      sweepIndex, sweepIndex+1, currIndex, missingVal);
 	   for (i=params.del_num_bins;i<numBins;i++) {
	     // Initialize Output Sweep with missing values:  
	     
	     //
	     // ASK CURTIS: why the cast to float.
	     //
	     //initval=(float)rvVolume->sweep[sweepIndex]->
	     //ray[currIndex]->h.invf(missingVal);

	      initval =  (missingVal - rvVolume->sweep[sweepIndex]->
	       ray[currIndex]->h.bias)/ rvVolume->sweep[sweepIndex]->
	       ray[currIndex]->h.scale;

	     rvVolume->sweep[sweepIndex]->ray[currIndex]->
	       range[i]=(Range) (initval);

	     // Assign uncorrect velocity bins to the array VALS:
	     
	     val=(float) VALS->sweep[sweepIndex]->
	       ray[currIndex]->h.scale * (VALS->sweep[sweepIndex]->ray
	       [currIndex]->range[i]) + VALS->sweep[sweepIndex]->
	       ray[currIndex]->h.bias;
	     
	     valcheck=val;
	     if (val==missingVal) GOOD[i][currIndex]=-1;
	     else {
	       if (filt==1) {
		 //Perform a 3x3 filter, as proposed by Bergen & Albers 1988 
		 countindex=0;
		 if (currIndex==0) left=numRays-1;
		 else left=currIndex-1;
		 if (currIndex==numRays-1) right=0;
		 else right=currIndex+1;
		 next=i+1;
		 prev=i-1;
		 // Look at all bins adjacent to current bin in question:  
		 if (i>params.del_num_bins) {

		   if (((float) VALS->sweep[sweepIndex]->
			ray[left]->h.scale * (VALS->sweep[sweepIndex]->ray
	       	        [left]->range[prev]) + VALS->sweep[sweepIndex]->
			ray[left]->h.bias ) != missingVal) {
		     countindex=countindex+1;
		   }
		   if (((float) VALS->sweep[sweepIndex]->
			ray[currIndex]->h.scale * (VALS->sweep[sweepIndex]->ray
		        [currIndex]->range[prev]) + VALS->sweep[sweepIndex]->
			ray[currIndex]->h.bias  )!=missingVal) {
		     countindex=countindex+1;
		   }
		   if (((float) VALS->sweep[sweepIndex]->
			ray[right]->h.scale * (VALS->sweep[sweepIndex]->ray
			[right]->range[prev]) + VALS->sweep[sweepIndex]->
			ray[right]->h.bias ) !=missingVal) {
		     countindex=countindex+1;
		   }
		 }
		 if (((float) VALS->sweep[sweepIndex]->
		      ray[left]->h.scale * (VALS->sweep[sweepIndex]->ray
		      [left]->range[i]) +  VALS->sweep[sweepIndex]->
		      ray[left]->h.bias) !=missingVal) {
		   countindex=countindex+1;
		 }
		 if (((float) VALS->sweep[sweepIndex]->
		      ray[right]->h.scale * (VALS->sweep[sweepIndex]->ray
		      [right]->range[i]) + VALS->sweep[sweepIndex]->
		      ray[right]->h.bias )!=missingVal) {
		   countindex=countindex+1;
		 }
		 if (i<numBins-1) {  
		   if (((float) VALS->sweep[sweepIndex]->
			ray[left]->h.scale * (VALS->sweep[sweepIndex]->ray
                        [left]->range[next]) + VALS->sweep[sweepIndex]->
			ray[left]->h.bias )!=missingVal) {
		     countindex=countindex+1;
		   }
		   if (((float) VALS->sweep[sweepIndex]->
			ray[currIndex]->h.scale * (VALS->sweep[sweepIndex]->ray
			[currIndex]->range[next]) + VALS->sweep[sweepIndex]->
			ray[currIndex]->h.bias )!=missingVal) {
		     countindex=countindex+1;
		   }
		   if (((float) VALS->sweep[sweepIndex]->
			ray[right]->h.scale * (VALS->sweep[sweepIndex]->ray
                        [right]->range[next]) + VALS->sweep[sweepIndex]->
			ray[right]->h.bias  )!=missingVal) {
		     countindex=countindex+1;
		   }
		 }
		 if (((i==numBins-1 || i==params.del_num_bins) && countindex>=3)||
		     (countindex>=5)) {
		   // Save the bin for dealiasing:  
		   GOOD[i][currIndex]=0;
		 } else {
		   // Assign missing value to the current bin.  
		   GOOD[i][currIndex]=-1;
		 }   
	       // End 3 x 3 filter  
	       } else {
		 // If no filter is being applied save bin for dealiasing:  
		 GOOD[i][currIndex]=0;
	       }
	     }
	     
	     if (GOOD[i][currIndex]==0) {
	       // Try to dealias the bin using vertical and temporal 
	       //   continuity (this is initial dealiasing).  
	       if (val!=missingVal && lastVolume!=NULL) {
		 prevval = (float) lastVolume->sweep[sweepIndex]->ray[prevIndex]
		   ->h.scale * (lastVolume->sweep[sweepIndex]->ray[prevIndex]->
		   range[i]) + lastVolume->sweep[sweepIndex]->ray[prevIndex]
		   ->h.bias;
	       } else {
		 prevval=missingVal;
	       }
	       if (val!=missingVal && soundVolume!=NULL && lastVolume==NULL) {
		 soundval=(float) soundVolume->
		   sweep[sweepIndex]->ray[currIndex]->h.scale * (soundVolume->
		   sweep[sweepIndex]->ray[currIndex]->range[i]) + soundVolume->
		   sweep[sweepIndex]->ray[currIndex]->h.bias;
	       } else {
		 soundval=missingVal;
	       }
	       if (val!=missingVal && sweepIndex<numSweeps-1) {
		 abval=(float) rvVolume->sweep[sweepIndex+1]->ray[abIndex]->
		   h.scale * (rvVolume->sweep[sweepIndex+1]->ray[abIndex]->range[i]) 
		   + rvVolume->sweep[sweepIndex+1]->ray[abIndex]->h.bias;
	       } else {
		 abval=missingVal;
	       }

	       if (val!=missingVal&&lastVolume==NULL&&soundval!=
		   missingVal&&abval==missingVal) {
		 cval=soundval;
		 dcase=1;
	       }
	       else if (val!=missingVal&&lastVolume==NULL&&soundval!=
			missingVal&&abval!=missingVal) {
		 cval=abval;
		 dcase=2;
	       }
	       else if (val!=missingVal&&prevval!=missingVal&&abval!=
			missingVal) {
		 cval=prevval;
		 dcase=3;
	       } 
	       else {
		 cval=missingVal;
		 dcase=0;
	       }
	       if (dcase>0) {
		 diff=cval-val;	     
		 if (diff<0.0) {
		   diff=-diff;
		   direction=-1;
		 } else direction=1;
		 numtimes=0;
		 while (diff>0.99999*NyqVelocity && numtimes<=params.max_count) {
		   val=val+NyqInterval*direction;
		   numtimes=numtimes+1;
		   diff=cval-val;
		   if (diff<0.0) {
		     diff=-diff;
		     direction=-1;
		   } else direction=1;
		 }
		 if (dcase==1&&diff<fraction*NyqVelocity&&
		     fabs(valcheck)>params.ck_val) {
		   //if (params.debug) printf("GOOD1: %f\n", val);
		   
		   //finalval=(float)rvVolume->sweep[sweepIndex]->
		   //ray[currIndex]->h.invf(val);

		   finalval = (val - rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.bias)/rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.scale; 

		   rvVolume->sweep[sweepIndex]->ray[currIndex]->
		     range[i]=(Range) (finalval);
		   GOOD[i][currIndex]=1;
		 }
		 else if (dcase==2&&diff<fraction*NyqVelocity&&fabs(soundval
		   -val)<fraction*NyqVelocity&&fabs(valcheck)>params.ck_val) {
		   //if (params.debug) printf("GOOD2: %f\n", val);

		   //finalval=(float)rvVolume->sweep[sweepIndex]->
		   //ray[currIndex]->h.invf(val);
		   
		   finalval = (val - rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.bias ) / rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.scale; 

		   rvVolume->sweep[sweepIndex]->ray[currIndex]->
		     range[i]=(Range) (finalval);
		   GOOD[i][currIndex]=1;
		 }
		 else if (dcase==3&&diff<fraction*NyqVelocity&&fabs(abval-val)<
			  fraction*NyqVelocity&&fabs(valcheck)>params.ck_val) {
		   //if (params.debug) printf("GOOD3: %f\n",val);

		   //finalval=(float)rvVolume->sweep[sweepIndex]->
		   //ray[currIndex]->h.invf(val);

		   finalval=(val - rvVolume->sweep[sweepIndex]->
			     ray[currIndex]->h.bias)/ rvVolume->sweep[sweepIndex]->
			     ray[currIndex]->h.scale;

		   rvVolume->sweep[sweepIndex]->ray[currIndex]->
		     range[i]=(Range) (finalval); 
		   GOOD[i][currIndex]=1;
		 }
	       }
	     }
	   }
	 }

	 // Now, unfold GOOD=0 bins assuming spatial continuity:  
	 loopcount=0;
	 while (flag==1) {
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
	   for (i=params.del_num_bins;i<numBins;i++) {
	     for (currIndex=startindex;currIndex!=endindex;currIndex=currIndex
		+step) {
	       if (GOOD[i][currIndex]==0) {
		 countindex=0;
		 countbins=0;
		 val=(float) VALS->sweep[sweepIndex]->ray[currIndex]->
		   h.scale * (VALS->sweep[sweepIndex]->ray[currIndex]->range[i])
		   + VALS->sweep[sweepIndex]->ray[currIndex]->h.bias;
		 //if (params.debug) printf("\n");
	         //if (params.debug) printf("Startval: %f\n", val);
       		 if (currIndex==0) left=numRays-1;
		 else left=currIndex-1;
		 if (currIndex==numRays-1) right=0;
		 else right=currIndex+1;
		 next=i+1;
		 prev=i-1;
		 // Look at all bins adjacent to current bin in question:  
		 if (i>params.del_num_bins) {
		   if (GOOD[prev][left]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=prev;
		     rayindex[countindex-1]=left;
		     //if (params.debug) printf("pl ");
		   }
		   if (GOOD[prev][left]==0) {
		     countbins=countbins+1;
		   }
		   if (GOOD[prev][currIndex]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=prev;
		     rayindex[countindex-1]=currIndex;
		     //if (params.debug) printf("pc ");
		   }
		   if (GOOD[prev][currIndex]==0) {
		     countbins=countbins+1;
		   }
		   if (GOOD[prev][right]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=prev;
		     rayindex[countindex-1]=right;
		     //if (params.debug) printf("pr ");
		   }
	           if (GOOD[prev][right]==0) {
		     countbins=countbins+1;
		   }
		 }
		 if (GOOD[i][left]==1) {
		   if (flag==0) flag=1;
		   countindex=countindex+1;
		   binindex[countindex-1]=i;
		   rayindex[countindex-1]=left;
		   //if (params.debug) printf("il ");
		 }
		 if (GOOD[i][left]==0) {
		   countbins=countbins+1;
		 }
		 if (GOOD[i][right]==1) {
		   if (flag==0) flag=1;
		   countindex=countindex+1;
		   binindex[countindex-1]=i;
		   rayindex[countindex-1]=right;
		   //if (params.debug) printf("ir "); 	   
		 }
		 if (GOOD[i][right]==0) {
		   countbins=countbins+1;
		 }
		 if (i<numBins-1) {  
		   if (GOOD[next][left]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=next;
		     rayindex[countindex-1]=left;
		     //if (params.debug) printf("nl "); 
		   }
		   if (GOOD[next][left]==0) {
		     countbins=countbins+1;
		   }
		   if (GOOD[next][currIndex]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=next;
		     rayindex[countindex-1]=currIndex;
		     //if (params.debug) printf("nc ");
		   }
		   if (GOOD[next][currIndex]==0) {
		     countbins=countbins+1;
		   }
		   if (GOOD[next][right]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=next;
		     rayindex[countindex-1]=right;
		     //if (params.debug) printf("nr ");
		   }
		   if (GOOD[next][right]==0) {
		     countbins=countbins+1;
		   }
		 }

		 // Perform last step of Bergen and Albers filter:  
		 if (loopcount==1 && countbins+countindex<1) 
		       GOOD[i][currIndex]=-1;  

		 if (countindex>=1) {
		   // Unfold against all adjacent values where GOOD==1  
		   numtimes=0;
		   while(val!=missingVal&&GOOD[i][currIndex]==0) {
		     numtimes=numtimes+1;
		     in=0;
		     out=0;
		     numneg=0;
		     numpos=0;
		     //if (params.debug) printf("countindex: %d\n", countindex); 
		     for (l=0;l<countindex;l++) {
		       //if (params.debug) printf("%d %d %f\n",binindex[l],
		       // rayindex[l],goodval);
		       goodval=(float)rvVolume->sweep[sweepIndex]->
			 ray[rayindex[l]]->h.scale * (rvVolume->sweep[sweepIndex]->
			 ray[rayindex[l]]->range[binindex[l]]) + rvVolume->sweep[sweepIndex]->
			 ray[rayindex[l]]->h.bias;
	               diffs[l]=goodval-val;
		       if (fabs(diffs[l])<pfraction*NyqVelocity) in=in+1;
		       else {
			 out=out+1;
			 if (diffs[l]>NyqVelocity) {
			   numpos=numpos+1;
			 } else if (diffs[l]<-NyqVelocity){
			   numneg=numneg+1;
			 }
		       }
		     }
		     if (in>0 && out==0) {

		       //finalval=(float)rvVolume->sweep[sweepIndex]->
		       //ray[currIndex]->h.invf(val);
		       

		        finalval=(val - rvVolume->sweep[sweepIndex]->
			 ray[currIndex]->h.bias)/ rvVolume->sweep[sweepIndex]->
			 ray[currIndex]->h.scale;

		       rvVolume->sweep[sweepIndex]->ray[currIndex]->
			 range[i]=(Range) (finalval);
		       GOOD[i][currIndex]=1;
		       //if (params.debug) printf("Finalval: %4.2f\n", val);
		     } else {
		       if (numtimes<=params.max_count) {
			 if ((numpos+numneg)<(in+out-(numpos+numneg))) {
			   if (loopcount>2)
			   { // Keep the value after two passes through
			     // data.  

			     //finalval=(float)rvVolume->sweep[sweepIndex]->
			     //ray[currIndex]->h.invf(val);

			     finalval= ( val - rvVolume->sweep[sweepIndex]->
			       ray[currIndex]->h.bias )/ rvVolume->sweep[sweepIndex]->
			       ray[currIndex]->h.scale;

			     rvVolume->sweep[sweepIndex]->ray[currIndex]->
			       range[i]=(Range) (finalval);
			     GOOD[i][currIndex]=1;
			     //if (params.debug) printf(
			     //"Keeping after two passes: %f\n",val);
			   }
			 } else if (numpos>numneg) {
			   val=val+NyqInterval;
			 } else if (numneg>numpos) {
			   val=val-NyqInterval;
			 } else {
			   // Save bin for windowing if unsuccessful after
			   // four passes:  
			   if (loopcount>4) GOOD[i][currIndex]=-2;
			   //if (params.debug) printf("Saving for windowing\n");
			 }
		       } else {
			 // Remove bin:  
			 GOOD[i][currIndex]=-2;
			 //if (params.debug) printf("Saving for windowing\n");
		       }
		     }
		   }
		 }
	       }
	     }
	   }
	 }

	 // Unfold remote bins or those that were previously unsuccessful
	 //   using a window with dimensions 2(params.proximity)+1 x 2(params.proximity)+1:
	 //   if still no luck delete data (or unfold against VAD if params.pass2).  
	 for (i=params.del_num_bins;i<numBins;i++) {
	   for (currIndex=0;currIndex<numRays;currIndex++) { 
	     if (GOOD[i][currIndex]==0 || GOOD[i][currIndex]==-2) {
	       val=(float) VALS->sweep[sweepIndex]->ray[currIndex]->
		   h.scale * (VALS->sweep[sweepIndex]->ray[currIndex]->range[i])
		 + VALS->sweep[sweepIndex]->ray[currIndex]->
		   h.bias ;
	       startray=currIndex-params.proximity;
	       endray=currIndex+params.proximity;
	       firstbin=i-params.proximity;
	       lastbin=i+params.proximity;
	       if (startray<0) startray=numRays+startray;
	       if (endray>numRays-1) endray=endray-numRays;
	       if (firstbin<0) firstbin=0;
	       if (lastbin>numBins-1) lastbin=numBins-1;
	       winval=window(rvVolume, sweepIndex, startray, endray, 
			     firstbin, lastbin, &std, missingVal, &wsuccess);
	       if (winval==missingVal && wsuccess==1){ // Expand the window:  
		 startray=currIndex-2*params.proximity;
		 endray=currIndex+2*params.proximity;
		 firstbin=i-2*params.proximity;
		 lastbin=i+2*params.proximity;
		 if (startray<0) startray=numRays+startray;
		 if (endray>numRays-1) endray=endray-numRays;
		 if (firstbin<0) firstbin=0;
		 if (lastbin>numBins-1) lastbin=numBins-1;
		 winval=window(rvVolume, sweepIndex, startray, endray, 
			       firstbin, lastbin, &std, missingVal, &wsuccess);
	       }
	       if (winval!=missingVal) {
		 diff=winval-val;
		 if (diff<0.0) {
		   diff=-diff;
		   direction=-1;
		 } else direction=1;
		 numtimes=0;
		 while (diff>0.99999*NyqVelocity && numtimes<=params.max_count) {
		   val=val+NyqInterval*direction;
		   numtimes=numtimes+1;
		   diff=winval-val;
		   if (diff<0.0) {
		     diff=-diff;
		     direction=-1;
		   } else direction=1;
		 }
		 if (diff<pfraction*NyqVelocity) {
		   // Return the value.  


		   //finalval=(float)rvVolume->sweep[sweepIndex]->
		   //ray[currIndex]->h.invf(val);

		   finalval= (val - rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.bias) / rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.scale;


		   rvVolume->sweep[sweepIndex]->ray[currIndex]->
		     range[i]=(Range) (finalval);
		   GOOD[i][currIndex]=1;
		 } else if (diff<(1.0 - (1.0 - pfraction)/2.0)*NyqVelocity) {
		   // If within relaxed threshold, then return value, but
		   //   do not use to dealias other bins.
  
		   //finalval=(float)rvVolume->sweep[sweepIndex]->
		   //ray[currIndex]->h.invf(val);

		   finalval=( val - rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.bias) / rvVolume->sweep[sweepIndex]->
		     ray[currIndex]->h.scale;  


		   rvVolume->sweep[sweepIndex]->ray[currIndex]->
		     range[i]=(Range) (finalval);

		   GOOD[i][currIndex]=-1;		   
		 } else {
		   // Remove bin  
		   GOOD[i][currIndex]=-1;
		 }
	       } else { 
		 if (wsuccess==0) {
		   // Remove bin  
		   GOOD[i][currIndex]=-1;
		 } else if (soundVolume==NULL || lastVolume==NULL) {
		   if (GOOD[i][currIndex]==0 && params.rm_cells!=1) {
		     // Leave bin untouched.  
		     val=(float) VALS->sweep[sweepIndex]->ray[currIndex]->
		       h.scale * (VALS->sweep[sweepIndex]->ray[currIndex]->range[i]) + 
		       VALS->sweep[sweepIndex]->ray[currIndex]->h.bias;


		     //finalval=(float)rvVolume->sweep[sweepIndex]->
		     //ray[currIndex]->h.invf(val);

		     finalval = (val - rvVolume->sweep[sweepIndex]->
		       ray[currIndex]->h.bias ) / rvVolume->sweep[sweepIndex]->
		       ray[currIndex]->h.scale;


		     rvVolume->sweep[sweepIndex]->ray[currIndex]->
		       range[i]=(Range) (finalval);
		     GOOD[i][currIndex]=-1; // Don't use to unfold other bins 
		   } else {
		     // Remove bin  
		     GOOD[i][currIndex]=-1;
		   }
		 } else if (GOOD[i][currIndex]==0 && params.pass2 &&
		     soundVolume!=NULL && lastVolume!=NULL) {
		   // Leave GOOD[i][currIndex]=0 bins for a second pass.
		   // In the second pass, we repeat unfolding, except this
		   // time we use soundVolume for comparison instead of
		   // lastVolume.  
		 } else {
		   // Remove bin  
		     GOOD[i][currIndex]=-1;
		 }
	       }
	     }
	   }
	 }
	 
	 // Beginning second pass through the data, this time using 
	 //  soundVolume only:  

	 if (lastVolume!=NULL && soundVolume!=NULL) {
	   flag=1;
	   for (currIndex=0;currIndex<numRays;currIndex++) {
	     if (sweepIndex<numSweeps-1) abIndex=findRay(rvVolume, rvVolume,
	         sweepIndex, sweepIndex+1, currIndex, missingVal);
	     for (i=params.del_num_bins;i<numBins;i++) {
	       if (GOOD[i][currIndex]==0) {
		 val = (float) VALS->sweep[sweepIndex]->ray[currIndex]->
		   h.scale * (VALS->sweep[sweepIndex]->ray[currIndex]->range[i]) +  
		   VALS->sweep[sweepIndex]->ray[currIndex]->h.bias;
		 valcheck=val;
		 soundval=(float) soundVolume->
		   sweep[sweepIndex]->ray[currIndex]->h.scale * (soundVolume->
		    sweep[sweepIndex]->ray[currIndex]->range[i]) + soundVolume->
		   sweep[sweepIndex]->ray[currIndex]->h.bias;
	         
		 if (soundval!=missingVal&&val!=missingVal) {
		   diff=soundval-val;	     
		   if (diff<0.0) {
		     diff=-diff;
		     direction=-1;
		   } else direction=1;
		   numtimes=0;
		   while (diff>0.99999*NyqVelocity&&numtimes<=params.max_count) {
		     val=val+NyqInterval*direction;
		     numtimes=numtimes+1;
		     diff=soundval-val;
		     if (diff<0.0) {
		       diff=-diff;
		       direction=-1;
		     } else direction=1;
		   }
		   if (diff<fraction2*NyqVelocity&&fabs(valcheck)>params.ck_val) {
		     // Return the good value.

		     //finalval=(float)rvVolume->sweep[sweepIndex]->
		     //ray[currIndex]->h.invf(val);

		     finalval= (val - rvVolume->sweep[sweepIndex]->
		       ray[currIndex]->h.bias)/ rvVolume->sweep[sweepIndex]->
		       ray[currIndex]->h.scale;

		     rvVolume->sweep[sweepIndex]->ray[currIndex]->
		       range[i]=(Range) (finalval);
		     GOOD[i][currIndex]=1;
		   }
		 }
	       }
	     }
	   }
		   
	   // Now, try to unfold the rest of the GOOD=0 bins, assuming spatial
	   //   continuity:  
	   loopcount=0;
	   while (flag==1) {
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
	     for (currIndex=startindex;currIndex!=endindex;currIndex=
		      currIndex+step) {
	       for (i=params.del_num_bins;i<numBins;i++) {
		 if (GOOD[i][currIndex]==0) {
		   countindex=0;
		   val=(float) VALS->sweep[sweepIndex]->ray[currIndex]
		     ->h.scale * (VALS->sweep[sweepIndex]->ray[currIndex]->
			   range[i]) + VALS->sweep[sweepIndex]->ray[currIndex]->h.bias;
		   valcheck=val;
		   if (currIndex==0) left=numRays-1;
		   else left=currIndex-1;
		   if (currIndex==numRays-1) right=0;
		   else right=currIndex+1;
		   next=i+1;
		   prev=i-1;
		   // Look at all bins adjacent to current bin in question:  
		   if (i>params.del_num_bins) {
		     if (GOOD[prev][left]==1) {
		       if (flag==0) flag=1;
		       countindex=countindex+1;
		       binindex[countindex-1]=prev;
		       rayindex[countindex-1]=left;
		       //if (params.debug) printf("pl ");
		     }
		     if (GOOD[prev][currIndex]==1) {
		       if (flag==0) flag=1;
		       countindex=countindex+1;
		       binindex[countindex-1]=prev;
		       rayindex[countindex-1]=currIndex;
		       //if (params.debug) printf("pc ");
		     }
		     if (GOOD[prev][right]==1) {
		       if (flag==0) flag=1;
		       countindex=countindex+1;
		       binindex[countindex-1]=prev;
		       rayindex[countindex-1]=right;
		       //if (params.debug) printf("pr ");
		     }
		   }
		   if (GOOD[i][left]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=i;
		     rayindex[countindex-1]=left;
		     //if (params.debug) printf("il ");
		   }
		   if (GOOD[i][right]==1) {
		     if (flag==0) flag=1;
		     countindex=countindex+1;
		     binindex[countindex-1]=i;
		     rayindex[countindex-1]=right;
		     //if (params.debug) printf("ir "); 	   
		   }
		   if (i<numBins-1) {  
		     if (GOOD[next][left]==1) {
		       if (flag==0) flag=1;
		       countindex=countindex+1;
		       binindex[countindex-1]=next;
		       rayindex[countindex-1]=left;
		       //if (params.debug) printf("nl "); 
		     }
		     if (GOOD[next][currIndex]==1) {
		       if (flag==0) flag=1;
		       countindex=countindex+1;
		       binindex[countindex-1]=next;
		       rayindex[countindex-1]=currIndex;
		       //if (params.debug) printf("nc ");
		     }
		     if (GOOD[next][right]==1) {
		       if (flag==0) flag=1;
		       countindex=countindex+1;
		       binindex[countindex-1]=next;
		       rayindex[countindex-1]=right;
		       //if (params.debug) printf("nr ");
		     }
		   }
		   // Unfold against all adjacent values with GOOD==1 
		   if (countindex>=1) {
		     numtimes=0;
		     while(val!=missingVal&&GOOD[i][currIndex]==0) {
		       numtimes=numtimes+1;
		       in=0;
		       out=0;
		       numneg=0;
		       numpos=0;
		       //if (params.debug) printf("%d: ", countindex); 
		       for (l=0;l<countindex;l++) {
			 goodval=(float)rvVolume->sweep[sweepIndex]->
			   ray[rayindex[l]]->h.scale * (rvVolume->sweep
			    [sweepIndex]->ray[rayindex[l]]->range
			    [binindex[l]]) + rvVolume->sweep[sweepIndex]->
			   ray[rayindex[l]]->h.bias;
			 diffs[l]=goodval-val;
			 if (fabs(diffs[l])<pfraction*NyqVelocity) in=in+1;
			 else {
			   out=out+1;
			   if (diffs[l]>NyqVelocity) {
			     numpos=numpos+1;
			   } else if (diffs[l]<-NyqVelocity){
			     numneg=numneg+1;
			   }
			 }
		       }
		       if (in>out) {

			 //finalval=(float)rvVolume->sweep[sweepIndex]->
			 //ray[currIndex]->h.invf(val);

			 finalval= (val -rvVolume->sweep[sweepIndex]->
			   ray[currIndex]->h.bias )/ rvVolume->sweep[sweepIndex]->
			   ray[currIndex]->h.scale;


			 rvVolume->sweep[sweepIndex]->ray[currIndex]->
			   range[i]=(Range) (finalval);
			 
			 GOOD[i][currIndex]=1;
			 //if (params.debug) printf("Val: %4.2f\n", val);
		       } else {
			 if (numtimes<=params.max_count) {
			   if (numpos+numneg<(in+out-(numpos+numneg))) {
			     if (loopcount<=2) val=missingVal; // Try later  
			     else {
			       // Keep the value after two passes through
			       // data  


			       //finalval=(float)rvVolume->sweep[sweepIndex]->
			       //ray[currIndex]->h.invf(val);

			       finalval=( val - rvVolume->sweep[sweepIndex]->
				 ray[currIndex]->h.bias) / rvVolume->sweep[sweepIndex]->
				 ray[currIndex]->h.scale;

			       rvVolume->sweep[sweepIndex]->ray[currIndex]->
				 range[i]=(Range) (finalval);
			       GOOD[i][currIndex]=1;
			     }
			   } else if (numpos>numneg) {
			     val=val+NyqInterval;
			   } else if (numneg>numpos) {
			     val=val-NyqInterval;
			   } else {
			     // Remove bin after four passes through data:  
			     if (loopcount>4) GOOD[i][currIndex]=-1;
			   }
			 } else {
			   // Remove bin:  
			   GOOD[i][currIndex]=-1;
			 }
		       }
		     }
		   }
		 }
	       }
	     }
	   }
	 }
       }
       *success=1;
     } else {
       printf("First guess not available.\n");
       *success=0;
     }
     RSL_free_volume(VALS);
     return;
}

//////////////////////////////////////////////////////////////////////////
//
//  METHOD: window
//
//  DESCRIPTION:
//      This routine averages the values in a range and azimuth window of a
//      sweep and computes the standard deviation.
//
//  DEVELOPER:
//	Curtis N. James    26 Jan 1999
//
//
//
//
float FourDD::window(Volume* rvVolume, int sweepIndex, int startray, 
	int endray, int firstbin, int lastbin, float* std, float
        missingVal, unsigned short* success) {

     int num, currIndex, rangeIndex, numRays, numBins;
     float val, sum, sumsq, ave, NyqVelocity;
     
     *success=0;
     NyqVelocity = rvVolume->sweep[sweepIndex]->ray[0]->
	 h.nyq_vel;
     numRays = rvVolume->sweep[sweepIndex]->h.nrays;
     numBins = rvVolume->sweep[sweepIndex]->ray[0]->h.nbins;

     // Now, sum the data in the window region between startray, 
     //  endray, firstbin, lastbin.  
     *std=0.0;
     ave=0.0;
     num=0;
     sum=0.0;
     sumsq=0.0;
       
     if (firstbin>=numBins || lastbin>=numBins || firstbin<0 || lastbin<0)
       return missingVal;
     if (startray>endray){
       for (currIndex=startray; currIndex<numRays; currIndex++) {
	 for (rangeIndex=firstbin; rangeIndex<=lastbin; rangeIndex++) {
	   val=(float)rvVolume->sweep[sweepIndex]->ray[currIndex]->
	     h.scale * (rvVolume->sweep[sweepIndex]->ray[currIndex]->
		 range[rangeIndex]) + rvVolume->sweep[sweepIndex]->ray[currIndex]->h.bias;
	   if (val!=missingVal) {
	     num=num+1;
	     sum=sum+val;
	     sumsq=sumsq+val*val;
	   }
	 }
       }
       for (currIndex=0; currIndex<=endray; currIndex++) {
	 for (rangeIndex=firstbin; rangeIndex<=lastbin; rangeIndex++) {
	   val=(float)rvVolume->sweep[sweepIndex]->ray[currIndex]->
	     h.scale * (rvVolume->sweep[sweepIndex]->ray[currIndex]->
		 range[rangeIndex]) + rvVolume->sweep[sweepIndex]->ray[currIndex]->h.bias;
	   if (val!=missingVal) {
	     num=num+1;
	     sum=sum+val;
	     sumsq=sumsq+val*val;
	   }
	 }
       }
     } else {
       for (currIndex=startray; currIndex<=endray; currIndex++) { 
	 for (rangeIndex=firstbin; rangeIndex<=lastbin; rangeIndex++) {
	   val=(float)rvVolume->sweep[sweepIndex]->ray[currIndex]->
	     h.scale * (rvVolume->sweep[sweepIndex]->ray[currIndex]->
		 range[rangeIndex]) + rvVolume->sweep[sweepIndex]->ray[currIndex]->h.bias;
	   if (val!=missingVal) {
	     num=num+1;
	     sum=sum+val;
	     sumsq=sumsq+val*val;
	   }
	 }
       }
     }
     if (num>=params.min_good) {
       ave=sum/num;
       *std=sqrt(fabs((sumsq-(sum*sum)/num)/(num-1)));
       if (*std<=params.std_thresh*NyqVelocity) *success=1;
       // printf("ave=%0.2f, std=%0.2f, sum=%0.2f\n", ave, *std, sum);  
     } else {
       ave=missingVal;
       *std=0.0;
       *success=1;
     }
     return ave; 
}



int FourDD::loadSoundingData( time_t issueTime )
{   
   //
   // Try to read a sounding
   // 
   int ret = sounding.readSounding( issueTime );

   if ( ret < 0 ) {
      fprintf( stderr, "Cannot read sounding data at %s\n",
                       DateTime::str( issueTime ).c_str() );
      return( ret  );
   }

   if ( ret == 0 ) {
      fprintf(stderr, "No sounding data available at %s\n",
                         DateTime::str( issueTime ).c_str() );
      return( ret);
   }

   DateTime soundingTime = sounding.getLaunchTime();
   string   soundingName = sounding.getSourceName();

   string   timeStampName = "Sounding data: ";
   timeStampName += soundingName;
   
   if(params.debug)
   fprintf( stderr, "Sounding: Got '%s' at %s\n",
	    soundingName.c_str(), soundingTime.dtime() );
   return( ret );
}








