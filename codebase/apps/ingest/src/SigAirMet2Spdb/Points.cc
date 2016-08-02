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
 ///////////////////////////////////////////////////////////////
 // Points.cc
 //
 // SigAirMet2Spdb object -- functions for retrieving and setting
//                           SIGMET polygon points
 //
 // Deirdre Garvey, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 //
 // Spring 2003
 //
 ///////////////////////////////////////////////////////////////

 #include <string>
 #include <vector>
 #include <cerrno>
 #include <toolsa/toolsa_macros.h>
 #include <toolsa/globals.h>
 #include <toolsa/pjg_flat.h>
 #include <euclid/geometry.h>
 #include "SigAirMet2Spdb.hh"

////////////////////////////////////////////////
// Search through the station database to find
// a valid station.

bool SigAirMet2Spdb::_searchForStation(const bool &centroid_set,
				       const double &centroid_lat,
				       const double &centroid_lon,
				       const string &station,
				       double &lat, double &lon, double &alt)

{

  // Skip bogus stations

  if (_isStationBogus(station)) {
    return false;
  }

  // Skip weather tokens

  if (_isWxTok(station)) {
    return false;
  }

  // Remove termination characters

  string strippedStation;
  _stripTermChars(station, strippedStation);

  // if the station name is 3 letters long, also make names starting with 
  // K and W, since the US NWS does not use the full 4 letter identifier

  string kStation, wStation;
  if (strippedStation.size() == 3) {
    kStation = "K" + strippedStation;
    wStation = "W" + strippedStation;
  }

  // Does this station exist at all?
  
  string type;
  bool found = false;
  if (kStation.size() > 0) {
    if (_stationLoc.FindPosition(kStation, lat, lon, alt, type) == 0) {
      found = true;
    }
  }
  if (wStation.size() > 0) {
    if (_stationLoc.FindPosition(wStation, lat, lon, alt, type) == 0) {
      found = true;
    }
  }
  if (_stationLoc.FindPosition(strippedStation, lat, lon, alt, type) == 0) {
    found = true;
  }
  if (!found) {
    return false;
  }

  // Get the list of station locations that match the input string

  // Set defaults

  double NOT_SET=-9999;
  lat=NOT_SET;
  lon=NOT_SET;
  alt=NOT_SET;

  vector <double> lats;
  vector <double> lons;
  vector <double> alts;
  vector <string> types;

  if (kStation.size() > 0) {
    _stationLoc.FindAllPosition(kStation, lats, lons, alts, types);
  }
  if (wStation.size() > 0) {
    _stationLoc.FindAllPosition(wStation, lats, lons, alts, types);
  }
  _stationLoc.FindAllPosition(strippedStation, lats, lons, alts, types);

  if (lats.size() == 0) {
    return false;
  }

  // We now have a vector of stations, try to find the best one

  found=false;
  double lastr = 0.0;
  for (size_t ii=0; ii<lats.size(); ii++) {

    double ilat=lats[ii];
    double ilon=lons[ii];
    double ialt=alts[ii];

    // Skip NDB type stations

    if (types[ii] != "NDB") {

      // If there is no centroid set, return the station we found
      
      if (!centroid_set) {

	lat=ilat;
	lon=ilon;
	alt=ialt;
	return true;

      } else {

        // If the centroid is set, how close is this station to it?
        // we will look for the closest one
        
	double r, theta;
        
	// first time through, just set the values
        
	if (lat == NOT_SET) {

	  lat=ilat;
	  lon=ilon;
	  alt=ialt;
	  PJGLatLon2RTheta(ilat, ilon, centroid_lat, centroid_lon, &r, &theta);
	  lastr=r;
	  found=true;

	} else {
        
          // how far is this station from the centroid? keep looking for
          // the closest one
          
	  PJGLatLon2RTheta(ilat, ilon, centroid_lat, centroid_lon, &r, &theta);
	  if (r < lastr) {
	    lat=ilat;
	    lon=ilon;
	    alt=ialt;
	    lastr=r;
	  }

	}
        
      } // if (!centroid_set)

    } // if (types != "NDB")

  } // ii

  // Extra check, if the centroid is set does the closest station make
  // sense at all?

  if (centroid_set) {
    if (lastr > _params.max_distance_to_closest_station) {
      found=false;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "searchForStation: station: " << strippedStation
             << ", r: " << lastr <<
          ", discard since too far from centroid " << endl;
      }
    }
  }

  return found;
}


/////////////////////////////////////////////////
// get vertex from token
//

bool SigAirMet2Spdb::_hasLatLon(const string &tok,
				double &lat,
				double &lon)

