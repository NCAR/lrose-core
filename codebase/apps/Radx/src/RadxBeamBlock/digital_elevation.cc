// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Ancilla Radar Quality Control System (ancilla)
// ** Copyright BOM (C) 2013
// ** Bureau of Meteorology, Commonwealth of Australia, 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from the BOM.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of the BOM nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%

#include "digital_elevation.h"
#include "array_utils.h"
#include "trace.h"

#include <arpa/inet.h> // just for ntohs()
#include <fstream>
#include <memory>
#include <iostream>
#include <toolsa/file_io.h>

using namespace std;

using namespace rainfields::ancilla;

digital_elevation::digital_elevation(const Params &params) :
        _params(params)
{
}

digital_elevation::~digital_elevation()
{
  
}

digital_elevation::model_error::model_error(std::string description)
        : runtime_error(description)
        , description_(std::move(description))
        , location_(nan<angle>(), nan<angle>())
{
  
}

digital_elevation::model_error::model_error(std::string description, latlon location)
        : runtime_error(msg{} << description << std::endl << "  location: " << location)
        , description_(std::move(description))
        , location_(location)
{
  
}

digital_elevation_srtm3::digital_elevation_srtm3(const Params &params,
                                                 std::string path, size_t cache_size)
        : digital_elevation(params)
        , path_(std::move(path))
        , cache_size_(cache_size)
        , wgs84_(spheroid::standard::wgs84)
{
  if (path_.empty())
    path_.assign("./");
  else if (path_.back() != '/')
    path_.append("/");
}

auto digital_elevation_srtm3::reference_spheroid() -> const spheroid&
{
  return wgs84_;
}

auto digital_elevation_srtm3::lookup(const latlon& loc) -> real
{

  real lat = std::abs(loc.lat.degrees());
  real lon = std::abs(loc.lon.degrees());
  int ilat = lat, ilon = lon;

  // determine the tile to use
  int tilelat = loc.lat.radians() < 0.0 ? -ilat - 1 : ilat;
  int tilelon = loc.lon.radians() < 0.0 ? -ilon - 1 : ilon;
  
  const srtm_tile &tile = get_tile(tilelat, tilelon);
  if (tile.nlat == 0 || tile.nlon == 0) {
    return 0.0; // sea location
  }

  // determine the indices within the tile
  int x, y;
  if (tilelat >= 0)
  {
    y = std::lround((ilat + 1 - lat) / tile.dlat);
  }
  else
  {
    y = std::lround((lat - ilat) / tile.dlat);
  }
  if (tilelon >= 0)
  {
    x = std::lround((lon - ilon) / tile.dlon);
  }
  else
  {
    x = std::lround((ilon + 1 - lon) / tile.dlon);
  }
  
  return tile.data[y][x];
     
}

auto digital_elevation_srtm3::lookup(latlonalt* values, size_t count) -> void
{
  throw model_error{"unimplemented feature: model multi lookup"};
}

auto digital_elevation_srtm3::testFTG(void) ->void
{
  test(39, -104, 40, -104, 39, -105, 39, -104);
}

auto digital_elevation_srtm3::testBOM(void) ->void
{
  test(-37, 144, -38, 144, -38, 145, -38, 146);
}

