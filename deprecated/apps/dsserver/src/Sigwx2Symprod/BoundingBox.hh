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
 *
 * @file BoundingBox.hh
 *
 * @class BoundingBox
 *
 * Class controlling access to a bounding box.
 *  
 * @date 10/19/2009
 *
 */

#ifndef _BoundingBox_hh
#define _BoundingBox_hh

using namespace std;


/**
 * @class BoundingBox
 */

class BoundingBox
{

public:

  /**
   * @brief Constructor.
   *
   * @param[in] min_lat Minimum latitude in the bounding box.
   * @param[in] max_lat Maximum latitude in the bounding box.
   * @param[in] min_lon Minimum longitude in the bounding box.
   * @param[in] max_lon Maximum longitude in the bounding box.
   */

  BoundingBox(const double min_lat = -90.0, const double max_lat = 90.0,
	      const double min_lon = 0.0, const double max_lon = 360.0);
  
  /**
   * @brief Destructor.
   */

  virtual ~BoundingBox();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Set the limits of the bounding box.
   *
   * @param[in] min_lat Minimum latitude in the bounding box.
   * @param[in] max_lat Maximum latitude in the bounding box.
   * @param[in] min_lon Minimum longitude in the bounding box.
   * @param[in] max_lon Maximum longitude in the bounding box.
   */

  void setLimits(const double min_lat, const double max_lat,
		 const double min_lon, const double max_lon)
  {
    _minLat = min_lat;
    _maxLat = max_lat;
    _minLon = min_lon;
    _maxLon = max_lon;
  }
  

  /**
   * @brief Get the minimum latitude in the bounding box.
   *
   * @return Returns the minimum latitude in the bounding box.
   */

  double getMinLat() const
  {
    return _minLat;
  }
  

  /**
   * @brief Get the maximum latitude in the bounding box.
   *
   * @return Returns the maximum latitude in the bounding box.
   */

  double getMaxLat() const
  {
    return _maxLat;
  }
  

  /**
   * @brief Get the minimum longitude in the bounding box.
   *
   * @return Returns the minimum longitude in the bounding box.
   */

  double getMinLon() const
  {
    return _minLon;
  }
  

  /**
   * @brief Get the maximum longitude in the bounding box.
   *
   * @return Returns the maximum longitude in the bounding box.
   */

  double getMaxLon() const
  {
    return _maxLon;
  }
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**
   * @brief Get the length of the maximum dimension of the bounding box in
   *        degrees.
   *
   * @return Returns the length of the maximum dimension.
   */

  double getMaxDim() const;
  

  /**
   * @brief Check to see if a point lies within the defined bounding box.
   *
   * @param[in] lat Point latitude.
   * @param[in] lon Point longitude.
   *
   * @return Returns true if the point lies within the bounding box,
   *         false otherwise.
   */

  bool isInterior(double lat,
		  double lon) const;
  

  /**
   * @brief Convert the given number of degrees in latitude to number of
   *        pixels.
   *
   * @param[in] deg_lat Number of degrees in the latitude direction.
   * @param[in] screen_size Number of pixels across the screen.
   *
   * @return Returns the equivalent number of pixels.
   */

  double degLatToPixels(const double deg_lat,
			const int screen_size) const;
  

  /**
   * @brief Convert the given number of degrees in longitude to number of
   *        pixels.
   *
   * @param[in] deg_lon Number of degrees in the longitude direction.
   * @param[in] screen_size Number of pixels across the screen.
   *
   * @return Returns the equivalent number of pixels.
   */

  double degLonToPixels(const double deg_lon,
			const int screen_size) const;
  

  /**
   * @brief Convert the given number of pixels to degrees in the latitude
   *        direction.
   *
   * @param[in] pixels Number of pixels to convert.
   * @param[in] screen_size Number of pixels across the screen.
   *
   * @return Returns the number of degrees in latitude.
   */

  double pixelsToDegLat(const int pixels,
			const int screen_size) const;
  

  /**
   * @brief Convert the given number of pixels to degrees in the longitude
   *        direction.
   *
   * @param[in] pixels Number of pixels to convert.
   * @param[in] screen_size Number of pixels across the screen.
   *
   * @return Returns the number of degrees in longitude.
   */

  double pixelsToDegLon(const int pixels,
			const int screen_size) const;
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Minimum latitude in the bounding box.
   */

  double _minLat;
  

  /**
   * @brief Maximum latitude in the bounding box.
   */

  double _maxLat;
  

  /**
   * @brief Minimum longitude in the bounding box.
   */

  double _minLon;
  

  /**
   * @brief Maximum longitude in the bounding box.
   */

  double _maxLon;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

};

  
#endif