{

  lat = -9999;
  lon = -9999;

  string tmpTok=tok;
  string func_name="hasLatLon";

  // Need some digits

  if (_allAlpha(tok)) {
    return false;
  }

  // Need some alphas

  if (_allDigits(tok)) {
    return false;
  }

  // look for N/S and E/W characters. Bail if we find something
  // other than N/S E/W

  bool foundNS, foundEW;
  size_t searchNSpos, searchEWpos;
  if (!_findNSEWchars(tmpTok, foundNS, searchNSpos, foundEW, searchEWpos)) {
    return false;
  }

  if (!foundNS || !foundEW) {
    return false;
  }

  int nsPos = (int)searchNSpos;
  int ewPos = (int)searchEWpos;
  
  // Remove a trailing .  or paren if there is one

  if (_lastString(tmpTok, ".")) {
    _stripTrailingSpacer(".", tmpTok, tmpTok);
  }

  if (_lastString(tmpTok, ")")) {
    _stripTrailingSpacer(")", tmpTok, tmpTok);
  }

  // Remove leading paren if there is one. Then have to adjust
  // the positions of ns and ew
  
  if (_firstString(tmpTok, "(")) {
    _stripLeadingSpacer("(", tmpTok, tmpTok);
    nsPos=nsPos-1;
    ewPos=ewPos-1;
    if (nsPos < 0 || ewPos < 0) {
      return false;
    }
  }

  size_t len = tmpTok.length();
  const char *start = tmpTok.c_str();
  
  char nsStr[4], ewStr[4];
  int latDeg = -9999, latMin = -9999, lonDeg = -9999, lonMin = -9999;

  if ((nsPos == 4) && (ewPos == 10)) {
    
    if (tmpTok.find(".", 0) != string::npos) {
      
      // eg, 24.0S041.0W
      
      if (sscanf(start, "%4lf%1s%5lf%1s",
		 &lat, nsStr,
		 &lon, ewStr) != 4) {
	return false;
      }

    } else {

      //  eg 4900N15130W
    
      if (sscanf(start, "%2d%2d%1s%3d%2d%1s",
		 &latDeg, &latMin, nsStr,
		 &lonDeg, &lonMin, ewStr) != 6) {
	return false;
      }
      
      lat = (double) latDeg + latMin / 60.0;
      lon = (double) lonDeg + lonMin / 60.0;

    }
    
  } else if ((nsPos == 2) && (ewPos == 8)) {

    // eg 30N15900E
    
    if (sscanf(start, "%2d%1s%3d%2d%1s",
	       &latDeg, nsStr,
	       &lonDeg, &lonMin, ewStr) != 6) {
      return false;
    }
    lat = (double) latDeg;
    lon = (double) lonDeg + lonMin / 60.0;
  
  } else if ((nsPos == 2) && (ewPos == 6)) {
    
    // eg 06S042W
    
    if (sscanf(start, "%2d%1s%3d%1s",
	       &latDeg, nsStr,
	       &lonDeg, ewStr) != 4) {
      return false;
    }
    lat = (double) latDeg;
    lon = (double) lonDeg;
    
  } else if ((nsPos == 2) && (ewPos == 5)) {
    
    // eg 19S40E
    
    if (sscanf(start, "%2d%1s%2d%1s",
	       &latDeg, nsStr,
	       &lonDeg, ewStr) != 4) {
      return false;
    }
    lat = (double) latDeg;
    lon = (double) lonDeg;

  } else if ((nsPos == 0) && (ewPos == 3)) {

    if (len == 7) {
      
      // eg N31E153

      if (sscanf(start, "%1s%2d%1s%3d",
		 nsStr, &latDeg,
		 ewStr, &lonDeg) != 4) {
	return false;
      }
      lat = (double) latDeg;
      lon = (double) lonDeg;

    } else if (len == 6) {

      // eg S05E13 
      if (sscanf(start, "%1s%2d%1s%2d",
		 nsStr, &latDeg,
		 ewStr, &lonDeg) != 4) {
	return false;
      }
      lat = (double) latDeg;
      lon = (double) lonDeg;

    } else if (len == 9) {
      
      if (tmpTok.find(".", 0) != string::npos) {

	// eg N24E117.5

	lonDeg=-9999;
	if (sscanf(start, "%1s%2d%1s%5lf",
		   nsStr, &latDeg,
		   ewStr, &lon) != 4) {
	  return false;
	}
	lat = (double) latDeg;

      } else {

	// eg N21E11750
	
	if (sscanf(start, "%1s%2d%1s%3d%2d",
		   nsStr, &latDeg,
		   ewStr, &lonDeg, &lonMin) != 5) {
	  return false;
	}
	lat = (double) latDeg;
	lon = (double) lonDeg + lonMin / 60.0;

      }

    } // endif len==9

  } else if ((nsPos == 0) && (ewPos == 5)) {

    if (tmpTok.find(".", 0) != string::npos) {
    
      // eg S37.2E174.8
      
      if (sscanf(start, "%1s%4lf%1s%5lf",
		 nsStr, &lat,
		 ewStr, &lon) != 4) {
	return false;
      }
      
    } else if (len == 11) {
      
      // eg S3600E16300
      
      if (sscanf(start, "%1s%2d%2d%1s%3d%2d",
		 nsStr, &latDeg, &latMin,
		 ewStr, &lonDeg, &lonMin) != 6) {
	return false;
      }
      lat = (double) latDeg + latMin / 60.0;
      lon = (double) lonDeg + lonMin / 60.0;

    } else if (len == 9) {
      
      if (len == 9) {
	// eg N2350E124
	if (sscanf(start, "%1s%2d%2d%1s%3d",
		   nsStr, &latDeg, &latMin,
		   ewStr, &lonDeg) != 5) {
	  return false;
	}
	lat = (double) latDeg + latMin / 60.0;
	lon = (double) lonDeg;
      } // endif len==9

    }
      
  } else if ((nsPos == 0) && (ewPos == 2)) {

    // eg N7E157

    if (len == 6) {
      if (sscanf(start, "%1s%1d%1s%3d",
		 nsStr, &latDeg,
		 ewStr, &lonDeg) != 4) {
	return false;
      }
    } else {
      return false;
    }

    lat = (double) latDeg;
    lon = (double) lonDeg;

  } else if ((nsPos == 4) && (ewPos == 9)) {
    
    if (tmpTok.find(".", 0) != string::npos) {
    
      // eg. 24.0S42.0W
      if (sscanf(start, "%4lf%1s%4lf%1s",
		 &lat, nsStr,
		 &lon, ewStr) != 4) {
	return false;
      }

    }

  } else {
    
    return false;

  }

  if (nsStr[0] == 'S') {
    lat *= -1.0;
  }
  
  if (ewStr[0] == 'W') {
    lon *= -1.0;
  }

  // Is the point valid?

  if ((lat < -180 || lat > 180) || (lon < -360 || lon > 360)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "invalid lat lon: " << lat << ", " << lon << endl;
    }
    return false;
  }

  return true;

}

/////////////////////////////////////////////////
// get lat or lon from token
//

