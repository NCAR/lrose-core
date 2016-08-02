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
 * Module: Dsr2Radar.cc
 *
 * Author: Sue Dettling
 *
 * Date:   10/5/01
 *
 * Description: Methods for transforming DsrRadarBeams
 *              to an RSL Radar struct.
 *     
 */

// Include files 

#include "Dsr2Radar.hh"
#include "Params.hh"
using namespace std;

//////////////////////////////////////////////////////
//
// Constructor
//
Dsr2Radar::Dsr2Radar(string prog_name, Params &parameters)
{
  progName = prog_name;
 
  params = parameters;

  numFields = 0;
  numGates = 0;
  dataLen = 0;
  unambigRange = 0;
  elevation = 0;
  gateSpacing = 0;
  pulseRepFreq = 0;
  targetElev = -1;
  latitude = -1;
  longitude = -1;
  altitude = -1;
  samplesPerBeam = 0;
  horizBeamWidth = 0;
  wavelength = 0;
  unambigVelocity = 0;
  startRange = 0;
  tiltNum = 0;
  azimuth = 0;
 
  
  nbeamsTilt = 0;
  nbeamsTotal = 0;
  nSweepsVol = 0;

  nbeamsLastTilt = 0;
  nMsgsLastTilt = 0;
  lastTiltTargElev = 0;
  lastTiltNum = 0;
  nMsgsTilt = 0;

  end_of_vol = false;

  firstMsg = true;

  isDeleted = false;
  
  radialVelIndex = -1;
  dbzIndex = -1;

  radar = NULL;
  fieldSweeps = NULL;
  fieldRays = NULL;
}

////////////////////////////////////////////////////////////////
//
// Destructor
//
Dsr2Radar::~Dsr2Radar()
{
  clearData();
}

