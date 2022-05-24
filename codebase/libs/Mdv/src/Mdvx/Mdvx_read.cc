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
// Mdvx_read.cc
//
// Read routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
//////////////////////////////////////////////////////////

#include <euclid/PjgTypes.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxPjg.hh>
#include <Mdv/MdvxVsectLut.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg.h>
#include <toolsa/file_io.h>
#include <toolsa/TaStr.hh>
#include <toolsa/Path.hh>
#include <dataport/bigend.h>
#include <sys/stat.h>
using namespace std;

//////////////////////////////
// Setting up to read
//

/////////////////////////////////////////
// clear all read requests, set defaults

void Mdvx::clearRead()

{
  clearReadTime();
  clearReadPath();
  clearReadHorizLimits();
  clearReadVertLimits();
  clearReadEncodingType();
  clearReadCompressionType();
  clearReadScalingType();
  clearReadComposite();
  clearReadFillMissing();
  clearReadFields();
  clearReadWayPts();
  clearReadNVsectSamples();
  clearReadMaxVsectSamples();
  clearReadVsectAsRhi();
  clearReadVsectDisableInterp();
  clearReadRemap();
  clearReadAutoRemap2LatLon();
  clearReadDecimate();
  clearReadVlevelType();
  clearReadFieldFileHeaders();
  clearReadChunks();
  clearReadTimeListAlso();
  clearReadAsSingleBuffer();
  clearReadFormat();
  clearCheckLatestValidModTime();
  clearConstrainFcastLeadTimes();
  setRead32BitHeaders(false);
  _readQualifiersActive = false;
}

////////////////////////////////////////////////////////////////////////
// set read time
//
// You must either set the readTime or readPath.
//
// If you specify read time, the path is determined from the dir
// the search_time and the search mode.
//
// Search modes are as follows:
//
//   READ_LAST: read last available data set
//   READ_CLOSEST: read closest data within search_margin of search_time
//   READ_FIRST_BEFORE: read first data at or before search_time,
//                      within search_margin
//   READ_FIRST_AFTER:  read first data at or after search_time,
//                      within search_margin
//   READ_BEST_FORECAST: read best forecast within 
//                       search_margin of search_time.
//                       Takes data from latest available gen time.
//   READ_SPECIFIED_FORECAST: read forecast generated at search time,
//                            closest to forecast_lead_time,
//                            within search_margin

void Mdvx::setReadTime(const read_search_mode_t mode,
                       const string &read_dir,
                       const int search_margin /* = 0*/,
                       const time_t search_time /* = 0*/,
                       const int forecast_lead_time /* = 0*/ )
  
{
  _readSearchMode = mode;
  _readDir = read_dir;
  _readSearchTime = search_time;
  _readSearchMargin = search_margin;
  _readForecastLeadTime = forecast_lead_time;
  _readTimeSet = true;
  clearReadPath();
}

void Mdvx::clearReadTime()

{
  _readSearchMode = READ_LAST;
  _readDir = ".";
  _readSearchTime = 0;
  _readSearchMargin = 0;
  _readForecastLeadTime = 0;
  _readTimeSet = false;
}

/////////////////////////////////////////////////////////////////
// Constrain the lead times to be considered in the time search,
// if data is stored in forecast format.
//
// Does not apply to READ_SPECIFIED_FORECAST mode.
//
// Only forecast lead times within the specified limits will
// be considered.
//
// Also, you can specify that the search_time is the generate time
// rather than valid time. The valid time will be computed as
// the request_time plus the mean of the min and max lead times.

void Mdvx::setConstrainFcastLeadTimes(int min_lead_time,
				      int max_lead_time,
				      bool request_by_gen_time)
{
  _timeList.setConstrainFcastLeadTimes(min_lead_time,
				       max_lead_time,
				       request_by_gen_time);
  if (request_by_gen_time) {
    // Only overwrite valid time in file when forecast lead times are
    // constrained and request_by_gen_time is true.
    // In this case, the MdvxTimeList object will set the gen times in
    // the valid times array, so that the valid time is overwritten
    // by the gen time
    _overwriteValidTimeByGenTime = true;
  }
}

void Mdvx::clearConstrainFcastLeadTimes()
{
  _timeList.clearConstrainFcastLeadTimes();
  _overwriteValidTimeByGenTime = false;
  _genTimeForOverwrite = 0;
}
  
////////////////////////////////////////////////////////////////////////
// set read path
//
// You must either set the readTime or readPath.
// If you specify the path, that exact path will be used.

void Mdvx::setReadPath(const string &read_path)
  
{
  _readPath = read_path;
  _readPathSet = true;
  clearReadTime();
}

void Mdvx::clearReadPath()

{
  _readPath = ".";
  _readPathSet = false;
}

//////////////////////
// set field requests

void Mdvx::addReadField(const int field_num)

{     
  if (_readFieldNames.size() > 0) {
    clearReadFields();
  }
  _readFieldNums.push_back(field_num);
  _readQualifiersActive = true;
}

void Mdvx::addReadField(const string &field_name)
  
{
  if (_readFieldNums.size() > 0) {
    clearReadFields();
  }
  _readFieldNames.push_back(field_name);
  _readQualifiersActive = true;
}

void Mdvx::clearReadFields()

{
  _readFieldNums.clear();
  _readFieldNames.clear();
}


//////////////////////
// set chunk requests

// add a chunk num

void Mdvx::addReadChunk(const int chunk_num)
{     
  _readChunkNums.push_back(chunk_num);
  _readQualifiersActive = true;
}

// set no chunks to be read - use chunk num of -1 to indicate this

void Mdvx::setReadNoChunks()
{
  clearReadChunks();
  _readChunkNums.push_back(-1);
  _readQualifiersActive = true;
}

void Mdvx::clearReadChunks()
{
  _readChunkNums.clear();
}


/////////////////////////////////
// set or clear horizontal limits

void Mdvx::setReadHorizLimits(const double min_lat,
                              const double min_lon,
                              const double max_lat,
                              const double max_lon)
  
{
  
  _readMinLat = min_lat;
  if (_readMinLat < -89.999) {
    _readMinLat = -89.999;
  }
  _readMinLon = min_lon;
  _readMaxLat = max_lat;
  if (_readMaxLat > 89.999) {
    _readMaxLat = 89.999;
  }
  _readMaxLon = max_lon;
  _readHorizLimitsSet = true;

  _readQualifiersActive = true;

}

void Mdvx::clearReadHorizLimits()

{
  _readMinLat = -89.999;
  _readMinLon = -180.0;
  _readMaxLat = 89.999;
  _readMaxLon = 180.0;
  _readHorizLimitsSet = false;
}

///////////////////////////////
// set or clear vertical limits

// vlevel limits - min and max vlevel values in floating point

void Mdvx::setReadVlevelLimits(const double min_vlevel,
                               const double max_vlevel)
  
{
  
  _readMinVlevel = min_vlevel;
  _readMaxVlevel = max_vlevel;
  _readVlevelLimitsSet = true;
  _readPlaneNumLimitsSet = false;

  _readQualifiersActive = true;

}

// plane num limits - min and max plane nums as integers

void Mdvx::setReadPlaneNumLimits(const int min_plane_num,
                                 const int max_plane_num)
  
{
  
  _readMinPlaneNum = min_plane_num;
  _readMaxPlaneNum = max_plane_num;
  _readPlaneNumLimitsSet = true;
  _readVlevelLimitsSet = false;

  _readQualifiersActive = true;

}

void Mdvx::clearReadVertLimits()

{
  _readPlaneNumLimitsSet = false;
  _readVlevelLimitsSet = false;
}

/////////////
// composite

void Mdvx::setReadComposite()
{
  _readComposite = true;
  _readQualifiersActive = true;
}

void Mdvx::clearReadComposite()
{
  _readComposite = false;
}

/////////////
// fillMissing

void Mdvx::setReadFillMissing()
{
  _readFillMissing = true;
  _readQualifiersActive = true;
}

void Mdvx::clearReadFillMissing()
{
  _readFillMissing = false;
}

///////////////////////////////////////////
// encoding, compression and scaling types

void Mdvx::setReadEncodingType(const encoding_type_t encoding_type)
{
  _readEncodingType = encoding_type;
  _readQualifiersActive = true;
}

void Mdvx::clearReadEncodingType()
{
  _readEncodingType = ENCODING_ASIS;
}

void Mdvx::setReadCompressionType(const compression_type_t compression_type)
{
  _readCompressionType = compression_type;
  _readQualifiersActive = true;
}

void Mdvx::clearReadCompressionType()
{
  _readCompressionType = COMPRESSION_ASIS;
}

void Mdvx::setReadScalingType(const scaling_type_t scaling_type,
                              const double scale /* = 1.0*/,
                              const double bias /* = 0.0*/ )
{
  _readScalingType = scaling_type;
  _readScale = scale;
  _readBias = bias;
  _readQualifiersActive = true;
}

void Mdvx::clearReadScalingType()
{
  _readScalingType = SCALING_ROUNDED;
  _readScale = 1.0;
  _readBias = 0.0;
  
}

/////////////////////////////////////////////
// number of sample points for vert section
//
// If nsamples is not set it will be computed based on
// the resolution of the grid data.

void Mdvx::setReadNVsectSamples(int n_samples)
  
{
  if (n_samples > 0) {
    _readNVsectSamples = n_samples;
  } else {
    n_samples = -1;
  }
  _readQualifiersActive = true;
}

void Mdvx::clearReadNVsectSamples()

{
  _readNVsectSamples = -1;
}

////////////////////////////////////////////////
// max number of sample points for vert section

void Mdvx::setReadMaxVsectSamples(int max_samples)

{
  _readMaxVsectSamples = max_samples;
  _readQualifiersActive = true;
}

void Mdvx::clearReadMaxVsectSamples()

{
  _readMaxVsectSamples = _defaultMaxVsectSamples;
  _readQualifiersActive = true;
}

// read vsection as RHI

void Mdvx::setReadVsectAsRhi(bool as_polar /* = true */,
			     double max_az_error /* = 2.0 */,
			     bool respectUserDistance /* = false */)
{
  _readVsectAsRhi = true;
  _readRhiAsPolar = as_polar;
  _readRhiMaxAzError = max_az_error;
  _readRhiRespectUserDist = respectUserDistance;
  _readQualifiersActive = true;
}

void Mdvx::clearReadVsectAsRhi()
{
  _readVsectAsRhi = false;
  _readRhiAsPolar = true;
  _readRhiMaxAzError = 2.0;
  _readRhiRespectUserDist = false;
}

////////////////////////////////////////////////
// Disable interpolation for vertical section
// If interp is disabled, nearest neighbor sampling is used.

void Mdvx::setReadVsectDisableInterp()

{
  _vsectDisableInterp = true;
  _readQualifiersActive = true;
}

void Mdvx::clearReadVsectDisableInterp()

{
  _vsectDisableInterp = false;
}

/////////////////////////////////////////////
// way points for reading vertical section

void Mdvx::addReadWayPt(const double lat, const double lon)

{
  vsect_waypt_t pt;
  pt.lat = lat;
  pt.lon = lon;
  // condition the longitude relative to the previous point
  if (_vsectWayPts.size() > 0) {
    double lonDiff = lon - _vsectWayPts[_vsectWayPts.size() - 1].lon;
    if (lonDiff > 180.0) {
      pt.lon -= 360.0;
    } else if (lonDiff < -180.0) {
      pt.lon += 360.0;
    }
  }
  _vsectWayPts.push_back(pt);
  _readQualifiersActive = true;
}

void Mdvx::clearReadWayPts()

{
  _vsectWayPts.clear();
}

///////////////////////////////////////////
// remapping grid on read

// remapping to latlon (cylindrical equidistant)

void Mdvx::setReadRemapLatlon(int nx, int ny,
                              double minx, double miny,
                              double dx, double dy)