bool SigAirMet2Spdb::_hasLatOrLon(const string &tok,
				  bool &isLat,
				  double &value)
{

  // Need some digits

  if (_allAlpha(tok)) {
    return false;
  }

  // Need some alphas

  if (_allDigits(tok)) {
    return false;
  }

  // look for N/S and E/W characters. Bail if we find something
  // other than N/S E/W.

  bool foundNS, foundEW;
  size_t searchNSpos, searchEWpos;
  bool foundDir=_findNSEWchars(tok, foundNS, searchNSpos, foundEW, searchEWpos);

  if (!foundDir) {
    return false;
  }

  if (!foundNS && !foundEW) {
    return false;
  }

  // Bail if we found both N/S and E/W

  if (foundNS && foundEW) {
    return false;
  }

  // Remove a trailing .

  string tmpTok=tok;
  if (_lastString(tmpTok, ".")) {
    _stripTrailingSpacer(".", tmpTok, tmpTok);
  }

  // Convert from a string to a lat or lon

  size_t len=tmpTok.length();
  int nsPos = (int)searchNSpos;
  int ewPos = (int)searchEWpos;
  char nsStr[4], ewStr[4];
  bool found=false;
  const char *start = tmpTok.c_str();

  // Search for lat first
  
  if (foundNS) {
    if (nsPos == 0) {
      if (len == 3) {

	// eg S05

	int latDeg;
	if (sscanf(start, "%1s%2d",
		   nsStr, &latDeg) == 2) {
	  found=true;
	  value = (double) latDeg;
	}
      } else if (len == 2) {
	
	// eg N7

	int latDeg;
	if (sscanf(start, "%1s%1d",
		   nsStr, &latDeg) == 2) {
	  found=true;
	  value = (double) latDeg;
	}
      } else if ((len ==5) && (tmpTok.find(".", 0) != string::npos)) {

	// eg S37.2

	double lat;
	if (sscanf(start, "%1s%4lf",
		   nsStr, &lat) == 2) {
	  found=true;
	  value=lat;
	}
      } else if (len == 5) {
     
	//  eg N4900

	int latDeg, latMin;
	if (sscanf(start, "%1s%2d%2d",
		   nsStr, &latDeg, &latMin) == 3) {
	  found=true;
	  value = (double) latDeg + latMin / 60.0;
	}
      }
    } //endif nsPos == 0

    else if (nsPos == 1) {

      // eg 3N

      int latDeg;
      if (sscanf(start, "%1d%1s",
		 &latDeg, nsStr) == 2) {
	found=true;
	value = (double) latDeg;
      }
    }

    else if (nsPos == 2) {

      // eg 30N

      int latDeg;
      if (sscanf(start, "%2d%1s",
		 &latDeg, nsStr) == 2) {
	found=true;
	value = (double) latDeg;
      }
    }

    else if (nsPos == 4) {
    
      //  eg 4900N

      int latDeg, latMin;
      if (sscanf(start, "%2d%2d%1s",
		 &latDeg, &latMin, nsStr) == 3) {
	found=true;
	value = (double) latDeg + latMin / 60.0;
      }
      if ((!found) && (tmpTok.find(".", 0) != string::npos)) {

	// eg, 24.0S

	double lat;
	if (sscanf(start, "%4lf%1s",
		   &lat, nsStr) == 2) {
	  found=true;
	  value = lat;
	}
      }
    }

    // found a lat, setup for return

    if (found) {
      isLat=true;
      if (nsStr[0] == 'S') {
	value *= -1.0;
      }
      return true;
    } else {
      return false;
    }
  } //endif foundNS

  if (foundEW) {

    if (ewPos == 0) {
      
      if (len == 4) {

	// eg E153

	int lonDeg;
	if (sscanf(start, "%1s%3d",
		 ewStr, &lonDeg) == 2) {
	  found=true;
	  value = (double) lonDeg;
	}
      }
      else if (len == 3) {

      // eg E13 

	int lonDeg;
	if (sscanf(start, "%1s%2d",
		   ewStr, &lonDeg) == 2) {
	  found=true;
	  value = (double) lonDeg;
	}
      }
      else if ((len == 6) && (tmpTok.find(".", 0) != string::npos)) {

      // E117.5

	double lon;
	if (sscanf(start, "%1s%5lf",
		   ewStr, &lon) == 2) {
	  found=true;
	  value=lon;
	}
      }

      else if (len == 6) {

	// eg E16300
      
	int lonDeg, lonMin;
	if (sscanf(start, "%1s%3d%2d",
		   ewStr, &lonDeg, &lonMin) == 3) {
	  found=true;
	  value=(double) lonDeg + lonMin / 60.0;
	}
      }


    } //endif ewPos==0

    else if (ewPos == 5) {

      //  eg 15130W

      int lonDeg, lonMin;
      if (sscanf(start, "%3d%2d%1s",
		 &lonDeg, &lonMin, ewStr) == 3) {
	found=true;
	value = (double) lonDeg + lonMin / 60.0;
      }
    }

    else if (ewPos == 3) {

      // eg 042W

      int lonDeg;
      if (sscanf(start, "%3d%1s",
		 &lonDeg, ewStr) == 2) {
	found=true;
	value = (double) lonDeg;
      }
    }

    else if (ewPos == 4) {

      if ((len == 5) && (tmpTok.find(".", 0) != string::npos)) {

	// eg 42.0W

	double lon;
	if (sscanf(start, "%4lf%1s",
		   &lon, ewStr) == 2) {
	  found = true;
	  value = lon;
	}
      }
    }
    
    else if (ewPos == 5) {

      if ((len == 6) && (tmpTok.find(".", 0) != string::npos)) {

      // eg 174.8E

	double lon;
	if (sscanf(start, "%1s%5lf",
		   ewStr, &lon) == 2) {
	  found = true;
	  value = lon;
	}
      }
    }
    

    // found a lon, setup for return

    if (found) {
      isLat=false;
      if (ewStr[0] == 'W') {
	value *= -1.0;
      }
      return true;
    } else {
      return false;
    }
  } //endif foundEW

  return false;

}


/////////////////////////////////////////////////
// modify station location with a direction
//

bool SigAirMet2Spdb::_modifyStationPosition(const string &modloc,
					    double &inlat,
					    double &inlon,
					    double &outlat,
					    double &outlon)
{
  outlat=inlat;
  outlon=inlon;

  // does the modloc have N/S/E/W characters?
  
  int dirPos = -1;
  for (size_t ii = 0; ii < modloc.size(); ii++) {
    int c = modloc[ii];
    if (c == 'N' || c == 'S' || c == 'E' || c == 'W') {
      dirPos = ii;
      break;
    }
  }
  
  if (dirPos < 0) {
    return false;
  }

  // Split the modloc between the direction and distance
  // could be 20E or 90NNW

  string dist, dir;
  dist.assign(modloc, 0, dirPos);
  dir.assign(modloc, dirPos, string::npos);

  // Modify the input inlat,inlon by the direction and distance
  // to generate the new outlat,outlon position

  // convert the distance from (assumed!) NM to KM

  int disti;
  double distkm;
  const char *distc = dist.c_str();
  if (sscanf(distc, "%d", &disti) != 1) {
    return false;
  }
  distkm = disti * KM_PER_NM;

  // force the direction into an angle such that the angle
  // is positive if east of North, negative (or > PI) if west of North,
  // 0 = North

  double angle;
  if (!_dir2Angle(dir, angle)) {
    return false;
  }

  // New location

  PJGLatLonPlusRTheta(inlat, inlon, distkm, angle, &outlat, &outlon);

  return true;

}

/////////////////////////////////////////////////
// Fix polyline points by removing outliers

void SigAirMet2Spdb::_fixPolylinePoints(vector<double> &lats,
                                        vector<double> &lons,
                                        vector<bool> &sourceIsLatLon,
                                        bool isClosed)
{

  if (!_params.check_polygon_size_and_shape) {
    return;
  }
  
  // this only works if >= 2 points
  
  if (lats.size() < 2) {
    return;
  }
  
  // if all sources are latlon, return now

  bool continueCheck = false;
  for (int ii = 0; ii < (int) sourceIsLatLon.size(); ii++) {
    if (!sourceIsLatLon[ii]) {
      continueCheck = true;
      continue;
    }
  }
  
  if (!continueCheck) {
    return;
  }

  // if any segment exceeds max length, remove any points which 
  // are more than the max length from half of the others
  
  if (_checkExceedsMaxLen(lats, lons)) {
    while (_removePtFarFromOthers(lats, lons)) {
    }
  }

  // if polygon was closed, make sure it still is

  if (isClosed) {
    if (lats[0] != lats[lats.size()-1] ||
        lons[0] != lons[lons.size()-1]) {
      lats.push_back(lats[0]);
      lons.push_back(lons[0]);
    }
  }

  // check for small angles, repeating until no points are removed

  while (_removeSmallAngles(lats, lons, isClosed) != 0) {
  }

  // check for long sides, repeating until no points are removed

  while (_removeLongSides(lats, lons) != 0) {
  }

  // if polygon was closed, make sure it still is

  if (isClosed) {
    if (lats[0] != lats[lats.size()-1] ||
        lons[0] != lons[lons.size()-1]) {
      lats.push_back(lats[0]);
      lons.push_back(lons[0]);
    }
  }

  // check for min number of points

  if (isClosed) {

    if (lats.size() < 4) {
      lats.clear();
      lons.clear();
      return;
    }

  } else {

    if (lats.size() < 2) {
      lats.clear();
      lons.clear();
      return;
    }

  }

  return;

}

