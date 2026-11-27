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
// Server.cc
//
// File Server object
//
// Niles Oien, copying from Nancy Rehak,
//  RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 2003
//
///////////////////////////////////////////////////////////////


#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <euclid/GridPoint.hh>
#include <vector>
#include <set>
#include <stdio.h> // For the formatting of the time labels
#include <stdio.h>
#include <cstring>

#include "Server.hh"
#include "IconDef.hh"


/*********************************************************************
 * Constructor
 *
 * Inherits from DsSymprodServer
 */

Server::Server(const string &prog_name,
	       Params *initialParams) :
  DsSymprodServer(prog_name,
		  initialParams->instance,
		  (void*)(initialParams),
		  initialParams->port,
		  initialParams->qmax,
		  initialParams->max_clients,
		  initialParams->no_threads,
		  initialParams->debug >= Params::DEBUG_NORM,
		  initialParams->debug >= Params::DEBUG_VERBOSE),
  _symprodFontStyle(Symprod::TEXT_NORM)
{

  _locOccupied = NULL;
  _filter = NULL;

}

//////////////////////////////////////////////////////////////////////
// Destructor
//

Server::~Server()

{
  if (_locOccupied) {
    ufree2((void **) _locOccupied);
  }
  if (_filter) {
    delete _filter;
  }
}

/*********************************************************************
 * loadLocalParams() - Load local params if they are to be overridden.
 */

int Server::loadLocalParams(const string &paramFile,
			    void **serverParams)
{

  Params  *localParams;
  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;

  const string routine_name = "_allocLocalParams";

  if (_isDebug)
    cerr << "Loading new params from file: " << paramFile << endl;

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
                        _isVerbose) != 0)
  {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Cannot load parameter file: " << paramFile << endl;
    
    delete localParams;
    return -1;
  }

  
  if (_isVerbose) {
    localParams->print(stderr, PRINT_SHORT);
  }
  *serverParams = (void*)localParams;


  // Convert param values to Symprod object values

   // Check that the station position is within the bounding box, if requested
   
   if (localParams->useBoundingBox && !_horizLimitsSet) {
     _minLat = localParams->bounding_box.min_lat;
     _minLon = localParams->bounding_box.min_lon;
     _maxLat = localParams->bounding_box.max_lat;
     _maxLon = localParams->bounding_box.max_lon;
     _horizLimitsSet = true;
   }

   if (_isDebug && _horizLimitsSet){
     cerr << "Horizontal limits set." << endl;
     cerr << "  Min lat: " << _minLat << endl;
     cerr << "  Min lon: " << _minLon << endl;
     cerr << "  Max lat: " << _maxLat << endl;
     cerr << "  Max lon: " << _maxLon << endl;
     cerr << endl;
   }

   if (localParams->decimate_spatially) {
     if (_locOccupied) {
       ufree2((void **) _locOccupied);
     }
     _locOccupied = (bool **) ucalloc2(localParams->decimate_n_lat,
				      localParams->decimate_n_lon,
				      sizeof(bool));
   }
   
  // Convert param values to Symprod object values

  _symprodVertAlign = _vertAlign(localParams->text_vert_align);
  _symprodHorizAlign = _horizAlign(localParams->text_horiz_align);
  _symprodFontStyle = _fontStyle(localParams->text_font_style);

  _hiddenTextVertAlign = _vertAlign(localParams->hidden_text_vert_align);
  _hiddenTextHorizAlign = _horizAlign(localParams->hidden_text_horiz_align);
  _hiddenTextFontStyle = _fontStyle(localParams->hidden_text_font_style);


  // Update objects dependent on the local parameters

  for (int i = 0; i < localParams->wx_icon_defs_n; ++i)
  {
    // Create the list of icon points

    vector< GridPoint > point_list;
    
    char *x_string = strtok(localParams->_wx_icon_defs[i].icon_points, " ");
    char *y_string;
    
    if (x_string == (char *)NULL)
    {
      cerr << "ERROR: Server::" << routine_name << endl;
      cerr << "Error in icon_points string for icon " <<
	localParams->_wx_icon_defs[i].icon_name << endl;
      cerr << "The string must contain at least 1 point" << endl;
      
      continue;
    }
    
    bool string_error = false;
    
    while (x_string != (char *)NULL)
    {
      // Get the string representing the Y coordinate of the icon point

      y_string = strtok(NULL, " ");
      
      if (y_string == (char *)NULL)
      {
	cerr << "ERROR: Server::" << routine_name << endl;
	cerr << "Error in icon_points string for icon " <<
	  localParams->_wx_icon_defs[i].icon_name << endl;
	cerr << "The string must contain an even number of values" << endl;
      
	string_error = true;
	
	break;
      }
      
      // Convert the string values to points

      GridPoint point(atoi(x_string), atoi(y_string));
      point_list.push_back(point);
      
      // Get the string representing the X coordinate of the icon point

      x_string = strtok(NULL, " ");
      
    } /* endwhile - x_string != (char *)NULL */
    
    // See if there was an error in the icon point processing

    if (string_error)
      continue;
    
    // Create the icon definition object and add it to our list

    string icon_name = localParams->_wx_icon_defs[i].icon_name;
    
    IconDef *wx_icon_def = new IconDef(icon_name, point_list);
    _iconDefList[icon_name] = wx_icon_def;

  } /* endfor - i */

  // set uniqueness

  if (localParams->forecast_uniqueness == Params::UNIQUE_LATEST) {
    setUnique(Spdb::UniqueLatest);
  } else if (localParams->forecast_uniqueness == Params::UNIQUE_EARLIEST) {
    setUnique(Spdb::UniqueEarliest);
  } else  {
    setUnique(Spdb::UniqueOff);
  }
    
  return 0;
}


/////////////////////////////////////////////////////////////////////
// overload transformData()