/////////////////////////////////////////////////////////////////
//
// reformat:  This method takes DsRadarMsgs and fills the various
//            RSL structs: Rays, Sweeps, Volumes, and Radar with
//            DsRadarMsg data 
//  
int Dsr2Radar::reformat(DsRadarMsg &radarMsg, int &contents)
{    
  //
  // If this is the first complete message of the volume, allocate space for 
  // for vectors of rays, sweeps, volumes
  //
  if(firstMsg == true)
    {
      //
      // Initialize data members, allocate memory for 
      // rays, sweeps, volumes.
      // 
      init(radarMsg,contents);
      
    }
  else
    {
      //
      // Initialize variable used to skip beam processing int the case
      // of repeated tilts.
      //
      bool skipBeam = false;
    
      //
      // Update field params, radar params, and radar flags.
      //
      updateParamsFlags(radarMsg,contents);

      //
      // If DsRadarMsg contains beam data, get radar beam and 
      // fill in appropriate data members.
      //
      if (contents & DsRadarMsg::RADAR_BEAM) 
	{      
	  radarBeam = radarMsg.getRadarBeam();
      
	  time_t dataTime = radarBeam.dataTime;
      
	  beamDateTime.set(dataTime);
	  
	  dataLen = radarBeam.dataLen();
	  
	  azimuth = radarBeam.azimuth;
	  
	  elevation = radarBeam.elevation;
	  
	  targetElev = radarBeam.targetElev;
	  
	  tiltNum = radarBeam.tiltNum;
	
          
	  //
	  // If we have a repeated tilt target elevation
	  // either delete it or dont process the beams in the tilt 
	  //
	  if (params.delete_repeat_tilts)
	    {
	      if (targetElev == lastTiltTargElev && tiltNum != lastTiltNum)
		{
		  if(params.delete_tilt_decision == Params::KEEP_RECENT_TILT)
		      {
			//
			// Delete previous sweep.
			//  		 
			if (!isDeleted)
			  {
			    deleteLastSweep();

			    isDeleted = true;
			  }
		      }
		  else 
		    //
		    // We keep the older tilt for this target elevation.
		    // Don't process the beams for this tilt.
		    //
		    skipBeam = true;
		}
	    }
	}

      //
      // Now that we have updated all data members
      // create rays if we have beam data and we arent skipping this tilt.
      //
      if (contents & DsRadarMsg::RADAR_BEAM && skipBeam == false) 
	{
	  beam2rays();
	}
      

      if(radarFlags.endOfTilt)
	{
	  
	  nSweepsVol++;

	  //
	  // Turn off beam skipping flag in case we just finished skipping a tilt.
	  //
	  skipBeam = false;

	  if (params.debug)
	    {
	      cerr << "endOfTilt "  << tiltNum << ": nSweepsVol     = " << nSweepsVol  << endl; 
	      cerr << "             nbeamsTotal    = " << nbeamsTotal << "\n" << endl;

	      cerr << "             nbeamsTilt     = " << nbeamsTilt  << endl;
	      cerr << "             nbeamsLastTilt = " << nbeamsLastTilt << "\n" << endl;
	      
	      
	      cerr << "             nMsgsTilt      = " << nMsgsTilt <<  endl;
	      cerr << "             nMsgsLastTilt  = " << nMsgsLastTilt << "\n" <<endl;
	
	    }

	  //
	  // We just finished collecting beams for a tilt
	  // so create Sweep structs to hold pointers to the beams.
	  //
	  fillSweeps();

	  //
	  // Do some bookkeepping
	  //
	  nbeamsLastTilt = nbeamsTilt;

	  nMsgsLastTilt = nMsgsTilt;

	  lastTiltTargElev = targetElev;

	  lastTiltNum = tiltNum;

	  nbeamsTilt = 0;
	  
	  nMsgsTilt = 0;
	  
	}

      if (end_of_vol)
	{
	  //
	  // We just finished collecting all tilts for the volume so 
	  // create the Volumes and Radar. 
	  //
	  fillVolumes();

	  fillRadar();	  
	}
      
      
      //
      // Save radar message and contents for processed beams
      //
      if(skipBeam == false)
	{
	 
	  DsRadarMsg *rMsg = new DsRadarMsg(radarMsg);
	  
	  int rContents = contents;
	  
	  RADARMSG_CONTENTS* msgContentPair = new RADARMSG_CONTENTS(rMsg, rContents);
	  
	  radarMsgs.push_back( msgContentPair);
	  
	  nMsgsTilt++;
	  
	} 
    }  // else (firstMsg not true...)

  return(0);

}


/////////////////////////////////////////////////////////////
//
// fillRadar: fill radar header, set pointers to volumes
//
void Dsr2Radar::fillRadar()
{
  //
  // Create radar struct, radar header member nvolumes is set, 
  // space for volume pointers allocated.
  // We leave space for the edited velocity field.
  //
  radar = RSL_new_radar(numFields + 1);

  //
  // Year
  // 
  radar->h.year = beamDateTime.getYear();
  
  //
  // Month (1-12)
  //
  radar->h.month = beamDateTime.getMonth();
      
  //
  // Day (1-31)
  //
  radar->h.day = beamDateTime.getDay();
      
  //
  // Hour (0-23)
  //
  radar->h.hour  = beamDateTime.getHour();
      
  //
  // Minute ( 0-59)
  //
  radar->h.minute = beamDateTime.getMin();
      
  //
  // Second + fraction of a second
  //
  radar->h.sec = beamDateTime.getSec();


  // radar->h.radar_type;
  // radar->h.number;
  // radar->h.name;
  // radar->h.radr_name;
  // radar->h.city;
  // radar->h.state;
  
  //
  // record latitude--convert to degress, minutes, seconds
  //
  radar->h.latd = (int) latitude;
 
  radar->h.latm = (int) ((latitude - radar->h.latd) * 60);
 
  radar->h.lats =
    (int)  (((latitude - radar->h.latd) * 60 - radar->h.latm) * 60);
 
  //
  // record longitude--convert to degress, minutes, seconds
  //
  radar->h.lond = (int) longitude;
 
  radar->h.lonm = (int) ((longitude - radar->h.lond) * 60) ;
 
  radar->h.lons =
    (int) (((longitude - radar->h.lond) * 60 - radar->h.lonm) * 60);
 
  //
  // antenna height in meters.
  // altitude has units of km.
  //
  radar->h.height = (int) (altitude * 1000 + .5);

  //
  // Short and long pulses ( ns)
  //
  //int radar->h.spulse = 0;

  //int radar->lpulse = 0;
 

  //
  // Fill radar with volumes
  //
  for (int k = 0; k < numFields; k++)
    radar->v[k] = volumes[k];
}