auto digital_elevation_srtm3::test(int lat0, int lon0, int lat1, int lon1, 
				   int lat2, int lon2, int lat3, int lon3) ->void
{
  array2<real> t0 = get_tile(lat0, lon0).data;
  array2<real> t1 = get_tile(lat1, lon1).data;
  bool match01_00 = true;
  bool match01_01 = true;
  bool match01_10 = true;
  bool match01_11 = true;
  for (size_t i=0; i<1201; ++i)
  {
    if (t0[0][i] != t1[1200][i])
    {
      match01_01 = false;
    }
    if (t0[0][i] != t1[0][i])
    {
      match01_00 = false;
    }
    if (t0[1200][i] != t1[0][i])
    {
      match01_10 = false;
    }
    if (t0[1200][i] != t1[1200][i])
    {
      match01_11 = false;
    }
  }
  if (match01_01)
  {
    printf("[%d,%d] row[0] matches [%d,%d] row[1200]\n",
	   lat0, lon0, lat1, lon1);
  }
  if (match01_10)
  {
    printf("[%d,%d] row[1200] matches [%d,%d] row[0]\n",
	   lat0, lon0, lat1, lon1);
  }
  if (match01_00)
  {
    printf("[%d,%d] row[0] matches [%d,%d] row[0]\n",
	   lat0, lon0, lat1, lon1);
  }
  if (match01_11)
  {
    printf("[%d,%d] row[1200] matches [%d,%d] row[1200]\n",
	   lat0, lon0, lat1, lon1);
  }


  t0 = get_tile(lat2, lon2).data;
  t1 = get_tile(lat3, lon3).data;
  match01_00 = true;
  match01_01 = true;
  match01_10 = true;
  match01_11 = true;
  for (size_t i=0; i<1201; ++i)
  {
    if (t0[i][0] != t1[i][1200])
    {
      match01_01 = false;
    }
    if (t0[i][0] != t1[i][0])
    {
      match01_00 = false;
    }
    if (t0[i][1200] != t1[i][0])
    {
      match01_10 = false;
    }
    if (t0[i][1200] != t1[i][1200])
    {
      match01_11 = false;
    }
  }
  if (match01_01)
  {
    printf("[%d,%d] col[0] matches [%d,%d] col[1200]\n",
	   lat2, lon2, lat3, lon3);
  }
  if (match01_10)
  {
    printf("[%d,%d] col[1200] matches [%d,%d] col[0]\n",
	   lat2, lon2, lat3, lon3);
  }
  if (match01_00)
  {
    printf("[%d,%d] col[0] matches [%d,%d] col[0]\n",
	   lat2, lon2, lat3, lon3);
  }
  if (match01_11)
  {
    printf("[%d,%d] col[1200] matches [%d,%d] col[1200]\n",
	   lat2, lon2, lat3, lon3);
  }
}

auto digital_elevation_srtm3::get_tile(int lat, int lon) -> const srtm_tile&
{

  // do we have this tile cached?
  for (auto i = tiles_.begin(); i != tiles_.end(); ++i)
  {
    if (i->lat == lat && i->lon == lon)
    {
      // promote tile to front of list
      if (i != tiles_.begin())
        tiles_.splice(tiles_.begin(), tiles_, i);
      return *i;
    }
  }

  // allocate or reuse a tile
  if (tiles_.size() < cache_size_)
    tiles_.emplace_front();
  else
    tiles_.splice(tiles_.begin(), tiles_, --tiles_.end());

  auto& tile = tiles_.front();

  // compute tile file name and path
  char file_name[128];
  snprintf(
          file_name
          , sizeof(file_name)
          , "%c%02d%c%03d.hgt"
          , lat < 0 ? 'S' : 'N'
          , std::abs(lat)
          , lon < 0 ? 'W' : 'E'
          , std::abs(lon));
  string file_path = path_ + file_name;

  // check tile dimensions

  long nBytes = ta_stat_get_len(file_path.c_str());
  if (nBytes < 1) {
    // file does not exist, set tile to have no dimension
    if (_params.debug) {
      cerr << "Missing tile path: " << file_path << endl;
      cerr << "  Probably a sea tile" << endl;
    }
    tile.lat = lat;
    tile.lon = lon;
    tile.nlat = 0;
    tile.nlon = 0;
    tile.dlat = 0;
    tile.dlon = 0;
    return tile;
  }

  long nCells = nBytes / 2; // data is 2 byte ints
  int tileDim = sqrt(nCells);
  if (_params.debug) {
    cerr << "Tile path: " << file_path << endl;
    cerr << " lat, lon: " << lat << ", " << lon << endl;
    cerr << "      dim: " << tileDim << endl;
  }

  // set tile metadata
  // depends on tile dimension
  // 3-sec data has tiles 1201 x 1201
  // 1-sec data has tiles 3601 x 3601

  tile.lat = lat;
  tile.lon = lon;
  tile.nlat = tileDim;
  tile.nlon = tileDim;
  tile.dlat = 1.0 / (tileDim - 1.0);
  tile.dlon = 1.0 / (tileDim - 1.0);

  // allocate space for tile data

  tile.data.resize(tile.nlat, tile.nlon);

  // fill tile data

  std::ifstream file((path_ + file_name).c_str(),
                     std::ifstream::in | std::ifstream::binary);
  if (file) {
    
    std::unique_ptr<std::int16_t[]>
      buf{new std::int16_t[tile.nlat * tile.nlon]};
    
    file.read(reinterpret_cast<char*>(buf.get()),
              sizeof(int16_t) * tile.nlat * tile.nlon);
    if (!file)
      throw model_error{
          msg{} << "srtm3: tile read failed: " << path_ << file_name
        , {lat * 1_deg, lon * 1_deg}};

    // convert from big-endian into host order
    for (int i = 0; i < tile.nlat * tile.nlon; ++i) {
      buf[i] = ntohs(buf[i]);
    }

    // convert to real replacing void values with NaN
    for (int i = 0; i < tile.nlat * tile.nlon; ++i) {
      tile.data.data()[i] = buf[i] == void_value ? nan() : buf[i];
    }

  }

  return tile;

}