void Server::transformData(const void *params,
			   const string &dir_path,
			   const int prod_id,
			   const string &prod_label,
			   const int n_chunks_in,
			   const Spdb::chunk_ref_t *chunk_refs_in,
			   const Spdb::aux_ref_t *aux_refs_in,
			   const void *chunk_data_in,
			   int &n_chunks_out,
			   MemBuf &refBufOut,
			   MemBuf &auxBufOut,
			   MemBuf &dataBufOut)
{
  
  _params = (Params*) params;

  // check prod_id

  if (prod_id != SPDB_TAF_ID) {
    cerr << "ERROR - " << _executableName
         << ":Server::transformData" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_TAF_ID: "
	 << SPDB_TAF_ID << endl;
    return;
  }
  
  // check the auxiliary XML to set limits
  if (_filter) {delete _filter; _filter = NULL;}
  if (_auxXml.size() != 0) {
    _filter = Filter::getCorrectFilter(_auxXml, _params);
    if (_filter && _filter->setRulesFromXml(_auxXml) < 0)
      { delete _filter; _filter = NULL; }
  }

  // set up vectors for accepted, rejected and required stations
  
  vector<si32> acceptedCodes;
  vector<si32> rejectedCodes;
  vector<si32> requiredCodes;

  if (_params->useAcceptedStationsList) {
    for (int i = 0; i < _params->acceptedStations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(_params->_acceptedStations[i]);
      acceptedCodes.push_back(code);
    } // i
  }
  
  if (_params->useRejectedStationsList) {
    for (int i = 0; i < _params->rejectedStations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(_params->_rejectedStations[i]);
      rejectedCodes.push_back(code);
    } // i
  }
  
  if (_params->decimate_spatially &&
      _params->decimate_required_stations_n > 0) {
    for (int i = 0; i < _params->decimate_required_stations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(_params->_decimate_required_stations[i]);
      requiredCodes.push_back(code);
    } // i
  }
  
  // initialize for transformation
  
  refBufOut.free();
  auxBufOut.free();
  dataBufOut.free();
  n_chunks_out = 0;
  MemBuf symprodBuf;
  
  // if spatial decimation is on, go through the list once, 
  // transforming only those stations on the required list

  vector<int> chunkDone;
  
  if (_horizLimitsSet && requiredCodes.size() > 0) {
    
    for (int ic = n_chunks_in - 1; ic >= 0; ic--) {

      Spdb::chunk_ref_t ref = chunk_refs_in[ic];
      Spdb::aux_ref_t aux = aux_refs_in[ic];

      for (size_t is = 0; is < requiredCodes.size(); is++) {

	if (requiredCodes[is] == ref.data_type) {

	  void *chunk_data = (void *)((char *)chunk_data_in + ref.offset);
	  symprodBuf.free();
	  if (convertToSymprod(_params, dir_path, prod_id, prod_label,
			       ref, aux, chunk_data, ref.len,
			       symprodBuf) == 0) {
	    ref.offset = dataBufOut.getLen();
	    ref.len = symprodBuf.getLen();
	    refBufOut.add(&ref, sizeof(ref));
	    auxBufOut.add(&aux, sizeof(aux));
	    dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());
	    n_chunks_out++;
	  }
	  chunkDone.push_back(ic);
	  
	} // if (stationCodes[is] == ref.data_type)

      } // is

    } // ic

  } // if (_horizLimitsSet && requiredCodes.size() > 0)

  if (_isDebug) {
    cerr << "Sending required list n stations: " << n_chunks_out << endl;
  }

  // go through list again for rest of stations
    
  for (int ic = n_chunks_in - 1; ic >= 0; ic--) {

    // skip any already done

    if (chunkDone.size() > 0) {
      for (size_t ii = 0; ii < chunkDone.size(); ii++) {
	if (ic == chunkDone[ii]) {
	  continue;
	}
      }
    }
    
    Spdb::chunk_ref_t ref = chunk_refs_in[ic];
    Spdb::aux_ref_t aux = aux_refs_in[ic];

    // check for acceptance

    bool accept = false;
    if (acceptedCodes.size() == 0) {
      accept = true;
    } else {
      for (size_t is = 0; is < acceptedCodes.size(); is++) {
	if (acceptedCodes[is] == ref.data_type) {
	  accept = true;
	  break;
	}
      }
    }
    if (accept && rejectedCodes.size() > 0) {
      for (size_t is = 0; is < rejectedCodes.size(); is++) {
	if (rejectedCodes[is] == ref.data_type) {
	  accept = false;
	  break;
	}
      }
    }

    if (accept) {
      void *chunk_data = (void *)((char *)chunk_data_in + ref.offset);
      symprodBuf.free();
      if (convertToSymprod(_params, dir_path, prod_id, prod_label,
			   ref, aux, chunk_data, ref.len,
			   symprodBuf) == 0) {
	ref.offset = dataBufOut.getLen();
	ref.len = symprodBuf.getLen();
	refBufOut.add(&ref, sizeof(ref));
	auxBufOut.add(&aux, sizeof(aux));
	dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());
	n_chunks_out++;
      }
    } // if (accept)
    
  } // ic

  if (_isDebug) {
    cerr << "Processing data for n TAFs: " << n_chunks_out << endl;
  }

}

