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
 * @file MainParams.cc
 *
 * @class MainParams
 *
 * Class controlling access to the MAIN_PARAMS section of the CIDD parameter
 * file.
 *  
 * @date 9/24/2010
 *
 */

#include <string.h>

#include <rapmath/math_macros.h>
#include <toolsa/str.h>

#include "MainParams.hh"

using namespace std;

// Globals

const int MainParams::MAX_TOKENS = 32;
const int MainParams::MAX_TOKEN_LEN = 1024;

/**********************************************************************
 * Constructor
 */

MainParams::MainParams () :
  ParamSection(),
  _doubleTokens(0),
  _displayProjectionRead(false),
  _domainLimitsRead(false)
{
}


/**********************************************************************
 * Destructor
 */

MainParams::~MainParams(void)
{
  if (_doubleTokens != 0)
    delete [] _doubleTokens;
}
  

/**********************************************************************
 * init()
 */

bool MainParams::init(const char *params_buf, const size_t buf_size)
{
  static const string method_name = "MainParams::init()";
  
  // Allocate space for the token buffer

  _doubleTokens = new double[MAX_TOKENS];
  
  // Read in the parameter file

  if (_params.read(params_buf, buf_size, "cidd") != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error loading parameters into internal buffer" << endl;
    
    return false;
  }

  return true;
}
  

/**********************************************************************
 * getDemoTime()
 */

DateTime MainParams::getDemoTime() const
{
  string demo_time_string = _params.getString("cidd.demo_time", "");
  
  if (demo_time_string.length() < 8)
    return DateTime::NEVER;
  
  return _parseStringIntoTime(demo_time_string);
}


/**********************************************************************
 * getZoom()
 */

