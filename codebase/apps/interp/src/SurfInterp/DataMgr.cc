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
#include <vector>

#include <rapmath/math_macros.h>
#include "DataMgr.hh"  

using namespace std;


/*********************************************************************
 * Constructors
 */

DataMgr::DataMgr(const string &progName,
                 const Params &params) :
        _progName(progName),
        _params(params),
        _debug(Params::DEBUG_OFF)
{
} 


/*********************************************************************
 * Destructor
 */

DataMgr::~DataMgr()
{
  _clearStationData();
  _clearGenptData();
  _clearSoundingData();
}
 

/*********************************************************************
 * getSurfaceData() - Get the indicated surface data
 *
 * Returns the number of station reports read.
 */

int DataMgr::getSurfaceData(const DateTime &start_time,
			    const DateTime &end_time,
			    const double max_dist_km,
			    const float max_vis, const float max_ceiling)
{
  // Clear out the old data
  
  _clearStationData();
  
  // Calculate the region of interest.  We need to get the lat/lon of the
  // current grid, expanded by max_dist_km.

  double ll_lat, ll_lon;
  double ur_lat, ur_lon;
  
  _proj.getLL(ll_lat, ll_lon);
  _proj.getUR(ur_lat, ur_lon);
  
  double min_lat, min_lon;
  double max_lat, max_lon;
  
  _proj.latlonPlusRTheta(ll_lat, ll_lon,
			 max_dist_km * M_SQRT2, -135.0,
			 min_lat, min_lon);
  _proj.latlonPlusRTheta(ur_lat, ur_lon,
			 max_dist_km * M_SQRT2, 45.0,
			 max_lat, max_lon);
  
  if (_debug >= Params::DEBUG_NORM)
  {
    cerr << "Calculating box of interest:" << endl;
    cerr << "    ll_lat = " << ll_lat << ", ll_lon = " << ll_lon << endl;
    cerr << "    ur_lat = " << ur_lat << ", ur_lon = " << ur_lon << endl;
    cerr << "    min_lat = " << min_lat << ", min_lon = " << min_lon << endl;
    cerr << "    max_lat = " << max_lat << ", max_lon = " << max_lon << endl;
  }
  
  // Get the surface data

  vector< string >::const_iterator url;
  
  for (url = _stationUrls.begin(); url != _stationUrls.end(); ++url)
    _getSurfaceData(*url, start_time, end_time,
		    min_lat, max_lat, min_lon, max_lon,
		    max_vis, max_ceiling);
  
  return _stationReps.size();
}


/*********************************************************************
 * replaceSurfaceDataCeiling() - Ceiling values above the given threshold
 *                               are replaced with the given replacement
 *                               value.
 */

void DataMgr::replaceSurfaceDataCeiling(const float ceiling_thresh,
					const float replacement_val)
{
  vector < station_report_t *> :: const_iterator j;
      
  for (j = _stationReps.begin(); j != _stationReps.end(); ++j)
  {
    if ((*j)->ceiling > ceiling_thresh)
      (*j)->ceiling = replacement_val;
  }
}


/*********************************************************************
 * getSoundingData() - Get the indicated sounding data
 */

int DataMgr::getSoundingData(const DateTime &start_time,
			     const DateTime &end_time,
			     const double max_dist_km)
{
  // Clear out the old data

  _clearSoundingData();
  
  // Calculate the region of interest.  We need to get the lat/lon of the
  // current grid, expanded by max_dist_km.

  double ll_lat, ll_lon;
  double ur_lat, ur_lon;
  
  _proj.getLL(ll_lat, ll_lon);
  _proj.getUR(ur_lat, ur_lon);
  
  double min_lat, min_lon;
  double max_lat, max_lon;
  
  _proj.latlonPlusRTheta(ll_lat, ll_lon,
			 max_dist_km * M_SQRT2, -135.0,
			 min_lat, min_lon);
  _proj.latlonPlusRTheta(ur_lat, ur_lon,
			 max_dist_km * M_SQRT2, 45.0,
			 max_lat, max_lon);
  
  if (_debug >= Params::DEBUG_NORM)
  {
    cerr << "Calculating box of interest:" << endl;
    cerr << "    ll_lat = " << ll_lat << ", ll_lon = " << ll_lon << endl;
    cerr << "    ur_lat = " << ur_lat << ", ur_lon = " << ur_lon << endl;
    cerr << "    min_lat = " << min_lat << ", min_lon = " << min_lon << endl;
    cerr << "    max_lat = " << max_lat << ", max_lon = " << max_lon << endl;
  }
  
  // Read in the new data

  vector< string >::const_iterator url;
  
  for (url = _soundingUrls.begin(); url != _soundingUrls.end(); ++url)
    _getSoundingData(*url, start_time, end_time,
		     min_lat, max_lat, min_lon, max_lon);
  
  return _soundings.size();
}