////////////////////////////////////////////////////////////////////
//
// fillVolumes: Fill volume headers for each field, 
//              set pointers to sweeps of each volume.
//
void Dsr2Radar::fillVolumes()
{

  //
  // Initialize field params iterator.
  //
  vector <DsFieldParams*> :: const_iterator j;
  j = fieldParams.begin();

  //
  // Foreach field, create a volume.
  //
  for (int i = 0; i < numFields; i++)
    {
      //
      // Create volume struct, volume header member nsweeps is set, 
      // space for sweep pointers allocated.  
      //
      Volume *volume =  RSL_new_volume(nSweepsVol); 
      
      volume->h.f = NULL;

      volume->h.invf = NULL;

      volume->h.calibr_const = 0;
     
      //
      // Fill vol with sweeps
      //
      for (int k = 0; k < nSweepsVol; k++)
	volume->sweep[k] = fieldSweeps[i][k];

      //
      // record data type in volume
      //
      if ( strncmp("DB", (*j)->name.c_str(),2 ) == 0)
	{
	  volume->h.type_str = new char[13];
	  sprintf(volume->h.type_str,"Reflectivity");
	}
      else if ( strncmp("VE", (*j)->name.c_str(),2 ) == 0)
        {
	  volume->h.type_str = new char[9];
	  sprintf(volume->h.type_str,"Velocity");
	}
      else 
        {
	  volume->h.type_str = new char[9];
	  sprintf(volume->h.type_str,"%8s", (*j)->name.c_str() );
	}
      
      //
      // Push volume on vector of volumes
      //
      volumes.push_back(volume);

      //
      // Increment the field params iterator
      //
      j++;

    }
}


///////////////////////////////////////////////////////////////////////
//
// fillSweeps: For each field, create sweeps and set pointers to rays.
//
void Dsr2Radar::fillSweeps()
{

  int startRayNum;
 
  for (int i = 0; i < numFields; i++)
    {
      //
      // Create sweep struct, sweep header member nrays is set, 
      // space for ray pointers allocated.  
      //
      Sweep *sweep = RSL_new_sweep(nbeamsTilt);
      
      sweep->h.sweep_num = tiltNum;
      
      sweep->h.elev = targetElev;
      
      sweep->h.beam_width = horizBeamWidth;
  
      sweep->h.vert_half_bw = horizBeamWidth/2;

      sweep->h.horz_half_bw = horizBeamWidth/2;

      sweep->h.f = NULL;

      sweep->h.invf = NULL;

      startRayNum = nbeamsTotal - nbeamsTilt;
      
      //
      // Fill sweep with rays
      //
      for (int k = 0; k < nbeamsTilt ; k++)
	sweep->ray[k] = fieldRays[i][startRayNum + k];

      //
      // Push sweep on vector containing sweeps of like field data
      //
      fieldSweeps[i].push_back(sweep); 
    }
}

