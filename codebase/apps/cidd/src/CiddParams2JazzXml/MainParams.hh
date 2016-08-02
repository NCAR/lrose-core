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
 * @file MainParams.hh
 *
 * @class MainParams
 *
 * Class controlling access to the MAIN_PARAMS section of the CIDD parameter
 * file.
 *  
 * @date 9/24/2010
 *
 */

#ifndef MainParams_HH
#define MainParams_HH

#include <ctype.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include <Mdv/MdvxProj.hh>
#include <toolsa/DateTime.hh>

#include "ParamSection.hh"
#include "Uparams.hh"
#include "Zoom.hh"

using namespace std;


class MainParams : public ParamSection
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  MainParams();
  
  /**
   * @brief Destructor
   */

  virtual ~MainParams(void);
  

  /**
   * @brief Initialize the parameters from the given buffer.
   *
   * @param[in] params_buf Parameter file buffer.  Must be null-terminated.
   * @param[in] buf_size   Size of the parameter buffer.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const char *params_buf, const size_t buf_size);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the aspect ratio of the window.
   *
   * @return Returns the aspect ratio of the window.
   */

  double getAspectRatio() const
  {
    double ratio = _params.getDouble("cidd.aspect_ratio", 1.0);
    if (ratio <= 0.0)
    {
      vector< Zoom > zooms = getZooms();
      if (zooms.size() != 0)
      {
	double delta_x = zooms[0].maxX - zooms[0].minX;
	double delta_y = zooms[0].maxY - zooms[0].minY;
	
	ratio = fabs(delta_x / delta_y);
      }
    }
    
    return ratio;
  }
  

  /**
   * @brief Get the azimuth interval.
   *
   * @return Returns the azimuth interval in degrees.
   */

  double getAzimuthInterval() const
  {
    return _params.getDouble("cidd.azmith_interval", 30.0);
  }
  

  /**
   * @brief Get the azimuth radius.
   *
   * @return Returns the azimuth radius in km.
   */

  double getAzimuthRadius() const
  {
    return _params.getDouble("cidd.azmith_radius", 200.0);
  }
  

  /**
   * @brief Get the background color.
   *
   * @return Returns the background color.
   */

  string getBackgroundColor() const
  {
    const char *color = _params.getString("cidd.background_color", "Black");
    char *color_buffer = new char[strlen(color) + 1];
    strcpy(color_buffer, color);
    
    for (size_t i = 0; i < strlen(color_buffer); ++i)
      color_buffer[i] = tolower(color_buffer[i]);
    
    string return_color = color_buffer;
    delete [] color_buffer;
    
    return return_color;
  }
  

  /**
   * @brief Get the central scale value.
   *
   * @return Returns the central scale value.
   */

  double getCentralScale() const
  {
    return _params.getDouble("cidd.central_scale", 1.0);
  }
  

  /**
   * @brief Get the color file subdirectory.
   *
   * @return Returns the color file subdirectory.
   */

  string getColorFileSubdir() const
  {
    static bool warning_msg_issued = false;
    
    // Get the value from the parameter file

    string color_file_string =
      _params.getString("cidd.color_file_subdir", "colorscales");

    // If there is a comma in the string, then there are multiple
    // locations specified.  Use the first.

    size_t comma_pos = color_file_string.find(",");
    if (comma_pos != string::npos)
    {
      // Warn the user

      if (!warning_msg_issued)
      {
	cerr << "WARNING: " << endl;
	cerr << "Your CIDD parameter file contains multiple color file locations" << endl;
	cerr << "The first location will be used for all colorscales in the Jazz" << endl;
	cerr << "file.  You will need to update any colorscales using a later location." << endl;
	cerr << endl;

	warning_msg_issued = true;
      }
      
      // Set the color file string to the first location

      color_file_string = color_file_string.substr(0, comma_pos-1);
    }
    
    return color_file_string;
  }
  

  /**
   * @brief Get the demo time specified in the parameter file.
   *
   * @return Returns the time specified in the parameter file, or
   *         DateTime::NEVER is no time is specified or if the specified
   *         time can't be read.
   */

  DateTime getDemoTime() const;
  

  /**
   * @brief Get the display projection.
   *
   * @return Returns the display projection.
   */

  MdvxProj getDisplayProjection() const
  {
    _getDisplayProjection();
  
    return _displayProj;
  }
  

  /**
   * @brief Get the domain limits max x value.
   *
   * @return Returns the domain limits max x value.
   */

  double getDomainLimitMaxX() const
  {
    _getDomainLimits();
    
    return _domainLimitMaxX;
  }
  

  /**
   * @brief Get the domain limits max y value.
   *
   * @return Returns the domain limits max y value.
   */

  double getDomainLimitMaxY() const
  {
    _getDomainLimits();
    
    return _domainLimitMaxY;
  }
  

  /**
   * @brief Get the domain limits min x value.
   *
   * @return Returns the domain limits min x value.
   */

  double getDomainLimitMinX() const
  {
    _getDomainLimits();
    
    return _domainLimitMinX;
  }
  

  /**
   * @brief Get the domain limits min y value.
   *
   * @return Returns the domain limits min y value.
   */

  double getDomainLimitMinY() const
  {
    _getDomainLimits();
    
    return _domainLimitMinY;
  }
  

  /**
   * @brief Get the default horizontal window height.
   *
   * @return Returns the default horizontal window height.
   */

  int getHorizDefaultHeight() const
  {
    return _params.getLong("cidd.horiz_default_height", 440);
  }
  

  /**
   * @brief Get the default horizontal window width.
   *
   * @return Returns the default horizontal window width.
   */

  int getHorizDefaultWidth() const
  {
    double height = (double)getHorizDefaultHeight();
    double aspect_ratio = getAspectRatio();
    
    return (int)((height * aspect_ratio) + 0.5);
  }
  

  /**
   * @brief Get the default X position for the horizontal window.
   *
   * @return Returns the default X position.
   */

  int getHorizDefaultXPos() const
  {
    return _params.getLong("cidd.horiz_default_x_pos", 0);
  }
  

  /**
   * @brief Get the default Y position for the horizontal window.
   *
   * @return Returns the default Y position.
   */

  int getHorizDefaultYPos() const
  {
    return _params.getLong("cidd.horiz_default_y_pos", 0);
  }
  

  /**
   * @brief Get the first latitude used to define the lambert display
   *        projection.
   *
   * @return Returns the lambert lat1 value.
   */

  double getLambertLat1() const
  {
    return _params.getDouble("cidd.lambert_lat1", 20.0);
  }
  

  /**
   * @brief Get the second latitude used to define the lambert display
   *        projection.
   *
   * @return Returns the lambert lat2 value.
   */

  double getLambertLat2() const
  {
    return _params.getDouble("cidd.lambert_lat2", 60.0);
  }
  

  /**
   * @brief Get the map file subdirectory.
   *
   * @return Returns the map file subdirectory.
   */

  string getMapFileSubdir() const
  {
    static bool warning_msg_issued = false;
    
    // Get the value from the parameter file

    string map_file_string =
      _params.getString("cidd.map_file_subdir", "maps");

    // If there is a comma in the string, then there are multiple
    // locations specified.  Use the first.

    size_t comma_pos = map_file_string.find(",");
    if (comma_pos != string::npos)
    {
      // Warn the user

      if (!warning_msg_issued)
      {
	cerr << "WARNING: " << endl;
	cerr << "Your CIDD parameter file contains multiple map file locations" << endl;
	cerr << "The first location will be used for all maps in the Jazz" << endl;
	cerr << "file.  You will need to update any maps using a later location." << endl;
	cerr << endl;

	warning_msg_issued = true;
      }
      
      // Set the color file string to the first location

      map_file_string = map_file_string.substr(0, comma_pos-1);
    }
    
    return map_file_string;
  }
  

  /**
   * @brief Get the delay at the end of the movie loop in msec.
   *
   * @return Returns the delay at the end of the movie loop.
   */

  int getMovieDelay() const
  {
    return _params.getLong("cidd.movie_delay", 3000);
  }
  

  /**
   * @brief Get the speed of the movie loop in msec per frame.
   *
   * @return Returns the speed of the movie loop.
   */

  int getMovieSpeed() const
  {
    return _params.getLong("cidd.movie_speed_msec", 75);
  }
  

  /**
   * @brief Get the north angle of the display.
   *
   * @return Returns the north angle of the display.
   */

  double getNorthAngle() const
  {
    return _params.getDouble("cidd.north_angle", 0.0);
  }
  

  /**
   * @brief Get the number of movie frames.
   *
   * @return Returns the number of movie frames.
   */

  int getNumFrames() const
  {
    return _params.getLong("cidd.starting_movie_frames",
			   _params.getLong("cidd.num_pixmaps", 5));
  }
  

  /**
   * @brief Get the origin latitude.
   *
   * @return Returns the origin latitude.
   */

  double getOriginLatitude() const
  {
    return _params.getDouble("cidd.origin_latitude", 39.8783);
  }
  

  /**
   * @brief Get the origin longitude.
   *
   * @return Returns the origin longitude.
   */

  double getOriginLongitude() const
  {
    return _params.getDouble("cidd.origin_longitude", -104.7568);
  }
  

  /**
   * @brief Get the range ring color.
   *
   * @return Returns the range ring color.
   */

  string getRangeRingColor() const
  {
    const char *color = _params.getString("cidd.range_ring_color", "grey");
    char *color_buffer = new char[strlen(color) + 1];
    strcpy(color_buffer, color);
    
    for (size_t i = 0; i < strlen(color_buffer); ++i)
      color_buffer[i] = tolower(color_buffer[i]);
    
    string return_color = color_buffer;
    delete [] color_buffer;
    
    return return_color;
  }
  

  /**
   * @brief Get the range ring spacing.
   *
   * @return Returns the range ring spacing in km.
   */

  double getRangeRingSpacing() const
  {
    return _params.getDouble("cidd.range_ring_spacing", -1.0);
  }
  

  /**
   * @brief Get the tangent latitude.
   *
   * @return Returns the tangent latitude.
   */

  double getTangentLat() const
  {
    return _params.getDouble("cidd.tangent_lat", 90.0);
  }
  

  /**
   * @brief Get the tangent longitude.
   *
   * @return Returns the tangent longitude.
   */

  double getTangentLon() const
  {
    return _params.getDouble("cidd.tangent_lon", 0.0);
  }
  

  /**
   * @brief Get the time interval in seconds.
   *
   * @return Returns the time interval in seconds.
   */

  int getTimeInterval() const
  {
    double time_interval_minutes =
      _params.getDouble("cidd.time_interval", 10.0);
    
    return (int)((time_interval_minutes * 60.0) + 0.5);
  }
  

  /**
   * @brief Get the update interval in seconds.
   *
   * @return Returns the update interval in seconds.
   */

  int getUpdateInterval() const
  {
    return _params.getLong("cidd.update_interval", 120);
  }
  

  /**
   * @brief Get the default wind marker type.
   *
   * @return Returns the default wind marker type.
   */

  string getWindMarkerType() const
  {
    return _params.getString("cidd.wind_marker_type", "arrow");
  }
  

  /**
   * @brief Get the zooms.
   *
   * @return Returns the zooms.  The units in the zooms are defined by
   *         the underlying display projection.
   */

  vector< Zoom > getZooms() const;
  

  /**
   * @brief Get the html_mode value.
   */

  int isHtmlMode() const
  {
    int html_mode = 0;
    
    if (isRunOnceAndExit())
      html_mode = 1;
    
    if (isSet("cidd.html_image_dir"))
      html_mode = 1;
    
    html_mode = _params.getLong("cidd.html_mode", html_mode);

    if (html_mode == 0)
      return false;
    else
      return true;
  }
  

  /**
   * @brief Get the render azimuth lines flag.
   *
   * @return Returns the render azimuth lines flag.
   */

  bool isRenderAzimuthLines() const
  {
    int azmith_lines = _params.getLong("cidd.azmith_lines", 0);

    if (azmith_lines == 0)
      return false;
    else
      return true;
  }
  

  /**
   * @brief Get the render range rings flag.
   *
   * @return Returns the render range rings flag.
   */

  bool isRenderRangeRings() const
  {
    int range_rings = _params.getLong("cidd.range_rings", 0);

    if (range_rings == 0)
      return false;
    else
      return true;
  }
  

  /**
   * @brief Get the replace_underscores value.
   */

  bool isReplaceUnderscores() const
  {
    int replace_underscores = _params.getLong("cidd.replace_underscores", 1);
    
    if (replace_underscores == 0)
      return false;
    else
      return true;
  }
  

  /**
   * @brief Get the run_once_and_exit value.
   */

  bool isRunOnceAndExit() const
  {
    int run_once_and_exit = _params.getLong("cidd.run_once_and_exit", 0);
    
    if (run_once_and_exit != 0)
      return true;
    
    return false;
  }
  

  /**
   * @brief Get the use_local_timestamps value.
   */

  bool isUseLocalTimestamps() const
  {
    int use_local_timestamps =
      _params.getLong("cidd.use_local_timestamps", 0);
    
    if (use_local_timestamps != 0)
      return true;
    
    return false;
  }
  

  /**
   * @brief Check to see if the given resource is set.
   */

  bool isSet(const string &resource_string) const
  {
    const char *resource = _params.getString(resource_string.c_str(), "");
    if (resource != 0 && strlen(resource) > 0)
      return true;
    else
      return false;
  }
  

 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Maximum number of tokens in a line.
   */

  static const int MAX_TOKENS;
  
  /**
   * @brief Maximum length of a token.
   */

  static const int MAX_TOKEN_LEN;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Token buffer.
   */

  double *_doubleTokens;
  
  /**
   * @brief Parameter buffer.
   */

  Uparams _params;
  
  /**
   * @brief Flag indicating whether the display projection has been read
   *        from the parameter file.
   */

  mutable bool _displayProjectionRead;
  
  /**
   * @brief The display projection read from the parameter file.
   */

  mutable MdvxProj _displayProj;
  
  /**
   * @brief Flag indicating whether the domain limits values have been
   *        read from the parameter file.
   */

  mutable bool _domainLimitsRead;
  
  /**
   * @brief The domain limits min x value read from the parameter file.
   *        This value is in the units of the underlying display projection.
   */

  mutable double _domainLimitMinX;
  
  /**
   * @brief The domain limits max x value read from the parameter file.
   *        This value is in the units of the underlying display projection.
   */

  mutable double _domainLimitMaxX;
  
  /**
   * @brief The domain limits min y value read from the parameter file.
   *        This value is in the units of the underlying display projection.
   */

  mutable double _domainLimitMinY;
  
  /**
   * @brief The domain limits max y value read from the parameter file.
   *        This value is in the units of the underlying display projection.
   */

  mutable double _domainLimitMaxY;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _getDisplayProjection() const;
  
  void _getDomainLimits() const;

  DateTime _parseStringIntoTime(const string &time_string) const;
  

};


#endif