/*********************************************************************
 * getGenptData() - Get the indicated GenPt surface data
 */

int DataMgr::getGenptData(const DateTime &start_time, const DateTime &end_time,
			  const double max_dist_km)
{
  // Clear out the old data

  _clearGenptData();
  
  // Calculate the region of interest.  We need to get the lat/lon of the
  // current grid, expanded by max_dist_km.

  double ll_lat, ll_lon;
  double ur_lat, ur_lon;
  
  _proj.getLL(ll_lat, ll_lon);
  _proj.getUR(ur_lat, ur_lon);
  
  double min_lat, min_lon;
  double max_lat, max_lon;
  
  _proj.latlonPlusRTheta(ll_lat, ll_lon,
			 max_dist_km * M_SQRT2, -135.0,
			 min_lat, min_lon);
  _proj.latlonPlusRTheta(ur_lat, ur_lon,
			 max_dist_km * M_SQRT2, 45.0,
			 max_lat, max_lon);
  
  if (_debug >= Params::DEBUG_NORM)
  {
    cerr << "Calculating box of interest:" << endl;
    cerr << "    ll_lat = " << ll_lat << ", ll_lon = " << ll_lon << endl;
    cerr << "    ur_lat = " << ur_lat << ", ur_lon = " << ur_lon << endl;
    cerr << "    min_lat = " << min_lat << ", min_lon = " << min_lon << endl;
    cerr << "    max_lat = " << max_lat << ", max_lon = " << max_lon << endl;
  }
  
  // Get the data

  vector< string >::const_iterator url;
  
  for (url = _genptUrls.begin(); url != _genptUrls.end(); ++url)
    _getGenptData(*url, start_time, end_time,
		  min_lat, max_lat, min_lon, max_lon);
  
  return _genptReps.size();
}


/*********************************************************************
 * findClosestSounding() - Search the soundings vector for the sounding
 *                         which is closest to the input lat and lon. If
 *                         soundings have the same distance to the input
 *                         lat and lon, we take the sounding which has a
 *                         launch time closest to the reference time.
 */

Sndg *DataMgr::findClosestSounding(const float pt_lat, const float pt_lon,
				   const time_t ref_time,
				   const float max_dist_km)
{
  vector< Sndg* >::const_iterator i;

  // Calculate the region of interest.  If the sounding it outside of this
  // square then we know it is too far from the point to be used so we don't
  // have to calculate the actual distance between the point and the sounding.

  double max_dist_x = _proj.km2x(max_dist_km);
  
  double pt_x, pt_y;

  _proj.latlon2xy(pt_lat, pt_lon, pt_x, pt_y);
  
  double min_lat, min_lon;
  double max_lat, max_lon;
  
  _proj.xy2latlon(pt_x - max_dist_x, pt_y - max_dist_x,
		  min_lat, min_lon);
  _proj.xy2latlon(pt_x + max_dist_x, pt_y + max_dist_x,
		  max_lat, max_lon);
  
  // Initialize variables

  Sndg *sndg_ptr = 0;
  
  double min_dist = max_dist_km;
  time_t min_dist_time = 0;

  int sndg_count = 0;
  int sndg_used = 0;

  for (i = _soundings.begin(); i != _soundings.end(); ++i)
  {      
    // get lat lon and time of sounding

    Sndg::header_t sndg_header = (*i)->getHeader();

    float sndg_lat = sndg_header.lat;
    float sndg_lon = sndg_header.lon;
    time_t sndg_time = sndg_header.launchTime;
      
    // Do the first rough check on the sounding location

    if (sndg_lat < min_lat || sndg_lat > max_lat ||
	sndg_lon < min_lon || sndg_lon > max_lon)
    {
      ++sndg_count;
      continue;
    }
    
    // Calculate distance between sounding and input lat and lon

    double dist;
    double theta;

    Pjg::latlon2RTheta((double)pt_lat, (double)pt_lon,
		       (double)sndg_lat, (double)sndg_lon, dist, theta);


    // If the distance between the sounding and input lat and lon
    // is the smallest yet, keep a pointer to it and record the 
    // distance and launch time.

    if (dist < min_dist)
    {
      min_dist = dist;
      min_dist_time = sndg_time; 
      sndg_ptr = *i;
      sndg_used = sndg_count;
    }
    else if (dist == min_dist)
    {	  
      // If the distance between the sounding and input lat and lon
      // is equal to the minimum, keep a pointer to the one with 
      // launch time which is closest to the input reference time. 

      if (fabs((double)sndg_time - ref_time) <
	  fabs((double)min_dist_time - ref_time))
      {
	min_dist = dist;
	min_dist_time = sndg_time; 
	sndg_ptr = *i;
	sndg_used = sndg_count;
      } 
    }
    ++sndg_count;
  }

  if (_debug >= Params::DEBUG_VERBOSE)
  {
    if (sndg_ptr != 0)
    {
      Sndg::header_t sndg_header = _soundings[sndg_used]->getHeader();
	  
      float sndg_lat = sndg_header.lat;
      float sndg_lon = sndg_header.lon;
	  
      cerr << "SoundingUsed " << sndg_used
	   << " distance to obs: " << min_dist
	   << " pt_lat: " << pt_lat << " pt_lon: " << pt_lon
	   << " sndg_lat: " << sndg_lat << " sndg_lon: " << sndg_lon << endl;
    }
    else
    {
      cerr << "No appropriate sounding found\n"; 
    }
    
  }

  return sndg_ptr;
}


