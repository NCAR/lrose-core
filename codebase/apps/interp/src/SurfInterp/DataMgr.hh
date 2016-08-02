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
#ifndef DATAMGR_HH
#define DATAMGR_HH

#include <vector>
#include <string>
#include <time.h>

#include <euclid/Pjg.hh>
#include <rapformats/station_reports.h>
#include <rapformats/Sndg.hh>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Spdb_typedefs.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

#include "Params.hh"

#define SUCCESS 0
#define FAILURE -1


class DataMgr {

 public: 

  /*********************************************************************
   * Constructors
   */

  DataMgr();


  /*********************************************************************
   * Destructor
   */

  ~DataMgr();


  /*********************************************************************
   * addStationUrl() - Add a station data URL to the list.
   */

  void addStationUrl(const string &station_url)
  {
    _stationUrls.push_back(station_url);
  }
  

  /*********************************************************************
   * addSoundingUrl() - Add a sounding data URL to the list.
   */

  void addSoundingUrl(const string &sounding_url)
  {
    _soundingUrls.push_back(sounding_url);
  }
  

  /*********************************************************************
   * addGenptUrl() - Add a GenPt data URL to the list.
   */

  void addGenptUrl(const string &genpt_url)
  {
    _genptUrls.push_back(genpt_url);
  }
  

  /*********************************************************************
   * getSurfaceData() - Get the indicated surface data
   *
   * Returns the number of station reports read.
   */

  int getSurfaceData(const DateTime &start_time,
		     const DateTime &end_time,
		     const double max_dist_km,
		     const float max_vis, const float max_ceiling);
  

  /*********************************************************************
   * getSoundingData() - Get the indicated sounding data
   */

  int getSoundingData(const DateTime &begin_time, const DateTime &end_time,
		      const double max_dist_km);


  /*********************************************************************
   * findClosestSounding() - Search the soundings vector for the sounding
   *                         which is closest to the input lat and lon. If
   *                         soundings have the same distance to the input
   *                         lat and lon, we take the sounding which has a
   *                         launch time closest to the reference time.
   */

  Sndg* findClosestSounding(const float lat1, const float lon1,
			    const time_t ref_time,
			    const float max_dist);


  /*********************************************************************
   * getGenptData() - Get GenPt surface data
   */

  int getGenptData(const DateTime &begin_time, const DateTime &end_time,
		   const double max_dist_km);


  /*********************************************************************
   * printSoundingData() - Print sounding data headers
   */

  void printSoundingData();


  /*********************************************************************
   * printSurfaceData() - Print surface data headers
   */

  void printSurfaceData();


  /*********************************************************************
   * nSurfaceReps() - Number of surface reports
   */

  int nSurfaceReps() { return _stationReps.size(); }


  /*********************************************************************
   * nGenptReps() - Number of GenPt reports
   */

  int nGenptReps() { return _genptReps.size(); }


  /*********************************************************************
   * nSoundings() - Number of soundings
   */

  int nSoundings() {  return _soundings.size(); }


  /*********************************************************************
   * getSurfaceRep() - Get pointer to surface rep
   */

  station_report_t *getSurfaceRep(const int i) { return _stationReps[i]; }


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * setDebug() - Set the debug level.
   */

  void setDebug(const Params::debug_t debug_level)
  {
    _debug = debug_level;
  }
  

  /*********************************************************************
   * setProgName() - Set the program name.
   */

  void setProgName(const string &prog_name)
  {
    _progName = prog_name;
  }
  

  /*********************************************************************
   * setProjection() - Set the grid projection.  This projection is used
   *                   to remove data points that don't affect the grid
   *                   area.
   */

  void setProjection(const Pjg &proj)
  {
    _proj = proj;
  }
  

  /*********************************************************************
   * getGenptRep() - Get pointer to surface rep
   */

  GenPt *getGenptRep(const int i) { return _genptReps[i]; }


  /*********************************************************************
   * replaceSurfaceDataCeiling() - Ceiling values above the given threshold
   *                               are replaced with the given replacement
   *                               value.
   */

  void replaceSurfaceDataCeiling(const float ceiling_thresh,
				 const float replacement_val);


 private:
 
  /////////////////////
  // Private members //
  /////////////////////

  string _progName;
  Params::debug_t _debug;
  
  Pjg _proj;
  
  vector< string > _stationUrls;
  vector< string > _soundingUrls;
  vector< string > _genptUrls;
  
  vector< station_report_t* > _stationReps;
  vector< Sndg* > _soundings;
  vector< GenPt* > _genptReps;


  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _clearGenptData() - Clear out the old GenPt data
   */

  void _clearGenptData();
  

  /*********************************************************************
   * _clearSoundingData() - Clear out the old sounding data
   */

  void _clearSoundingData();
  

  /*********************************************************************
   * _clearStationData() - Clear out the old station data
   */

  void _clearStationData();
  

  /*********************************************************************
   * _getChunkData() - Get the indicated data from an SPDB database
   *
   * Returns the number of chunks read on success, -1 on failure.
   */

  int _getChunkData(string data_url,
		    const DateTime &begin_time, const DateTime &end_time,
		    DsSpdb &spdbMgr, vector< Spdb::chunk_t > &chunks);
  

  /*********************************************************************
   * _getGenptData() - Get GenPt data from the given URL
   */

  void _getGenptData(const string &data_url,
		     const DateTime &start_time,
		     const DateTime &end_time,
		     const double min_lat, const double max_lat,
		     const double min_lon, const double max_lon);
  

  /*********************************************************************
   * _getSoundingData() - Get sounding data from the given URL
   */

  void _getSoundingData(const string &data_url,
			const DateTime &start_time,
			const DateTime &end_time,
			const double min_lat, const double max_lat,
			const double min_lon, const double max_lon);
  

  /*********************************************************************
   * _getSurfaceData() - Get surface data from the specified URL
   */

  void _getSurfaceData(const string &data_url,
		       const DateTime &start_time,
		       const DateTime &end_time,
		       const double min_lat, const double max_lat,
		       const double min_lon, const double max_lon,
		       const float max_vis, const float max_ceiling);
  

};

#endif
