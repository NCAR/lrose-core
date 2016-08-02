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

#ifndef ANCILLA_MODELS_DIGITAL_ELEVATION_H
#define ANCILLA_MODELS_DIGITAL_ELEVATION_H

#include "spheroid.h"
#include "angle.h"
#include "array.h"
#include "latlon.h"

#include <list>

namespace rainfields {
namespace ancilla {
  class digital_elevation
  {
  public:
    class model_error;

  public:
    virtual ~digital_elevation();

    /// Get the reference spheroid used for by this elevation model
    virtual auto reference_spheroid() -> const spheroid& = 0;

    /// Lookup the elevation above the reference spheroid at a given location
    virtual auto lookup(const latlon& loc) -> real = 0;

    /// Lookup the altitude at multiple locations
    virtual auto lookup(latlonalt* values, size_t count) -> void = 0;

    virtual auto testBOM(void) -> void = 0;
    virtual auto testFTG(void) -> void = 0;
  };

  class digital_elevation::model_error : public std::runtime_error
  {
  public:
    model_error(std::string description);
    model_error(std::string description, latlon location);

    auto description() const -> const std::string&  { return description_; }
    auto location() const -> const latlon&          { return location_; }

  private:
    std::string description_;
    latlon      location_;
  };

  /// Shuttle Radar Topography Mission DEM (3 arc-second version)
  /**
   * Tiles freely available for downlow from USGS at 
   * http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/
   */
  class digital_elevation_srtm3 : public digital_elevation
  {
  public:
    digital_elevation_srtm3(std::string path, size_t cache_size = 16);

    auto reference_spheroid() -> const spheroid&;
    auto lookup(const latlon& loc) -> real;
    auto lookup(latlonalt* values, size_t count) -> void;

    auto testBOM(void) -> void;
    auto testFTG(void) -> void;
    auto test(int lat0, int lon0, int lat1, int lon1, 
	      int lat2, int lon2, int lat3, int lon3) -> void;

  private:
    static constexpr size_t tile_samples_x = 1201;
    static constexpr size_t tile_samples_y = 1201;
    static constexpr real   sample_width = 3.0/3600.0; // 3 seconds
    static constexpr int    void_value = -32768;

  private:
    struct tile
    {
      tile();
      int lat;
      int lon;
      array2<real> data;
    };

  private:
    auto get_tile(int lat, int lon) -> const array2<real>&;

  private:
    std::string     path_;
    size_t          cache_size_;
    spheroid        wgs84_;
    std::list<tile> tiles_;
  };

  /// ESRI ASCII grid based DEM
  class digital_elevation_esri : public digital_elevation
  {
  public:
    digital_elevation_esri(
          const std::string& path
        , latlon sw
        , latlon ne
        , spheroid::standard reference_spheroid = spheroid::standard::wgs84);
    digital_elevation_esri(
          const std::string& path
        , latlon sw
        , latlon ne
        , spheroid reference_spheroid);

    auto reference_spheroid() -> const spheroid&;
    auto lookup(const latlon& loc) -> real;
    auto lookup(latlonalt* values, size_t count) -> void;
    auto testBOM(void) -> void {}
    auto testFTG(void) -> void {}

  private:
    spheroid      spheroid_;
    latlon        nw_;
    real          sample_width_;
    array2<real>  data_;
  };
}}

#endif