////////////////////////////////////////////////////////////////////
//
// beam2rays: Create rays from member radarBeam( one ray per field)
//
void Dsr2Radar::beam2rays()
{

  //
  // Initialize field params iterator.
  //
  vector <DsFieldParams*> :: const_iterator j;
  j = fieldParams.begin();

  //
  // get bytedata from radarBeam.
  //
  unsigned char *byteData = radarBeam.data();
  
  for (int i = 0; i < numFields; i++)
    {
      //
      // Allocate memory for the ray and the range array.
      //
      Ray *ray = RSL_new_ray(numGates);
      
      //
      // Month (1-12)
      //
      ray->h.month = beamDateTime.getMonth();
      
      //
      // Day (1-31)
      //
      ray->h.day = beamDateTime.getDay();
      
      //
      // Hour (0-23)
      //
      ray->h.hour  = beamDateTime.getHour();
      
      //
      // Minute ( 0-59)
      //
      ray->h.minute = beamDateTime.getMin();
      
      //
      // Second + fraction of a second
      //
      ray->h.sec = beamDateTime.getSec();
      
      //
      // Unambiguous Range (km) 
      //
      //ray->h.unam_range = unambigRange;
      
      //
      // Azimuth angle (degrees).
      // This angle is the mean azimuth for the whole ray
      // Must be positive 0 == North, 90 == east, 270 == west
      //
      ray->h.azimuth = azimuth;
      
      //
      // Ray number within this elevation scan
      // 
      ray->h.ray_num = 0;  
	
      //
      // Elevation angle (degrees)
      // 
      ray->h.elev = elevation;
	
      //
      // Elevation number within scan. (Tilt number)
      // 
      ray->h.elev_num =  tiltNum;
	
      //
      // Range to first gate (meters)
      // 
      ray->h.range_bin1 = (int)startRange;
	
      //
      // Data gate size (meters)
      // 
      ray->h.gate_size = (int)gateSpacing;
	
      //
      // Doppler velocity resolution
      // 
      ray->h.vel_res = 0;
	
      //
      // Sweep rate (full sweeps/minute)
      // 
      ray->h.sweep_rate = 0; 
	
      //
      // Pulse repetition frequency ( Hz)
      //
      ray->h.prf = (int)pulseRepFreq;
	
      //
      //  
      //
      ray->h.azim_rate = 0;
	
      //
      // 
      //
      ray->h.fix_angle = (int)targetElev;
	
      //
      // Pitch angle
      //
      ray->h.pitch =  0;
	
	
      //
      // Roll angle
      //
      ray->h.roll = 0;
	
      //
      // Heading
      // 
      ray->h.heading = 0;
	
      //
      // Pitch rate (angle/sec)
      //
      ray->h.pitch_rate = 0;	
	
      //
      // Roll rate (angle/sec)
      //
      ray->h.roll_rate = 0;
	
      //
      // Heading rate (angle/sec)
      // 
      ray->h.heading_rate = 0;
	
      //
      // Latitude (degrees)
      // 
      ray->h.lat = latitude;
      
      //
      // Longitude (degrees)
      //
      ray->h.lon = longitude;
      
      //
      // Altitude (meters)
      //
      ray->h.alt = (int)altitude;
      
      //
      // Radial velocity correction
      //
      ray->h.rvc = 0;
	
      //
      // Platform velcity to the east (m/sec)
      //
      ray->h.vel_east = 0;
	
      //
      // Platform velcity to the north (m/sec)
      //
      ray->h.vel_north = 0;
  
      //
      // Platform velcity  up (m/sec)
      //
      ray->h.vel_up = 0;
  
      //
      // 
      //
      ray->h.pulse_count = samplesPerBeam;
  
      //
      // Pusle width (microseconds)
      //
      ray->h.pulse_width = 1/pulseRepFreq * 100000;
  
      //
      // Beam width (degrees)
      // 
      ray->h.beam_width = horizBeamWidth;
  
      //
      // Bandwidth (MHz)
      // 
      ray->h.frequency = 0;
	
      //
      // Wavelength (meters) 
      // 
      ray->h.wavelength = radarParams.wavelength;
      
      //
      // Nyquist Velocity (meters/second)
      //
      if ( unambigVelocity != 0)
	ray->h.nyq_vel = unambigVelocity;
      else
	ray->h.nyq_vel = pulseRepFreq * wavelength/4 ;
      
      //
      // Data conversion function
      //
      ray->h.f = NULL;

      //
      // Data conversion function
      //
      ray->h.f = NULL;

      //
      // Number of array elements for Range struct
      // 
      ray->h.nbins = numGates;
      
      //
      // Jump gate to gate and record byte
      // data for particular field.
      //
      int l = 0;
      for(int k = i ; k < dataLen; k += numFields)
	{
	  ray->range[l] = byteData[k];
	  l++;
	} 

      //
      // record scale and bias 
      // 
      ray->h.scale = (*j)->scale;
      ray->h.bias = (*j)->bias;
      
      //
      // Push ray onto vector containing rays of the same field  
      //
      fieldRays[i].push_back(ray);

      //
      // increment field params iterator
      //
      j++;

    }

  //
  // Update beam count.
  //
  nbeamsTotal++;
  
  nbeamsTilt++;
}