vector< Zoom > MainParams::getZooms() const
{
  // Make sure that the domain limits have been retrieved

  _getDomainLimits();
  
  // Create the vector of zooms to return

  vector< Zoom > zooms;
  
  // Loop through each of the zooms in the parameter file and add it to the
  // vector.  Also get the default zoom level so we can set that flag as
  // we create the zooms.

  int num_zooms = _params.getLong("cidd.num_zoom_levels", 1);
  int default_zoom = _params.getLong("cidd.start_zoom_level", 1) - 1;
  
  char param_string[40];
  char label_default[10];
  
  double max_delta_x = _domainLimitMaxX - _domainLimitMinX;
  double max_delta_y = _domainLimitMaxY - _domainLimitMinY;
  
  for (int i = 0; i < num_zooms; ++i)
  {
    // Create the zoom

    Zoom zoom;
    
    sprintf(param_string, "cidd.level%d_label", i + 1);
    sprintf(label_default, "%d", i + 1);
    zoom.name = _params.getString(param_string, label_default);
    
    if (i == default_zoom)
      zoom.isDefault = true;
    
    // Set the limits of the zoom

    sprintf(param_string, "cidd.level%d_min_xkm", i + 1);
    zoom.minX = _params.getDouble(param_string, -200.0/(i+1));

    sprintf(param_string, "cidd.level%d_min_ykm", i + 1);
    zoom.minY = _params.getDouble(param_string, -200.0/(i+1));

    sprintf(param_string, "cidd.level%d_max_xkm", i + 1);
    zoom.maxX = _params.getDouble(param_string, 200.0/(i+1));

    sprintf(param_string, "cidd.level%d_max_ykm", i + 1);
    zoom.maxY = _params.getDouble(param_string, 200.0/(i+1));

    double delta_x = zoom.maxX - zoom.minX;
    double delta_y = zoom.maxY - zoom.minY;

    if (delta_x > max_delta_x) delta_x = max_delta_x;
    if (delta_y > max_delta_y) delta_y = max_delta_y;
            
    // Trap bogus values

    if (zoom.minX < _domainLimitMinX)
    {
      zoom.minX = _domainLimitMinX;
      zoom.maxX = _domainLimitMinX + delta_x;
    }

    if (zoom.minY < _domainLimitMinY)
    {
      zoom.minY = _domainLimitMinY;
      zoom.maxY = _domainLimitMinY + delta_y;
    }

    if (zoom.maxX > _domainLimitMaxX)
    {
      zoom.maxX = _domainLimitMaxX;
      zoom.minX = _domainLimitMaxX - delta_x;
    }

    if (zoom.maxY > _domainLimitMaxY)
    {
      zoom.maxY = _domainLimitMaxY;
      zoom.minY = _domainLimitMaxY - delta_y;
    }

    // Get the aspect ratio information

    double aspect_ratio = _params.getDouble("cidd.aspect_ratio", 1.0);
    if (aspect_ratio <= 0.0) aspect_ratio = fabs(delta_x/delta_y);

    double aspect_correction =
      cos(((zoom.maxY + zoom.minY)/2.0) * DEG_TO_RAD);

    // Forshorten the Y coords for the lat/lon projection to make things
    // look better.

    if (_displayProj.getProjType() == Mdvx::PROJ_LATLON)
      delta_y /= aspect_correction;

    // I'm not sure why we do this

    delta_x /= aspect_ratio;

    // Now fit the zoom into the window.

    if (delta_x > delta_y)
    {
      zoom.maxY += (delta_x - delta_y) / 2.0;
      zoom.minY -= (delta_x - delta_y) / 2.0;
    }
    else
    {
      zoom.maxX += (delta_y - delta_x) / 2.0;
      zoom.minX -= (delta_y - delta_x) / 2.0;
    }

    // Add the zoom to the list

    zooms.push_back(zoom);
    
  } /* endfor - i */
  
  return zooms;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _getDisplayProjection()
 */

void MainParams::_getDisplayProjection() const
{
  if (_displayProjectionRead)
    return;
  
  // Initialize the projection as a lat/lon projection by default.  This
  // will be returned (and a warning message will be printed) if we can't
  // get the projection information from the CIDD parameter file.

  _displayProj.initLatlon();
  
  // Get the projection information from the parameter file

  string proj_string =
    _params.getString("cidd.projection_type", "CARTESIAN");

  if (strncasecmp(proj_string.c_str(), "CARTESIAN", strlen("CARTESIAN")) == 0)
  {
    _displayProj.initFlat(getOriginLatitude(), getOriginLongitude(),
			  getNorthAngle());
  }
  else if (strncasecmp(proj_string.c_str(), "LAT_LON", strlen("LAT_LON")) == 0)
  {
    // Do nothing -- this is the same as the default projection we set
    // above.
  }
  else if (strncasecmp(proj_string.c_str(), "LAMBERT", strlen("LAMBERT")) == 0)
  {
    _displayProj.initLambertConf(getOriginLatitude(),getOriginLongitude(),
				 getLambertLat1(), getLambertLat2());
  }
  else if (strncasecmp(proj_string.c_str(), "STEREOGRAPHIC",
		       strlen("STEREOGRAPHIC")) == 0)
  {
    _displayProj.initStereographic(getTangentLat(), getTangentLon(),
				   getCentralScale());
    _displayProj.setOffsetOrigin(getOriginLatitude(), getOriginLongitude());
  }
  else if (strncasecmp(proj_string.c_str(), "POLAR_STEREO",
		       strlen("POLAR_STEREO")) == 0)
  {
    Mdvx::pole_type_t pole_type =
      getTangentLat() < 0.0 ? Mdvx::POLE_SOUTH : Mdvx::POLE_NORTH;
    
    _displayProj.initPolarStereo(getTangentLon(), pole_type,
				 getCentralScale());
    _displayProj.setOffsetOrigin(getOriginLatitude(), getOriginLongitude());
  }
  else if (strncasecmp(proj_string.c_str(), "MERCATOR",
		       strlen("MERCATOR")) == 0)
  {
    _displayProj.initMercator(getOriginLatitude(), getOriginLongitude());
  }
  
  _displayProjectionRead = true;
}


/**********************************************************************
 * _getDomainLimits()
 */

void MainParams::_getDomainLimits() const
{
  // If we've already pulled the domain limits, we don't need to do it again

  if (_domainLimitsRead)
    return;
  
  // We need the display projection so we know which defaults to use for the
  // domain limits

  _getDisplayProjection();

  // Get the domain limits, using the appropriate units for the underlying
  // display projection.

  switch (_displayProj.getProjType())
  {
  case Mdvx::PROJ_LATLON :
  default:
    _domainLimitMinX = _params.getDouble("cidd.domain_limit_min_x", -360);
    _domainLimitMaxX = _params.getDouble("cidd.domain_limit_max_x", 360);
    _domainLimitMinY = _params.getDouble("cidd.domain_limit_min_y", -90);
    _domainLimitMaxY = _params.getDouble("cidd.domain_limit_max_y", 90);
    break;
    
  case Mdvx::PROJ_FLAT :
    _domainLimitMinX = _params.getDouble("cidd.domain_limit_min_x", -1000);
    _domainLimitMaxX = _params.getDouble("cidd.domain_limit_max_x", 1000);
    _domainLimitMinY = _params.getDouble("cidd.domain_limit_min_y", -1000);
    _domainLimitMaxY = _params.getDouble("cidd.domain_limit_max_y", 1000);
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    _domainLimitMinX = _params.getDouble("cidd.domain_limit_min_x", -10000);
    _domainLimitMaxX = _params.getDouble("cidd.domain_limit_max_x", 10000);
    _domainLimitMinY = _params.getDouble("cidd.domain_limit_min_y", -10000);
    _domainLimitMaxY = _params.getDouble("cidd.domain_limit_max_y", 10000);
    break;
  }
  
  // Fix the order of the limits

  double temp;
  
  if (_domainLimitMinX > _domainLimitMaxX)
  {
    temp = _domainLimitMinX;
    _domainLimitMinX = _domainLimitMaxX;
    _domainLimitMaxX = temp;
  }
  
  if (_domainLimitMinY > _domainLimitMaxY)
  {
    temp = _domainLimitMinY;
    _domainLimitMinY = _domainLimitMaxY;
    _domainLimitMaxY = temp;
  }
  
  // Sanitize the full earth domain limits

  if (_displayProj.getProjType() == Mdvx::PROJ_LATLON)
  {
    if (_domainLimitMinX == _domainLimitMaxX)
    {
      _domainLimitMinX -= 180.0;
      _domainLimitMaxX += 180.0;
    }
    
    if (_domainLimitMinX < -360.0) _domainLimitMinX = -360.0;
    if (_domainLimitMaxX > 360.0) _domainLimitMaxX = 360.0;
    if (_domainLimitMinY < -180.0) _domainLimitMinY = -180.0;
    if (_domainLimitMaxY > 180.0) _domainLimitMaxY = 180.0;

    // Set the origin of the underlying projection

    _displayProj.initLatlon((_domainLimitMinX + _domainLimitMaxX) / 2.0);
  }
  
  _domainLimitsRead = true;
}


/**********************************************************************
 * _parseStringIntoTime()
 */

DateTime MainParams::_parseStringIntoTime(const string &time_string) const
{
  // Parse the time string into tokens

  int year, month, day, hour, min, sec;
  
  int num_fields = STRparse_double(time_string.c_str(), _doubleTokens,
				   time_string.length(), MAX_TOKENS);

  switch(num_fields)
  {
  case 6:     /* hour:min:sec month/day/Year */
    hour = (int)(_doubleTokens[0]) % 24;
    min = (int)(_doubleTokens[1]) % 60;
    sec = (int)(_doubleTokens[2]) % 60;
    
    month = (int)(_doubleTokens[3]) % 13;
    day = (int)(_doubleTokens[4]) % 32;

    if (_doubleTokens[5] < 50)
      _doubleTokens[5] += 2000;
    if (_doubleTokens[5] < 1900)
      _doubleTokens[5] += 1900;
    year = (int)(_doubleTokens[5]);
    break;

  case 5:     /* hour:min month/day/year */
    hour = (int)(_doubleTokens[0]) % 24;
    min = (int)(_doubleTokens[1]) % 60;
    sec = 0;

    month = (int)(_doubleTokens[2]) % 13;
    day = (int)(_doubleTokens[3]) % 32;
    if (_doubleTokens[4] < 50)
      _doubleTokens[4] += 2000;
    if (_doubleTokens[4] < 1900)
      _doubleTokens[4] += 1900;
    year = (int)(_doubleTokens[4]);
    break;

  default:
    return DateTime::NEVER;
  }

  DateTime return_time(year, month, day, hour, min, sec);
  
  // Compensate for the user entering Local time

  if (isUseLocalTimestamps())
  {
    time_t now = time(0);
    double tdiff = difftime(mktime(localtime(&now)),mktime(gmtime(&now)));

    return_time -= tdiff;
  }
  
  return return_time;
}