/////////////////////////////////////////////////
// Check for any segments exceed max length
//
// Return true if any segment exceeds the max length,
// false if not

bool SigAirMet2Spdb::_checkExceedsMaxLen(vector<double> &lats,
                                         vector<double> &lons)
  
{
  
  for (int ii = 0; ii < (int) lats.size() - 1; ii++) {
    double dist, theta;
    PJGLatLon2RTheta(lats[ii], lons[ii],
                     lats[ii+1], lons[ii+1], &dist, &theta);
    if (dist > _params.max_valid_polygon_distance) {
      return true;
    }
  }
  return false;

}

/////////////////////////////////////////////////
// Remove any point far from the others
//
// Return -1 if must be rechecked, 0 otherwise

int SigAirMet2Spdb::_removePtFarFromOthers(vector<double> &lats,
                                           vector<double> &lons)
  
{
  
  // Go through each point and see if it is far away from
  // half or more of the other points
  
  int nhalf = ((int) lats.size()) / 2;
  
  for (int ii = 0; ii < (int) lats.size(); ii++) {

    int nbad=0;
    for (int jj = 0; jj < (int) lats.size(); jj++) {
      
      // do not compare against ourselves
      if (jj != ii) {
	double r, theta;
	PJGLatLon2RTheta(lats[ii], lons[ii],
                         lats[jj], lons[jj], &r, &theta);
        if (r > _params.max_valid_polygon_distance) {
	  nbad++;
	}
      }
    } // endfor jj

    if (nbad >= nhalf) {
      lats.erase(lats.begin() + ii);
      lons.erase(lons.begin() + ii);
      return -1;
    }

  } // ii

  return 0;

}

/////////////////////////////////////////////////
// Remove any point on a long side
//
// Return -1 if must be rechecked, 0 otherwise

int SigAirMet2Spdb::_removeLongSides(vector<double> &lats,
                                     vector<double> &lons)
  
{
  
  for (int ii = 0; ii < (int) lats.size() - 1; ii++) {
    double dist, theta;
    PJGLatLon2RTheta(lats[ii], lons[ii],
                     lats[ii+1], lons[ii+1], &dist, &theta);
    if (dist > _params.max_valid_polygon_distance) {
      lats.erase(lats.begin() + ii);
      lons.erase(lons.begin() + ii);
      return -1;
    }
  }
  return 0;

}

/////////////////////////////////////////////////
// Remove small enclosed angles
//
// Return -1 if must be rechecked, 0 otherwise

int SigAirMet2Spdb::_removeSmallAngles(vector<double> &lats,
                                       vector<double> &lons,
                                       bool isClosed)
  
{

  int startIndex = 1;
  int minPoints = 3;
  if (isClosed) {
    startIndex = 0;
    minPoints = 4;
  }

  // check vertices
  
  for (int ii = startIndex; ii < (int) lats.size() - 1; ii++) {

    double _lat[3];
    double _lon[3];

    if (ii == 0) {
      // first vertex, used in closed polygon
      _lat[0] = lats[lats.size() - 2];
      _lon[0] = lons[lats.size() - 2];
      _lat[1] = lats[0];
      _lon[1] = lons[0];
      _lat[2] = lats[1];
      _lon[2] = lons[1];
    } else {
      _lat[0] = lats[ii-1];
      _lon[0] = lons[ii-1];
      _lat[1] = lats[ii];
      _lon[1] = lons[ii];
      _lat[2] = lats[ii+1];
      _lon[2] = lons[ii+1];
    }
    
    double dist1_0, theta1_0;
    PJGLatLon2RTheta(_lat[1], _lon[1], _lat[0], _lon[0], &dist1_0, &theta1_0);
    
    double dist1_2, theta1_2;
    PJGLatLon2RTheta(_lat[1], _lon[1], _lat[2], _lon[2], &dist1_2, &theta1_2);
    
    double angDiff = fabs(theta1_0 - theta1_2);
    if (angDiff > 180) {
      angDiff = 360.0 - angDiff;
    }

    bool discard = false;

    if (angDiff < _params.min_valid_polygon_angle) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "     Polyline vertex angle: " << angDiff << endl;
        cerr << "     Angle below: "
             << _params.min_valid_polygon_angle << endl;
      }
      discard = true;
    }
     
    if (discard) {

      // remove this point

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "     Fixing polyline" << endl;
        for (int ii = 0; ii < (int) lats.size(); ii++) {
          cerr << "ii, lat, lon: " << ii << ", "
               << lats[ii] << ", " << lons[ii] << endl;
        }
      }

      if (ii == 0) {
        // remove end points
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Removing start and end points" << endl;
        }
        lats.erase(lats.begin() + 0);
        lons.erase(lons.begin() + 0);
        lats.erase(lats.end() - 1);
        lons.erase(lons.end() - 1);
        // copy start to end
        lats.push_back(lats[0]);
        lons.push_back(lons[0]);
      } else {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Removing interior point: " << ii << endl;
        }
        lats.erase(lats.begin() + ii);
        lons.erase(lons.begin() + ii);
      }

      return -1;

    } // if (angDiff ...

  } // ii

  return 0;

}

/////////////////////////////////////////////////
// check polygon points and see if it is a valid
// return true if a valid polygon, false otherwise
// 
//

bool SigAirMet2Spdb::_isValidPolygon(vector<double> &inlats,
				     vector<double> &inlons)
{
  // this only works if >= 2 points

  if (inlats.size() < 2) {
    return false;
  }

  size_t nPts=inlats.size();

  // Look for duplicate points

  size_t nDups=0;
  for (size_t ii=0; ii<nPts; ii++) {

    bool foundDup=false;
    for (size_t jj=0; jj<nPts; jj++) {
      
      // do not compare against ourselves
      if (jj != ii) {
	if ((inlats[ii] == inlats[jj]) &&
	    (inlons[ii] == inlons[jj])) {
	  foundDup=true;
	  nDups++;

	}
      }
    } // endfor jj
  } // endfor ii

  // If we have too many duplicates for the number of points, return false

  if ((nDups > nPts-1) || (nDups > nPts/2)) {
    return false;
  }


  return true;

}

/////////////////////////////////////////////////
// try to assemble a lat,lon using the token at the
// startIdx position in the vector. Must use the
// startIdx token.
// 
// Return true if found a possible lat,lon string.
// Return false if no lat,lon string found.
//

