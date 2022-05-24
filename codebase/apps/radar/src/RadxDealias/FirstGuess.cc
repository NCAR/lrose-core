
#include "FirstGuess.hh"
#include "ClassIngest.hh"

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
FirstGuess::FirstGuess(bool debug,
               char *sounding_url,
               float sounding_look_back,
               float wind_alt_min,
               float wind_alt_max,
               float avg_wind_u,
               float avg_wind_v,
               float max_shear,
               int sign)
{
  _debug = debug;
  _sounding_url = sounding_url;
  _sounding_look_back = sounding_look_back;
  _wind_alt_min = wind_alt_min;
  _wind_alt_max = wind_alt_max;
  _avg_wind_u = avg_wind_u;
  _avg_wind_v = avg_wind_v;
  _max_shear = max_shear;
  _sign = sign;

  //
  // Initialize the sounding
  // We do this even if the url is unspecified, so that we can fall back
  // on the avg sounding wind if all other wind input fails
  //
  if (_debug)
    fprintf(stderr, "Initializing sounding at %s\n", _sounding_url );
  
  sounding.init( _sounding_url,
                  (time_t)_sounding_look_back*60,
                  _wind_alt_min*1000,
                  _wind_alt_max*1000,
                  _avg_wind_u,
                  _avg_wind_v );

}

///////////////////////////////////////////////////////////////
//
// Destructor
//
FirstGuess::~FirstGuess()
{

}

