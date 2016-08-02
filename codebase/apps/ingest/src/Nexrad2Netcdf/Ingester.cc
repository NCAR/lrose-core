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
// $Id: Ingester.cc,v 1.20 2016/03/07 01:23:03 dixon Exp $
//
//////////////////////////////////////////////////////
#include <ctype.h>
#include <toolsa/pmu.h>
#include <rapformats/swap.h>
#include <toolsa/MsgLog.hh>
#include "Ingester.hh"
#include "Params.hh"
#include "SweepData.hh"

Ingester::Ingester(Params *P) 
{
  params = P;
  firstBeam      = true;
  fileStart      = 0;
  fileEnd        = 0;
  curElevNumber = -1;
  prevElevNumber = -1;
  currentSweep   = NULL;
  prevSweep      = NULL;
  ncOutput       = NULL;
  bypassMap      = NULL;
  newBypassMap   = NULL;
  clutterMap     = NULL;
  newClutterMap  = NULL;
  vcpData        = NULL;
  newVcpData     = NULL;
  statusData     = NULL;
  newStatusData  = NULL;
  adaptData      = NULL;
  newAdaptData   = NULL;

  ncOutput = new NcOutput( params );
  
  PMU_auto_register( "Setting up sweeps" ); 
  
  currentSweep = new SweepData( params );
  prevSweep    = new SweepData( params );
  
}

Ingester::~Ingester() 
{
  delete currentSweep;
  delete prevSweep;
  delete ncOutput;
  if(clutterMap)
    deleteClutterMap(clutterMap);
  if(newClutterMap)
    deleteClutterMap(newClutterMap);
  if(bypassMap)
    deleteBypassMap(bypassMap);
  if(newBypassMap)
    deleteBypassMap(newBypassMap);
  if(vcpData)
    delete vcpData;
  if(newVcpData)
    delete newVcpData;
  if(statusData)
    delete statusData;
  if(newStatusData)
    delete newStatusData;
  if(adaptData)
    delete adaptData;
  if(newAdaptData)
    delete newAdaptData;

  currentSweep   = NULL;
  prevSweep      = NULL;
  ncOutput       = NULL;
  bypassMap      = NULL;
  newBypassMap   = NULL;
  clutterMap     = NULL;
  newClutterMap  = NULL;
  vcpData        = NULL;
  newVcpData     = NULL;
  statusData     = NULL;
  newStatusData  = NULL;
  adaptData      = NULL;
  newAdaptData   = NULL;
}

void Ingester::addAdaptation(RIDDS_adaptation_data *adapt_data)
{
  if(newAdaptData)
    delete newAdaptData;
  newAdaptData = adapt_data;
}

void Ingester::addBypassMap(BypassMap_t *bypass_Map)
{
  if(newBypassMap)
    deleteBypassMap(newBypassMap);
  newBypassMap = bypass_Map;
}

void Ingester::addVCP(VCP_data_t *vcp_data)
{
  if(newVcpData)
    delete newVcpData;
  newVcpData = vcp_data;
}

void Ingester::addStatus(RIDDS_status_data *status_data)
{
  if(newStatusData) {
    delete newStatusData;
  }
  newStatusData = status_data;
}

RIDDS_status_data *Ingester::getStatus()
{
  return statusData;
}

RIDDS_elevation_angle *Ingester::getElevationAngle()
{
  if(vcpData) {
    int cn = curElevNumber-1;
    int pn = prevElevNumber-1;
    if(currentSweep->merged()) {
      if(vcpData->angle[cn].surveillance_prf_num == 0 && 
	 vcpData->angle[pn].surveillance_prf_num > 0) {
	vcpData->angle[cn].surveillance_prf_num = vcpData->angle[pn].surveillance_prf_num;
	vcpData->angle[cn].surveillance_prf_pulse_count = vcpData->angle[pn].surveillance_prf_pulse_count;
      }

      if(vcpData->angle[cn].surveillance_prf_num > 0 && 
	 vcpData->angle[pn].surveillance_prf_num == 0) {
	vcpData->angle[pn].surveillance_prf_num = vcpData->angle[cn].surveillance_prf_num;
	vcpData->angle[pn].surveillance_prf_pulse_count = vcpData->angle[cn].surveillance_prf_pulse_count;
	return &(vcpData->angle[pn]);
      }

    }
    return &(vcpData->angle[cn]);
  }
  return NULL;
}