{

  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_LATLON;

  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to flat (azimuthal equidistant)

void Mdvx::setReadRemapFlat(int nx, int ny,
                            double minx, double miny,
                            double dx, double dy,
                            double origin_lat, double origin_lon,
                            double rotation)

{

  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_FLAT;

  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;
  _readRemapCoords.proj_params.flat.rotation = rotation;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to lambert conformal

void Mdvx::setReadRemapLambertConf(int nx, int ny,
				   double minx, double miny,
				   double dx, double dy,
				   double origin_lat, double origin_lon,
				   double lat1, double lat2)

{

  MEM_zero(_readRemapCoords);

  _readRemapCoords.proj_type = PROJ_LAMBERT_CONF;
  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;

  _readRemapCoords.proj_params.lc2.lat1 = lat1;
  _readRemapCoords.proj_params.lc2.lat2 = lat2;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to polar stereographic

void Mdvx::setReadRemapPolarStereo(int nx, int ny,
				   double minx, double miny,
				   double dx, double dy,
				   double origin_lat, double origin_lon,
				   double tangent_lon, 
				   Mdvx::pole_type_t poleType,
				   double central_scale,
				   double lad)
{

  MEM_zero(_readRemapCoords);

  _readRemapCoords.proj_type = PROJ_POLAR_STEREO;
  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lon = tangent_lon;
  
  _readRemapCoords.proj_params.ps.tan_lon = tangent_lon;
  if (poleType == Mdvx::POLE_NORTH) {
    _readRemapCoords.proj_params.ps.pole_type = 0;
    _readRemapCoords.proj_origin_lat = 90;
  } else {
    _readRemapCoords.proj_params.ps.pole_type = 1;
    _readRemapCoords.proj_origin_lat = -90;
  }
  _readRemapCoords.proj_params.ps.central_scale = central_scale;
  _readRemapCoords.proj_params.ps.lad = lad;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to oblique stereographic

void Mdvx::setReadRemapObliqueStereo(int nx, int ny,
                                     double minx, double miny,
                                     double dx, double dy,
                                     double origin_lat, double origin_lon,
                                     double tangent_lat, double tangent_lon,
                                     double central_scale)
{
  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_OBLIQUE_STEREO;

  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;
  
  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;

  _readRemapCoords.proj_params.os.tan_lat = tangent_lat;
  _readRemapCoords.proj_params.os.tan_lon = tangent_lon;
  _readRemapCoords.proj_params.os.central_scale = central_scale;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to mercator

void Mdvx::setReadRemapMercator(int nx, int ny,
				double minx, double miny,
				double dx, double dy,
				double origin_lat, double origin_lon)

{

  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_MERCATOR;

  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to transverse_mercator

void Mdvx::setReadRemapTransverseMercator(int nx, int ny,
					  double minx, double miny,
					  double dx, double dy,
					  double origin_lat, double origin_lon,
					  double central_scale)
  
{

  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_TRANS_MERCATOR;
  
  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;
  _readRemapCoords.proj_params.tmerc.central_scale = central_scale;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to albers equal area conic

void Mdvx::setReadRemapAlbers(int nx, int ny,
			      double minx, double miny,
			      double dx, double dy,
			      double origin_lat, double origin_lon,
			      double lat1, double lat2)
  
{

  MEM_zero(_readRemapCoords);

  _readRemapCoords.proj_type = PROJ_ALBERS;
  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;

  _readRemapCoords.proj_params.albers.lat1 = lat1;
  _readRemapCoords.proj_params.albers.lat2 = lat2;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to lambert azimithal

void Mdvx::setReadRemapLambertAzimuthal(int nx, int ny,
					double minx, double miny,
					double dx, double dy,
					double origin_lat, double origin_lon)
  
{

  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_LAMBERT_AZIM;

  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

// remapping to vertical perspective

void Mdvx::setReadRemapVertPersp(int nx, int ny,
                                 double minx, double miny,
                                 double dx, double dy,
                                 double origin_lat, double origin_lon,
                                 double persp_radius)
  
{

  MEM_zero(_readRemapCoords);
  _readRemapCoords.proj_type = PROJ_VERT_PERSP;
  
  _readRemapCoords.nx = nx;
  _readRemapCoords.ny = ny;
  _readRemapCoords.minx = minx;
  _readRemapCoords.miny = miny;
  _readRemapCoords.dx = dx;
  _readRemapCoords.dy = dy;

  _readRemapCoords.proj_origin_lat = origin_lat;
  _readRemapCoords.proj_origin_lon = origin_lon;
  _readRemapCoords.proj_params.vp.persp_radius = persp_radius;

  _readRemapSet = true;
  _readQualifiersActive = true;

}

///////////////////////////////////////////
// set false coord correction for remapping
// Normally these are not set.
// The false values are added to the real coords
// to keep the values positive.

void Mdvx::setReadFalseCoords(double false_northing,
                              double false_easting)

{
  _readRemapCoords.false_northing = false_northing;
  _readRemapCoords.false_easting = false_easting;
}
  
/////////////////////////////////////////
// remapping to the selected projection

int Mdvx::setReadRemap(const MdvxProj &proj)
{

  const coord_t &coord = proj.getCoord();

  switch (proj.getProjType()){

    case Mdvx::PROJ_LATLON: {
      setReadRemapLatlon(coord.nx, coord.ny, 
                         coord.minx, coord.miny,
                         coord.dx, coord.dy);
      break;
    }
    
    case Mdvx::PROJ_FLAT: {
      setReadRemapFlat(coord.nx, coord.ny, 
                       coord.minx, coord.miny,
                       coord.dx, coord.dy,
                       coord.proj_origin_lat,
                       coord.proj_origin_lon,
                       coord.proj_params.flat.rotation);
      break;
    }
    
    case Mdvx::PROJ_LAMBERT_CONF: {
      setReadRemapLambertConf(coord.nx, coord.ny, 
                              coord.minx, coord.miny,
                              coord.dx, coord.dy,
                              coord.proj_origin_lat,
                              coord.proj_origin_lon,
                              coord.proj_params.lc2.lat1,
                              coord.proj_params.lc2.lat2);
      break;
    }
    
    case Mdvx::PROJ_POLAR_STEREO: {
      pole_type_t poleType = POLE_NORTH;
      if (coord.proj_params.ps.pole_type != 0) {
        poleType = POLE_SOUTH;
      }
      setReadRemapPolarStereo(coord.nx, coord.ny, 
                              coord.minx, coord.miny,
                              coord.dx, coord.dy,
                              coord.proj_origin_lat, 
                              coord.proj_origin_lon,
                              coord.proj_params.ps.tan_lon,
                              poleType,
                              coord.proj_params.ps.central_scale,
                              coord.proj_params.ps.lad);
      break;
    }

    case Mdvx::PROJ_OBLIQUE_STEREO: {
      setReadRemapObliqueStereo(coord.nx, coord.ny, 
                                coord.minx, coord.miny,
                                coord.dx, coord.dy,
                                coord.proj_origin_lat, 
                                coord.proj_origin_lon,
                                coord.proj_params.os.tan_lat,
                                coord.proj_params.os.tan_lon,
                                coord.proj_params.os.central_scale);
      break;
    }

    case Mdvx::PROJ_MERCATOR: {
      setReadRemapMercator(coord.nx, coord.ny, 
                           coord.minx, coord.miny,
                           coord.dx, coord.dy,
                           coord.proj_origin_lat, 
                           coord.proj_origin_lon);
      break;
    }

    case Mdvx::PROJ_TRANS_MERCATOR: {
      setReadRemapTransverseMercator(coord.nx, coord.ny, 
                                     coord.minx, coord.miny,
                                     coord.dx, coord.dy,
                                     coord.proj_origin_lat, 
                                     coord.proj_origin_lon,
                                     coord.proj_params.tmerc.central_scale);
      break;
    }

    case Mdvx::PROJ_ALBERS: {
      setReadRemapAlbers(coord.nx, coord.ny, 
                         coord.minx, coord.miny,
                         coord.dx, coord.dy,
                         coord.proj_origin_lat,
                         coord.proj_origin_lon,
                         coord.proj_params.albers.lat1,
                         coord.proj_params.albers.lat2);
      break;
    }
    
    case Mdvx::PROJ_LAMBERT_AZIM: {
      setReadRemapLambertAzimuthal(coord.nx, coord.ny, 
                                   coord.minx, coord.miny,
                                   coord.dx, coord.dy,
                                   coord.proj_origin_lat, 
                                   coord.proj_origin_lon);
      break;
    }

    case Mdvx::PROJ_VERT_PERSP: {
      setReadRemapVertPersp(coord.nx, coord.ny, 
                            coord.minx, coord.miny,
                            coord.dx, coord.dy,
                            coord.proj_origin_lat, 
                            coord.proj_origin_lon,
                            coord.proj_params.vp.persp_radius);
      break;
    }
    
    default: {
      _errStr += "ERROR - Mdvx::setReadRemap\n";
      _errStr += string("Unsupported projection: ")
        + Mdvx::projType2Str(proj.getProjType()) + "\n";
      return -1;
    }

  } // switch

  if (coord.false_northing != 0.0 || coord.false_easting != 0.0) {
    setReadFalseCoords(coord.false_northing,
                       coord.false_easting);
  }

  return 0;
}

// remapping to the given projection

int Mdvx::setReadRemap(const MdvxPjg &proj)
{

  switch (proj.getProjType()) {

    case Mdvx::PROJ_LATLON: {
      setReadRemapLatlon(proj.getNx(), proj.getNy(),
                         proj.getMinx(), proj.getMiny(),
                         proj.getDx(), proj.getDy());
      break;
    }
    
    case Mdvx::PROJ_FLAT: {
      setReadRemapFlat(proj.getNx(), proj.getNy(),
                       proj.getMinx(), proj.getMiny(),
                       proj.getDx(), proj.getDy(),
                       proj.getOriginLat(), proj.getOriginLon(),
                       proj.getRotation());
      break;
    }
    
    case Mdvx::PROJ_LAMBERT_CONF: {
      setReadRemapLc2(proj.getNx(), proj.getNy(),
                      proj.getMinx(), proj.getMiny(),
                      proj.getDx(), proj.getDy(),
                      proj.getOriginLat(), proj.getOriginLon(),
                      proj.getLat1(), proj.getLat2());
      break;
    }
    
    default: {
      _errStr += "ERROR - Mdvx::setReadRemap\n";
      _errStr += string("Unsupported projection: ")
        + Mdvx::projType2Str(proj.getProjType()) + "\n";
      return -1;
    
    }
    
  } // switch
  
  return 0;

}

void Mdvx::clearReadRemap()

{
  MEM_zero(_readRemapCoords);
  _readRemapSet = false;
}

// auto remap to lat-lon

void Mdvx::setReadAutoRemap2LatLon()
{
  _readAutoRemap2LatLon = true;
  _readQualifiersActive = true;
}

void Mdvx::clearReadAutoRemap2LatLon()
{
  _readAutoRemap2LatLon = false;
}

// decimation

void Mdvx::setReadDecimate(size_t max_nxy)
{
  _readDecimateMaxNxy = max_nxy;
  _readDecimate = true;
  _readQualifiersActive = true;
}

void Mdvx::clearReadDecimate()
{
  _readDecimateMaxNxy = 0;
  _readDecimate = false;
}

// vlevel type to be returned from read

void Mdvx::setReadVlevelType(vlevel_type_t vlevel_type)
{
  _readVlevelType = vlevel_type;
  _readSpecifyVlevelType = true;
  _readQualifiersActive = true;
}

void Mdvx::clearReadVlevelType()
{
  _readVlevelType = (vlevel_type_t) 0;
  _readSpecifyVlevelType = false;
}

////////////////////
// FieldFileHeaders

void Mdvx::setReadFieldFileHeaders()
{
  _readFieldFileHeaders = true;
}

void Mdvx::clearReadFieldFileHeaders()
{
  _readFieldFileHeaders = false;
}

/////////////////////////////////////////////////
// time list also
// Also read time list when reading volume etc.
// You must call setTimeListModeXxxx() as well.
// Time list data will be returned.
// Use compileTimeList() if only time list is required.

void Mdvx::setReadTimeListAlso()
{
  _readTimeListAlso = true;
}

void Mdvx::clearReadTimeListAlso()
{
  _readTimeListAlso = false;
}

///////////////////////
// readAsSingleBuffer

void Mdvx::setReadAsSingleBuffer()
{
  _readAsSingleBuffer = true;
}

void Mdvx::clearReadAsSingleBuffer()
{
  _readAsSingleBuffer = false;
}

void Mdvx::setReadAsSinglePart()
{
  setReadAsSingleBuffer();
}

void Mdvx::clearReadAsSinglePart()
{
  clearReadAsSingleBuffer();
}

///////////////////////
// read format

void Mdvx::setReadFormat(mdv_format_t read_format)
{
  _readFormat = read_format;
}

void Mdvx::clearReadFormat()
{
  _readFormat = FORMAT_MDV;
}

/////////////////////////////////////////////////
// Set latest valid file mod time.
// Option to check file modify times and only use
// files with mod times before a given time.

void Mdvx::setCheckLatestValidModTime(time_t latest_valid_mod_time)
{
  _timeList.setCheckLatestValidModTime(latest_valid_mod_time);
}

void Mdvx::clearCheckLatestValidModTime()
{
  _timeList.clearCheckLatestValidModTime();
}

///////////////////////////////////////////////////////////
// Set the weight given to valid time differences, over gen
// time differences, when searching for the best forecast
// for a specified time
//
// Finding the best forecast for a given time is tricky. Do
// you care more about differences between the valid time
// and the requested time, or do you want to give more weight
// to the closest gen time.
//
// The default value is 2.5. This works well for most situations.
// 
// If the time between model runs is long (say 6 hours) as compared
// to the time between model output times (say 30 mins) then you
// need to increase the weight to say 25. Setting it to 100
// will weight the decision very heavily in favor of the diff
// between the valid and requested time, and put very little
// weight on which model run to use.

void Mdvx::setValidTimeSearchWt(double wt)
{
  _timeList.setValidTimeSearchWt(wt);
}

void Mdvx::clearValidTimeSearchWt()
{
  _timeList.clearValidTimeSearchWt();
}

double Mdvx::getValidTimeSearchWt() const
{
  return _timeList.getValidTimeSearchWt();
}

//////////////////////
// print read request

void Mdvx::printReadRequest(ostream &out)

{

  out << "Mdvx read request" << endl;
  out << "-----------------" << endl;

  if (_readHorizLimitsSet) {
    out << "  Min lat: " << _readMinLat << endl;
    out << "  Min lon: " << _readMinLon << endl;
    out << "  Max lat: " << _readMaxLat << endl;
    out << "  Max lon: " << _readMaxLon << endl;
  }

  if (_readVlevelLimitsSet) {
    out << "  Min vlevel: " << _readMinVlevel << endl;
    out << "  Max vlevel: " << _readMaxVlevel << endl;
  }

  if (_readPlaneNumLimitsSet) {
    out << "  Min plane num: " << _readMinPlaneNum << endl;
    out << "  Max plane num: " << _readMaxPlaneNum << endl;
  }

  out << "  Encoding type: "
      << encodingType2Str(_readEncodingType) << endl;

  out << "  Compression type: "
      << compressionType2Str(_readCompressionType) << endl;

  out << "  Scaling type: "
      << scalingType2Str(_readScalingType) << endl;
  
  if (_readScalingType == SCALING_SPECIFIED) {
    out << "  Scale: " << _readScale << endl;
    out << "  Bias: " << _readBias << endl;
  }

  out << "  Composite?: " << _readComposite << endl;
  out << "  FillMissing?: " << _readFillMissing << endl;

  if (_readFieldNums.size() > 0) {
    out << "  Field nums: ";
    for (size_t i = 0; i < _readFieldNums.size(); i++) {
      out << _readFieldNums[i] << " ";
    }
    out << endl;
  }

  if (_readFieldNames.size() > 0) {
    out << "  Field names: ";
    for (size_t i = 0; i < _readFieldNames.size(); i++) {
      out << _readFieldNames[i];
      if (i < _readFieldNames.size() - 1) {
        out << ", ";
      }
    }
    out << endl;
  }
  
  if (_readChunkNums.size() > 0) {
    out << "  Chunk nums: ";
    for (size_t i = 0; i < _readChunkNums.size(); i++) {
      out << _readChunkNums[i] << " ";
    }
    out << endl;
  }

  if (_vsectWayPts.size() > 0) {
    out << "  Number of way points: " << _vsectWayPts.size() << endl;
    for (size_t i = 0; i < _vsectWayPts.size(); i++) {
      out << "    " << i << ": lat " << _vsectWayPts[i].lat << ", lon "
          << _vsectWayPts[i].lon << endl;
    }
  }

  if (_readNVsectSamples != -1) {
    out << "  N vsect samples: " << _readNVsectSamples << endl;
  }
  if (_readMaxVsectSamples != _defaultMaxVsectSamples) {
    out << "  Max vsect samples: " << _readMaxVsectSamples << endl;
  }
  if (_vsectDisableInterp) {
    out << "  Vert section interpolation disabled." << endl;
    out << "    Nearest neighbor sampling will be used." << endl;
  }

  if (_readRemapSet) {
    out << "  Remapping coords:" << endl;
    MdvxProj::printCoord(_readRemapCoords, out);
  }

  if (_readAutoRemap2LatLon) {
    out << "  Auto remap to LatLon" << endl;
  }

  if (_readDecimate) {
    out << "  Decimation true, maxNxy: " << _readDecimateMaxNxy << endl;
  }

  if (_readSpecifyVlevelType) {
    out << "  Requesting vlevel type: "
	<< vertType2Str(_readVlevelType) << endl;
  }

  if (_readVsectAsRhi) {
    out << "  Requesting vsection as RHI" << endl;
    out << "    as Polar?: " << _readRhiAsPolar << endl;
    out << "    MaxAzError: " << _readRhiMaxAzError << endl;
  }

  out << "  FieldFileHeaders?: " << _readFieldFileHeaders << endl;

  if (_readTimeSet) {

    switch (_readSearchMode) {
      case READ_LAST:
        out << "  Search mode: READ_LAST" << endl;
        break;
      case READ_CLOSEST:
        out << "  Search mode: READ_CLOSEST" << endl;
        break;
      case READ_FIRST_BEFORE:
        out << "  Search mode: READ_FIRST_BEFORE" << endl;
        break;
      case READ_FIRST_AFTER:
        out << "  Search mode: READ_FIRST_AFTER" << endl;
        break;
      case READ_BEST_FORECAST:
        out << "  Search mode: READ_BEST_FORECAST" << endl;
        break;
      case READ_SPECIFIED_FORECAST:
        out << "  Search mode: READ_SPECIFIED_FORECAST" << endl;
        break;
      default:
        out << "  Search mode: UNKNOWN" << endl;
    }

    if (_readSearchMode != READ_LAST) {
      out << "  Search time: " << utimstr(_readSearchTime) << endl;
      out << "  Search margin: " << _readSearchMargin << " secs" << endl;
    }

    if (_readSearchMode == READ_SPECIFIED_FORECAST) {
      out << "  Forecast lead time: " << _readForecastLeadTime
          << " secs" << endl;
    }

    out << "  Read dir: " << _readDir << endl;

  } else if (_readPathSet) {

    out << "  Read path: " << _readPath << endl;

  }

  if (_timeList.getConstrainFcastLeadTimes()) {
    out << "  Constrain forecast lead times: TRUE" << endl;
    out << "    Min lead time: " << _timeList.getMinFcastLeadTime() << endl;
    out << "    Max lead time: " << _timeList.getMaxFcastLeadTime() << endl;
    if (_timeList.getSpecifyFcastByGenTime()) {
      out << "    Specify search by gen time: TRUE" << endl;
    }
  }

  if (_readTimeListAlso) {
    out << "  Read time list also?: true" << endl;
    printTimeListRequest(out);
  }

  if (_readAsSingleBuffer) {
    out << "  ReadAsSingleBuffer?: true" << endl;
  }

  if (_readFormat != FORMAT_MDV) {
    out << "  ReadFormat: " << format2Str(_readFormat) << endl;
  }

  if (_timeList.checkLatestValidModTime()) {
    out << "  Set check latest valid mod time on read?: true" << endl;
    out << "  Latest valid mod time: "
	<< DateTime::str(_timeList.getLatestValidModTime(), false) << endl;
  }

  if (_read32BitHeaders) {
    out << "  Read32BitHeaders?: true" << endl;
  } else {
    out << "  Read32BitHeaders?: false" << endl;
  }

}

////////////////////////////////////////////////////////////////////
// verify()
//
// Verify that a file is in MDV format, by checking the magic cookie
//
// Returns true or false
//
 
bool Mdvx::verify(const string &file_path)

{

  // check for 64-bit version

  if (checkIs64Bit(file_path) == 0) {
    return true;
  } else {
    return false;
  }

}

////////////////////////////////////////////////////////////////////
// checkIs64bit()
//
// Check if an MDV file is a 64-bit version
//
// Returns 0 on success, -1 on failure
// Sets the _is64Bit flag appropriately.
// Use getIs64Bit() after this call.
 
int Mdvx::checkIs64Bit(const string &file_path)

{

  // open file - will be closed when it goes out of scope

  TaFile file;
  if (file.fopenUncompress(file_path.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "Cannot open file: ";
    _errStr += file_path;
    _errStr += ": ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // make sure there is enough space for magic cookie

  if (file.fstat()) {
    int errNum = errno;
    _errStr += "Cannot stat file: ";
    _errStr += file_path;
    _errStr += ": ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  size_t fileSize = file.getStat().st_size;
  if (fileSize < (int) (2 * sizeof(si32))) {
    _errStr += "File too small for MDV format: ";
    _errStr += file_path;
    return -1;
  }
  
  // Read in the fortran size and magic cookie

  master_header_t mhdr;
  if (file.fread(&mhdr, sizeof(si32), 2) != 2) {
    _errStr += "File is not in MDV format: ";
    _errStr += file_path;
    _errStr += "\n";
    return -1;
  }
  
  file.fclose();

  // check the magic cookie

  si32 magic_cookie = BE_to_si32(mhdr.struct_id);

  if (magic_cookie == MASTER_HEAD_MAGIC_COOKIE_32) {
    _is64Bit = false;
    if (fileSize < sizeof(master_header_32_t)) {
      return -1;
    } else {
      return 0;
    }
  } else if (magic_cookie == MASTER_HEAD_MAGIC_COOKIE_64) {
    _is64Bit = true;
    if (fileSize < sizeof(master_header_64_t)) {
      return -1;
    } else {
      return 0;
    }
  } else {
    _errStr += "File is not in MDV format: ";
    _errStr += file_path;
    _errStr += "\n";
    return -1;
  }
  
}

//////////////////////////////////////////////////////////////////////
// Read all of the headers in a file. The headers are read in exaclty
// as they exist in the file.
//
// You must call either setReadTime() or setReadPath() before calling
// this function.
//
// Returns 0 on success, -1 on failure.
// getErrStr() retrieves the error string.
//
// If successful, you can access the headers using:
//
//  getMasterHeaderFile()
//  getFieldHeaderFile()
//  getVlevelHeaderFile()
//  getChunkHeaderFile()

int Mdvx::readAllHeaders()
{

  if (_readTimeListAlso) {
    if (compileTimeList()) {
      _errStr += "ERROR - Mdvx::readAllHeaders\n";
      _errStr += "  Time list requested in addition to volume data.\n";
      return -1;
    }
  }

  if (_computeReadPath()) {
    _errStr += "ERROR - Mdvx::readAllHeaders\n";
    return -1;
  }
  if (_debug) {
    cerr << "Mdvx::readAllHeaders - reading file: " << _pathInUse << endl;
  }

  // check file format

  if (isXmlFile(_pathInUse)) {

    // For XML-based file, read in volume to get at headers
  
    if (_readVolumeXml(false, false, false, false, -180, 180)) {
      _errStr += "ERROR - Mdvx::readAllHeaders\n";
      _errStr += "  Reading XML format file\n";
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }
    _mhdrFile = _mhdr;
    for (int ii = 0; ii < (int) _fields.size(); ii++) {
      _fhdrsFile.push_back(_fields[ii]->getFieldHeader());
      _vhdrsFile.push_back(_fields[ii]->getVlevelHeader());
    }
    for (int ii = 0; ii < (int) _chunks.size(); ii++) {
      _chdrsFile.push_back(_chunks[ii]->getHeader());
    }

    clearFields();
    clearChunks();

  } else if (isRadxFile(_pathInUse)) {

    // read RADX radial radar format file

    if (_readAllHeadersRadx(_pathInUse)) {
      _errStr += "ERROR - Mdvx::_readAllHeadersLocal.\n";
      TaStr::AddStr(_errStr, "  Reading headers from RADX file");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }

  } else if (isNcfFile(_pathInUse)) {

    // NetCDF file

    if (_readAllHeadersNcf(_pathInUse)) {
      _errStr += "ERROR - Mdvx::readAllHeaders.\n";
      TaStr::AddStr(_errStr, "  Reading headers from NCF file");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }
    
  } else {

    // MDV native format
    
    if (_readAllHeadersMdv()) {
      return -1;
    }
    
  }

  // success

  return 0;

}

////////////////////////////////////////////////////////////////////
// Read volume, according to the read settings.
//
// To specify the file path, you must either
//   setReadTime() or
//   setReadPath().
//
// The following calls are optional prior to calling read():
//
//   addReadField(): the specified fields are read.
//                   If not called, all fields are read.
//
//   addReadChunk(): the specified chunks are read.
//                   If not called, all chunks are read.
//
//   setReadHorizLimits(): horizontal limits apply.
//
//   setReadVlevelLimits(): vlevel limits apply.
//     or
//   setReadPlaneNumLimits(): plane num limits apply.
//                       
//   setReadEncodingType(): specify encoding type
//                          default is ENCODING_ASIS
//
//   setReadCompressionType(): specify compression type
//                             default is COMPRESSION_ASIS
//
//   setReadScalingType(): specify scaling type for data conversions
//                         default is SCALING_ROUNDED
//
//   setReadComposite(): compute the composite.
//                       This uses vlevel or plane num limits if set.
//
//   setReadFieldFileHeaders(): return file headers with field
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int Mdvx::readVolume()

{

  // clear pertinent components
  
  clearErrStr();
  clearMasterHeader();
  clearFields();
  clearChunks();

  // read time list as appropriate

  if (_readTimeListAlso) {
    if (compileTimeList()) {
      _errStr += "ERROR - Mdvx::readVolume\n";
      _errStr += "  Time list requested in addition to volume data.\n";
      return -1;
    }
  }

  // get the path of the file

  if (_computeReadPath()) {
    _errStr += "ERROR - Mdvx::readVolume\n";
    return -1;
  }

  if (_debug) {
    cerr << "Mdvx::readVolume - reading file: " << _pathInUse << endl;
  }

  // check file format

  if (isRadxFile(_pathInUse)) {
    
    // read RADX radial radar format file

    if (_readRadx(_pathInUse)) {
      _errStr += "ERROR - Mdvx::readVolume.\n";
      TaStr::AddStr(_errStr, "  Reading RADX file");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }

  } else if (isNcfFile(_pathInUse)) {

    // read NCF file
    
    if (_readNcf(_pathInUse)) {
      _errStr += "ERROR - Mdvx::readVolume.\n";
      TaStr::AddStr(_errStr, "  Reading NCF file");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }

  } else {
  
    // native MDV or MDV-XML
    
    if (_readVolumeMdv(_readFillMissing, _readDecimate, true)) {
      return -1;
    }

  }

  // convert to NetCDF or XML buffers if requested

  if (_convertFormatOnRead("readVolume")) {
    return -1;
  }

  // success

  return 0;
  
}

//////////////////////////////////////////////////////////
// Read vertical section, according to the read settings.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int Mdvx::readVsection()
  
{
  
  // read time list as appropriate

  if (_readTimeListAlso) {
    if (compileTimeList()) {
      _errStr += "ERROR - Mdvx::readVsection\n";
      _errStr += "  Time list requested in addition to volume data.\n";
      return -1;
    }
  }

  // get the path of the file
  
  if (_computeReadPath()) {
    _errStr += "ERROR - Mdvx::readVsection\n";
    return -1;
  }
  if (_debug) {
    cerr << "Mdvx::readVsection - reading file: " << _pathInUse << endl;
  }

  // Check file format
  
  if (isRadxFile(_pathInUse)) {

    // read RADX radial radar format file
    // vsection taken care of by Ncf2MdvTrans

    if (_readRadx(_pathInUse)) {
      _errStr += "ERROR - Mdvx::readVsection.\n";
      TaStr::AddStr(_errStr, "  Reading RADX file");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }

  } else if (isNcfFile(_pathInUse)) {

    // read NCF file
    // vsection taken care of by Ncf2MdvTrans

    if (_readNcf(_pathInUse)) {
      _errStr += "ERROR - Mdvx::readVsection.\n";
      TaStr::AddStr(_errStr, "  Reading NCF file");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }

  } else {

    // native MDV or MDV-XML
    
    if (_readVsectionMdv()) {
      return -1;
    }

  }
    
  // convert to NetCDF or XML buffers if requested

  if (_convertFormatOnRead("readVsection")) {
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////
// convert internal representation format on read

int Mdvx::_convertFormatOnRead(const string &caller)

{

  if (_readFormat == FORMAT_NCF) {

    if (_debug) {
      cerr << "DEBUG - Mdvx::_convertFormatOnRead - to FORMAT_NCF" << endl;
    }

    if (_writeToNcfBuf(_pathInUse)) {
      TaStr::AddStr(_errStr, "ERROR - Mdvx::", caller);
      TaStr::AddStr(_errStr, "  Converting format after read");
      TaStr::AddStr(_errStr, "  File: ", _pathInUse);
      return -1;
    }
    _internalFormat = FORMAT_NCF;

  } else if (_readFormat == FORMAT_XML) {

    if (_debug) {
      cerr << "DEBUG - Mdvx::_convertFormatOnRead - to FORMAT_XML" << endl;
    }

    writeToXmlBuffer(_xmlHdr, _xmlBuf, _pathInUse);
    _internalFormat = FORMAT_XML;
    
  }

  return 0;

}
  
//////////////////////////////////////////////////////////
// Compute the number of samples for a vsection

int Mdvx::_computeNVsectSamples() const
  
{

  
  int n_samples = _defaultMaxVsectSamples;

  if (_readNVsectSamples > 0) {

    n_samples = _readNVsectSamples;
    
  } else {
    
    // compute total length
    
    double totLength = 0.0;
    for (size_t i = 1; i < _vsectWayPts.size(); i++) {
      double dist, azimuth;
      PJGLatLon2RTheta(_vsectWayPts[i-1].lat, _vsectWayPts[i-1].lon,
                       _vsectWayPts[i].lat, _vsectWayPts[i].lon,
                       &dist, &azimuth);
      totLength += dist;
    }

    // compute the min resolution in km for all fields

    double minDeltaKm = 1000.0;
    
    for (size_t i = 0; i < _fields.size(); i++) {
      const field_header_t &fhdr = _fields[i]->getFieldHeader();
      if (fhdr.proj_type == PROJ_LATLON) {
        double dyKm = fhdr.grid_dy / DEG_PER_KM_AT_EQ;
        minDeltaKm = MIN(dyKm, minDeltaKm);
        double maxLat = fhdr.grid_miny + fhdr.ny * fhdr.grid_dy;
        double dxKm =
          (fhdr.grid_dx / DEG_PER_KM_AT_EQ) * cos(maxLat * DEG_TO_RAD);
        minDeltaKm = MIN(fabs(dxKm), minDeltaKm);
        double minLat = fhdr.grid_miny;
        dxKm = (fhdr.grid_dx / DEG_PER_KM_AT_EQ) * cos(minLat * DEG_TO_RAD);
        minDeltaKm = MIN(fabs(dxKm), minDeltaKm);
      } else if (fhdr.proj_type == PROJ_POLAR_RADAR) {
        minDeltaKm = fhdr.grid_dx;
      } else {
        minDeltaKm = MIN(fhdr.grid_dy, minDeltaKm);
        minDeltaKm = MIN(fhdr.grid_dx, minDeltaKm);
      }
    }
    minDeltaKm = MAX(0.00001, minDeltaKm);

    // compute number of samples
    // start with 10 samples per grid location

    double dn_samples = (totLength / minDeltaKm) * 10.0;
    if (dn_samples < _readMaxVsectSamples) {
      n_samples = (int) (dn_samples + 0.5);
    } else {
      n_samples = _readMaxVsectSamples;
    }
    n_samples = MAX(n_samples, _defaultMaxVsectSamples / 4);
    n_samples = MIN(n_samples, _defaultMaxVsectSamples * 2);

  } // if (_readNVsectSamples > 0) 

  if (n_samples < 2) {
    n_samples = 2;
  }

  return n_samples;

}

///////////////////////////////////////////////////////////
// Read from buffer
//
// Read an Mdvx object from a buffer as if it were a file.
//
// Returns 0 on success, -1 on failure

int Mdvx::readFromBuffer(const MemBuf &buf)
  
{

  clearErrStr();
  clearFields();
  clearChunks();

  if (_debug) {
    cerr << "Mdvx - reading object from buffer." << endl;
  }

  // check for 64 bit
  
  if (buf.getLen() < (int) (2 * sizeof(si32))) {
    _errStr += "ERROR - Mdvx::readFromBuffer.\n";
    _errStr += "  Buffer too short for master header.\n";
    TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
    return -1;
  } else {
    master_header_t mhdr;
    memcpy(&mhdr, buf.getPtr(), 2 * sizeof(si32));
    si32 magic_cookie = BE_to_si32(mhdr.struct_id);
    if (magic_cookie == MASTER_HEAD_MAGIC_COOKIE_64) {
      _is64Bit = true;
      if (_debug) {
        cerr << "  Found 64-bit master header" << endl;
      }
    } else if (magic_cookie == MASTER_HEAD_MAGIC_COOKIE_32) {
      _is64Bit = false;
      if (_debug) {
        cerr << "  Found 32-bit master header" << endl;
      }
    } else {
      _errStr += "ERROR - Mdvx::readFromBuffer.\n";
      _errStr += "  Bad magic cookie in master header.\n";
      TaStr::AddInt(_errStr, "  cookie: ", magic_cookie);
      return -1;
    }
  }

  // master header

  master_header_t mhdr;
  if (_is64Bit) {
    if (buf.getLen() < (int) sizeof(master_header_t)) {
      _errStr += "ERROR - Mdvx::readFromBuffer.\n";
      _errStr += "  Buffer too short for master_header_t.\n";
      TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
      return -1;
    }
    memcpy(&mhdr, buf.getPtr(), sizeof(mhdr));
    master_header_from_BE(mhdr);
    setMasterHeader(mhdr);
  } else {
    // 32 bit
    if (buf.getLen() < (int) sizeof(master_header_32_t)) {
      _errStr += "ERROR - Mdvx::readFromBuffer.\n";
      _errStr += "  Buffer too short for master_header_32_t.\n";
      TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
      return -1;
    }
    master_header_32_t mhdr32;
    memcpy(&mhdr32, buf.getPtr(), sizeof(mhdr32));
    master_header_from_BE_32(mhdr32);
    _copyMasterHeader32to64(mhdr32, mhdr);
    setMasterHeader(mhdr);
  }
  
  // fields
  
  if (mhdr.n_fields > 0) {

    field_header_t fhdr;
    vlevel_header_t vhdr;
    
    if (_is64Bit) {

      int64_t min_len =
        mhdr.vlevel_hdr_offset + mhdr.n_fields * sizeof(vlevel_header_t);
      if ((int) buf.getLen() < min_len) {
        _errStr += "ERROR - Mdvx::readFromBuffer.\n";
        _errStr += "  Buffer too short for field and vlevel headers.\n";
        TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
        return -1;
      }
    
      for (int ifield = 0; ifield < mhdr.n_fields; ifield++) {
        
        memcpy(&fhdr, ((char *) buf.getPtr() + mhdr.field_hdr_offset +
                       ifield * sizeof(field_header_t)), sizeof(fhdr));
        field_header_from_BE(fhdr);
        
        memcpy(&vhdr, ((char *) buf.getPtr() + mhdr.vlevel_hdr_offset +
                       ifield * sizeof(vlevel_header_t)), sizeof(vhdr));
        vlevel_header_from_BE(vhdr);
        
        int64_t min_len = fhdr.field_data_offset + fhdr.volume_size;
        if ((int) buf.getLen() < min_len) {
          _errStr += "ERROR - Mdvx::readFromBuffer.\n";
          _errStr += "  Buffer too short for field volume data.\n";
          TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
          return -1;
        }
        
      } // ifield
      
    } else {
      
      // 32-bit
      
      int64_t min_len =
        mhdr.vlevel_hdr_offset + mhdr.n_fields * sizeof(vlevel_header_32_t);
      if ((int) buf.getLen() < min_len) {
        _errStr += "ERROR - Mdvx::readFromBuffer.\n";
        _errStr += "  Buffer too short for 32-bit field and vlevel headers.\n";
        TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
        return -1;
      }
    
      for (int ifield = 0; ifield < mhdr.n_fields; ifield++) {
        
        field_header_32_t fhdr32;
        memcpy(&fhdr32, ((char *) buf.getPtr() + mhdr.field_hdr_offset +
                         ifield * sizeof(field_header_t)), sizeof(fhdr32));
        field_header_from_BE_32(fhdr32);
        _copyFieldHeader32to64(fhdr32, fhdr);
        
        vlevel_header_32_t vhdr32;
        memcpy(&vhdr32, ((char *) buf.getPtr() + mhdr.vlevel_hdr_offset +
                         ifield * sizeof(vlevel_header_t)), sizeof(vhdr32));
        vlevel_header_from_BE_32(vhdr32);
        _copyVlevelHeader32to64(vhdr32, vhdr);
        
        int64_t min_len = fhdr.field_data_offset + fhdr.volume_size;
        if ((int) buf.getLen() < min_len) {
          _errStr += "ERROR - Mdvx::readFromBuffer.\n";
          _errStr += "  Buffer too short for field volume data.\n";
          TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
          return -1;
        }
        
      } // ifield

    }
    
    // create a tmp buffer for the data, byte-swap if not compressed
    
    void *volData = ((char *) buf.getPtr() + fhdr.field_data_offset);
    MemBuf tmpbuf;
    tmpbuf.add(volData, fhdr.volume_size);
    MdvxField::_data_from_BE(fhdr, tmpbuf.getPtr(), tmpbuf.getLen());
    
    // create new field
    
    MdvxField *field = new MdvxField(fhdr, vhdr, tmpbuf.getPtr());
    
    // add field to object
    
    addField(field);
    
  }

  // chunks
  
  if (mhdr.n_chunks > 0) {

    if (_is64Bit) {
      
      // 64 bit

      int64_t min_len =
        mhdr.chunk_hdr_offset + mhdr.n_chunks * sizeof(chunk_header_t);
      if ((int) buf.getLen() < min_len) {
        _errStr += "ERROR - Mdvx::readFromBuffer.\n";
        _errStr += "  Buffer too short for chunk headers.\n";
        TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
        TaStr::AddInt(_errStr, "  min_len: ", min_len);
        return -1;
      }
      
      for (int i = 0; i < mhdr.n_chunks; i++) {
        chunk_header_t chdr =
          *((chunk_header_t *) ((char *) buf.getPtr() + mhdr.chunk_hdr_offset +
                                i * sizeof(chunk_header_t)));
        chunk_header_from_BE(chdr);
        int64_t min_len = chdr.chunk_data_offset + chdr.size;
        if ((int) buf.getLen() < min_len) {
          _errStr += "ERROR - Mdvx::readFromBuffer.\n";
          _errStr += "  Buffer too short for chunk data.\n";
          TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
          TaStr::AddInt(_errStr, "  min_len: ", min_len);
          return -1;
        }
        void *chunkData = ((char *) buf.getPtr() + chdr.chunk_data_offset);
        MdvxChunk *chunk = new MdvxChunk(chdr, chunkData);
        addChunk(chunk);
      }

    } else {
      
      // 32 bit

      int64_t min_len =
        mhdr.chunk_hdr_offset + mhdr.n_chunks * sizeof(chunk_header_32_t);
      if ((int) buf.getLen() < min_len) {
        _errStr += "ERROR - Mdvx::readFromBuffer.\n";
        _errStr += "  Buffer too short for chunk headers.\n";
        TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
        TaStr::AddInt(_errStr, "  min_len: ", min_len);
        return -1;
      }
      
      for (int i = 0; i < mhdr.n_chunks; i++) {

        chunk_header_32_t chdr32 =
          *((chunk_header_32_t *) ((char *) buf.getPtr() + mhdr.chunk_hdr_offset +
                                   i * sizeof(chunk_header_32_t)));
        chunk_header_from_BE_32(chdr32);
        int64_t min_len = chdr32.chunk_data_offset + chdr32.size;
        if ((int) buf.getLen() < min_len) {
          _errStr += "ERROR - Mdvx::readFromBuffer.\n";
          _errStr += "  Buffer too short for chunk data.\n";
          TaStr::AddInt(_errStr, "  Buffer len: ", buf.getLen());
          TaStr::AddInt(_errStr, "  min_len: ", min_len);
          return -1;
        }
        chunk_header_t chdr;
        _copyChunkHeader32to64(chdr32, chdr);
        void *chunkData = ((char *) buf.getPtr() + chdr.chunk_data_offset);
        MdvxChunk *chunk = new MdvxChunk(chdr, chunkData);
        addChunk(chunk);
      }

    } // 32 bit

  } // if (mhdr.n_chunks > 0)
    
  // set data set info from chunks as appropriate

  _setDataSetInfoFromChunks();

  return 0;
  
}

///////////////////////////////////////////
// Read using the buffer routines.
//
// This is intended for testing only.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int Mdvx::readUsingBuf()

{

  clearErrStr();

  if (_computeReadPath()) {
    _errStr += "ERROR - Mdvx::readUsingBuf\n";
    return -1;
  }
  if (_debug) {
    cerr << "Mdvx::readUsingBuf - reading file: " << _pathInUse << endl;
  }

  // open file

  TaFile infile;
  if (infile.fopenUncompress(_pathInUse.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::readUsingBuf\n";
    _errStr += "File: ";
    _errStr += _pathInUse;
    _errStr += "\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // get the file size

  if (infile.fstat()){
    _errStr += "ERROR - Mdvx::readUsingBuf\n";
    _errStr += "File: ";
    _errStr += _pathInUse;
    _errStr += "\n";
    _errStr += " Failure to stat file";
    _errStr += "\n";
    return -1; 
  }
  size_t fsize = infile.getStat().st_size;

  // prepare mem buffer

  MemBuf buf;
  buf.reserve(fsize);
  if ( !buf.getPtr()) {
    _errStr += "ERROR - Mdvx::readUsingBuf\n";
    _errStr += "Error allocating mem in MemBuf object" ;
    return -1;
  } 
  // read in entire buffer
  
  if((infile.fread(buf.getPtr(), 1, fsize)) != fsize) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::readUsingBuf\n";
    _errStr += "File: ";
    _errStr += _pathInUse;
    _errStr += "\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // close input file

  infile.fclose();

  // load the object from the buffer
  
  readFromBuffer(buf);

  return 0;

}

  
/////////////////////////////////////////////////////////////
// _computeReadPath()
//
// computes the read path
// Loads up _pathInUse and _overwriteValidTimeByGenTime
//
// returns 0 on success, -1 on failure

int Mdvx::_computeReadPath()

{

  char errstr[512];

  _noFilesFoundOnRead = false;

  // read path explicitly set
  
  if (_readPathSet) {
    _pathInUse = _readPath;
    return 0;
  }

  // set read time?

  if (!_readTimeSet) {
    _errStr += "ERROR - computeReadPath\n";
    _errStr += "  Must set either read time or read path.\n";
    return -1;
  }

  // if the read dir is actually a file, return that path

  if (ta_stat_is_file(_readDir.c_str())) {
    _pathInUse = _readDir;
    if (_debug) {
      cerr << "Mdvx::_computeReadPath():" << endl;
      cerr << "  Using _readDir as read path because it is a file." << endl;
      cerr << "  _readDir: " << _readDir << endl;
    }
    return 0;
  }
  
  switch (_readSearchMode) {

    case READ_LAST: {
      MdvxTimeList tlist;
      tlist.setModeLast(_readDir);
      if (_timeList.checkLatestValidModTime()) {
        tlist.setCheckLatestValidModTime(_timeList.getLatestValidModTime());
      }
      tlist.compile();
      if (tlist.getValidTimes().size() == 0) {
        _errStr += "ERROR - computeReadPath\n";
        TaStr::AddStr(_errStr, " Read last failed, dir: ", _readDir);
        _noFilesFoundOnRead = true;
        return -1;
      }
      _pathInUse = tlist.getPathList()[0];
      if (_overwriteValidTimeByGenTime) {
        _genTimeForOverwrite = tlist.getValidTimes()[0];
      }
      break;
    }
  
    case READ_CLOSEST: {
      MdvxTimeList tlist;
      tlist.setModeClosest(_readDir, _readSearchTime, _readSearchMargin);
      if (_timeList.checkLatestValidModTime()) {
        tlist.setCheckLatestValidModTime(_timeList.getLatestValidModTime());
      }
      if (_timeList.getConstrainFcastLeadTimes()) {
        tlist.setConstrainFcastLeadTimes
          (_timeList.getMinFcastLeadTime(),
           _timeList.getMaxFcastLeadTime(),
           _timeList.getSpecifyFcastByGenTime());
      }
      tlist.setValidTimeSearchWt(getValidTimeSearchWt());
      tlist.compile();
      if (tlist.getValidTimes().size() == 0) {
        _errStr += "ERROR - computeReadPath\n";
        TaStr::AddStr(_errStr, "  Read closest failed, dir: ", _readDir);
        TaStr::AddInt(_errStr, "  Search margin (secs): ", _readSearchMargin);
        char timeErrStr[1024];
        sprintf(timeErrStr, "  Search time: %s", utimstr(_readSearchTime));
        TaStr::AddStr(_errStr, timeErrStr);
        _noFilesFoundOnRead = true;
        return -1;
      }
      _pathInUse = tlist.getPathList()[0];
      if (_overwriteValidTimeByGenTime) {
        _genTimeForOverwrite = tlist.getValidTimes()[0];
      }
      break;
    }
    
    case READ_FIRST_BEFORE: {
      MdvxTimeList tlist;
      tlist.setModeFirstBefore(_readDir, _readSearchTime, _readSearchMargin);
      if (_timeList.checkLatestValidModTime()) {
        tlist.setCheckLatestValidModTime(_timeList.getLatestValidModTime());
      }
      if (_timeList.getConstrainFcastLeadTimes()) {
        tlist.setConstrainFcastLeadTimes
          (_timeList.getMinFcastLeadTime(),
           _timeList.getMaxFcastLeadTime(),
           _timeList.getSpecifyFcastByGenTime());
      }
      tlist.setValidTimeSearchWt(getValidTimeSearchWt());
      tlist.compile();
      if (tlist.getValidTimes().size() == 0) {
        _errStr += "ERROR - computeReadPath\n";
        TaStr::AddStr(_errStr, " Read first before failed, dir: ", _readDir);
        TaStr::AddInt(_errStr, " Search margin (secs): ", _readSearchMargin);
        char timeErrStr[1024];
        sprintf(timeErrStr, "  Search time: %s", utimstr(_readSearchTime));
        _noFilesFoundOnRead = true;
        return -1;
      }
      _pathInUse = tlist.getPathList()[0];
      if (_overwriteValidTimeByGenTime) {
        _genTimeForOverwrite = tlist.getValidTimes()[0];
      }
      break;
    }
    
    case READ_FIRST_AFTER: {
      MdvxTimeList tlist;
      tlist.setModeFirstAfter(_readDir, _readSearchTime, _readSearchMargin);
      if (_timeList.checkLatestValidModTime()) {
        tlist.setCheckLatestValidModTime(_timeList.getLatestValidModTime());
      }
      if (_timeList.getConstrainFcastLeadTimes()) {
        tlist.setConstrainFcastLeadTimes
          (_timeList.getMinFcastLeadTime(),
           _timeList.getMaxFcastLeadTime(),
           _timeList.getSpecifyFcastByGenTime());
      }
      tlist.setValidTimeSearchWt(getValidTimeSearchWt());
      tlist.compile();
      if (tlist.getValidTimes().size() == 0) {
        _errStr += "ERROR - computeReadPath\n";
        TaStr::AddStr(_errStr, " Read first after failed, dir: ", _readDir);
        TaStr::AddInt(_errStr, " Search margin (secs): ", _readSearchMargin);
        char timeErrStr[1024];
        sprintf(timeErrStr, "  Search time: %s", utimstr(_readSearchTime));
        _noFilesFoundOnRead = true;
        return -1;
      }
      _pathInUse = tlist.getPathList()[0];
      if (_overwriteValidTimeByGenTime) {
        _genTimeForOverwrite = tlist.getValidTimes()[0];
      }
      break;
    }
    
    case READ_BEST_FORECAST: {
      MdvxTimeList tlist;
      tlist.setModeBestForecast(_readDir, _readSearchTime, _readSearchMargin);
      if (_timeList.checkLatestValidModTime()) {
        tlist.setCheckLatestValidModTime(_timeList.getLatestValidModTime());
      }
      if (_timeList.getConstrainFcastLeadTimes()) {
        tlist.setConstrainFcastLeadTimes
          (_timeList.getMinFcastLeadTime(),
           _timeList.getMaxFcastLeadTime(),
           _timeList.getSpecifyFcastByGenTime());
      }
      tlist.setValidTimeSearchWt(getValidTimeSearchWt());
      tlist.compile();
      if (tlist.getValidTimes().size() == 0) {
        _errStr += "ERROR - computeReadPath\n";
        TaStr::AddStr(_errStr, " Read best forecast failed, dir: ", _readDir);
        TaStr::AddInt(_errStr, " Search margin (secs): ", _readSearchMargin);
        char timeErrStr[1024];
        sprintf(timeErrStr, "  Search time: %s", utimstr(_readSearchTime));
        _noFilesFoundOnRead = true;
        return -1;
      }
      _pathInUse = tlist.getPathList()[0];
      if (_overwriteValidTimeByGenTime) {
        _genTimeForOverwrite = tlist.getValidTimes()[0];
      }
      break;
    }

    case READ_SPECIFIED_FORECAST: {
      MdvxTimeList tlist;
      time_t genTime = _readSearchTime;
      time_t validTime = _readSearchTime + _readForecastLeadTime;
      tlist.setModeSpecifiedForecast(_readDir, genTime, validTime,
                                     _readSearchMargin);
      if (_timeList.checkLatestValidModTime()) {
        tlist.setCheckLatestValidModTime(_timeList.getLatestValidModTime());
      }
      tlist.compile();
      if (tlist.getValidTimes().size() == 0) {
        _errStr += "ERROR - computeReadPath\n";
        TaStr::AddStr(_errStr,
                      " Read specified forecast failed, dir: ", _readDir);
        TaStr::AddInt(_errStr, " Search margin (secs): ", _readSearchMargin);
        char timeErrStr[1024];
        sprintf(timeErrStr, "  Search time: %s", utimstr(_readSearchTime));
        _noFilesFoundOnRead = true;
        return -1;
      }
      _pathInUse = tlist.getPathList()[0];
      if (_overwriteValidTimeByGenTime) {
        _genTimeForOverwrite = tlist.getValidTimes()[0];
      }
      break;
    }

    default: {
      _errStr += "ERROR - computeReadPath\n";
      sprintf(errstr, "  Unknown search mode: %d\n", _readSearchMode);
      _errStr += errstr;
      return -1;
    }
    
  } // switch

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// _readMasterHeader()
//
// Read mdv master header. Swaps to host byte order
//
// returns 0 on success, -1 on failure.

int Mdvx::_readMasterHeader(master_header_t &mhdr,
                            TaFile &infile)
  
{

  if (infile.fseek(0, SEEK_SET)) {
    _errStr += "Cannot seek to start to read master header\n";
    return -1;
  }

  if (_is64Bit) {
    if((infile.fread(&mhdr, sizeof(master_header_t), 1)) != 1) {
      _errStr += "Cannot read master header\n";
      return -1;
    }
    master_header_from_BE(mhdr);
    if (mhdr.struct_id != MASTER_HEAD_MAGIC_COOKIE_64) {
      _errStr += "Cannot read 64-bit master header\n";
      TaStr::AddInt(_errStr, " Bad magic cookie: ", mhdr.struct_id);
      return -1;
    }
  } else {
    // 32-bit header
    master_header_32_t mhdr32;
    if((infile.fread(&mhdr32, sizeof(master_header_32_t), 1)) != 1) {
      _errStr += "Cannot read master header 32\n";
      return -1;
    }
    master_header_from_BE_32(mhdr32);
    // print 32-bit header in debug mode
    if (_debug) {
      cerr << "========== Reading 32-bit master header ==========" << endl;
      Mdvx::printMasterHeader(mhdr32, cerr);
      cerr << "==================================================" << endl;
    }
    if (mhdr32.struct_id != MASTER_HEAD_MAGIC_COOKIE_32) {
      _errStr += "Cannot read 32-bit master header\n";
      TaStr::AddInt(_errStr, " Bad magic cookie: ", mhdr32.struct_id);
      return -1;
    }
    // copy 32 to 64 bit version
    _copyMasterHeader32to64(mhdr32, mhdr);
  }
  
  return 0;

}
 
///////////////////////////////////////////////////////////////////////////
// _readFieldHeader()
//
// Read mdv field header. Swaps to host byte order
//
// returns 0 on success, -1 on failure.

int Mdvx::_readFieldHeader(const int field_num,
                           field_header_t &fhdr,
                           TaFile &infile)

{
  
  char errstr[128];

  if (_is64Bit) {
    int64_t hdr_offset =
      sizeof(master_header_t) + (field_num * sizeof(field_header_t));
    if (infile.fseek(hdr_offset, SEEK_SET)) {
      _errStr += "ERROR - Mdvx::_readFieldHeader\n";
      sprintf(errstr, "Cannot seek to field header, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    if((infile.fread(&fhdr, sizeof(field_header_t), 1)) != 1) {
      _errStr += "ERROR - Mdvx::_readFieldHeader\n";
      sprintf(errstr, "Cannot read field header, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    // swap bytes
    field_header_from_BE(fhdr);
  } else {
    // 32-bit header
    int64_t hdr_offset =
      sizeof(master_header_32_t) + (field_num * sizeof(field_header_32_t));
    if (infile.fseek(hdr_offset, SEEK_SET)) {
      _errStr += "ERROR - Mdvx::_readFieldHeader\n";
      sprintf(errstr, "Cannot seek to field header 32, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    field_header_32_t fhdr32;
    if((infile.fread(&fhdr32, sizeof(field_header_32_t), 1)) != 1) {
      _errStr += "ERROR - Mdvx::_readFieldHeader\n";
      sprintf(errstr, "Cannot read field header 32, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    // swap bytes
    field_header_from_BE_32(fhdr32);
    // copy 32 to 64 bit version
    _copyFieldHeader32to64(fhdr32, fhdr);
    // print 32-bit header in debug mode
    if (_debug) {
      cerr << "========== Reading 32-bit field header ==========" << endl;
      Mdvx::printFieldHeader(fhdr32, cerr);
      cerr << "=================================================" << endl;
    }
  }
  
  // make sure data dimension is set correctly

  if (fhdr.nz == 1) {
    fhdr.data_dimension = 2;
  } else {
    fhdr.data_dimension = 3;
  }

  // set file-based properties at the read

  fhdr.zoom_clipped = false;
  fhdr.zoom_no_overlap = false;

  // Some older files do not have the vlevel types in the
  // field header. In those cases, make the field header
  // consistent with the master header.

  if (fhdr.native_vlevel_type == 0 &&
      _mhdr.native_vlevel_type != 0) {
    fhdr.native_vlevel_type = _mhdr.native_vlevel_type;
  }
  
  if (fhdr.vlevel_type == 0 &&
      _mhdr.vlevel_type != 0) {
    fhdr.vlevel_type = _mhdr.vlevel_type;
  }

  // check the data_element_nbytes

  if (fhdr.data_element_nbytes == 0) {
    cerr << "WARNING - Mdvx::_readFieldHeader" << endl;
    cerr << "  fhdr.data_element_nbytes == 0" << endl;
    cerr << "  Setting according to encoding type" << endl;
    fhdr.data_element_nbytes =
      dataElementSize((Mdvx::encoding_type_t) fhdr.encoding_type);
    cerr << "  data_element_nbytes set to: "
	 << fhdr.data_element_nbytes << endl;
  }
  
  return 0;

}

///////////////////////////////////////////////////////////////////////////
// _readVlevelHeader()
//
// Read mdv vlevel header. Swaps to host byte order.
// Note: first_vlevel_offset is the offset to the first vlevel in the file.
//
// returns 0 on success, -1 on failure.

int Mdvx::_readVlevelHeader(const int field_num,
                            const int first_vlevel_offset,
                            const field_header_t &fhdr,
                            vlevel_header_t &vhdr,
                            TaFile &infile)

{
  
  char errstr[128];

  if (_is64Bit) {
    int64_t offset = first_vlevel_offset + (field_num * sizeof(vlevel_header_t));
    if (infile.fseek(offset, SEEK_SET)) {
      _errStr += "ERROR - Mdvx::_readVlevelHeader\n";
      sprintf(errstr, "Cannot seek to vlevel header, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    if((infile.fread(&vhdr, sizeof(vlevel_header_t), 1)) != 1) {
      _errStr += "ERROR - Mdvx::_readVlevelHeader\n";
      sprintf(errstr, "Cannot read vlevel header, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    vlevel_header_from_BE(vhdr);
  } else {
    // 32 bit headers
    int64_t offset = first_vlevel_offset + (field_num * sizeof(vlevel_header_32_t));
    if (infile.fseek(offset, SEEK_SET)) {
      _errStr += "ERROR - Mdvx::_readVlevelHeader\n";
      sprintf(errstr, "Cannot seek to vlevel header 32, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    vlevel_header_32_t vhdr32;
    if((infile.fread(&vhdr32, sizeof(vlevel_header_32_t), 1)) != 1) {
      _errStr += "ERROR - Mdvx::_readVlevelHeader\n";
      sprintf(errstr, "Cannot read vlevel header 32, field %d\n", field_num);
      _errStr += errstr;
      return -1;
    }
    vlevel_header_from_BE_32(vhdr32);
    _copyVlevelHeader32to64(vhdr32, vhdr);
    // print 32-bit header in debug mode
    if (_debug) {
      cerr << "========== Reading 32-bit field header ==========" << endl;
      Mdvx::printVlevelHeader(vhdr32, fhdr.nz, fhdr.field_name, cerr);
      cerr << "=================================================" << endl;
    }
  }
  
  return 0;

}


///////////////////////////////////////////////////////////////////////////
// _readChunkHeader()
//
// Read mdv chunk header. Swaps to host byte order
// Note: first_chunk_offset is the offset to the first chunk in the file.
//
// returns 0 on success, -1 on failure.

int Mdvx::_readChunkHeader(const int chunk_num,
                           const int first_chunk_offset,
                           chunk_header_t &chdr,
                           TaFile &infile)

{

  char errstr[128];

  if (_is64Bit) {
    int64_t offset = first_chunk_offset + chunk_num * sizeof(chunk_header_t);
    if(infile.fseek(offset, SEEK_SET)) {
      _errStr += "ERROR - Mdvx::_readChunkHeader\n";
      sprintf(errstr, "Cannot seek to chunk header, field %d\n", chunk_num);
      _errStr += errstr;
      return -1;
    }
    if((infile.fread(&chdr, sizeof(chunk_header_t), 1)) != 1) {
      _errStr += "ERROR - Mdvx::_readChunkHeader\n";
      sprintf(errstr, "Cannot read chunk header, field %d\n", chunk_num);
      _errStr += errstr;
      return -1;
    }
    chunk_header_from_BE(chdr);
  } else {
    // 32 bit headers
    int64_t offset = first_chunk_offset + chunk_num * sizeof(chunk_header_32_t);
    if(infile.fseek(offset, SEEK_SET)) {
      _errStr += "ERROR - Mdvx::_readChunkHeader\n";
      sprintf(errstr, "Cannot seek to chunk header 32, field %d\n", chunk_num);
      _errStr += errstr;
      return -1;
    }
    chunk_header_32_t chdr32;
    if((infile.fread(&chdr32, sizeof(chunk_header_32_t), 1)) != 1) {
      _errStr += "ERROR - Mdvx::_readChunkHeader\n";
      sprintf(errstr, "Cannot read chunk header 32, field %d\n", chunk_num);
      _errStr += errstr;
      return -1;
    }
    chunk_header_from_BE_32(chdr32);
    _copyChunkHeader32to64(chdr32, chdr);
    // print 32-bit header in debug mode
    if (_debug) {
      cerr << "========== Reading 32-bit chunk header ==========" << endl;
      Mdvx::printChunkHeader(chdr32, cerr);
      cerr << "=================================================" << endl;
    }
  }
  
  return 0;

}

///////////////////////////////////
// private read all headers method
//
// This method assumes that compileTimeList() and
// _computeReadPath() have been called appropriately

int Mdvx::_readAllHeadersMdv()
{

  // check for 64 bit

  if (checkIs64Bit(_pathInUse)) {
    return -1;
  }
  
  // open file - will be closed when it goes out of scope

  TaFile infile;
  if (infile.fopenUncompress(_pathInUse.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::readAllHeaders\n";
    _errStr += "File: ";
    _errStr += _pathInUse;
    _errStr += "\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // master header

  if (_readMasterHeader(_mhdrFile, infile)) {
    _errStr += "ERROR - Mdvx::readAllHeaders\n";
    _errStr += "File: ";
    _errStr += _pathInUse;
    _errStr += "\n";
    return -1;
  }

  if (_overwriteValidTimeByGenTime) {
    _mhdrFile.time_begin = _genTimeForOverwrite;
    _mhdrFile.time_centroid = _genTimeForOverwrite;
    _mhdrFile.time_end = _genTimeForOverwrite;
  }

  // field headers

  _fhdrsFile.clear();
  for (int i = 0; i < _mhdrFile.n_fields; i++) {
    field_header_t fhdr;
    if (_readFieldHeader(i, fhdr, infile)) {
      _errStr += "ERROR - Mdvx::readAllHeaders\n";
      _errStr += "File: ";
      _errStr += _pathInUse;
      _errStr += "\n";
      return -1;
    }

    // check if vlevel type set

    if (fhdr.native_vlevel_type == 0) {
      fhdr.native_vlevel_type = _mhdrFile.native_vlevel_type;
    }
    if (fhdr.vlevel_type == 0) {
      fhdr.vlevel_type = _mhdrFile.vlevel_type;
    }

    _fhdrsFile.push_back(fhdr);
  }

  // vlevel headers

  _vhdrsFile.clear();
  for (int i = 0; i < _mhdrFile.n_fields; i++) {

    field_header_t &fhdr = _fhdrsFile[i];
    vlevel_header_t vhdr;
    MEM_zero(vhdr);

    if (_mhdrFile.vlevel_included) {
      
      // read vlevel header from file
      
      if (_readVlevelHeader(i, _mhdrFile.vlevel_hdr_offset,
                            fhdr, vhdr, infile)) {
        _errStr += "ERROR - Mdvx::readAllHeaders\n";
        _errStr += "File: ";
        _errStr += _pathInUse;
        _errStr += "\n";
        return -1;
      }

      // check if dz is constant

      if (!fhdr.dz_constant) {
        if (dzIsConstant(fhdr, vhdr)) {
          fhdr.dz_constant = true;
          if (fhdr.nz < 2) {
            fhdr.grid_dz = 1.0;
          } else {
            fhdr.grid_dz = vhdr.level[1] - vhdr.level[0];
          }
        }
      }

    } else {

      // no vlevel headers in file, so compute vlevels and fill header

      fhdr.dz_constant = true;
      for (int iz = 0; iz < fhdr.nz; iz++) {
        vhdr.level[iz] = fhdr.grid_minz + iz * fhdr.grid_dz;
        vhdr.type[iz] = _mhdrFile.vlevel_type;
      }

    }
    
    _vhdrsFile.push_back(vhdr);

  } // i

  // we have forced vlevel headers to be included

  _mhdrFile.vlevel_included = true;

  // chunk headers

  _chdrsFile.clear();
  for (int i = 0; i < _mhdrFile.n_chunks; i++) {

    chunk_header_t chdr;
    if (_readChunkHeader(i, _mhdrFile.chunk_hdr_offset,
                         chdr, infile)) {
      _errStr += "ERROR - Mdvx::readAllHeaders\n";
      _errStr += "File: ";
      _errStr += _pathInUse;
      _errStr += "\n";
      return -1;
    }
    _chdrsFile.push_back(chdr);

    // if data set info chunk, read it in and set data set info

    if (chdr.chunk_id == CHUNK_DATA_SET_INFO) {
      MdvxChunk chunk(chdr, NULL);
      if (chunk._read_data(infile)) {
        _errStr += "ERROR - Mdvx::readAllHeaders\n";
        _errStr += "File: ";
        _errStr += _pathInUse;
        _errStr += "\n";
        return -1;
      }
      const char *data = (const char *) chunk.getData();
      _dataSetInfo = data;
    }

  }

  // close file

  infile.fclose();

  return 0;

}

////////////////////////////////////////////////////
// private read volume method

int Mdvx::_readVolumeMdv(bool fill_missing,
                         bool do_decimate,
                         bool do_final_convert,
                         bool is_vsection,
                         double vsection_min_lon,
                         double vsection_max_lon)
  
{

  // clear pertinent components

  clearErrStr();
  clearMasterHeader();
  clearFields();
  clearChunks();

  // check file extension to determine which type of file it is

  if (isXmlFile(_pathInUse)) {
    
    // this is an xml file, so call the XML read method
    
    if (_readVolumeXml(fill_missing,
                       do_decimate,
                       do_final_convert,
                       is_vsection,
                       vsection_min_lon,
                       vsection_max_lon)) {
      _errStr += "ERROR - Mdvx::_read_volume\n";
      _errStr += "  Reading XML format file\n";
      _errStr += "  File: ";
      _errStr += _pathInUse;
      _errStr += "\n";
      return -1;
    }
    return 0;

  }

  // read in all the headers

  if (_readAllHeadersMdv()) {
    _errStr += "ERROR - Mdvx::_readVolumeMdv\n";
    return -1;
  }
  _mhdr = _mhdrFile;

  // open file

  TaFile infile;
  
  if (infile.fopenUncompress(_pathInUse.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeMdv\n";
    _errStr += "File: ";
    _errStr += _pathInUse;
    _errStr += "\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // check requested field numbers

  if (_readFieldNums.size() > 0) {
    for (size_t i = 0; i < _readFieldNums.size(); i++) {
      if (_readFieldNums[i] > _mhdr.n_fields - 1) {
        _errStr += "ERROR - Mdvx::_readVolumeMdv\n";
        _errStr += "  Requested field number out of range\n";
        TaStr::AddInt(_errStr, "  Requested field number: ",
		      _readFieldNums[i]);
        TaStr::AddInt(_errStr, "  Max field number: ",
		      _mhdr.n_fields - 1);
        _errStr += "File: ";
        _errStr += _pathInUse;
        _errStr += "\n";
        return -1;
      }
    }
  }

  // if field names are specified, look up field numbers, and load
  // up _readFieldNames.

  if (_readFieldNames.size() > 0) {
    char *mdvReadLongOnly = getenv("MDV_READ_LONG_FIELD_NAMES_ONLY");
    bool readLongOnly = (mdvReadLongOnly != NULL);
    bool error = false;
    _readFieldNums.clear();
    for (size_t i = 0; i < _readFieldNames.size(); i++) {
      bool fieldFound = false;
      for (int j = 0; j < _mhdr.n_fields; j++) {
	if (readLongOnly) {
	  if (!strcmp((char *) _readFieldNames[i].c_str(), _fhdrsFile[j].field_name_long)) {
	    _readFieldNums.push_back(j);
	    fieldFound = true;
	    break;
	  }
	} else {
	  // short or long field name
	  if (!strcmp((char *) _readFieldNames[i].c_str(), _fhdrsFile[j].field_name) || 
	      !strcmp((char *) _readFieldNames[i].c_str(), _fhdrsFile[j].field_name_long)) {
	    _readFieldNums.push_back(j);
	    fieldFound = true;
	    break;
	  }
        }
      } // j
      if (!fieldFound) {
        _errStr += "ERROR - Mdvx::_readVolumeMdv\n";
        _errStr += "  Field: ";
        _errStr += _readFieldNames[i];
        _errStr += " not found in file: ";
        _errStr += _pathInUse;
        _errStr += "\n";
        error = true;
      }
    } // i
    if (error) {
      return -1;
    }

  }

  // if no field numbers specified, use all fields

  if (_readFieldNums.size() == 0) {
    for (int i = 0; i < _mhdr.n_fields; i++) {
      _readFieldNums.push_back(i);
    }
  }

  // check requested chunk numbers

  if (_readChunkNums.size() > 0) {
    for (size_t i = 0; i < _readChunkNums.size(); i++) {
      if (_readChunkNums[i] > _mhdr.n_chunks - 1) {
        _errStr += "ERROR - Mdvx::_readVolumeMdv\n";
        _errStr += "  Requested chunk number out of range\n";
        TaStr::AddInt(_errStr, "  Requested chunk number: ",
		      _readChunkNums[i]);
        TaStr::AddInt(_errStr, "  Max chunk number: ", _mhdr.n_chunks - 1);
        _errStr += "File: ";
        _errStr += _pathInUse;
        _errStr += "\n";
        return -1;
      }
    }
  }

  if (_readChunkNums.size() == 0) {

    // if no chunk numbers specified, use all chunks

    for (int i = 0; i < _mhdr.n_chunks; i++) {
      _readChunkNums.push_back(i);
    }

  } else if (_readChunkNums.size() > 0) {

    // if a negative chunk number is specified, read no chunks
    // Calling setReadNoChunks() sets a chunk number of -1
    
    for (size_t i = 0; i < _readChunkNums.size(); i++) {
      if (_readChunkNums[i] < 0) {
	clearReadChunks();
	break;
      }
    } // i

  }

  // create the fields, read in the data volume for each field

  MdvxRemapLut remapLut;

  for (size_t i = 0; i < _readFieldNums.size(); i++) {
    
    MdvxField *field = new MdvxField(_fhdrsFile[_readFieldNums[i]],
                                     _vhdrsFile[_readFieldNums[i]], NULL);
    if (field == NULL) {
      _errStr += "ERROR - Mdvx::_readVolumeMdv.\n";
      char errstr[128];
      sprintf(errstr, " Allocating field mem");
      _errStr += errstr;
      return -1;
    }
    
    if (field->_read_volume(infile, *this, fill_missing,
			    do_decimate, do_final_convert, remapLut,
			    is_vsection, vsection_min_lon, vsection_max_lon)) {
      _errStr += "ERROR - Mdvx::_readVolumeMdv.\n";
      char errstr[128];
      sprintf(errstr, "  Reading field %d\n", (int) i);
      _errStr += errstr;
      _errStr += field->getErrStr();
      delete field;
      return -1;
    }

    _fields.push_back(field);

    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Mdvx::_readVolumeMdv");
    }

  }

  // create chunks, read in data for each chunk

  for (size_t i = 0; i < _readChunkNums.size(); i++) {
    
    MdvxChunk *chunk = new MdvxChunk(_chdrsFile[_readChunkNums[i]], NULL);
    if (chunk == NULL){
      _errStr += "ERROR - Mdvx::_readVolumeMdv.\n";
      char errstr[128];
      sprintf(errstr, " Allocating chunk mem");
      _errStr += errstr;
      return -1;
    }
   
    if (chunk->_read_data(infile)) {
      _errStr += "ERROR - Mdvx::_readVolumeMdv.\n";
      char errstr[128];
      sprintf(errstr, "  Reading chunk %d\n", (int) i);
      _errStr += errstr;
      _errStr += chunk->getErrStr();
      delete chunk;
      return -1;
    }

    _chunks.push_back(chunk);

  }

  infile.fclose();

  // set data set info from chunks as appropriate

  _setDataSetInfoFromChunks();

  // update the master header to match any changes in the fields

  updateMasterHeader();

  return 0;

}

//////////////////////////////////////////////////////////
// Private read vertical section method
// Returns 0 on success, -1 on failure

int Mdvx::_readVsectionMdv()
  
{

  // special case - read RHI
  
  if (_readVsectAsRhi) {
    return _readRhi(_readRhiRespectUserDist);
  }
  
  // compute min_lon and max_lon
  
  double min_lon = 360.0;
  double max_lon = -360.0;
  for (size_t i = 0; i < _vsectWayPts.size(); i++) {
    min_lon = MIN(min_lon,  _vsectWayPts[i].lon);
    max_lon = MAX(max_lon,  _vsectWayPts[i].lon);
  }

  // read in the volume - do not fill missing or decimate
  
  if (_readVolumeMdv(false, false, false, true, min_lon, max_lon)) {
    _errStr += "ERROR - _readVsectionMdv\n";
    return -1;
  }

  // Determine the the number of samples if required
  // If n_samples is not -1, then it has been passed in to this routine.
  // If it is -1, then we must compute it.
  // If _readMaxVsectSamples is negative, we take its absolute value
  // as n_samples.

  int n_samples = _computeNVsectSamples();
  
  // convert each field to a vertical section
  
  MdvxVsectLut lut;
  for (size_t i = 0; i < _fields.size(); i++) {
    if (_fields[i]->convert2Vsection(_mhdr, _vsectWayPts,
                                     n_samples, lut,
				     _readFillMissing,
				     !_vsectDisableInterp,
				     _readSpecifyVlevelType,
				     _readVlevelType, false)) {
      _errStr += "ERROR - _readVsectionMdv\n";
      return -1;
    }
  }

  // convert to requested output type

  for (size_t i = 0; i < _fields.size(); i++) {
    if (_fields[i]->convertType(_readEncodingType,
				_readCompressionType,
				_readScalingType,
				_readScale,
				_readBias)) {
      _errStr += "ERROR - _readVsectionMdv\n";
      return -1;
    }
  }

  // set the members appropriately

  _mhdr.grid_orientation = ORIENT_SN_WE;
  updateMasterHeader();

  _vsectWayPts = lut.getWayPts();
  _vsectSamplePts = lut.getSamplePts();
  _vsectSegments = lut.getSegments();
  _vsectDxKm = lut.getDxKm();
  _vsectTotalLength = lut.getTotalLength();

  // add vert-section-specific chunks
  
  _addVsectChunks32();

  return 0;

}

//////////////////////////////////////////////////////////
// Read RHI-type vertical section
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int Mdvx::_readRhi(bool respectUserDistance /* = false */)
  
{

  if (_debug) {
    cerr << "start - Mdvx::_readRhi" << endl;
  }

  // read in the volume - do not fill missing or decimate
  
  if (_readVolumeMdv(false, false, false)) {
    _errStr += "ERROR - _readRhi\n";
    return -1;
  }

  // check we have RHI data

  if (_fields.size() == 0) {
    return -1;
  } else {
    if (_fields[0]->getFieldHeader().proj_type != PROJ_RHI_RADAR) {
      return -1;
    }
  }

  if (_loadClosestRhi(respectUserDistance)) {
    return -1;
  }

  // convert to requested output type

  for (size_t i = 0; i < _fields.size(); i++) {
    if (_fields[i]->convertType(_readEncodingType,
				_readCompressionType,
				_readScalingType,
				_readScale,
				_readBias)) {
      _errStr += "ERROR - _readRhi\n";
      return -1;
    }
  }
  
  updateMasterHeader();
  return 0;

}

//////////////////////////////////////////////////////////
// Load RHI closest to requested azimuth
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int Mdvx::_loadClosestRhi(bool respectUserDistance)
  
{

  if (_debug) {
    cerr << "start - Mdvx::_loadClosestRhi" << endl;
  }

  // For this to be a measured RHI, both the start and end point
  // must be on much the same azimuth relative to the radar.
  // See if this is the case.

  vsect_waypt_t radarPos;
  radarPos.lat = _mhdr.sensor_lat;
  radarPos.lon = _mhdr.sensor_lon;

  double d1, a1, d2, a2;
  PJGLatLon2RTheta(radarPos.lat, radarPos.lon,
		   _vsectWayPts[0].lat, _vsectWayPts[0].lon,
		   &d1, &a1);

  PJGLatLon2RTheta(radarPos.lat, radarPos.lon,
		   _vsectWayPts[1].lat, _vsectWayPts[1].lon,
		   &d2, &a2);

  // Make sure the points are in order, closest to the sensor first

  if (d2 == d1) {
    return -1;
  }

  if (d2 < d1){
    double tmp;
    tmp = d1; d1 = d2; d2 = tmp;
    tmp = a1; a1 = a2; a2 = tmp;
    tmp = _vsectWayPts[0].lat; _vsectWayPts[0].lat = _vsectWayPts[1].lat;
    _vsectWayPts[1].lat = tmp;
    tmp = _vsectWayPts[0].lon; _vsectWayPts[0].lon = _vsectWayPts[1].lon;
    _vsectWayPts[1].lon = tmp;
  }

  // Get the azimuth between the points and the azimuth from the
  // radar to the end point and compare the two to se if they
  // are within tolerance.

  double azRadarToEndPoint, azPointToPoint, dummy;

  azRadarToEndPoint = a2;

  PJGLatLon2RTheta(_vsectWayPts[0].lat, _vsectWayPts[0].lon,
		   _vsectWayPts[1].lat, _vsectWayPts[1].lon,
		   &dummy, &azPointToPoint);

  double azDiff = fabs(azRadarToEndPoint - azPointToPoint);
  if (azDiff > 180.0) {
    azDiff = fabs(azDiff - 360.0);
  }

  if (azDiff > _readRhiMaxAzError) {
    return -1;
  }

  // If we got here, then the two entered points are at least on a direct
  // line from the radar. See if at least 80% of the line lies between
  // the start and the end of our measured RHI. If it does, assume
  // we can return a measured RHI.

  const field_header_t &fhdr = _fields[0]->getFieldHeader();
  double maxRange = fhdr.grid_minx + fhdr.nx * fhdr.grid_dx;
  double minRange = fhdr.grid_minx;

  // Return if there is no overlap at all
  if (d1 > maxRange) {
    return -1;
  }
  if (d2 < minRange) {
    return -1;
  }

  double outsideDist = 0;

  if (d1 < minRange) outsideDist += (minRange - d1);
  if (d2 > maxRange) outsideDist += (d2 - maxRange);

  double percentOutside = 100.0*outsideDist/(d2-d1);
  
  if (percentOutside > 80) {
    return -1;
  }

  // compute the azimuth of the mid-pt of the first 2 way-points
  
  vsect_waypt_t midPt;
  double dist, azimuth;
  if (_vsectWayPts.size() < 2) {
    midPt = _vsectWayPts[0];
  } else {
    PJGLatLon2RTheta(_vsectWayPts[0].lat, _vsectWayPts[0].lon,
                     _vsectWayPts[1].lat, _vsectWayPts[1].lon,
                     &dist, &azimuth);
    PJGLatLonPlusRTheta(_vsectWayPts[0].lat, _vsectWayPts[0].lon,
			dist / 2.0, azimuth,
			&midPt.lat, &midPt.lon);
  }

  // compute the azimuth of this mid point from the radar

  PJGLatLon2RTheta(_mhdr.sensor_lat, _mhdr.sensor_lon,
		   midPt.lat, midPt.lon,
		   &dist, &azimuth);
  if (azimuth < 0) {
    azimuth += 360.0;
  }
  
  // find the RHI closest to this azimuth

  const vlevel_header_t &vhdr = _fields[0]->getVlevelHeader();

  double minDiff = 360.0;
  int rhiIndex = 0;
  double rhiAz = 0.0;
  for (int ii = 0; ii < fhdr.nz; ii++) {
    double diff = fabs(vhdr.level[ii] - azimuth);
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (minDiff > diff) {
      minDiff = diff;
      rhiIndex = ii;
      rhiAz = vhdr.level[ii];
    }
  }

  // check the min diff

  if (minDiff > _readRhiMaxAzError) {
    return -1;
  }

  // reset the waypoints, to go from the radar to the end of the RHI
  
  vsect_waypt_t endPos;
  if (respectUserDistance){

    double userDist, userAz;
    PJGLatLon2RTheta(radarPos.lat, radarPos.lon,
		     _vsectWayPts[1].lat, _vsectWayPts[1].lon,
		     &userDist, &userAz);
    
    // It's arguable that we should be doing something more like this,
    // but we'll leave that for later, since it requires trimming
    // out the RHI data properly - Niles and Mike.
    //
    //    PJGLatLon2RTheta(_vsectWayPts[0].lat, _vsectWayPts[0].lon,
    //                     _vsectWayPts[1].lat, _vsectWayPts[1].lon,
    //                     &userDist, &userAz);
    
    if (userDist > maxRange) userDist = maxRange;
    
    PJGLatLonPlusRTheta(radarPos.lat, radarPos.lon,
			userDist, rhiAz,
			&endPos.lat, &endPos.lon);
  } else {
    PJGLatLonPlusRTheta(radarPos.lat, radarPos.lon,
			maxRange, rhiAz,
			&endPos.lat, &endPos.lon);
  }
  _vsectWayPts.clear();
  _vsectWayPts.push_back(radarPos);
  _vsectWayPts.push_back(endPos);

  // set the number of samples to the number of gates

  MdvxVsectLut lut;

  // convert each field to vsection
  
  for (size_t i = 0; i < _fields.size(); i++) {
    if (_readRhiAsPolar) {
      if (_fields[i]->convert2SingleRhi(_mhdr, rhiIndex,
					_vsectWayPts, lut, false)) {
	_errStr += "ERROR - _loadClosestRhi\n";
	return -1;
      }
    } else {
      if (_fields[i]->convertRhi2Vsect(_mhdr, rhiIndex,
				       _vsectWayPts, lut, false)) {
	_errStr += "ERROR - _loadClosestRhi\n";
	return -1;
      }
    }
  }
  
  // set the members appropriately
  
  if (_readRhiAsPolar) {
    _mhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
  } else {
    _mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  }

  _mhdr.grid_orientation = ORIENT_SN_WE;
  _mhdr.data_ordering = ORDER_XZY;
  updateMasterHeader();

  _vsectWayPts = lut.getWayPts();
  _vsectSamplePts = lut.getSamplePts();
  _vsectSegments = lut.getSegments();
  _vsectDxKm = lut.getDxKm();
  _vsectTotalLength = lut.getTotalLength();

  return 0;

}

/////////////////////////////////////////////////////////////
// Check if the path is an XML-based file
// Returns true or false

bool Mdvx::isXmlFile(const string &path)
{

  Path cpath(path);
  const string &fileName = cpath.getFile();
  if (fileName.find(".xml") != string::npos) {  // UF radar
    return true;
  }
  return false;
}

/////////////////////////////////////////////////////////////
// Check if the path is a CF-compliant NetCDF file
// Returns true or false

bool Mdvx::isNcfFile(const string &path)
{

  // first check for RADX file types

  if (isRadxFile(path)) {
    return false;
  }

  Path cpath(path);
  const string &fileName = cpath.getFile();
  if (fileName.find("ncf") != string::npos ||
      fileName.find(".nc") != string::npos) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////////
// Check if the path is a Radx (radial data data) file
// Returns true or false

bool Mdvx::isRadxFile(const string &path)
{
  
  Path cpath(path);
  const string &fileName = cpath.getFile();
  if (fileName.find("cfrad") != string::npos || // CF radial volume
      fileName.find("odim") != string::npos  || // ODIM HDF5
      fileName.find(".uf") != string::npos  || // UF radar sweep
      fileName.find("swp.") != string::npos  || // DORADE radar sweep
      fileName.find("ncswp") != string::npos || // FORAY radar sweep
      fileName.find(".msg1") != string::npos || // NEXRAD level2 data
      fileName.find(".msg31") != string::npos || // NEXRAD level2 data
      fileName.find(".ar2") != string::npos || // NEXRAD level2 data
      fileName.find(".nex2") != string::npos || // NEXRAD level2 data
      fileName.find(".RAW") != string::npos || // SIGMET RAW volume
      fileName.find(".rapic") != string::npos) { // BOM rapic
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////
// for each field, constrain the data in the vertical,
// based on read limits set the mdvx object

void Mdvx::constrainVertical()

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->constrainVertical(*this);
  }

}

////////////////////////////////////////////////////////////////
// for each field, constrain the data in the horizontal
// based on read limits set the mdvx object

void Mdvx::constrainHorizontal()

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->constrainHorizontal(*this);
  }

}



