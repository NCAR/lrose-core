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
/**
 * @file DemProvider.cc
 */
#include "DemProvider.hh"
#include <toolsa/LogMsg.hh>
#include <toolsa/mem.h>
using namespace rainfields;
using namespace rainfields::ancilla;


//----------------------------------------------------------------
DemProvider::DemProvider(const Params &params) :
        _params(params)
{

  cerr << "aaaaaaaaaaaaaaaaaa dem_path: " << _params.dem_path << endl;
  
  // allocate pointers to tiles
  
  _tiles = (SrtmTile ***) umalloc2(nLat, nLon, sizeof(SrtmTile *));
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double centerLat = ilat + 0.5 - 90.0;
      double centerLon = ilon + 0.5 - 180.0;
      _tiles[ilat][ilon] = new SrtmTile(_params.dem_path,
                                        centerLat, centerLon,
                                        _params.debug);
    }
  }
  
}

//----------------------------------------------------------------
DemProvider::~DemProvider(void)
{

  // free up tiles
  
  if (_tiles != NULL) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
        delete _tiles[ilat][ilon];
      }
    }
    ufree2((void **) _tiles);
  }
  
}

//----------------------------------------------------------------
bool DemProvider::set(const std::pair<double,double> &sw,
                      const std::pair<double,double> &ne)
{
  spheroid::standard which;

  switch (_params.dem_data_format)
  {
    case Params::SHUTTLE_RADAR_TOPOGRAPHY:
      _dem.reset(new digital_elevation_srtm3(_params.debug >= Params::DEBUG_VERBOSE,
                                             _params.dem_path));
      break;
    case Params::ESRI_I65:
    case Params::ESRI_ANS:
    case Params::ESRI_CLARKE1858:
    case Params::ESRI_GRS80:
    case Params::ESRI_WGS84:
    case Params::ESRI_WGS72:
    case Params::INTERNATIONAL1924:
    case Params::AUSTRALIAN_NATIONAL:
      which = _toSphereStandard(_params.dem_data_format);
      _set(sw, ne, which);
      break;
    default:
      LOG(LogMsg::ERROR, "format Unknown");
      exit(1);
  }

  return true;
}

//----------------------------------------------------------------
latlonalt DemProvider::radarOrigin(const latlonalt &radar) const
{
  spheroid wgs84{spheroid::standard::wgs84};
  latlonalt ret =
    _dem->reference_spheroid().ecefxyz_to_latlon(wgs84.latlon_to_ecefxyz(radar));

  // warn if the stated site location is less than 3 meters above the DEM location of that site
  LOGF(LogMsg::DEBUG, "Altitude (native spheroid): %lf",
       _params.radar_location.heightKm*1000.0);
  LOGF(LogMsg::DEBUG, "Altitude (DEM spheroid): %lf", ret.alt);
  LOGF(LogMsg::DEBUG, "DEM:  %lf", _dem->lookup(ret));
  if (ret.alt < _dem->lookup(ret) + 3.0)
  {
    LOG(LogMsg::WARNING, 
	"site altitude is less than 3 meters above DEM altitude");
  }
  return ret;
}

//----------------------------------------------------------------
void
  DemProvider::determine_dem_segment_peak(const latlon& origin,
                                          angle bearing,
                                          real min_range,
                                          real max_range,
                                          real& peak_ground_range,
                                          real& peak_altitude,
                                          size_t bin_samples
                                          ) const
{
  const auto delta_range = (max_range - min_range) / bin_samples;
  const auto& sphere = _dem->reference_spheroid();

  peak_altitude = -10000.0_r;

  // loop over our ground range 
  for (size_t i = 0; i < bin_samples; ++i)
  {
    // determine range at this step
    auto range = min_range + i * delta_range;

    // convert range bearing into latlon
    auto loc = sphere.bearing_range_to_latlon(origin, bearing, range);

    // get height from DEM
    auto altitude = _dem->lookup(loc);

    // are we a new peak? (nan's will always fail this test)
    if (altitude > peak_altitude)
    {
      peak_altitude = altitude;
      peak_ground_range = range;
    }
  }
}

//----------------------------------------------------------------
double
  DemProvider::getElevation(const rainfields::latlon& loc) const
{
  return _dem->lookup(loc);
}

//----------------------------------------------------------------
void DemProvider::_set(const std::pair<double,double> &sw,
                       const std::pair<double,double> &ne,
                       spheroid::standard which)
{

  angle a0, a1;
  a0.set_degrees(sw.first);
  a1.set_degrees(sw.second);
  latlon lsw(a0, a1);

  a0.set_degrees(ne.first);
  a1.set_degrees(ne.second);
  latlon lne(a0, a1);

  _dem.reset(new digital_elevation_esri(_params.debug,
                                        _params.dem_path, lsw, lne,
					spheroid(which)));
}

//------------------------------------------------------------------