BypassSegment_t *Ingester::getBypassSegment()
{
  if(bypassMap == NULL) {
    POSTMSG( DEBUG, "No Bypass Map found." );
    return NULL;
  }
  if(adaptData == NULL) {
    POSTMSG( DEBUG, "No Adaptation data found. Cannot write out bypass map." );
    return NULL;
  }
  float elevation = currentSweep->getFixedAngle();
  int segment = 0;
  if(elevation > adaptData->seg1lim && bypassMap->num_message_segs > 1) {
    segment = 1;
    if(elevation > adaptData->seg2lim && bypassMap->num_message_segs > 2) {
      segment = 2;
      if(elevation > adaptData->seg3lim && bypassMap->num_message_segs > 3) {
	segment = 3;
	if(elevation > adaptData->seg4lim && bypassMap->num_message_segs > 4) {
	  segment = 4;
	}
      }
    }
  }
  if(segment < bypassMap->num_message_segs && bypassMap->segment[segment] != NULL)
    return bypassMap->segment[segment];

  POSTMSG( ERROR, "Error calculationg bypass map segment from elevation." );
  return NULL;
}

void Ingester::deleteBypassMap(BypassMap_t *map)
{
  for(int a = 0; a < map->num_message_segs; a++)
    delete map->segment[a];
  delete map;
  map = NULL;
}

void Ingester::addClutterMap(ClutterMap_t *clutter_Map)
{
  if(newClutterMap)
    deleteClutterMap(newClutterMap);
  newClutterMap = clutter_Map;
}

ClutterSegment_t *Ingester::getClutterSegment()
{
  if(clutterMap == NULL) {
    POSTMSG( DEBUG, "No Clutter Map found." );
    return NULL;
  }
  if(adaptData == NULL) {
    POSTMSG( DEBUG, "No Adaptation data found. Cannot write out clutter map." );
    return NULL;
  }
  float elevation = currentSweep->getFixedAngle();
  int segment = 0;
  if(adaptData->seg1lim > 0.0 && elevation > adaptData->seg1lim && clutterMap->num_message_segs > 1) {
    segment = 1;
    if(adaptData->seg2lim > 0.0 && elevation > adaptData->seg2lim && clutterMap->num_message_segs > 2) {
      segment = 2;
      if(adaptData->seg3lim > 0.0 && elevation > adaptData->seg3lim && clutterMap->num_message_segs > 3) {
	segment = 3;
	if(adaptData->seg4lim > 0.0 && elevation > adaptData->seg4lim && clutterMap->num_message_segs > 4) {
	  segment = 4;
	}
      }
    }
  }
  if(segment < clutterMap->num_message_segs && clutterMap->segment[segment] != NULL)
    return clutterMap->segment[segment];

  POSTMSG( ERROR, "Error calculationg clutter map segment from elevation." );
  return NULL;
}

void Ingester::deleteClutterMap(ClutterMap_t *map)
{
  for(int a = 0; a < map->num_message_segs; a++)
    delete map->segment[a];
  delete map;
  map = NULL;
}

void Ingester::_setFirstBeamInfo(time_t beamTime, int msec_past_midnight, int elev_num, 
				 float elevation)
{

  fileStart = time( NULL );
  
  //
  // Set times for output file
  //
  ncOutput->setVolStartTime( beamTime );
  ncOutput->setBaseTime( beamTime, msec_past_midnight );
  ncOutput->setRadarInfo(params->radarName, params->radarLocation.altitude,
			 params->radarLocation.latitude, params->radarLocation.longitude,
			 params->radarId, params->siteName);

  //
  // Set flags for next message
  //
  firstBeam      = false;
  curElevNumber = elev_num;
  curElevation = elevation;

}