void FirstGuess::outputSoundVolume() {
  /* moved to RadxDealias ...
      int nSweeps = currVelVol->h.nsweeps;
      int nRays = currVelVol->sweep[0]->h.nrays;
      int nBins = currVelVol->sweep[0]->ray[0]->h.nbins;
	  
      for (int i = 0; i < nSweeps;  i++)
	for (int j = 0; j < nRays ; j ++)
	  for (int k = 0; k < nBins  ; k++) { 
	    currVelVol->sweep[i]->ray[j]->range[k] = soundVolume->sweep[i]->ray[j]->range[k];
	  }
      fprintf(stderr, "\nREPLACED VELOCITY DATA WITH SOUNDING DATA!!\n");
  */
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
// soundVolume is an in/out parameter. As input, it contains a copy of the current
//  velocity volume.  On output, it contains a first guess of velocity field
//  if success is true.
// 
bool FirstGuess::firstGuess(Volume* soundVolume, time_t soundingTime) 
{
  bool success;
  int numLevs;

  float meanShearU = 0.0, meanShearV = 0.0;

  int  alt, i, sweepIndex, currIndex, index, numBins, numRays, 
       numSweeps;
  unsigned short flag = 0;
  float ke,dRdz,height,rnge,elev,az,start_range,h_range,gate_size;
  float wind = 0.0, wind_val_rv,dir = 0.0,offset,ang, U, V;

  success = true;

  //
  // Load and retrieve spdb sounding data
  //
  double *soundingU = NULL;
  double *soundingV = NULL;
  double *soundingAlt = NULL;
  int numPoints = 0;
  float missingValue = sounding.getMissingValue();

  int ret =  loadSoundingData(soundingTime);
  if( ret <= 0 ) {
      // try reading a text file for the sounding
      ClassIngest *classIngest = loadSoundingDataText(soundingTime);
      if (classIngest != NULL) {
        soundingU = classIngest->getU();
        soundingV = classIngest->getV();
        soundingAlt = classIngest->getAlts();
        numPoints = classIngest->getNumPoints();
        missingValue = classIngest->getMissingValue();
        delete classIngest;
      }
      //success = false;
      //return success;
  } else {
      // data in sounding is good, extract it
      soundingU = sounding.getU();
      soundingV = sounding.getV();
      soundingAlt = sounding.getAlts();
      numPoints = sounding.getNumPoints();
      missingValue = sounding.getMissingValue();
  }

  if( (soundingU == NULL) || (soundingV == NULL) || (soundingAlt == NULL) )
    {
      if (_debug)
	      cerr << "Failed to obtain U and V from sounding. Sounding will not be used\n";
      success = false;
      return success;
    }

  //
  // Allocate space for matrix to a hold sounding data and derived data
  //
  //int numPoints = sounding.getNumPoints();

  float** ua_data = new float*[numPoints + 1];
  
  for (int i = 0; i < numPoints + 1 ; i++)
    ua_data[i] = new float[5];

  ua_data[0][0]=0.0;
  ua_data[0][1]=0.0;
  ua_data[0][2]=0.0;
  ua_data[0][3]=0.0;
  ua_data[0][4]=0.0;
  

  int k = 1;
  //float missingValue = sounding.getMissingValue();
  if (_debug) cerr << "sounding missing value = " << missingValue << endl;
  for( int i = 0 ; i < numPoints; i++)
    {
      //
      // Get U,V and Alt
      //
      ua_data[k][0] = soundingAlt[i];
      ua_data[k][1] = soundingU[i];
      ua_data[k][2] = soundingV[i];
      
      // TODO: what if the missing value for the sounding is different than
      // the missing value for the velocity field?

      if (ua_data[k][0] == missingValue || 
	      ua_data[k][1] == missingValue ||
	      ua_data[k][2]  == missingValue)
	        continue;

      //
      // calculate shear U
      //
      ua_data[k][3]=(ua_data[k][1]-ua_data[k-1][1])/
                     (ua_data[k][0]-ua_data[k-1][0]);

      //
      // calculate shear V
      //
      ua_data[k][4]=(ua_data[k][2]-ua_data[k-1][2])/
                     (ua_data[k][0]-ua_data[k-1][0]);
      
      if (fabs( ua_data[k][3] ) <= _max_shear && 
  	  fabs( ua_data[k][4] ) <= _max_shear )
  	  {
  	    k++; 
  	  }
       
    }

  numLevs = k - 1;

  //
  // Force wind at ground to be same as that of first level: 
  //
  ua_data[0][1] = ua_data[1][1];
  ua_data[0][2] = ua_data[1][2];
  ua_data[1][3] = 0.0;
  ua_data[1][4] = 0.0;

  if(_debug)
    fprintf(stderr, "Number of sounding levels used: %d\n\n", numLevs);

  numSweeps = soundVolume->h.nsweeps;

  //
  // Standard Atmosphere refractivity gradient in km^-1
  //
  dRdz=-39.2464; 

  alt = soundVolume->sweep[0]->ray[0]->h.alt;
  
  if(_debug)
    fprintf(stderr,"Radar altitude: %d\n",alt);

  for(sweepIndex = 0; sweepIndex < numSweeps; sweepIndex++) {
    numRays = soundVolume->sweep[sweepIndex]->h.nrays;
    numBins = soundVolume->sweep[sweepIndex]->ray[0]->h.nbins;
    start_range = soundVolume->sweep[sweepIndex]->ray[0]->h.range_bin1;
    gate_size = soundVolume->sweep[sweepIndex]->ray[0]->h.gate_size;
    elev = PI*(soundVolume->sweep[sweepIndex]->ray[0]->h.elev)/180.0;
    index=0;

    //
    // Now we create a first-guess velocity field using the sounding
    // or VAD profile. (We assume standard atmospheric refraction).
    //
    for(i = 0; i < numBins; i++) {

      //
      //To print out a range circle of radial velocity values: 
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
	 
      if (height >= ua_data[0][0] && height <= ua_data[numLevs-1][0]) {
	while(1) {
	  if (height >= ua_data[index][0] && height < ua_data[index+1][0]) {
	    U = ua_data[index][1] + ua_data[index+1][3] * 
	      (height-ua_data[index][0]);
		    
	    V = ua_data[index][2] + ua_data[index+1][4] * 
	      (height-ua_data[index][0]);
		    
	    wind=sqrt(pow(U,2)+pow(V,2));

	    if (_sign<0) 
	      offset=0.0;
	    else 
	      offset=PI;
		    
	    if ( U >= 0) 
	      dir = (acos(V/wind)+offset) * 180/PI;
	    else {
	      if (offset == PI)
		dir = (offset-acos(V/wind)) * 180/PI;
	      else // offset == 0
		dir = (2*PI - acos(V/wind)) * 180/PI;
	    }
	    break;
	  } else {
	    if ( height < ua_data[index][0]) 
	      index--;
	    else 
	      index++;
	  }
	}// end while
      } else {
	if ( height > ua_data[numLevs-1][0]) {
	  U = ua_data[numLevs-1][1] + meanShearU*
	    (height-ua_data[numLevs-1][0]);

	  V = ua_data[numLevs-1][2]+ meanShearV*
	    (height-ua_data[numLevs-1][0]);
		
	  wind = sqrt(pow(U,2)+pow(V,2));

	  if (_sign<0) 
	    offset = 0.0;
	  else 
	    offset = PI;
		
	  if (U >= 0) {
	    dir = (acos(V/wind)+offset) * 180/PI;
	  } else {
	    if (offset == PI)
	      dir = (offset-acos(V/wind)) * 180/PI;
	    else // offset == 0
	      dir = (2*PI - acos(V/wind)) * 180/PI;
	  }
	}
      }
      for(currIndex = 0; currIndex < numRays; currIndex++) {	      
	if ( wind >= 0.0 && dir >= 0.0) {
	  az = PI * (soundVolume->sweep[sweepIndex]->ray[currIndex]->h.azimuth)/180.0;
	  wind_val_rv = wind*cos(ang);
	  wind_val_rv = wind_val_rv*cos(PI*dir/180.0-az);
	  soundVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = wind_val_rv;  	 
	  flag=1;
	} else {
	  soundVolume->sweep[sweepIndex]->ray[currIndex]->range[i] = missingValue;
	} 
      } // end for curr index
    } // end for i < num_bins
  }// end for sweepindex < numsweeps

  if (numLevs==0) {
    flag = 0;
  }
  if (flag) 
    success = true;

  //
  // free memory 
  //
  for (int i = 0; i < numPoints + 1; i++)
    delete[](ua_data[i]);

  delete[](ua_data);
     
  return success;
 
}


int FirstGuess::loadSoundingData( time_t issueTime )
{   
  
   //
   // Try to read a sounding
   // 
   int ret = sounding.readSounding( issueTime );

   if ( ret < 0 ) {
      fprintf( stderr, "Cannot read spdb sounding data at %s\n",
                       DateTime::str( issueTime ).c_str() );
      return( ret  );
   }

   if ( ret == 0 ) {
      fprintf(stderr, "No spdb sounding data available at %s\n",
                         DateTime::str( issueTime ).c_str() );
      return( ret);
   }

   DateTime soundingTime = sounding.getLaunchTime();
   string   soundingName = sounding.getSourceName();

   string   timeStampName = "Sounding data: ";
   timeStampName += soundingName;
   
   fprintf( stderr, "Spdb Sounding: Got '%s' at %s\n",
	    soundingName.c_str(), soundingTime.dtime() );

   //   if (success) 
   // outputSoundVolume();

   return( ret );

}

ClassIngest *FirstGuess::loadSoundingDataText( time_t issueTime )
{   
   //
   // Try to read a sounding
   // 
  time_t startTime = issueTime - (time_t)_sounding_look_back*60;

  ClassIngest *classIngest = new ClassIngest(_sounding_url,
    _debug, //  >= Params::DEBUG_VERBOSE,
    _sounding_url,
    startTime, issueTime);

  int ret = classIngest->readSoundingText();

  // get the sounding from classIngest
  if ( ret < 0 ) {
    fprintf( stderr, "Cannot read text sounding data at %s\n",
                       DateTime::str( issueTime ).c_str() );
  } else if ( ret == 0 ) {
    fprintf(stderr, "No text sounding data available at %s\n",
                         DateTime::str( issueTime ).c_str() );
  } else {

    // TODO: fix up this info; grab from classIngest instead of sounding.
    DateTime soundingTime = classIngest->getLaunchTime();
    string   soundingName = classIngest->getSourceName();

    string   timeStampName = "Sounding data: ";
    timeStampName += soundingName;
     
    
    fprintf( stderr, "Text Sounding: Got '%s' at %s\n",
      soundingName.c_str(), soundingTime.dtime() );
    

  }
  if (ret <= 0) {
    if (classIngest != NULL) {
      delete classIngest;
      classIngest = NULL;
    }
  }
  return( classIngest );
}