/*********************************************************************
 * printSoundingData() - Print sounding data headers
 */

void DataMgr::printSoundingData()
{
  vector< Sndg* >::const_iterator i;

  for (i = _soundings.begin(); i != _soundings.end(); ++i)
    (*i)->print_header(cerr,"");
}


/*********************************************************************
 * printSurfaceData() - Print surface data headers
 */

void DataMgr::printSurfaceData()
{
  static const string method_name = "DataMgr::printSurfaceData()";
  
  vector< station_report_t* >::const_iterator i;

  cerr << _progName << ": " << method_name << ": " << endl;

  for (i = _stationReps.begin(); i != _stationReps.end(); ++i)
  {
    cerr << "lat: " << (*i)->lat << endl;
    cerr << "lon: " << (*i)->lon << endl;
    cerr << "alt: " << (*i)->alt << endl << endl;
  }   
}


/*********************************************************************
 * _clearGenptData() - Clear out the old GenPt data
 */

void DataMgr::_clearGenptData()
{
  vector< GenPt* >::const_iterator genpt_iter;

  for (genpt_iter = _genptReps.begin(); genpt_iter != _genptReps.end();
       ++genpt_iter)
    delete *genpt_iter;
  _genptReps.erase(_genptReps.begin(), _genptReps.end());
}
 

/*********************************************************************
 * _clearSoundingData() - Clear out the old sounding data
 */

void DataMgr::_clearSoundingData()
{
  vector< Sndg* >::const_iterator sndg_iter;

  for (sndg_iter = _soundings.begin(); sndg_iter != _soundings.end();
       ++sndg_iter)
    delete *sndg_iter;
  _soundings.erase(_soundings.begin(), _soundings.end());

}
 

/*********************************************************************
 * _clearStationData() - Clear out the old station data
 */

void DataMgr::_clearStationData()
{
  vector< station_report_t * >::const_iterator stn_iter;

  for (stn_iter = _stationReps.begin(); stn_iter != _stationReps.end();
       ++stn_iter)
    delete *stn_iter;
  _stationReps.erase(_stationReps.begin(), _stationReps.end());
}
 

/*********************************************************************
 * _getChunkData() - Get the indicated data from an SPDB database
 *
 * Returns the number of chunks read on success, -1 on failure.
 */