void Ingester::_setFirstBeamInfo(time_t beamTime, int msec_past_midnight, int elev_num, float elevation, 
				 char *radarName, int radarHeight, float radarLat, float radarLon)
{

  fileStart = time( NULL );

  radarName[0] = toupper(radarName[0]);
  radarName[1] = toupper(radarName[1]);
  radarName[2] = toupper(radarName[2]);
  radarName[3] = toupper(radarName[3]);

  //
  // Set times for output file
  //
  ncOutput->setVolStartTime( beamTime );
  ncOutput->setBaseTime( beamTime, msec_past_midnight );
  if(radarName[0] == ' ' && radarName[1] == ' ' && radarName[2] == ' ' && radarName[3] == ' ')
    ncOutput->setRadarInfo(params->radarName, params->radarLocation.altitude,
			   params->radarLocation.latitude, params->radarLocation.longitude,
			   params->radarId, params->siteName);
  else
    ncOutput->setRadarInfo(radarName, (double)radarHeight / 1000.0, radarLat, radarLon);

  //
  // Set flags for next message
  //
  firstBeam      = false;
  curElevNumber = elev_num;
  curElevation = elevation;

  // Set auxillary header info

  if(newVcpData) {
    if(vcpData)
      delete vcpData;
    vcpData = newVcpData;
    newVcpData = NULL;
  }
  if(vcpData)
    currentSweep->setVcpInfo(&(vcpData->hdr), getElevationAngle());

  if(newStatusData) {
    if(statusData)
      delete statusData;
    statusData = newStatusData;
    newStatusData = NULL;
  }
  if(statusData)
    currentSweep->setStatusData(statusData);

  if(newAdaptData) {
    if(adaptData)
      delete adaptData;
    adaptData = newAdaptData;
    newAdaptData = NULL;
  }

  if(newBypassMap) {
    if(bypassMap)
      deleteBypassMap(bypassMap);
    bypassMap = newBypassMap;
    newBypassMap = NULL;
  }
  currentSweep->setBypassSegment(getBypassSegment());

  if(newClutterMap) {
    if(clutterMap)
      deleteClutterMap(clutterMap);
    clutterMap = newClutterMap;
    newClutterMap = NULL;
  }
  currentSweep->setClutterSegment(getClutterSegment());

}

void Ingester::_setNewElevationInfo(time_t beamTime, int msec_past_midnight, int elev_num,
				    float elevation, bool volumeTitleSeen, int radial_status)
{

  //
  // If we have at least some of the data from the fields
  // we want...
  //
  if( currentSweep->allFieldsPresent() ) {
    
    // If necessary, fill in missing rec data
    currentSweep->fillRec();

    // If needed and requested, calculated pulse count
    currentSweep->calculatePulseCount();

    PMU_auto_register( "Writing sweep" );
    
    if( ncOutput->writeFile( currentSweep ) != 0 ) {
      POSTMSG( ERROR, "Failed to write out NetCdf file '%s'\n", ncOutput->getFileName() );
    }
    
    fileEnd = time( NULL );
    POSTMSG( DEBUG, "Process time = %d seconds", fileEnd - fileStart );
    
    //
    // If the previous sweep was not merged, write
    // out a warning
    if( currentSweep->sweepLost() ) {
      POSTMSG( WARNING, "Sweep number %d was not merged", 
	       prevSweep->getElevIndex() );
    }

    // Don't delete prev sweep if it is the Survalence scan in a MPDA vcp.
    if(elevation < prevElevation + 0.1 && prevSweep->getScanType() == SweepData::REFL_ONLY && 
       (prevSweep->getVCP() == 121 || prevSweep->getVCP() == 112 || prevSweep->getVCP() == 113) )
    {
       
    } else {
      prevSweep->clear();
      prevElevNumber = -1;
    }

    currentSweep->clear();
    
    fileStart = time( NULL );
    
    //
    // Increment the sweep number and set the base time.
    ncOutput->setBaseTime( beamTime, msec_past_midnight );
    
    //
    // Increment the volume number and set the start of volume
    // time.  
    if( radial_status == 3 || volumeTitleSeen ||
	elev_num < prevElevNumber ) {
      ncOutput->setVolStartTime( beamTime );
    }
    
  }
  else {

    // Current sweep is not complete keep it around
    // as the previous sweep.
    
    SweepData *temp = prevSweep;
    
    prevElevNumber = curElevNumber;
    prevElevation = curElevation;
    prevSweep    = currentSweep;
    currentSweep = temp;
    
    currentSweep->clear();
    
    POSTMSG( DEBUG, "Sweep not complete saving for possible merge." );

  }

  //
  // Keep track of the elevation number
  curElevNumber = elev_num;  
  curElevation = elevation;

  // Set auxillary header info
  if(newVcpData) {
    if(vcpData)
      delete vcpData;
    vcpData = newVcpData;
    newVcpData = NULL;
  }
  if(vcpData)
    currentSweep->setVcpInfo(&(vcpData->hdr), getElevationAngle());

  if(newStatusData) {
    if(statusData)
      delete statusData;
    statusData = newStatusData;
    newStatusData = NULL;
  }
  if(statusData)
    currentSweep->setStatusData(statusData);

  if(newAdaptData) {
    if(adaptData)
      delete adaptData;
    adaptData = newAdaptData;
    newAdaptData = NULL;
  }

  if(newBypassMap) {
    if(bypassMap)
      deleteBypassMap(bypassMap);
    bypassMap = newBypassMap;
    newBypassMap = NULL;
  }
  currentSweep->setBypassSegment(getBypassSegment());

  if(newClutterMap) {
    if(clutterMap)
      deleteClutterMap(clutterMap);
    clutterMap = newClutterMap;
    newClutterMap = NULL;
  }
  currentSweep->setClutterSegment(getClutterSegment());

}