/*********************************************************************
 * convertToSymprod() - Convert the given data chunk from the SPDB
 *                      database to symprod format.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
                             const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
{

  // check prod_id
  
  if (prod_id != SPDB_TAF_ID) {
    cerr << "ERROR - " << _executableName
         << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_TAF_ID: " << SPDB_TAF_ID << endl;
    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *_params = (Params*) params;

  // set the request time

  _requestTime = _readMsg.getRequestTime();

  // disassemble the message into a taf

  _taf.disassemble(spdb_data, spdb_len);

  // Check that the station position is within the bounding box, if requested

  if (_horizLimitsSet) {

    // Normalize the station longitude

    double station_lat = _taf.getLatitude();
    double station_lon = _taf.getLongitude();

    while (station_lon < _minLon){
      station_lon += 360.0;
    }
    while (station_lon >= _minLon + 360.0){
      station_lon -= 360.0;
    }

    _taf.setLongitude(station_lon);

    // See if the station lies within the requested bounding box

    bool accept = false;

    if (station_lat >= _minLat && station_lat <= _maxLat &&
	station_lon >= _minLon && station_lon <= _maxLon) {
	accept = true;
    }

    // See if the station is one of the required ones

    if (!accept) {
      for (int is = 0;
	   is < _params->decimate_required_stations_n; is++) {
	if (Spdb::hash4CharsToInt32
            (_params->_decimate_required_stations[is])
	    == chunk_ref.data_type) {
	  accept = true;
	}
      } // is
    }

    if (!accept) {
      return -1;
    }

    if (_params->decimate_spatially && _locOccupied) {

      // decimation is active, so check to make sure that there is 
      // not already a metar close by

      int ilat = (int) (((station_lat - _minLat) /
			 (_maxLat - _minLat))
			* _params->decimate_n_lat);
      if (ilat < 0) {
	ilat = 0;
      }
      if (ilat > _params->decimate_n_lat - 1) {
	ilat = _params->decimate_n_lat - 1;
      }

      int ilon = (int) (((station_lon - _minLon) /
			 (_maxLon - _minLon))
			* _params->decimate_n_lon);
      if (ilon < 0) {
	ilon = 0;
      }
      if (ilon > _params->decimate_n_lon - 1) {
	ilon = _params->decimate_n_lon - 1;
      }
      
      if (_locOccupied[ilat][ilon]) {
	if (_isVerbose) {
	  cerr << "---->> Reject: lat, lon, ilat, ilon: "
	       << station_lat << ", " << station_lon << ", "
	       << ilat << ", " << ilon
	       << endl;
	}
	return -1; // do not process this one.
      }
      if (_isVerbose) {
	cerr << "---->> Accept: lat, lon, ilat, ilon: "
	     << station_lat << ", " << station_lon << ", "
	     << ilat << ", " << ilon
	     << endl;
      }
      _locOccupied[ilat][ilon] = true;

    }

  } // if (_params->useBoundingBox)

  // copy period data into the local members

  const vector<Taf::ForecastPeriod> &periods = _taf.getPeriods();
  if (periods.size() < 1) {
    cerr << "ERROR - " << _executableName
         << ":Server::convertToSymprod" << endl;
    cerr << "  Bad TAF, no forecast periods" << endl;
    cerr << "  " << _taf.getText() << endl;
    return -1;
  }

  // set initial normal period and tempo periods

  _normalPeriod = periods[0];
  _tempoActive = false;
  for (int ii = 0; ii < (int) periods.size(); ii++) {
    const Taf::ForecastPeriod &period = periods[ii];
    if (period.pType == Taf::PERIOD_TEMPO) {
      _tempoActive = true;
      _tempoPeriod = period;
    }
  }
  
  // override normal and tempo periods if request time lies
  // within their start and end times
  
  for (int ii = 0; ii < (int) periods.size(); ii++) {
    const Taf::ForecastPeriod &period = periods[ii];
    if (_requestTime >= period.startTime &&
        _requestTime <= period.endTime) {
      if (period.pType == Taf::PERIOD_FROM ||
          period.pType == Taf::PERIOD_BECMG) {
        _normalPeriod = period;
      } else if (period.pType == Taf::PERIOD_TEMPO) {
        _tempoPeriod = period;
      }
    }
  }

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_TAF_LABEL);

  // add data for normal period, if requested

  if (_params->plot_normal_periods) {
    _addForPeriod(prod, _normalPeriod);
  }
  
  // add data for tempo period, if requested
  
  if (_params->plot_tempo_periods && _tempoActive) {
    _addForPeriod(prod, _tempoPeriod);
  }

  // set return buffer

  if (_isVerbose) {
    prod.print(cerr);
  }
  
  prod.serialize(symprod_buf);

  return 0;
}

//////////////////////////////////////////////////
// add products for the specified forecast period

void Server::_addForPeriod(Symprod &prod,
                           const Taf::ForecastPeriod &period)

{

  if (_params->draw_location_icon) {
    _addLocationIcon(prod);
  }

  int fcatIndex = _findFlightCatIndex(period);

  if(_params->draw_wind_barb) {
    _addWindBarb(prod, period, false, fcatIndex);
  }

  if(_params->draw_gust_barb) {
    _addWindBarb(prod, period, true, fcatIndex);
  }

  if (_params->activate_hidden_text) {
    _addHiddenText(prod, period, fcatIndex);
  }
  
  if (_params->draw_flight_category) {
    _addFlightCat(prod, period, fcatIndex);
  }
  
  _addLabels(prod, period, fcatIndex);

  if (_params->draw_wx_icon) {
    _addIcon(prod, period);
  }
  
}

//////////////////////////////////
// find the flight category index
//
// Returns index on success, -1 on failure

int Server::_findFlightCatIndex(const Taf::ForecastPeriod &period)
  
{
  
  // check for missing values (-9999)
  
  if (period.ceilingKm < -9990 || period.visKm < -9990) {
    return -1;
  }
  
  double ceiling = period.ceilingKm * 3048.0;
  double vis = period.visKm;
  
  for (int i = 0; i < _params->flight_category_n; i++) {
    if (ceiling >= _params->_flight_category[i].ceiling_threshold &&
	vis >= _params->_flight_category[i].visibility_threshold) {
      return i;
    }
  }

  return -1;

}

////////////////////////////////////////////////////////////////////
// _addWindBarb()
//
// Add wind barb

void Server::_addWindBarb(Symprod &prod,
			  const Taf::ForecastPeriod &period,
                          bool plotGust,
			  int fcatIndex)
  
{

  if (plotGust && (period.windGustKmh <= period.windSpeedKmh)) {
    return;
  }

  int detail_level = 0;
  if (_params->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }
  
  char *color = _params->wind_barb_color;
  if (_params->override_wind_barb_color_from_flight_cat &&
      fcatIndex >= 0) {
    color = _params->_flight_category[fcatIndex].color;
  }
  if (plotGust) {
    color = _params->gust_barb_color;
  }

  // get speed in desired units

  double speed_to_plot = 0;
  double speedKmh = period.windSpeedKmh;
  if (plotGust) {
    speedKmh = period.windGustKmh;
  }

  switch( _params->wind_speed_units) {
    case Params::WIND_MS:
      speed_to_plot = speedKmh / MPERSEC_TO_KMPERHOUR;
      break;
      
    case Params::WIND_KTS:
      speed_to_plot = speedKmh / KM_PER_NM;
      break;
      
    case Params::WIND_MPH:
      speed_to_plot = speedKmh / KM_PER_MI;
      break;
      
    case Params::WIND_KPH:
      speed_to_plot = speedKmh;
      break;
  }

  // abort if wind value is too high

  if (speed_to_plot > 999.0) {
    return;
  }

  //
  // Color code by wind speed, if desired.
  //
  if (_params->override_wind_barb_color_from_wind_speed) {
    for (int k=0; k < _params->wind_speed_colors_n; k++){
      if (
	  (speed_to_plot >= _params->_wind_speed_colors[k].min) &&
	  (speed_to_plot <= _params->_wind_speed_colors[k].max)
	  ){
	color = _params->_wind_speed_colors[k].color;
	break;
      }
    }
  }

  //need this because we might want to use the color set from the filter
  //and that is a const char*
  const char* constColor = color;	
  if (_filter)
      constColor = _filter->getColor();

  Symprod::wpt_t origin;
  origin.lat = _taf.getLatitude();
  origin.lon = _taf.getLongitude();
  int shaft_len = _params->wind_barb_shaft_len;

  if (_params->render_missing_ceiling_icon && period.ceilingKm < -9990) {
    
    // plot missing ceiling icon
    
    int nIconPts = _params->missing_ceiling_icon_n;
    double iconScale = _params->missing_ceiling_icon_scale;
    Symprod::ppt_t *iconPts = new Symprod::ppt_t[nIconPts];
    for (int ii = 0; ii < nIconPts; ii++) {
      iconPts[ii].x = (int)
        floor(_params->_missing_ceiling_icon[ii].x * iconScale + 0.5);
      iconPts[ii].y = (int)
        floor(_params->_missing_ceiling_icon[ii].y * iconScale + 0.5);
    }

    prod.addIconline(_taf.getLatitude(), _taf.getLongitude(),
                     nIconPts, iconPts,
                     _params->missing_ceiling_icon_color,		     
                     Symprod::LINETYPE_SOLID,
                     _params->missing_ceiling_icon_line_width,
                     Symprod::CAPSTYLE_BUTT,
                     Symprod::JOINSTYLE_BEVEL, false,
                     Symprod::FILL_NONE,
                     0, detail_level);
    
  } else {
  
    // add circle to show location
    if (_params->station_posn_circle_radius > 0) {
      prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
                  _params->station_posn_circle_radius,
                  _params->station_posn_circle_radius,
                  constColor, true,
                  0, 360.0, 0.0, 60,		     
                  Symprod::LINETYPE_SOLID, 1,
                  Symprod::CAPSTYLE_BUTT,
                  Symprod::JOINSTYLE_BEVEL,
                  Symprod::FILL_NONE,
                  0, detail_level);
    }

  }

  // return now if speed below 2.5

  if (speed_to_plot < 2.5) {
    return;
  }

  // add shaft
  
  double dirrad = period.windDirnDegT * DEG_TO_RAD;
  double cosdir = cos(dirrad);
  double sindir = sin(dirrad);
  
  Symprod::ppt_t shaft[2];
  shaft[0].x = 0;
  shaft[0].y = 0;
  shaft[1].x = (int) floor(shaft_len * sindir + 0.5);
  shaft[1].y = (int) floor(shaft_len * cosdir + 0.5);
  
  prod.addIconline(_taf.getLatitude(), _taf.getLongitude(),
		   2, shaft,
		   constColor,		     
		   Symprod::LINETYPE_SOLID,
		   _params->wind_barb_line_width,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL, false,
		   Symprod::FILL_NONE,
		   0, detail_level);

  // add flags for each 50 kts

  double tick_rel_angle;
  if (_taf.getLatitude() >= 0.0) {
    tick_rel_angle = _params->wind_ticks_angle_to_shaft;
  } else {
    tick_rel_angle = -_params->wind_ticks_angle_to_shaft;
  }
  double tick_rad = dirrad + tick_rel_angle * DEG_TO_RAD;
  double tick_len = _params->wind_barb_tick_len;
  double tick_point_offset_x = tick_len * sin(tick_rad);
  double tick_point_offset_y = tick_len * cos(tick_rad);
  
  double tick_spacing;
  if (speed_to_plot < 150.0) {
    tick_spacing = shaft_len / 7.5;
  } else {
    tick_spacing = shaft_len / 10.0;
  }
  if (tick_spacing < 3.0) {
    tick_spacing = 3.0;
  }
  double tick_dx = tick_spacing * sindir;
  double tick_dy = tick_spacing * cosdir;
  
  // Add the speed markings

  double speed_left = speed_to_plot;
  double shaft_left = shaft_len;

  double x_inner = shaft_left * sindir;
  double y_inner = shaft_left * cosdir;

  // flags for each 50 kts

  while (speed_left >= 47.5) {
    
    double x_outer = x_inner;
    double y_outer = y_inner;

    x_inner -= tick_dx;
    y_inner -= tick_dy;

    double x_point = x_outer + tick_point_offset_x;
    double y_point = y_outer + tick_point_offset_y;

    Symprod::ppt_t flag[3];
    flag[0].x = (int) floor(x_outer + 0.5);
    flag[0].y = (int) floor(y_outer + 0.5);
    flag[1].x = (int) floor(x_point + 0.5);
    flag[1].y = (int) floor(y_point + 0.5);
    flag[2].x = (int) floor(x_inner + 0.5);
    flag[2].y = (int) floor(y_inner + 0.5);
    prod.addIconline(_taf.getLatitude(), _taf.getLongitude(),
		     3, flag,
		     constColor,
		     Symprod::LINETYPE_SOLID,
		     _params->wind_barb_line_width,
		     Symprod::CAPSTYLE_BUTT,
		     Symprod::JOINSTYLE_BEVEL,
		     true,
		     Symprod::FILL_SOLID,		     
		     0, detail_level);

    speed_left -= 50.0;
    shaft_left =- tick_spacing;

  } // while (speed_left >= 47.5)

  // full ticks for each 10 kts
  // space away from the 50 kt flags

  if (speed_to_plot > 47.5) {
    x_inner -= tick_dx / 2.0;
    y_inner -= tick_dy / 2.0;
  }

  while (speed_left >= 7.5) {
    
    double x_outer = x_inner;
    double y_outer = y_inner;

    x_inner -= tick_dx;
    y_inner -= tick_dy;

    double x_point = x_outer + tick_point_offset_x;
    double y_point = y_outer + tick_point_offset_y;

    Symprod::ppt_t full_tick[2];
    full_tick[0].x = (int) floor(x_outer + 0.5);
    full_tick[0].y = (int) floor(y_outer + 0.5);
    full_tick[1].x = (int) floor(x_point + 0.5);
    full_tick[1].y = (int) floor(y_point + 0.5);
    prod.addIconline(_taf.getLatitude(), _taf.getLongitude(),
		     2, full_tick,
		     constColor,		     
		     Symprod::LINETYPE_SOLID,
 		     _params->wind_barb_line_width,
		     Symprod::CAPSTYLE_BUTT,
		     Symprod::JOINSTYLE_BEVEL, false,
		     Symprod::FILL_NONE,
		     0, detail_level);
    
    speed_left -= 10.0;
    shaft_left =- tick_spacing;

  } // while (speed_left >= 7.5)

  // half ticks for each 5 kts

  // single 5-kt barb is rendered in from the end

  if (speed_to_plot < 7.5) {
    x_inner -= tick_dx;
    y_inner -= tick_dy;
  }
  
  if (speed_left >= 2.5) {
    
    double x_point = x_inner + tick_point_offset_x / 2.0;
    double y_point = y_inner + tick_point_offset_y / 2.0;
    
    Symprod::ppt_t half_tick[2];
    half_tick[0].x = (int) floor(x_inner + 0.5);
    half_tick[0].y = (int) floor(y_inner + 0.5);
    half_tick[1].x = (int) floor(x_point + 0.5);
    half_tick[1].y = (int) floor(y_point + 0.5);
    prod.addIconline(_taf.getLatitude(), _taf.getLongitude(),
		     2, half_tick,
		     constColor,		     
		     Symprod::LINETYPE_SOLID,
 		     _params->wind_barb_line_width,
		     Symprod::CAPSTYLE_BUTT,
		     Symprod::JOINSTYLE_BEVEL, false,
		     Symprod::FILL_NONE,
		     0, detail_level);

  } // if (speed_left >= 5)

}


/////////////////////////
// add flight category

void Server::_addFlightCat(Symprod &prod,
			   const Taf::ForecastPeriod &period,
                           int fcatIndex)
  
{
  
  int detail_level = 0;
  if (_params->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }
  
  if (fcatIndex < 0) {
    double vis = period.visKm;
    if (vis < 0) {
      // convert from extinction coeff to visibility
      vis = -3.0 / vis;
    }
    if (_isVerbose) {
      cerr << "WARNING - no flight cat found." << endl;
      cerr << "  Station: " << _taf.getStationId() << endl;
      cerr << "  Ceiling (km): " << period.ceilingKm << endl;
      cerr << "  Visibility (km): " << vis << endl;
    }
    return;
  }

  // compute fraction covered
  
  double fractionCovered = 0.0;
  for (int ii = 0; ii < (int) period.layers.size(); ii++) {
    if (fractionCovered < period.layers[ii].cloudCover) {
      fractionCovered = period.layers[ii].cloudCover;
    }
  }
  
  if (fractionCovered > 0.99) {
    
    // obscured
    
    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		0, 360.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);

  } else if (fractionCovered > 0.74) {
    
    // broken - 3/4 circle

    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		180.0, 450.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);
    
    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		90.0, 180.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);
    
  } else if (fractionCovered > 0.49) {

    // scattered - half circle
    
    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		270.0, 450.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);

    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		90.0, 270.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  } else if (fractionCovered > 0.24) {

    // few - quarter circle
    
    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		0.0, 90.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);

    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		90.0, 360.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  } else if (fractionCovered > -0.01) {
    
    // clear
    
    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		0, 360.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  } else {
    
    // no cloud coverage mentioned
    
    prod.addArc(_taf.getLatitude(), _taf.getLongitude(),
		_params->flight_category_circle_radius,
		_params->flight_category_circle_radius,
		_params->_flight_category[fcatIndex].color, true,
		0, 360.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  }

}

/////////////////////////
// add hidden text

void Server::_addHiddenText(Symprod &prod,
			   const Taf::ForecastPeriod &period,
                           int fcatIndex)
  
{
  
  // add full METAR message as hidden text
  
  char *foreground_color = _params->hidden_text_foreground_color;
  if (strlen(foreground_color) == 0 && fcatIndex >= 0) {
    foreground_color = _params->_flight_category[fcatIndex].color;
  }

  prod.addText(_taf.getText().c_str(),
               _taf.getLatitude(),
               _taf.getLongitude(),
               foreground_color,
               _params->hidden_text_background_color,
               _params->hidden_text_x_offset,
               _params->hidden_text_y_offset,
               _hiddenTextVertAlign,
               _hiddenTextHorizAlign,
               _params->hidden_text_font_size,
               _hiddenTextFontStyle,
               _params->hidden_text_font_name,
               0, // Object ID
               Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
               Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
               Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS);
}

////////////////////////////////////////////////////////////////
// draw location icon

void Server::_addLocationIcon(Symprod &prod)

{

  // create ICON shape
  
  int nPointsIcon = _params->location_icon_points_n;
  double iconScale = _params->location_icon_scale;
  TaArray<Symprod::ppt_t> _ppts;
  Symprod::ppt_t *ppts = _ppts.alloc(nPointsIcon);
  for (int ii = 0; ii < nPointsIcon; ii++) {
    Params::stroked_icon_point_t point = _params->_location_icon_points[ii];
    if (point.x == -999 || point.y == -999) {
      ppts[ii].x = Symprod::PPT_PENUP;
      ppts[ii].y = Symprod::PPT_PENUP;
    } else {
      if (iconScale != 1.0) {
        double scaledX = point.x * iconScale;
        double scaledY = point.y * iconScale;
        ppts[ii].x = (int) floor(scaledX + 0.5);
        ppts[ii].y = (int) floor(scaledY + 0.5);
      } else {
        ppts[ii].x = point.x;
        ppts[ii].y = point.y;
      }
    }
  } // ii

  Symprod::wpt_t icon_origin;
  icon_origin.lat = _taf.getLatitude();
  icon_origin.lon = _taf.getLongitude();
  
  Symprod::detail_level_t detailLevel = Symprod::DETAIL_LEVEL_NONE;
  if (!_params->location_icon_allow_client_scaling) {
    detailLevel = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }
  
  prod.addStrokedIcons(_params->location_icon_color,
                       nPointsIcon,
                       ppts,
                       1,
                       &icon_origin,
                       0,
                       detailLevel,
                       _params->location_icon_line_width);

}

/////////////////////////
// add text labels

void Server::_addLabels(Symprod &prod,
                        const Taf::ForecastPeriod &period,
                        int fcatIndex)
  
{
  
  // Current weather - as string
  
  if (_params->weather_label.do_draw && period.wx.size() > 0) {
    string wx;
    for (int ii = 0; ii < (int) period.wx.size(); ii++) {
      if (ii != 0) {
        wx += " ";
      }
      wx += period.wx[ii];
    }
    prod.addText(wx.c_str(),
                 _taf.getLatitude(), _taf.getLongitude(),
                 _params->weather_label.color,
                 _params->weather_label.background_color,
                 _params->weather_label.x_offset,
                 _params->weather_label.y_offset,
                 (Symprod::vert_align_t)
                 _params->weather_label.vert_align,
                 (Symprod::horiz_align_t)
                 _params->weather_label.horiz_align,
                 _params->weather_label.font_size,
                 Symprod::TEXT_NORM,
                 _params->weather_label.font_name);
  }

  // Wind gust

  double windgust = period.windGustKmh;
  if (_params->wind_gust_label.do_draw && windgust > 0) {
    char text[64];
    sprintf(text, "G%d", (int) (windgust / KM_PER_NM + 0.5));
    prod.addText(text,
		 _taf.getLatitude(), _taf.getLongitude(),
		 _params->wind_gust_label.color,
		 _params->wind_gust_label.background_color,
		 _params->wind_gust_label.x_offset,
		 _params->wind_gust_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->wind_gust_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->wind_gust_label.horiz_align,
		 _params->wind_gust_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->wind_gust_label.font_name);
  }

  // Ceiling

  double ceiling = period.ceilingKm;
  if (_params->ceiling_label.do_draw && ceiling >= 0) {
    char text[64];
    if (ceiling > 9.5) {
      text[0] = '\0';
    } else {
      switch (_params->ceiling_units) {
        case Params::CEILING_METERS: {
          double cm = ceiling * 1000.0;
          if (cm > 1000) {
            cm = ((int) (cm / 100.0 + 0.5)) * 100.0;
          } else {
            cm = ((int) (cm / 10.0 + 0.5)) * 10.0;
          }
          sprintf(text, "c%dm", (int) cm);
        }
          break;
        case Params::CEILING_KM: {
          double ckm = ceiling;
          ckm = ((int) (ckm / 0.010 + 0.5)) * 0.010;
          sprintf(text, "c%gkm", ckm);
        }
          break;
        case Params::CEILING_FT: {
          double cft =
            _nearest((ceiling / KM_PER_MI * FT_PER_MI) + 0.5, 100.0);
          sprintf(text, "c%dft", (int) cft);
        }
          break;
        case Params::CEILING_KFT: {
          double cft =
            _nearest((ceiling / KM_PER_MI * FT_PER_MI) + 0.5, 100.0);
          double ckft = cft / 1000.0;
          sprintf(text, "c%gkft", ckft);
        }
          break;
        case Params::CEILING_FL: {
          double cft =
            _nearest((ceiling / KM_PER_MI * FT_PER_MI) + 0.5, 100.0);
          int fl = (int) (cft / 100.0);
          sprintf(text, "c%.3d", fl);
        }
          break;
      } // switch
    }
    prod.addText(text,
		 _taf.getLatitude(), _taf.getLongitude(),
		 _params->ceiling_label.color,
		 _params->ceiling_label.background_color,
		 _params->ceiling_label.x_offset,
		 _params->ceiling_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->ceiling_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->ceiling_label.horiz_align,
		 _params->ceiling_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->ceiling_label.font_name);
  }

  // Visibility
  
  if (_params->visibility_label.do_draw) {
    char text[64];
    double vis = period.visKm;
    if (vis < 0) {
      // convert from extinction coeff to visibility
      vis = -3.0 / vis;
    }
    if (vis > 20) {
      text[0] = '\0';
    } else {
      switch (_params->visibility_units) {
        case Params::VISIBILITY_METERS: {
          double vm = vis * 1000.0;
          if (vm > 1000) {
            vm = ((int) (vm / 100.0 + 0.5)) * 100.0;
          } else {
            vm = ((int) (vm / 10.0 + 0.5)) * 10.0;
          }
          sprintf(text, "v%gm", vm);
        }
          break;
        case Params::VISIBILITY_KM: {
          double vkm = vis;
          vkm = ((int) (vkm / 0.010 + 0.5)) * 0.010;
          sprintf(text, "v%gkm", vkm);
        }
          break;
        case Params::VISIBILITY_METERS_KM: {
          double vkm = vis;
          if (vkm > 5.0) {
            vkm = ((int) (vkm / 0.010 + 0.5)) * 0.010;
            sprintf(text, "v%gkm", vkm);
          } else {
            double vm = vis * 1000.0;
            if (vm > 1000) {
              vm = ((int) (vm / 100.0 + 0.5)) * 100.0;
            } else {
              vm = ((int) (vm / 10.0 + 0.5)) * 10.0;
            }
            sprintf(text, "v%gm", vm);
          }
        }
          break;
        case Params::VISIBILITY_MILES: {
          double vmiles = vis / KM_PER_MI;
          vmiles = ((int) (vmiles / 0.1 + 0.5)) * 0.1;
          sprintf(text, "v%gmi", vmiles);
        }
          break;
      } // switch
    }
    prod.addText(text,
		 _taf.getLatitude(), _taf.getLongitude(),
		 _params->visibility_label.color,
		 _params->visibility_label.background_color,
		 _params->visibility_label.x_offset,
		 _params->visibility_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->visibility_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->visibility_label.horiz_align,
		 _params->visibility_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->visibility_label.font_name);
  }

  // Max Temperature
  
  if (_params->max_temp_label.do_draw && period.maxTempC > -9990.0) {
    
    double maxTemp = period.maxTempC;
    if (_params->display_temp == Params::DISPLAY_TEMP_F) {
      maxTemp = (9.0/5.0) * maxTemp + 32.0;
    }
    
    char text[64];
    sprintf(text, "%g", maxTemp);
    
    if (_isVerbose){
      cerr << "Temperature point text : " << text << endl;
    }

    prod.addText(text, _taf.getLatitude(), _taf.getLongitude(),
		 _params->max_temp_label.color,
		 _params->max_temp_label.background_color,
		 _params->max_temp_label.x_offset,
		 _params->max_temp_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->max_temp_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->max_temp_label.horiz_align,
		 _params->max_temp_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->max_temp_label.font_name);
  }

  // Min Temperature
  
  if (_params->min_temp_label.do_draw && period.minTempC > -9990.0) {
    
    double minTemp = period.minTempC;
    if (_params->display_temp == Params::DISPLAY_TEMP_F) {
      minTemp = (9.0/5.0) * minTemp + 32.0;
    }
    
    char text[64];
    sprintf(text, "%g", minTemp);
    
    if (_isVerbose){
      cerr << "Temperature point text : " << text << endl;
    }

    prod.addText(text, _taf.getLatitude(), _taf.getLongitude(),
		 _params->min_temp_label.color,
		 _params->min_temp_label.background_color,
		 _params->min_temp_label.x_offset,
		 _params->min_temp_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->min_temp_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->min_temp_label.horiz_align,
		 _params->min_temp_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->min_temp_label.font_name);
  }

  // station name

  if (_params->station_name_label.do_draw) {
    char text[64];
    int offset =
      (int) _taf.getStationId().size() - _params->station_name_label_len;
    if (offset < 0) {
      offset = 0;
    }
    strcpy(text, _taf.getStationId().c_str() + offset);
    prod.addText(text,
		 _taf.getLatitude(), _taf.getLongitude(),
		 _params->station_name_label.color,
		 _params->station_name_label.background_color,
		 _params->station_name_label.x_offset,
		 _params->station_name_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->station_name_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->station_name_label.horiz_align,
		 _params->station_name_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->station_name_label.font_name);
  }

  // issue time

  if (_params->issue_time_label.do_draw) {
    char text[64];
    sprintf(text, "%s", utimstr(_taf.getIssueTime()));
    prod.addText(text,
		 _taf.getLatitude(), _taf.getLongitude(),
		 _params->issue_time_label.color,
		 _params->issue_time_label.background_color,
		 _params->issue_time_label.x_offset,
		 _params->issue_time_label.y_offset,
		 (Symprod::vert_align_t)
		 _params->issue_time_label.vert_align,
		 (Symprod::horiz_align_t)
		 _params->issue_time_label.horiz_align,
		 _params->issue_time_label.font_size,
		 Symprod::TEXT_NORM,
		 _params->issue_time_label.font_name);
  }

}

/////////////////////////
// add icon

void Server::_addIcon(Symprod &prod,
                      const Taf::ForecastPeriod &period)
  
{

  int iMatch = -1;
  string wxStr;
  for (int ii = 0; ii < (int) period.wx.size(); ii++) {
    wxStr += period.wx[ii];
  }
  
  for (int iw=0; iw < _params->wx_items_n; iw++) {
    if (_wildCard(_params->_wx_items[iw].WxWildcard,
		  wxStr.c_str(), false)) {
      iMatch = iw;
      break;
    }
  }
  
  if (iMatch == -1) {
    return;
  }

  // set params

  bool renderIcon = _params->_wx_items[iMatch].renderIcon;
  if (!renderIcon) {
    return;
  }

  char *iconName = _params->_wx_items[iMatch].iconName;
  char *iconNameSouth = _params->_wx_items[iMatch].iconNameSouth;
  if (_taf.getLatitude() < 0 && strlen(iconNameSouth) > 0) {
    iconName = iconNameSouth;
  }

  float iconScale = _params->_wx_items[iMatch].iconScale;
  bool allowClientScaling =
    _params->_wx_items[iMatch].allowClientScaling;
  const char *color = _params->_wx_items[iMatch].color;

  _drawIcon(iconName, color, 
            _taf.getLatitude(), _taf.getLongitude(),
            iconScale, allowClientScaling, prod);

}

///////////////////////////////////////////
// get nearest value, given a resolution

double Server::_nearest(double target, double delta)
{

  double answer;
  double rem;                                                                 
  
  delta = fabs(delta);                   
  rem = remainder(target,delta);
  
  if(target >= 0.0) {
    if(rem > (delta / 2.0)) {
      answer = target + (delta - rem);
    } else {
      answer = target -  rem;
    }
  } else {
    if(fabs(rem) > (delta / 2.0)) {
      answer = target - (delta + rem);
    } else {
      answer = target -  rem;
    }
  }
  
  return answer;

}

//////////////////////////////////////////////////////////////////////
// Wildcard checker.

int Server::_wildCard(const char *WildSpec, const char *String, bool debug){

  //
  // WildCard returns zero if a wild card specification is not
  // matched, 1 if there is a match.
  //
  // You pass in :
  //
  // * WildSpec - the wild carded specification, ie.
  //              "*.dat" or "20*.mdv"
  //
  // * String - the string to test against the wildcard,
  //            ie. "Station.dat" or "200402.mdv"
  //
  //
  // * debug - if non-zero, then messages are printed to stderr
  //           explaining the string parsing as it happens.
  //


  int istart = 0;
  int iend = 0;

  const char *searchString = String;

  do {
    //
    // Position istart at first non-wildcard.
    //

    char buffer[1024];
    do{
      //
      // If it is a wildcard, skip it.
      //
      if (WildSpec[istart] == '*'){
	istart++;
      }

      //
      // If we have reached the end of the string, return OK.
      //
      if (WildSpec[istart] == char(0)){
	return 1;
      }

      //
      // If it is not a wildcard, copy it into the buffer.
      //
      if (
	  (WildSpec[istart] != '*') &&
	  (WildSpec[istart] != char(0))
	  ){
	//
	// Find the end of this item.
	//
	iend = istart;
	do {
	  iend++;

	} while (
		 (WildSpec[iend] != '*') &&
		 (WildSpec[iend] != char(0))
		 );

	for (int i=istart; i < iend; i++){
	  buffer[i-istart] = WildSpec[i];
	  buffer[i-istart+1] = char(0); 
	}

	if (debug) fprintf(stderr,"ITEM : %s : ",buffer);

	int MustMatchExact = 0;
	if (WildSpec[iend] == char(0)){
	   MustMatchExact = 1;
	}


	if (MustMatchExact){
	  if (debug) fprintf(stderr, "MUST MATCH EXACT : ");

	  if (strlen(searchString) <  strlen(buffer)){
	    //
	    // What we are looking for cannot be here - string too short.
	    //
	    if (debug) fprintf(stderr, "IT CANNOT.\n");
	    return 0;
	  }

	  const char *p = searchString + strlen(searchString) - strlen(buffer);
	  if (debug) fprintf(stderr,"%s cf. %s : ",
		  buffer, p);
	  if (!(strcmp(buffer,p))){
	    //
	    // It is an exact match - at the end of the wild card.
	    // We have a match.
	    //
	    if (debug) fprintf(stderr,"IT DOES.\n");
	    return 1;
	    //
	  } else {
	    //
	    // No match - a failure.
	    //
	    if (debug) fprintf(stderr,"IT DOES NOT.\n");
	    return 0;
	  }
	}

	if (istart == 0){
	  if (debug) fprintf(stderr, "MUST BE THE START OF %s: ", searchString);
	  if (!(strncmp(buffer, searchString,strlen(buffer)))){
	    if (debug) fprintf(stderr,"IT IS.\n");
	    searchString = searchString + strlen(buffer);
	  } else {
	    if (debug) fprintf(stderr,"IT IS NOT.\n");
	    return 0;
	  }
	}



	if (istart != 0){
	  if (debug) fprintf(stderr, "MUST BE PRESENT IN %s: ", searchString);
	  char *p = (char *)strstr(searchString, buffer);
	  if (p == NULL){
	    if (debug) fprintf(stderr,"IT IS NOT.\n");
	    return 0;
	  } else {
	    if (debug) fprintf(stderr,"IT IS.\n");
	    searchString = p + strlen(buffer);
	  }
	}


	//
	// If that was the last item, return success.
	//
	if (WildSpec[iend] == char(0)){
	  return 1;
	}
	
	istart = iend;
      }

    } while (1);

  } while (1);


}

////////////////////////////////////////////////////////////////
// draw an icon

void Server::_drawIcon(const char *iconName,
                       const char *colorToUse,
                       double centerLat,
                       double centerLon,
                       double iconScale,
                       bool allowClientScaling,
                       Symprod &prod)


{

  if (_iconDefList.find(iconName) == _iconDefList.end()) {
    if (_isVerbose) {
      cerr << "ERROR: Server::_drawIcon" << endl;
      cerr << "Icon <" << iconName <<
        "> not found in icon list created from parameter file" << endl;
      cerr << "Not rendering icon" << endl;
    }
    return;
  }
  
  IconDef *wx_icon_def = _iconDefList[iconName];
  
  // Apply the scale factor and create the array of points
  // also get the icon width so we can offset the text correctly

  Symprod::ppt_t *iconPoints = wx_icon_def->getPointList();
  Symprod::ppt_t *scaledPoints = (Symprod::ppt_t *)
    umalloc(wx_icon_def->getNumPoints() * sizeof(Symprod::ppt_t));

  int min = 0, max = 0, check_pt;
  bool isFirst=true;

  for (int ii = 0; ii<wx_icon_def->getNumPoints(); ii++) {

    // Apply the scale factor

    if (iconPoints[ii].x != Symprod::PPT_PENUP &&
        iconPoints[ii].y != Symprod::PPT_PENUP) {
      
      double scaledX = iconPoints[ii].x * iconScale;
      double scaledY = iconPoints[ii].y * iconScale;
      
      scaledPoints[ii].x = (int) scaledX;
      scaledPoints[ii].y = (int) scaledY;
      
    } else {
      
      scaledPoints[ii].x = iconPoints[ii].x;
      scaledPoints[ii].y = iconPoints[ii].y;

    }

    // Get the min max

    bool doCheck=true;
    check_pt=scaledPoints[ii].y;
    if (check_pt == Symprod::PPT_PENUP) {
      doCheck=false;
    }
    
    if (doCheck) {
      if (isFirst) {
	min=check_pt;
	max=check_pt;
	isFirst=false;
      } else {
	if (check_pt < min) {
	  min=check_pt;
	}
	if (check_pt > max) {
	  max=check_pt;
	}
      }
    }
  }

  Symprod::wpt_t icon_origin;
	
  icon_origin.lat = centerLat;
  icon_origin.lon = centerLon;

  if (!allowClientScaling) {
    prod.addStrokedIcons(colorToUse,
			 wx_icon_def->getNumPoints(),
			 scaledPoints,
			 1,
			 &icon_origin,
			 0,
			 Symprod::DETAIL_LEVEL_DO_NOT_SCALE,
			 _params->wx_icon_line_width);
  } else {
    prod.addStrokedIcons(colorToUse,
			 wx_icon_def->getNumPoints(),
			 scaledPoints,
			 1,
			 &icon_origin,
			 0,
			 0,
			 _params->wx_icon_line_width);
  }

  ufree(scaledPoints);

  // Done  
  return;

}

///////////////////////////////////////////////////////////////////
// Convert param values to Symprod object values
  
Symprod::vert_align_t Server::_vertAlign(Params::vert_align_t align)

{
   
  switch (align) {

    case Params::VERT_ALIGN_TOP :
      return Symprod::VERT_ALIGN_TOP;
      break;
      
    case Params::VERT_ALIGN_CENTER :
      return Symprod::VERT_ALIGN_CENTER;
      break;
      
    case Params::VERT_ALIGN_BOTTOM :
      return Symprod::VERT_ALIGN_BOTTOM;
      break;

    default:
      return Symprod::VERT_ALIGN_CENTER;
      
  }

}

Symprod::horiz_align_t Server::_horizAlign(Params::horiz_align_t align)

{

  switch (align) {

    case Params::HORIZ_ALIGN_LEFT :
      return Symprod::HORIZ_ALIGN_LEFT;
      break;
      
    case Params::HORIZ_ALIGN_CENTER :
      return Symprod::HORIZ_ALIGN_CENTER;
      break;
      
    case Params::HORIZ_ALIGN_RIGHT :
      return Symprod::HORIZ_ALIGN_RIGHT;
      break;

    default:
      return Symprod::HORIZ_ALIGN_CENTER;
      
  }

}

Symprod::font_style_t Server::_fontStyle(Params::font_style_t style)

{
  
  switch (style) {

    case Params::TEXT_NORM :
      return Symprod::TEXT_NORM;
      break;
      
    case Params::TEXT_BOLD :
      return Symprod::TEXT_BOLD;
      break;
      
    case Params::TEXT_ITALICS :
      return Symprod::TEXT_ITALICS;
      break;
      
    case Params::TEXT_SUBSCRIPT :
      return Symprod::TEXT_SUBSCRIPT;
      break;
      
    case Params::TEXT_SUPERSCRIPT :
      return Symprod::TEXT_SUPERSCRIPT;
      break;
      
    case Params::TEXT_UNDERLINE :
      return Symprod::TEXT_UNDERLINE;
      break;
      
    case Params::TEXT_STRIKETHROUGH :
      return Symprod::TEXT_STRIKETHROUGH;
      break;

    default:
      return Symprod::TEXT_NORM;
      
  }

}