///////////////////////////////////////////////////////////////////
//
// Delete data, re-initialize all members
//
void Dsr2Radar::clearData()
{
  //
  // Free Radar struct. Note that this function will free all volumes,
  // sweeps of each volume, and rays of each sweep.
  //
  if (radar)
    RSL_free_radar(radar);
  
  radar = NULL;

  //
  // Erase vectors of Sweep and Ray pointers, 
  // delete the char* type_str for each volume.
  //
  for( int i = 0; i < numFields ; i++)
   {
     fieldRays[i].erase( fieldRays[i].begin(), fieldRays[i].end());

     fieldSweeps[i].erase( fieldSweeps[i].begin(), fieldSweeps[i].end());
     
   }
  
  //
  // delete arrays of vectors of Sweep and Ray pointers
  //

  if (fieldRays)
    {
      delete[](fieldRays);
      fieldRays = NULL;
    }

  if (fieldSweeps)
    {
      delete[](fieldSweeps);
      fieldSweeps = NULL;
    }
  
  //
  // Erase vectors of Volumes ptrs.
  //
  volumes.erase( volumes.begin(), volumes.end());
  

  //
  // clear other data members:
  //
  numFields = 0;
  numGates = 0;
  dataLen = 0;
  unambigRange = 0;
  elevation = 0;
  gateSpacing = 0;
  pulseRepFreq = 0;
  targetElev = -1;
  latitude = -1;
  longitude = -1;
  altitude = -1;
  samplesPerBeam = 0;
  horizBeamWidth = 0;
  wavelength = 0;
  unambigVelocity = 0;
  startRange = 0;
  tiltNum = 0;
  azimuth = 0;

  lastTiltTargElev = 0;
  lastTiltNum = 0;
  nbeamsTilt = 0;
  nbeamsTotal = 0;  
  nSweepsVol = 0;

  nbeamsLastTilt = 0;
  nMsgsLastTilt = 0;
  nMsgsTilt = 0;

  firstMsg = true;
  end_of_vol = false;
  
  isDeleted = 0;

  radialVelIndex = -1;
  dbzIndex = -1;

  //
  // Clear out old radarMsgs.
  //
  vector < RADARMSG_CONTENTS* > :: const_iterator i;

  for ( i = radarMsgs.begin(); i != radarMsgs.end(); i++)
    {
      delete (*i)->first;
      delete *i;
    }

  radarMsgs.erase(radarMsgs.begin(),radarMsgs.end());

  
}