spheroid::standard DemProvider::_toSphereStandard(Params::DigitalElevationModel_t t)
{
  std::string s="";
  switch (t)
  {
    case Params::ESRI_I65:
      s = "i65";
      break;
    case Params::ESRI_ANS:
      s = "ans";
      break;
    case Params::ESRI_CLARKE1858:
      s = "clarke1858";
      break;
    case Params::ESRI_GRS80:
      s = "grs80";
      break;
    case Params::ESRI_WGS84:
      s = "wgs84";
      break;
    case Params::ESRI_WGS72:
      s = "wgs72";
      break;
    case Params::INTERNATIONAL1924:
      s = "international924";
      break;
    case Params::AUSTRALIAN_NATIONAL:
      s = "australian_national";
      break;
    default:
      s = "?";
      break;
  }

  return from_string<spheroid::standard>(s);
}

//------------------------------------------------------------------
//
// Get the DEM model name as a string

string DemProvider::ModelName(Params::DigitalElevationModel_t t)
{

  std::string name;

  switch (t)
  {
    case Params::ESRI_I65:
      name = "ESRI_I65";
      break;
    case Params::ESRI_ANS:
      name = "ESRI_ANS";
      break;
    case Params::ESRI_CLARKE1858:
      name = "ESRI_CLARKE1858";
      break;
    case Params::ESRI_GRS80:
      name = "ESRI_GRS80";
      break;
    case Params::ESRI_WGS84:
      name = "ESRI_WGS84";
      break;
    case Params::ESRI_WGS72:
      name = "ESRI_WGS72";
      break;
    case Params::INTERNATIONAL1924:
      name = "INTERNATIONAL1924";
      break;
    case Params::AUSTRALIAN_NATIONAL:
      name = "AUSTRALIAN_NATIONAL";
      break;
    case Params::SHUTTLE_RADAR_TOPOGRAPHY:
    default:
      name = "SHUTTLE_RADAR_TOPOGRAPHY";
  }

  return name;

}

//////////////////////////////////////////////////////////
// get terrain ht and water flag for a point
// returns 0 on success, -1 on failure
// sets terrainHtM and isWater args

int DemProvider::getHt(double lat, double lon, int16_t &terrainHtM)
  
{

  // condition the longitude and latitude
  
  if (lon > 180.0) {
    lon -= 360.0;
  }
  
  // compute tile indices
  
  int ilat = (int) (lat - -90.0);
  int ilon = (int) (lon - -180.0);

  if (ilat < 0) ilat = 0;
  if (ilat > nLat - 1) ilat = nLat - 1;
  if (ilon < 0) ilon = 0;
  if (ilon > nLon - 1) ilon = nLon - 1;
  
  if (_tiles[ilat][ilon]->getHt(lat, lon, terrainHtM)) {
    cerr << "ERROR - DemProvider::getHt()" << endl;
    cerr << "  Cannot get height for lat, lon: " << lat << ", " << lon << endl;
    cerr << "  Tile indices: ilat, ilon: " << ilat << ", " << ilon << endl;
    return -1;
  }
  
  // save location of latest request

  _latestLat = lat;
  _latestLon = lon;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Got ht request: lat, lon: " << lat << ", " << lon << endl;
    cerr << "Got ht request: htM: " << terrainHtM << endl;
  }

  return 0;

}

//////////////////////////////////////////////////////////
// update cache of 1 tile around the latest access point
// to prepare for upcoming reads

void DemProvider::_updateCache()

{

  if (_latestLat == -9999.0 || _latestLon == -9999.0) {
    // no activity yet
    return;
  }

  _readForCache(_latestLat + 1.0, _latestLon - 1.0);
  _readForCache(_latestLat + 1.0, _latestLon);
  _readForCache(_latestLat + 1.0, _latestLon + 1.0);

  _readForCache(_latestLat, _latestLon - 1.0);
  _readForCache(_latestLat, _latestLon + 1.0);

  _readForCache(_latestLat - 1.0, _latestLon - 1.0);
  _readForCache(_latestLat - 1.0, _latestLon);
  _readForCache(_latestLat - 1.0, _latestLon + 1.0);
  
}

//////////////////////////////////////////////////////////
// read in a tile for the cache
// returns 0 on success, -1 on failure

int DemProvider::_readForCache(double lat, double lon)

{

  // compute tile indices

  int ilat = (int) (lat - -90.0);
  int ilon = (int) (lon - -180.0);

  // check for out of bounds

  if (ilat < 0) return 0;
  if (ilat > nLat - 1) return 0;
  if (ilon < 0) return 0;;
  if (ilon > nLon - 1) return 0;

  // read tile for cache

  if (_tiles[ilat][ilon]->readForCache()) {
    cerr << "ERROR - DemProvider::getHt()" << endl;
    cerr << "  Cannot read for cache, lat, lon: " << lat << ", " << lon << endl;
    cerr << "  Tile indices: ilat, ilon: " << ilat << ", " << ilon << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////
// free up tile memory that has not been used recently

void DemProvider::_freeTileMemory()

{

  time_t now = time(NULL);

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      time_t ltime = _tiles[ilat][ilon]->getLatestAccessTime();
      if (ltime > 0) {
        double secsSinceLastAccess = (double) now - (double) ltime;
        if (secsSinceLastAccess > 1800) {
          _tiles[ilat][ilon]->freeHtArray();
        }
      } // if (ltime > 0)
    } // ilon
  } // ilat

}