status_t Ingester::addMsg1Beam(RIDDS_data_hdr* nexradData, bool volumeTitleSeen) 
{
  PMU_auto_register( "Ingester::addMsg1Beam" );
  
  //
  // Find the time associated with this beam
  time_t beamTime = ((nexradData->julian_date - 1) * 86400 +
		     nexradData->millisecs_past_midnight / 1000);
  
  if( firstBeam ) {
    
    _setFirstBeamInfo(beamTime, nexradData->millisecs_past_midnight, nexradData->elev_num,
		      nexradData->elevation);
    
    if( currentSweep->setInfo( nexradData ) != ALL_OK ) {
      POSTMSG( ERROR, "Could not get information needed to process "
	       "the sweep" );
      return( FAILURE );
    }
    
  }
  else if( nexradData->elev_num != curElevNumber ) {
    // Beginning of a new tilt

    _setNewElevationInfo(beamTime, nexradData->millisecs_past_midnight, nexradData->elev_num, 
			 nexradData->elevation, volumeTitleSeen, nexradData->radial_status);

    if( currentSweep->setInfo( nexradData, prevSweep ) != ALL_OK ) {
      POSTMSG( ERROR, "Could not get information needed to process "
	       "the sweep" );
      return( FAILURE );
    }
  }
   
  //
  // Put the data into the sweep
  //
  PMU_auto_register( "Copying data into current sweep" );
  
  if( currentSweep->copyData( nexradData, beamTime ) != ALL_OK ) {
    return( FAILURE );
  }

  return( ALL_OK );
}

status_t Ingester::addMsg31Beam(RIDDS_data_31_hdr *nexradData, bool volumeTitleSeen) 
{
  PMU_auto_register( "Ingester::addMsg31Beam" );

  time_t beamTime = ((nexradData->julian_date - 1) * 86400 +
		     nexradData->millisecs_past_midnight / 1000);

  if(firstBeam) {
    POSTMSG( DEBUG, "First Tilt. Elevation Num: %d", nexradData->elev_num );

    RIDDS_volume_31_hdr *volData = (RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->volume_ptr);

    _setFirstBeamInfo(beamTime, nexradData->millisecs_past_midnight, nexradData->elev_num,
		      nexradData->elevation,
		      nexradData->radar_icao, volData->height + volData->feedhorn_height,
		      volData->lat, volData->lon);
  
    if( currentSweep->setInfo( nexradData ) != ALL_OK ) {
      return( FAILURE );
    }

  }
  else if( nexradData->elev_num != curElevNumber ) {

    // Elevation number change signals the Beginning of a new tilt
    _setNewElevationInfo(beamTime, nexradData->millisecs_past_midnight, nexradData->elev_num,
			 nexradData->elevation, volumeTitleSeen, nexradData->radial_status);

    POSTMSG( DEBUG, "New Tilt Detected. Starting Elevation Num: %d", nexradData->elev_num );

    if( currentSweep->setInfo( nexradData, prevSweep ) != ALL_OK ) {
      POSTMSG( ERROR, "Could not get information needed to process "
	       "the sweep" );
      return( FAILURE );
    }
  }

  //
  // Put the data into the sweep
  //
  PMU_auto_register( "Copying data into current sweep" );
  
  if( currentSweep->copyData( nexradData, beamTime ) != ALL_OK ) {
    return( FAILURE );
  }

  return( ALL_OK );
}

status_t Ingester::endOfData() 
{
  //
  // Never received data don't do anything
  if( firstBeam )
    return( END_OF_DATA );

  PMU_auto_register( "Writing remaining data" );
  
  currentSweep->fillRec();
  currentSweep->setBypassSegment(getBypassSegment());
  currentSweep->setClutterSegment(getClutterSegment());
  if(vcpData)
    currentSweep->setVcpInfo(&(vcpData->hdr), getElevationAngle());

  if(statusData)
    currentSweep->setStatusData(statusData);

  if( ncOutput->writeFile( currentSweep ) != 0 ) {
    return( FAILURE );
  }
  
  fileEnd = time( NULL );
  POSTMSG( DEBUG, "Process time = %d seconds", fileEnd - fileStart );
  
  return( END_OF_DATA );
}