///////////////////////////////////////////////////////////////////////
//
// Initialize data members( only if message contains all flags, params) 
//
void Dsr2Radar::init(DsRadarMsg &radarMsg, int &contents)
{
  //
  // Return if not all flags and params in set
  //
  if (!radarMsg.allParamsSet() )
    return;

  numFields = radarMsg.numFields();

  //
  // get radar params and relevant data members 
  //
  
  radarParams = radarMsg.getRadarParams();
      
  numGates = radarParams.getNumGates();
  
  unambigRange = radarParams.unambigRange;
  
  gateSpacing = radarParams.gateSpacing * 1000;
  
  pulseRepFreq = radarParams.pulseRepFreq;
  
  samplesPerBeam =  radarParams.samplesPerBeam;
  
  horizBeamWidth = radarParams.horizBeamWidth;
  
  wavelength = radarParams.wavelength/100;
  
  unambigVelocity = radarParams.unambigVelocity;
  
  startRange = radarParams.startRange * 1000;
  
  //
  // record latitude, lon , alt:
  // check for location override and adjust as necessary
  //     
  if(params.override_radar_location == TRUE)
    {
      latitude = params.radar_location.latitude;
      longitude = params.radar_location.longitude;
      altitude = params.radar_location.altitude * 1000;
    }
  else
    {
      latitude = radarParams.latitude;
      longitude = radarParams.longitude;
      altitude = radarParams.altitude * 1000;
    }


  //
  // get field params
  //
  fieldParams = radarMsg.getFieldParams();

  //
  // Update radar flags and relevant data members.
  // Check for end of volume decision.
  //
  
  radarFlags = radarMsg.getRadarFlags();

  if (params.end_of_vol_decision == Params::END_OF_VOL_FLAG) 
    {
      if (radarFlags.endOfVolume) 
	{
	  end_of_vol = true;
	}
    } 
  else 
    if (radarFlags.endOfTilt &&
	radarFlags.tiltNum == params.last_tilt_in_vol) 
      {
	end_of_vol = true;
      } 
    else 
      if (radarFlags.newScanType) 
	{
	  end_of_vol = true;
	}      

  // 
  // Allocate memory for holding the RSL
  // structs rays, sweeps, volumes.
  //
  fieldRays = new (vector <Ray*>)[numFields];
  
  fieldSweeps = new (vector <Sweep*>)[numFields];

  //
  // record radial velocity field position in field order
  //
  vector <DsFieldParams*> :: const_iterator j;
  j = fieldParams.begin();
  for (int i = 0; i < numFields; i++)
    {
      if ( strncmp("VE", (*j)->name.c_str(),2 ) == 0)
	{
	  radialVelIndex = i;
	}
      if ( strncmp("DB", (*j)->name.c_str(),2 ) == 0)
	{
	  dbzIndex = i;
	}
      j++;
    }

  firstMsg = false;
}


///////////////////////////////////////////////////////////////////////
//
// updateParamsFlags: Update params and flags if present in DsRadarMsg
//
void Dsr2Radar::updateParamsFlags(DsRadarMsg &radarMsg, int &contents)
{

  numFields = radarMsg.numFields();

  //
  // Update radar params and relevant data members 
  //
  if (contents & DsRadarMsg::RADAR_PARAMS) 
    {
      radarParams = radarMsg.getRadarParams();
      
      numGates = radarParams.getNumGates();
      
      unambigRange = radarParams.unambigRange;
      
      gateSpacing = radarParams.gateSpacing * 1000;
      
      pulseRepFreq = radarParams.pulseRepFreq;
      
      samplesPerBeam =  radarParams.samplesPerBeam;
      
      horizBeamWidth = radarParams.horizBeamWidth;
      
      wavelength = radarParams.wavelength/100;
      
      unambigVelocity = radarParams.unambigVelocity;
      
      startRange = radarParams.startRange * 1000;
      
      //
      // record latitude, lon , alt:
      // check for location override and adjust as necessary
      //     
      if(params.override_radar_location == TRUE)
	{
	  latitude = params.radar_location.latitude;
	  longitude = params.radar_location.longitude;
	  altitude = params.radar_location.altitude * 1000;
	}
      else
	{
	  latitude = radarParams.latitude;
	  longitude = radarParams.longitude;
	  altitude = radarParams.altitude * 1000;
	}
    }
  
  //
  // Update field params
  //
  if (contents & DsRadarMsg::FIELD_PARAMS) 
    fieldParams = radarMsg.getFieldParams();

  //
  // Update radar flags and relevant data members.
  // Check for end of volume decision.
  //
  if (contents & DsRadarMsg::RADAR_FLAGS) 
    {
      radarFlags = radarMsg.getRadarFlags();

      if (params.end_of_vol_decision == Params::END_OF_VOL_FLAG) 
	{
	  if (radarFlags.endOfVolume) 
	    {
	      end_of_vol = true;
	    }
	} 
      else 
	if (radarFlags.endOfTilt &&
	    radarFlags.tiltNum == params.last_tilt_in_vol) 
	  {
	    end_of_vol = true;
	  } 
	else 
	  if (radarFlags.newScanType) 
	    {
	      end_of_vol = true;
	    }      
    }
}