bool SigAirMet2Spdb::_searchForLatLonString(size_t &startIdx,
					    const vector<string> &toks,
					    string &latlonstr,
					    size_t &endIdx)
{

  // set defaults

  endIdx=0;
  latlonstr="";
  int max_search_toks=5;
  size_t useStartIdx=startIdx+1;
  bool foundNS=false;
  bool foundNSNum=false;
  bool foundEW=false;
  bool foundEWNum=false;

  // Are we near the end of the input vector

  if ((startIdx + max_search_toks) > toks.size()) {
    max_search_toks=toks.size() - startIdx;
  }
  if (max_search_toks < 2) {
    return false;
  }

  // Look for maybe the start of a lat lon in the first token

  string useTok=toks[startIdx];
  if ((useTok.find("N", 0) != string::npos) ||
      (useTok.find("S", 0) != string::npos)) {
    foundNS=true;
  }

  // Is the first token a number and the second token is a N or S?

  if ((!foundNS && (_hasDigit(useTok))) &&
      ((toks[startIdx+1].find("N", 0) != string::npos) ||
       (toks[startIdx+1].find("S", 0) != string::npos))) {
    foundNS=true;
    useTok+=toks[startIdx+1];
    useStartIdx=startIdx+2;
  }

  // Is the first token a number, followed by a "DEG" , and the third
  // token is a N or S? Need at least 2 tokens for this option

  if (max_search_toks > 2) {
    if ((!foundNS && (_hasDigit(useTok))) &&
	(toks[startIdx+1] == "DEG") &&
	((toks[startIdx+2].find("N", 0) != string::npos) ||
	 (toks[startIdx+2].find("S", 0) != string::npos))) {
      foundNS=true;
      useTok+=toks[startIdx+2];
      useStartIdx=startIdx+2;
    }
  }

  if (!foundNS) {
    return false;
  }

  // Is the first token a string that happens to have N or S in it?

  if ((_allAlpha(useTok)) && (useTok.length() > 1)) {
    return false;
  }

  // Do we have some digits in the N/S string?

  foundNSNum=_hasDigit(useTok);

  // Start assembling the return string

  latlonstr += useTok;

  // Now look forward in the vector for other parts of a lat,lon string
  // For example: 53N 172 E
  
  for (size_t ii=useStartIdx; ii< startIdx+max_search_toks; ii++) {
    useTok=toks[ii];
    endIdx=ii;

    // bail if we find an all alpha string longer than 1 char.
    // exception is DEG
    
    if ((!foundEW) && (_allAlpha(useTok)) && (useTok.length() > 1)) {
      if (useTok == "DEG") {
	useTok="";
      }
      else {
	endIdx=0;
	latlonstr="";
	return false;
      }
    }

    // bail if we find a duplicate direction

    if (foundNS &&
	((useTok.find("N", 0) != string::npos) ||
	(useTok.find("S", 0) != string::npos))) {
      endIdx=0;
      latlonstr="";
      return false;
    }

    // Remove any trailing . or =

    _stripTermChars(useTok, useTok);

    // Look for the E/W component and if it has digits with it

    if ((useTok.find("E", 0) != string::npos) ||
	(useTok.find("W", 0) != string::npos)) {

      // Bail if this is a duplicate
      if (foundEW) {
	endIdx=0;
	latlonstr="";
	return false;
      }

      foundEW=true;

      if (_hasDigit(useTok)) {
	foundEWNum=true;
      }

      latlonstr += useTok;

      // Do we have some E/W digits already that preceeded the EW string token?
      // eg. 06S045 W

      if (!foundEWNum) {
	bool fNS, fEW;
	size_t nsPos, ewPos;
	if (_findNSEWchars(latlonstr, fNS, nsPos, fEW, ewPos)) {
	  if ((fNS && fEW) && (ewPos - nsPos > 0) && (nsPos > 0)) {
	    foundEWNum=true;
	  }
	}
      }
    }

    // have we found the N/S digits?

    else if ((!foundNSNum) && (!foundEW) && (_hasDigit(useTok))) {
      foundNSNum=true;
      latlonstr += useTok;
    }

    // have we found the E/W digits?

    else if ((!foundEWNum) && (foundNSNum) && (_hasDigit(useTok))) {
      foundEWNum=true;
      latlonstr += useTok;
    }

    // Are we done?
    
    if ((foundNS) && (foundEW) && (foundNSNum) && (foundEWNum)) {
      return true;
    }

  } // end ii

  endIdx=0;
  latlonstr="";
  return false;
}

///////////////////////////////////////////////////////////////
// try to assemble a station and position modifier using the token at the
// startIdx position in the vector. Must use the
// startIdx token. This is hoping to find items like: 40E CRG -.
// This will find station location modifiers BEFORE a station but not after
// a station.
// 
// Return true if found a possible station and position modifier string.
// Return false if no string found.
//

bool SigAirMet2Spdb::_searchForStationString(size_t &startIdx,
					     vector<string> &toks,
					     double &lat,
					     double &lon, 
					     size_t &endIdx)
{

  // set defaults

  endIdx=0;
  int max_search_toks=4;

  // Are we near the end of the input vector

  if ((startIdx + max_search_toks) > toks.size()) {
    max_search_toks=toks.size() - startIdx;
    if (max_search_toks < 2) {
      return false;
    }
  }

  // Is there some delimiter token somewhere in the vector?

  bool foundDelim=false;
  size_t delimTokNum=0;
  for (size_t ii=startIdx+1; ii< startIdx+max_search_toks; ii++) {
    if (toks[ii] == "TO" ||
	toks[ii] == "-" ||
	toks[ii] == "/" ||
	toks[ii] == "PSN") {
      delimTokNum=ii;
      foundDelim=true;
    }
  }

  // If we have a delimiter, do not search beyond it only before it
  // We need 2 tokens for this function

  if (foundDelim) {
    if (delimTokNum == startIdx) {
      return false;
    }
    if (delimTokNum < startIdx+2) {
      return false;
    }
  }

  // set the tokens to search

  string dirTok=toks[startIdx];
  string stationTok=toks[startIdx+1];
  endIdx=startIdx+1;

  // Look for a direction string in the first token

  if (!_isDirTok(dirTok)) {
    return false;
  }

  // Do we have some digits in the Dir string?

  if (!_hasDigit(dirTok)) {
    return false;
  }

  // Do we have only alphas in the station string?

  if (!_allAlpha(stationTok)) {
    return false;
  }

  // search for the station then modify its position

  bool found=false;
  double alt;
  if (_searchForStation(_decoded.centroidSet(),
                        _decoded.getCentroidLat(), 
                        _decoded.getCentroidLon(),
                        stationTok, lat, lon, alt)) {

    // modify the station location

    double newlat, newlon;
    if (_modifyStationPosition(dirTok, lat, lon, newlat, newlon) == true) {
      lat=newlat;
      lon=newlon;
      found=true;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "     station modLoc: " << dirTok << ", "
             << stationTok << ", lat,lon: " << lat << ", " << lon << endl;
	}
    } //endif modifyStationPosition
  }
  return found;
}