int DataMgr::_getChunkData(string data_url,
			   const DateTime &start_time,
			   const DateTime &end_time,
			   DsSpdb &spdbMgr, vector< Spdb::chunk_t > &chunks)
{
  static const string method_name = "DataMgr::_getChunkData()";
  
  if (_debug >= Params::DEBUG_NORM)
  {
    cerr << _progName << ": " << method_name
	 << ": Looking at " << data_url.c_str() 
	 << " for data in interval ["
	 << start_time << ", " << end_time << "]." << endl;
  }

  if (spdbMgr.getInterval(data_url,
			  start_time.utime(), end_time.utime()) != 0)
  {
    cerr << _progName << ": " << method_name
	 << ": Problem reading data at " << data_url.c_str()  
	 << " in interval " << "[ "
	 <<  start_time << ", " << end_time << " ]" << endl;      
    return -1;
  }
 
  // Get number of chunks

  int nChunks = spdbMgr.getNChunks();

  if (nChunks == 0)
  {
    cerr << _progName << ": " << method_name
	 << ": No data found at " <<  data_url.c_str()
	 << " in interval " << "[ "
	 <<  start_time << ", " << end_time << " ]" << endl;
  }
  
  if (_debug >= Params::DEBUG_NORM)
    cerr << " " << nChunks << " chunks found." << endl;
  
  // Get chunk data
  
  chunks  = spdbMgr.getChunks(); 
  
  return nChunks;
}


/*********************************************************************
 * _getGenptData() - Get GenPt data from the given URL
 */

void DataMgr::_getGenptData(const string &data_url,
			    const DateTime &start_time,
			    const DateTime &end_time,
			    const double min_lat, const double max_lat,
			    const double min_lon, const double max_lon)
{
  // Get the data

  DsSpdb spdbMgr;

  vector < Spdb::chunk_t > chunks;
  int n_reps = _getChunkData(data_url, start_time, end_time, spdbMgr, chunks);

  // Disassemble chunks. Store in soundings vector.

  for (int i = 0; i < n_reps; ++i)
  {
    GenPt *genpt_ptr = new GenPt();
     
    genpt_ptr->disassemble( chunks[i].data, chunks[i].len );
     
    if (genpt_ptr->getLat() < min_lat || genpt_ptr->getLat() > max_lat ||
	genpt_ptr->getLon() < min_lon || genpt_ptr->getLon() > max_lon)
    {
      delete genpt_ptr;
      continue;
    }
    
    _genptReps.push_back(genpt_ptr);
  }
}


/*********************************************************************
 * _getSoundingData() - Get sounding data from the given URL
 */

void DataMgr::_getSoundingData(const string &data_url,
			       const DateTime &start_time,
			       const DateTime &end_time,
			       const double min_lat, const double max_lat,
			       const double min_lon, const double max_lon)
{
  static const string method_name = "DataMgr::_getSoundingData()";
  
  DsSpdb spdbMgr;

  vector < Spdb::chunk_t > chunks;
  int n_soundings = _getChunkData(data_url, start_time, end_time,
				  spdbMgr, chunks);

  PMU_auto_register("getting sounding data"); 

  // Disassemble chunks. Store in soundings vector.

  int n_good_soundings = 0;

  for (int i = 0; i < n_soundings; ++i)
  {
    PMU_auto_register("looping through soundings.");
    Sndg *sndg_ptr = new Sndg();
     
    int iret = sndg_ptr->disassemble(chunks[i].data, chunks[i].len);
     
    if (iret)
    {
      cerr << "WARNING: " << method_name
	   << ": Could not disassemble sounding." << endl;

      delete sndg_ptr;
      
      continue;
    }

    // Get the needed sounding information
	
    Sndg::header_t sndg_header1 = sndg_ptr->getHeader();
	
    float lat1 = sndg_header1.lat;
    float lon1 = sndg_header1.lon;
    float time1 = sndg_header1.launchTime;
	
    // Don't process soundings outside of our area of interest

    if (lat1 < min_lat || lat1 > max_lat ||
	lon1 < min_lon || lon1 > max_lon)
    {
      delete sndg_ptr;
      
      continue;
    }
    
    // Lets only keep the latest sounding from each site
    // Compare lat and lon of sndg_ptr to all soundings 
    // in the soundings vector. If two soundings match in 
    // location, only keep the latest one

    n_good_soundings++;
	
    vector< Sndg * >::iterator j;
	
    bool push = true;

    for (j = _soundings.begin(); j != _soundings.end(); ++j)
    {
      PMU_auto_register("evaluating sounding data"); 
      Sndg::header_t sndg_header2 = (*j)->getHeader();
	
      float lat2 = sndg_header2.lat;
      float lon2 = sndg_header2.lon;
      long time2 = sndg_header2.launchTime;
	    
      if (fabs(lat1 - lat2) < .0001 && fabs(lon1 - lon2) < .0001)
      {
	// The soundings have the same location,
	// check the times and keep the later one.

	if ( time1 > time2)
	{
	  // Keep sndg_ptr, delete the one in soundings vec

	  delete *j;
	    
	  *j = sndg_ptr;
		    
	  // We dont need to push this on the vector, it
	  // takes the deleted sounding's spot.

	  push = false;
	}
	else 
	{
	  // Keep the one already in the vector, get rid of 
	  // sndg_ptr.

	  j = _soundings.end() - 1;

	  push = false;

	  delete sndg_ptr;
	} 
	
	break;
      }
    } 

    if (push)
    {
      _soundings.push_back(sndg_ptr);
    }
  }
   
  if (_debug >= Params::DEBUG_NORM)
  {
    cerr << method_name << ": " <<  n_good_soundings << " soundings read, "
	 <<  _soundings.size() << " soundings kept from "
	 << data_url << " in interval (" << start_time << ", " 
	 << end_time << ")." << endl;
  }
}