digital_elevation_esri::digital_elevation_esri(
        const Params &params
        , const std::string& path
        , latlon sw
        , latlon ne
        , spheroid::standard reference_spheroid)
        : digital_elevation_esri(params, path, sw, ne, spheroid{reference_spheroid})
{

}

digital_elevation_esri::digital_elevation_esri(
        const Params &params
        , const std::string& path
        , latlon sw
        , latlon ne
        , spheroid reference_spheroid)
        : digital_elevation(params)
        , spheroid_(std::move(reference_spheroid))
{
  std::ifstream file(path);
  if (!file)
    throw model_error{msg{} << "esri: failed to open dataset: " << path};

  std::string label;
  int cols, rows;
  real llx, lly;
  real nodata;
  
  file >> label >> cols;
  if (!file || label != "ncols")
    throw model_error{msg{} << "esri: dataset expected ncols: " << path};

  file >> label >> rows;
  if (!file || label != "nrows")
    throw model_error{msg{} << "esri: dataset expected nrows: " << path};

  file >> label >> llx;
  if (!file || label != "xllcorner")
    throw model_error{msg{} << "esri: dataset expected xllcorner: " << path};

  file >> label >> lly;
  if (!file || label != "yllcorner")
    throw model_error{msg{} << "esri: dataset expected yllcorner: " << path};

  file >> label >> delta_deg_;
  if (!file || label != "cellsize")
    throw model_error{msg{} << "esri: dataset expected cellsize: " << path};

  file >> label >> nodata;
  if (!file || label != "NODATA_value")
    throw model_error{msg{} << "esri: dataset expected NODATA_value: " << path};

  // determine the subset of points to load
  int miny = rows - (ne.lat.degrees() - lly) / delta_deg_;
  int maxy = miny + (ne.lat - sw.lat).degrees() / delta_deg_;
  int minx = (sw.lon.degrees() - llx) / delta_deg_;
  int maxx = minx + (ne.lon - sw.lon).degrees() / delta_deg_;

  // clamp subset to the actual data
  bool warn = false;
  if (miny < 0)
  {
    miny = 0;
    warn = true;
  }
  if (maxy >= rows)
  {
    maxy = rows;
    warn = true;
  }
  if (minx < 0)
  {
    minx = 0;
    warn = true;
  }
  if (maxx >= cols)
  {
    maxx = cols;
    warn = true;
  }
  if (warn)
    trace::log() << "esri: requested subset " << sw << " - " << ne << " exceeds dataset bounds";

  // allocate our data
  data_ = array2<real>(maxy - miny, maxx - minx);

  // record central position of 0,0 point of our array
  nw_.lat.set_degrees(lly + (rows - miny - 0.5_r) * delta_deg_);
  nw_.lon.set_degrees(llx + (minx + 0.5_r) * delta_deg_);

  // skip to the first row we want to load
  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (int i = 0; i < miny; ++i)
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // read each row in the interesting region
  for (size_t y = 0; y < data_.rows(); ++y)
  {
    // skip to the first column we want to load
    for (int i = 0; i < minx; ++i)
      file.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    // read the data
    real* raw = data_[y];
    for (size_t x = 0; x < data_.cols(); ++x)
    {
      file >> raw[x];
      if (std::abs(raw[x] - nodata) < 0.0001_r)
        raw[x] = nan();
    }

    // skip until the start of the next row
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
}

auto digital_elevation_esri::reference_spheroid() -> const spheroid&
{
  return spheroid_;
}

auto digital_elevation_esri::lookup(const latlon& loc) -> real
{
  // determine the array coordinates
  auto y = std::lround((nw_.lat - loc.lat).degrees() / delta_deg_);
  auto x = std::lround((loc.lon - nw_.lon).degrees() / delta_deg_);

  if (   y < 0 || y >= (int) data_.rows()
      || x < 0 || x >= (int) data_.cols())
    return nan();

  return data_[y][x];
}

auto digital_elevation_esri::lookup(latlonalt* values, size_t count) -> void
{
  throw model_error{"unimplemented feature: model multi lookup"};
}