//////////////////////////////////////////////////////////////////
//
// writeVol: Write DsRadarMsgs to fmq. Replace original beam data
//           with data from RSL rays.
//
void Dsr2Radar::writeVol(DsRadarQueue &outputQueue)
{
  
  if( params.debug )
    {
      cerr <<  "Writing " << radarMsgs.size() << " radar messages to fmq." << endl;
    }
 
  //
  // initialize radar beam iterator
  //
  int beam_it = 0;

  vector < RADARMSG_CONTENTS* > :: const_iterator msg_it; 

  for ( msg_it = radarMsgs.begin(); msg_it != radarMsgs.end(); msg_it++)
    {	      
      //
      // Get original DsRadarMsg and contents
      //
      DsRadarMsg rMsg = *((*msg_it)->first);

      int contents = (*msg_it)->second;

      //
      // If radar message contains beam data, replace original data
      // with beam data from RSL rays.
      //    
      if ( contents & DsRadarMsg::RADAR_BEAM)
	{
	  DsRadarBeam &rbeam = rMsg.getRadarBeam();
	  
	  unsigned char *beamData = new unsigned char[ numFields * numGates ];

	  //
	  // The DsRadarBeam data is stored gate by gate 
	  // and contains values for each field at each gate 
	  //
	  int l = 0;
	  for (int k = 0; k < numGates ; k++)
	    {
		for( int j = 0; j < numFields; j++)
		{
		  
		  beamData[l] = fieldRays[j][beam_it]->range[k];

		  l++;
		}
	    }

	  //
	  // load beam data
	  //
	  rbeam.loadData(beamData, numFields*numGates);
	  
	  //
	  // free memory
	  //
	  delete[](beamData);

	  //
	  // increment beam iterator
	  //
	  beam_it++;

	} // end if there is beam data... 
      
      //
      // send off message
      //
      if(params.debug)
	{
	  if(beam_it%500 == 0)
	    cerr << "writing beam " <<  beam_it << endl;
	}

      if( outputQueue.putDsMsg( rMsg, contents ) != 0 ) 
	{
	  fprintf( stderr, "Could not write message to message queue" );
	}      
    } // end for ...  
}