////////////////////////////////////////////////
// get points from input tokens
//
// returns npoints found and vectors of lats, lons.
// returns a vector, source[] for each lat,lon that is true
// if the lat,lon was from a latlon value and false if from
// a station.

int SigAirMet2Spdb::_findPoints(const vector<string> &toks,
				const size_t &startIdx,
				const size_t &endIdx,
				const bool &skipIcaoStation,
				const int &minPtsNeeded,
				vector <double> &lats,
				vector <double> &lons,
				vector <bool> &sourceIsLatLon,
				size_t &usedStartIdx,
				size_t &usedEndIdx)

{
  // set defaults

   lats.clear();
   lons.clear();
   sourceIsLatLon.clear();
   usedStartIdx=0;
   usedEndIdx=0;

  // Create a new set of tokens and then remove any that do not fit
  // a possible location

   string tok;
   bool foundPsn=false;
   bool foundStnr=false;
   size_t validTimeTok=0;
   size_t stopIdx=0;
   size_t possibleStartIdx=0;
   vector <bool> wantToks;

   bool foundOneFL=false;

   // determine if we are in a FCST section
   bool inFCST=false;
   if (startIdx > 0 && toks[startIdx-1] == "FCST")
     inFCST = true;



   // Set all tokens to wanted by default

   wantToks.clear();
   for (size_t ii = 0; ii < endIdx; ii++) {
     wantToks.push_back(true);
   }

   // Go through the array of tokens and remove any that are something other than
   // a possible location

   for (size_t ii = startIdx; ii < endIdx; ii++) {

     tok=toks[ii];

     if (_params.debug >= Params::DEBUG_VERBOSE) {
       cerr << "findPoints: ii: " << ii << ", tok: " << tok
            << ", _used: " << _used[ii] << endl;
     }

     // Skip already used tokens

     if (_used[ii]) {
       wantToks[ii]=false;
     }

     // Is this a weather string?

     if (wantToks[ii] && (_isWxTok(tok))) {
       wantToks[ii]=false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is a weather string " << endl;
       }
     }

     // Add back in FCST in case it was taken out as a weather string
     // so that we can identify its location later
     // when parsing for lat/lon
     if (tok == "FCST"){
       wantToks[ii] = true;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   Adding back FCST " << endl;
       }
     }
     // Is this a time string?

     time_t isTime;
     if (_computeStartEndTimes(tok, isTime, isTime) == 0) {
       wantToks[ii]=false;
       validTimeTok=ii;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is a time string " << endl;
       }
     }

     if (wantToks[ii] && (_isTimeTok(tok, isTime))) {
       wantToks[ii]=false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is a time string " << endl;
       }
     }

     // Is this a state abbreviation?

     if (wantToks[ii] && (ii-1 >= startIdx) && (ii+1 < endIdx)) {
       string prevTok=toks[ii-1];
       string nextTok=toks[ii+1];
       if (_isStateAbbr(tok, prevTok, nextTok)) {
	 wantToks[ii]=false;
	 if (_params.debug >= Params::DEBUG_VERBOSE) {
	   cerr << "   SKIP token: is a state abbreviation" << endl;
	 }
       }
     }

     // Is this a stationary movement token

     if (!foundStnr) {
       string tok2;
       if (ii+1 < toks.size()) {
	 tok2=toks[ii+1];
       } else {
	 tok2="NULL";
       }
       bool useTok2;
       if (_isStatMovTok(tok, tok2, useTok2)) {
	 foundStnr=true;
	 wantToks[ii]=false;
	 if (useTok2) {
	   wantToks[ii+1]=false;
	   ii=ii+1;
	   if (_params.debug >= Params::DEBUG_VERBOSE) {
	     cerr << "   SKIP token: is a stationary movement string " << endl;
	   }
	 }
       }
     }

     // Is this an FIR token?

     if (wantToks[ii] && (_isFirTok(tok))) {

       wantToks[ii] = false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is an FIR token" << endl;
       }

       // Are other tokens involved in the fir name? If so, need to skip
       // them as well.

       vector <size_t> firToks;
       string firName;
       if (_isFirName(toks, ii, firToks, firName)) {
	 for (size_t jj=0; jj < firToks.size(); jj++) {
	   size_t tnum=firToks[jj];
	   wantToks[tnum] = false;
	 }
       }
     }

     // Is this a diameter token?

     int diam;
     if ((wantToks[ii]) && (_isDiameterTok(tok, diam))) {
       wantToks[ii]=false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is a diameter " << endl;
       }
     }

     // Is this a specific keyword that may indicate the start of points?

     if (wantToks[ii] && 
	 (tok == "BOUNDED" ||
	  tok == "METEO" ||
	  tok == "METEOSAT" ||
	  tok == "METEO-SAT")) {
       possibleStartIdx=ii;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   token: is a keyword indicating possible start of points" << endl;
       }
     }

     // Is this a specific keyword that we need to skip?

     if ((wantToks[ii] && (tok == "SAT")) && 
	 ((ii-1 >= startIdx) && (toks[ii-1] == "METEO"))) {
       wantToks[ii] = false;
       wantToks[ii-1] = false;
       possibleStartIdx=ii-1;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is a keyword indicating possible start of points string " << endl;
       }
     }

     // Is this the location/position of a volcanic eruption
     if (wantToks[ii] && 
	 strcmp(_decoded.getWx(),"VA") == 0 &&
	 (tok == "LOC" ||
	  tok == "PSN")) {
       // set LOC/PSN & next to toks (lat & lon) to not want
       wantToks[ii] = false;
       wantToks[ii+1] = false;
       wantToks[ii+2] = false;

       // we should also set the centroid to this location, just in case the FIR
       // is unable to be decoded & there is no other vertices information in the 
       // SIGMET, this LOC/PSN will be used to plot the VA icon
       string ll;
       size_t llEndIdx,pos;  // throwaway
       double lat, lon;
       pos = ii+1;
       if (_searchForLatLonString(pos,toks,ll,llEndIdx)){
	   if (_hasLatLon(ll,lat,lon)){
	     _decoded.setCentroid(lat,lon);
	   }
	 }

        if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is a keyword indicating LOC of VA" << endl;
       }      
     }

     // Is this the source?

     if (wantToks[ii] && (tok == _decoded.getSource())) {
       wantToks[ii] = false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is the source" << endl;
       }
     }

     if (wantToks[ii] && (tok == _decoded.getId())) {
       wantToks[ii] = false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is the ID" << endl;
       }
     }

     if (wantToks[ii] && (tok == _decoded.getQualifier())) {
       wantToks[ii] = false;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is the qualifier" << endl;
       }
     }

     // Is this a flight level?
     /*
     if (wantToks[ii] && (ii+1 < endIdx)) {
       int nt=_isFLTok(tok, toks[ii+1]);
       if (nt == 1) {
	 wantToks[ii] = false;
       } else if (nt == 2) {
	 wantToks[ii] = false;
	 wantToks[ii+1] = false;
	 ii++;
       }
       if ((nt > 0) && (_params.debug >= Params::DEBUG_VERBOSE)) {
	 cerr << "   SKIP token: is a flight level string, nt: " << nt << endl;
       }
     }
     */
     // Does this have PSN tokens?

     if ((wantToks[ii] && (tok.find("PSN", 0) != string::npos))) {
       foundPsn=true;
     }

     // We want the current location not the fcast or the outlook so stop 
     // looking after this point. When we search for fcast or outlook we
     // will start one after the keyword string for fcast or outlook.
     // Unfortunately, can only enforce for outlook, fcast is too unpredictable...

     if (toks[ii] == "OUTLOOK" || 
	 toks[ii] == "OTLK") {
       stopIdx=ii;
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   SKIP token: is OUTLOOK" << endl;
       }
       break;
     }

     // Tokens after which to stop looking...
     // e.g., intensity
     
     if (_isIntensityTok(tok)) {
       wantToks[ii]=false;
       if (tok == "INTSF" || (ii+5 > endIdx)) {
         // for tok INTSF, stop right away
         // for other toks, less sure,
         // see if we have enough tokens for other things
         if (_params.debug >= Params::DEBUG_VERBOSE) {
           cerr << "   token " << tok
                << " is intensity string, stop looking for points" << endl;
         }
	 stopIdx=ii;
         break;
       }
     }
       
     // movement

     vector <size_t> moveToks;
     string moveType, moveDir, moveSpeed;
     if (_isMovementTok(toks, ii, moveToks, moveType, moveDir, moveSpeed)) {
       if (_params.debug >= Params::DEBUG_VERBOSE) {
         if (_params.debug >= Params::DEBUG_VERBOSE) {
           cerr << "   token " << tok
                << " is movement string, stop looking for points" << endl;
         }
       }
       wantToks[ii]=false;
       stopIdx=ii;
       break;
     }

     // Is this a bogus station. Do not skip delimiters that may
     // be listed as bogus stations! Put this check last so that do not exclude
     // items that I need as keywords but may be in the bogus station list.
     
     if (wantToks[ii] && (_isStationBogus(tok))) {
       if (tok != "TO" && tok != "DUE") {
	 wantToks[ii]=false;
	 if (_params.debug >= Params::DEBUG_VERBOSE) {
	   cerr << "   SKIP token: is a bogus station string" << endl;
	 }
       }
     }

   } // ii

   // If I found a stopIdx, remove all tokens after it

   if (stopIdx > 0) {
     for (size_t ii=stopIdx; ii<toks.size(); ii++) {
       wantToks[ii]=false;
     }
   }

   // If I found a possibleStartIdx, remove all tokens prior to it

   if (possibleStartIdx > 0) {
     for (size_t ii=startIdx; ii<possibleStartIdx; ii++) {
       wantToks[ii]=false;
     }
   }

   // In theory, we have a set of tokens that might be locations, remove any unwanted
   // tokens from the set and retokenize

   string polygonStr="";
   vector <string> polygonToks;
   for (size_t ii = startIdx; ii < endIdx; ii++) {
     if (wantToks[ii]) {
       polygonStr += toks[ii];
       polygonStr += " ";
     }
   }
   
   if (_params.debug >= Params::DEBUG_VERBOSE) {
     cerr << "findPoints: polygonStr: " << polygonStr << endl;
   }

   _tokenize(polygonStr, " \n\t\r", polygonToks);

   // Now we have a lightweight set of tokens we are going to retokenize it several times

   // Replace imbedded slashes with space-slash-space BEFORE retokenize
   // then repeat for imbedded dashes

   int nImbedDash, nImbedSlash, nImbedComma;
   bool doReplaceSpacer=false;
   size_t startIn=0;
   nImbedDash=_isImbedDelimiter(startIn, polygonToks, "-");
   nImbedSlash=_isImbedDelimiter(startIn, polygonToks, "/");
   nImbedComma=_isImbedDelimiter(startIn, polygonToks, ",");

   if (nImbedSlash > 0) {
     doReplaceSpacer=_replaceSpacerRetokenize("/", " / ", " \r\n\t", startIn, polygonToks, polygonToks);
   }
   if (nImbedDash > 0) {
     doReplaceSpacer=_replaceSpacerRetokenize("-", " - ", " \r\n\t", startIn, polygonToks, polygonToks);
   }
   if (nImbedComma > 0) {
     doReplaceSpacer=_replaceSpacerRetokenize(",", " , ", " \r\n\t", startIn, polygonToks, polygonToks);
   }

   // Set up for search for lats, lons

   lats.clear();
   lons.clear();
   sourceIsLatLon.clear();
   int nfound=0;
   size_t endInd=0;
   bool breakAfterTok=false;
   bool latlonsOnly=false;

   if (_tcFound) {
     latlonsOnly=true;
   }

   // Go through the tokens looking for points or stations

   for (size_t ii = 0; ii < polygonToks.size(); ii++) {

     bool found=false;

     // Which token to read

     string useTok=polygonToks[ii];

     //if (_params.debug >= Params::DEBUG_VERBOSE) {
     //       cerr << "findPoints: useTok: " << useTok << endl;
     //     }
     // Cleanup the token

     _stripLeadingSpacer(" ", useTok, useTok);

     // Look for a termination character or string. Be sure it is not at the end of
     // a lat,lon or a station since they sometimes occur in the same token as the
     // termination character. Strip the termination character if found.

     if ((_lastString(useTok, "=")) || (_lastString(useTok, "."))) {
       double lat, lon, alt;
       string tmpTok;
       _stripTermChars(useTok, tmpTok);
       useTok=tmpTok;
       bool foundLatLon=_hasLatLon(useTok, lat, lon);
       bool foundStation=_searchForStation(_decoded.centroidSet(),
                                           _decoded.getCentroidLat(), 
					   _decoded.getCentroidLon(),
                                           useTok, lat, lon, alt);

       // Break if we found a termination character and already have
       // enough points. If found a station or latlon, break after add the
       // lat lon to the array below.

       if (nfound >= minPtsNeeded) {
	 if (!foundLatLon && !foundStation) {
	   break;
	 }
	 else {
	   breakAfterTok=true;
	 }
       }
     }

     // Stop if we have hit a termination string

     if ((nfound >= minPtsNeeded) && (useTok == "TO") && (polygonToks[ii-1] == "DUE")) {
       break;
     }

     // Stop if we have hit the minPtsNeeded and this is only 1 (center case)

     if ((nfound >= minPtsNeeded) && (minPtsNeeded <=1)) {
       break;
     }

     // Stop if we have hit FCST & have the minPtsNeeded
     if ((nfound >= minPtsNeeded) && useTok == "FCST"){
       if (_params.debug >= Params::DEBUG_VERBOSE) {
	 cerr << "   Had enough points when we hit a FCST, so stop here." << endl;
       }
       break;
     }

     // If we see another flight level in here, it means there are two 
     // areas defined at two different flight levels.  Rather than try
     // to parse and handle two different polygons, give up (this should
     // cause the FIR to be used as the boundary instead).
     if (ii+1 <  polygonToks.size())
       if (_isFLTok(useTok,polygonToks[ii+1]) > 0){
	 if (foundOneFL || !inFCST){
	   if (_params.debug >= Params::DEBUG_VERBOSE) {
	     cerr << "   Found a second flight level -> not parsing points " << endl;
	   }
	   lats.clear();
	   lons.clear();
	   sourceIsLatLon.clear();
	   usedStartIdx=0;
	   usedEndIdx=0;
	   return -1;
	 }
	 //when in a FCST, the 1st FL is not already 'used', so we have to find 2 before giving up
	 foundOneFL = true;  
       }


     // Skip delimiter tokens

     bool skipToken=false;
     if (useTok == "-" || useTok == "/" || useTok == "," || useTok == "TO" || 
	 useTok == "PSN" || useTok == ":") {
       skipToken=true;
     }

     // try to parse the token

     size_t startTokIdx=ii;

     if (!skipToken) {

       double lat, lon, alt;
       string newTok;
       endInd=0;
     
       // easiest case -- lat lon

       if ((_hasLatLon(useTok, lat, lon) == true)) {
	 lats.push_back(lat);
	 lons.push_back(lon);
	 sourceIsLatLon.push_back(true);
	 found=true;
	 nfound++;

	 if (_params.debug >= Params::DEBUG_VERBOSE) {
	   cerr << "    tok: " << useTok << ", lat,lon: "
                << lat << ", " << lon << endl;
	 }

	 // If we are in the Canadian case, set the latlonsOnly to true so
	 // we do not contaminate points with stations

	 if ((ii>0) && (!latlonsOnly) && (!foundPsn) &&
	     (polygonToks[ii-1] == "/") && (polygonToks[ii+1] == "/")) {
	   latlonsOnly=true;
	 }
       }

       // lat lon in multiple tokens

       if (!found) {
	 if (_searchForLatLonString(ii, polygonToks, newTok, endInd)) {
	   if (_hasLatLon(newTok, lat, lon)) {
	     lats.push_back(lat);
	     lons.push_back(lon);
	     sourceIsLatLon.push_back(true);
	     found=true;
	     nfound++;
	     startTokIdx=ii;
	     ii=endInd;
	     if (_params.debug >= Params::DEBUG_VERBOSE) {
	       cerr << "    latlonNewTok: " << newTok << ", lat,lon: "
                    << lat << "," << lon << endl;
	     }
	   }
	 }
       }

       // a station and direction in multiple tokens, direction first
       // Need to do this BEFORE search for a station by itself because
       // of finding a station like: 40N when it really is a modifier

       if ((!found) && (!latlonsOnly)) {
	 if (_searchForStationString(ii, polygonToks, lat, lon, endInd)) {
	   lats.push_back(lat);
	   lons.push_back(lon);
	   sourceIsLatLon.push_back(false);
	   found=true;
	   nfound++;
	   startTokIdx=ii;
	   ii=endInd;
	 }
       }

       // a station by itself. Skip those less than 3 chars and those
       // that are only a direction. Directions are in the database
       // as stations (confusing!)

       if ((!found) && (useTok.size() > 2) && (!latlonsOnly) &&
           (!_isDirTok(useTok))) {

	 if (_searchForStation(_decoded.centroidSet(),
                               _decoded.getCentroidLat(), 
			       _decoded.getCentroidLon(),
                               useTok, lat, lon, alt)) {

	   lats.push_back(lat);
	   lons.push_back(lon);
	   sourceIsLatLon.push_back(false);
	   found=true;
	   nfound++;
	   if (_params.debug >= Params::DEBUG_VERBOSE) {
	     cerr << "     station: " << useTok << ", lat,lon: "
                  << lat << "," << lon << endl;
	   }
	 }

       } // if ((!found) ...

       if ((!found)
           && (_params.debug >= Params::DEBUG_VERBOSE)
           && !latlonsOnly) {
	 cerr << "     cannot find: " << useTok << endl;
       }

       // set the usedStartIdx and usedEndIdx. Need to set these to the
       // indices from the input toks[] array, not the polygon subset
       // array

       if (found || breakAfterTok) {
	 size_t idx;
	 size_t useStart=startIdx;
	 if (usedStartIdx >=0) {
	   useStart=usedStartIdx+1;
	 }
	 bool findIdx=_findMatchInToks(polygonToks[startTokIdx],
                                       useStart, endIdx, toks, idx);
	 if (findIdx && idx > 0) {
	   if (usedStartIdx <= 0) {
	     usedStartIdx=idx;
	   } else {
	     usedEndIdx=idx;
	     if (endInd > startTokIdx) {
	       usedEndIdx=idx + (endInd - startTokIdx);
	     }
	   }
	 }
       }

     } //endif !skipToken

     // Break if the flag is set. We found a termination character.

     if (breakAfterTok) {
       break;
     }
   } //ii

   // Include the possibleStartIdx in the used tokens

   if ((possibleStartIdx > 0) && (usedStartIdx > possibleStartIdx)) {
     usedStartIdx=possibleStartIdx;
   }

   // check for lat/lon and non lat/lon points

   bool foundLatLon = false;
   bool foundNonLatLon = false;
   for (int ii = 0; ii < (int) lats.size(); ii++) {
     if (sourceIsLatLon[ii]) {
       foundLatLon = true;
     } else {
       foundNonLatLon = true;
     }
   }

   // if mixed type, erase the non latlon ones
   
   if (foundLatLon && foundNonLatLon) {
     bool doAgain = true;
     while (doAgain) {
       doAgain = false;
       for (int ii = 0; ii < (int) lats.size(); ii++) {
         if (!sourceIsLatLon[ii]) {
           if (_params.debug >= Params::DEBUG_VERBOSE) {
             cerr << "Found mixed lat/lon and non-lat/lon points" << endl;
             cerr << "Removing pt lat, lon: "
                  << lats[ii] << ", " << lons[ii] << endl;
           }
           lats.erase(lats.begin() + ii);
           lons.erase(lons.begin() + ii);
           sourceIsLatLon.erase(sourceIsLatLon.begin() + ii);
           doAgain = true;
           break;
         }
       } // ii
     } // while
   } // if (foundLatLon && foundNonLatLon)

   // return the number of points 

   return((int) lats.size());
}