/*********************************************************************
 * _getSurfaceData() - Get surface data from the specified URL
 */

void DataMgr::_getSurfaceData(const string &data_url,
			      const DateTime &start_time,
			      const DateTime &end_time,
			      const double min_lat, const double max_lat,
			      const double min_lon, const double max_lon,
			      const float max_vis, const float max_ceiling)
{
  static const string method_name = "DataMgr::_getSurfaceData()";
  
  // Get the surface data

  DsSpdb spdbMgr;

  vector < Spdb::chunk_t > chunks;
  int nreps = _getChunkData(data_url, start_time, end_time, spdbMgr, chunks);

  int nreps_kept = 0;
  
  for (int i = 0; i < nreps; i++)
  {

    // get the observation

    WxObs obs;
    obs.disassemble(chunks[i].data, chunks[i].len);

    // Convert to report

    station_report_t *repPtr = new station_report_t;
    obs.loadStationReport(*repPtr);
    
    // Don't use this station if it is too far outside of the boundaries
    // of the output grid.

    if (repPtr->lat < min_lat || repPtr->lat > max_lat ||
	repPtr->lon < min_lon || repPtr->lon > max_lon)
    {
      delete repPtr;
      continue;
    }
      
    // Do some QC on the ceiling and visibility values

    if (repPtr->ceiling != STATION_NAN &&
        repPtr->ceiling > max_ceiling) {
      repPtr->ceiling = STATION_NAN;
    }
    
    if (repPtr->visibility != STATION_NAN &&
        repPtr->visibility > max_vis) {
      repPtr->visibility = STATION_NAN;
    }

    if (repPtr->temp != STATION_NAN &&
        repPtr->temp > _params.MaxValidTempC) {
      repPtr->temp = STATION_NAN;
    }

    if (repPtr->temp != STATION_NAN &&
        repPtr->temp < _params.MinValidTempC) {
      repPtr->temp = STATION_NAN;
    }

    if (repPtr->dew_point != STATION_NAN &&
        repPtr->dew_point > _params.MaxValidTempC) {
      repPtr->dew_point = STATION_NAN;
    }

    if (repPtr->dew_point != STATION_NAN &&
        repPtr->dew_point < _params.MinValidTempC) {
      repPtr->dew_point = STATION_NAN;
    }
	  
    // Avoid duplicate stations. If two reps have the same lat and lon
    // then keep the rep with the latest time.
      
    vector < station_report_t *> :: iterator j;
      
    bool duplicate_station = false;
      
    for (j = _stationReps.begin(); j != _stationReps.end(); ++j)
    {
      if ( (*j)->lat == repPtr->lat && (*j)->lon == repPtr->lon)
      {
	duplicate_station = true;
	    
	if ((*j)->time < repPtr->time)
	{
	  // Keep repPtr, delete the old one

	  delete *j;
  		
	  *j = repPtr;   
	}
	else
	{
	  delete repPtr;
	}
	
	break;
      }
    }   
    
    // If this is a duplicate station, then it was taken care of above.
    // If not, we need to add this station to our list.

    if (!duplicate_station)
    {
      _stationReps.push_back(repPtr);
      ++nreps_kept;
    }
    
  }

  if (_debug >= Params::DEBUG_NORM)
  {
    cerr << _progName << ": " << method_name << ":" << endl;
    cerr << "    " << nreps << " reports read" << endl;
    cerr << "    " << nreps_kept << " reports kept" << endl;
  }
  
}