/////////////////////////////////////////////////////////////////////
//
// deleteLastSweep: delete last sweep created. Delete corresponding 
//                  DsRadarMsg-contents pairs. Erase spots in appropriate
//                  vectors which held pointers to the sweep, the rays.
//                  and the DsRadarMsg-contents pair.
//
void Dsr2Radar::deleteLastSweep()
{
 
  if(params.debug)
    {
      cerr << " ============= Deleting last sweep:  tiltNum  " << lastTiltNum <<  ", targElev " << lastTiltTargElev << " =============\n" << endl;
      cerr << "           nMsgsLastTilt  : " << nMsgsLastTilt    << endl;
      cerr << "           total Msgs     : " << radarMsgs.size() << "\n" << endl;

      cerr << "           nbeamsLastTilt : " << nbeamsLastTilt << endl;
      cerr << "           total beams    : " << fieldRays[0].size() <<"\n" << endl;
    }

  //
  // Free sweep for each field. Note that the rays of the sweep
  // will be freed through the RSL_free_sweep( Sweep *)
  //
  for( int i = 0; i < numFields; i++)
    {
      int n = fieldSweeps[i].size();
 
      if ( n > 0 )
	RSL_free_sweep( fieldSweeps[i][n - 1] );

      fieldSweeps[i].erase(fieldSweeps[i].end() - 1, fieldSweeps[i].end()); 
    }

  //
  // Delete original DsRadarMsg, content pairs, delete pointers
  // to these objects, erase the spots they occupied in the vector.
  //
  vector < RADARMSG_CONTENTS* > :: iterator msg_it = radarMsgs.end() - 1;

  int delMsgCount = 0;
 
  while( delMsgCount <  nMsgsLastTilt) 
    {
      //
      // Delete DsRadarMsg pair
      //
      delete (*msg_it)->first;
      
      delete *msg_it;

      msg_it--;

      delMsgCount++;
      
    } 

   msg_it = radarMsgs.erase(msg_it + 1, radarMsgs.end());

   //
   // Erase ray pointers corresponding to  Rays from last tilt.
   //
   int delBeamCount = 0;
   
   for( int i = 0; i < numFields; i++)
    {
      delBeamCount = 0;

      vector < Ray* > :: iterator ray_it = fieldRays[i].end() - 1;

      while ( delBeamCount < nbeamsLastTilt )
      {	
	if ( delBeamCount != nbeamsLastTilt)
	  ray_it--;
      
	delBeamCount++;
      }

      fieldRays[i].erase(ray_it + 1, fieldRays[i].end());
    } 

   //
   // Do some bookkeeping.
   //
   nSweepsVol--;
   nbeamsTotal = nbeamsTotal - nbeamsLastTilt;
   nMsgsLastTilt = 0;
   nbeamsLastTilt = 0;

   if (nSweepsVol >= 1)
     {
       lastTiltTargElev = ( *(fieldSweeps[0].end() - 1) )->h.elev;
       lastTiltNum = ( *(fieldSweeps[0].end()- 1) )->h.sweep_num;
     }
   else
     {
        lastTiltTargElev = 0;
	lastTiltNum = 0;
     }

   if(params.debug)
    {
      cerr << "           Msgs  left    : " << radarMsgs.size()       << endl;
      cerr << "           Beams left    : " << fieldRays[0].size() << "\n" << endl;
      cerr << "           lastTiltTargElev now " << lastTiltTargElev << endl;
      cerr << "           lastTiltNum now      " << lastTiltNum << "\n" << endl;

      cerr << "==================== Sweep Deleted =========================\n" << endl;
    }
}      


Volume* Dsr2Radar::getVelVolume()
{
  if ( radialVelIndex < (int)volumes.size() &&  radialVelIndex != -1)
    return( volumes[ radialVelIndex ] );
  else 
    return (NULL);
}

Volume* Dsr2Radar::getDbzVolume()
{
  if ( dbzIndex < (int) volumes.size() &&  dbzIndex != -1)
    return( volumes[ dbzIndex ] );
  else 
    return (NULL);
}

time_t Dsr2Radar::getVolTime()
{
  
  if (radar != NULL )
    {
      DateTime *volTime = new DateTime(radar->h.year,   radar->h.month,
				       radar->h.day,    radar->h.hour,
				       radar->h.minute, radar->h.sec);
      return( volTime->utime() );

      delete volTime;
    }
  else 
    return -1;
}









