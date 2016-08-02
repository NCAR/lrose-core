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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include "Server.hh"
#include "Filter.hh"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

Server::Server(const string &prog_name,
	       Params *initialParams)
  : DsSymprodServer(prog_name,
		 initialParams->instance,
		 (void*)(initialParams),
		 initialParams->port,
		 initialParams->qmax,
		 initialParams->max_clients,
		 initialParams->no_threads,
		 initialParams->debug >= Params::DEBUG_NORM,
		 initialParams->debug >= Params::DEBUG_VERBOSE)
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
  if (_filter)
    delete _filter;
}

//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int
Server::loadLocalParams( const string &paramFile, void **serverParams)

{
   Params  *localParams;
   char   **tdrpOverrideList = NULL;
   bool     expandEnvVars = true;

   const char *routine_name = "_allocLocalParams";

   if (_isDebug) {
     cerr << "Loading new params from file: " << paramFile << endl;
   }

   localParams = new Params( *((Params*)_initialParams) );
   if ( localParams->load( (char*)paramFile.c_str(),
                           tdrpOverrideList,
                           expandEnvVars,
                           _isVerbose ) != 0 ) {
      cerr << "ERROR - " << _executableName << "::" << routine_name << endl
           << "Cannot load parameter file: " << paramFile << endl;
      delete localParams;
      return( -1 );
   }

   if (_isVerbose) {
     localParams->print(stderr, PRINT_SHORT);
   }

   *serverParams = (void*)localParams;

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
   
   return( 0 );

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
  
  Params *serverParams = (Params*) params;

  // check prod_id

  if (prod_id != SPDB_STATION_REPORT_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_STATION_REPORT_ID: "
	 << SPDB_STATION_REPORT_ID << endl;
    return;
  }
  
  // check the auxiliary XML to set limits
  if (_filter) {delete _filter; _filter = NULL;}
  if (_auxXml.size() != 0) {
    _filter = Filter::getCorrectFilter(_auxXml, serverParams);
    if (_filter && _filter->setRulesFromXml(_auxXml) < 0)
      { delete _filter; _filter = NULL; }
  }

  // set up vectors for accepted, rejected and required stations
  
  vector<si32> acceptedCodes;
  vector<si32> rejectedCodes;
  vector<si32> requiredCodes;

  if (serverParams->useAcceptedStationsList) {
    for (int i = 0; i < serverParams->acceptedStations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(serverParams->_acceptedStations[i]);
      acceptedCodes.push_back(code);
    } // i
  }
  
  if (serverParams->useRejectedStationsList) {
    for (int i = 0; i < serverParams->rejectedStations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(serverParams->_rejectedStations[i]);
      rejectedCodes.push_back(code);
    } // i
  }
  
  if (serverParams->decimate_spatially &&
      serverParams->decimate_required_stations_n > 0) {
    for (int i = 0; i < serverParams->decimate_required_stations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(serverParams->_decimate_required_stations[i]);
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
	  if (convertToSymprod(serverParams, dir_path, prod_id, prod_label,
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
      if (convertToSymprod(serverParams, dir_path, prod_id, prod_label,
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
    cerr << "Sending total list n stations: " << n_chunks_out << endl;
  }

}

//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

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
  
  Params *serverParams = (Params*) params;

  // disassemble into WxObs object

  WxObs obs;
  obs.disassemble(spdb_data, spdb_len);
  
  if (_isVerbose) {
    cerr << "Station data, time: "
         << DateTime::strm(obs.getObservationTime()) << endl;
  }
  
  // check we have required fields

  if (serverParams->check_for_required_fields) {
    if (_checkForRequiredFields(serverParams, obs)) {
      return -1;
    }
  }

  // check against the XML filter
  if (_filter && !_filter->testObs(obs))
    {
      cerr << "Observation did not pass the filter\n";
      return -1;
    }
  // Check that the station is within the desired pressure range, if requested

  if (serverParams->usePressureRange)
  {
    double station_pressure = obs.getSeaLevelPressureMb();
    
    if (station_pressure < serverParams->minStationPressure ||
	station_pressure > serverParams->maxStationPressure)
      return -1;
  }
  
  // Check that the station position is within the bounding box, if requested

  if (_horizLimitsSet) {

    // Normalize the station longitude

    double station_lat = obs.getLatitude();
    double station_lon = obs.getLongitude();

    while (station_lon < _minLon)
      station_lon += 360.0;
    while (station_lon >= _minLon + 360.0)
      station_lon -= 360.0;

    obs.setLongitude(station_lon);
    
    // See if the station is within the area of interest

    bool accept = false;

    if (station_lat >= _minLat && station_lat <= _maxLat &&
	station_lon >= _minLon && station_lon <= _maxLon) {
	accept = true;
    }
    
    if (!accept) {
      for (int is = 0;
	   is < serverParams->decimate_required_stations_n; is++) {
	if (Spdb::hash4CharsToInt32
            (serverParams->_decimate_required_stations[is])
	    == chunk_ref.data_type) {
	  accept = true;
	}
      } // is
    }

    if (!accept) {
      return -1;
    }

    if (serverParams->decimate_spatially && _locOccupied) {

      // decimation is active, so check to make sure that there is 
      // not already a metar close by

      int ilat = (int) (((station_lat - _minLat) /
			 (_maxLat - _minLat))
			* serverParams->decimate_n_lat);
      if (ilat < 0) {
	ilat = 0;
      }
      if (ilat > serverParams->decimate_n_lat - 1) {
	ilat = serverParams->decimate_n_lat - 1;
      }

      int ilon = (int) (((station_lon - _minLon) /
			 (_maxLon - _minLon))
			* serverParams->decimate_n_lon);
      if (ilon < 0) {
	ilon = 0;
      }
      if (ilon > serverParams->decimate_n_lon - 1) {
	ilon = serverParams->decimate_n_lon - 1;
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

  } // if (serverParams->useBoundingBox)

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now, chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       prod_label.c_str());

  if (serverParams->do_translation) {
    
    int fcatIndex = _findFlightCatIndex(serverParams, obs);
    
    _addLabels(serverParams, prod, obs, fcatIndex);

    if (serverParams->draw_wind_barb) {
      _addWindBarb(serverParams, prod, obs, fcatIndex);
    }
    
    if (serverParams->draw_flight_category) {
      _addFlightCat(serverParams, prod, obs, fcatIndex);
    }
    
    if (serverParams->activate_hidden_text) {
      _addHiddenText(serverParams, prod, obs, fcatIndex);
    }
      
    // Add untranslated binary chunk

    if (serverParams->add_raw_data_as_chunk) {
      prod.addChunk(-90.0, -180.0, 90.0, 180.0,
                    prod_id,
                    spdb_len, spdb_data,
                    serverParams->color,
                    serverParams->background_color);
    }

  }
  
  // set return buffer
  
  prod.serialize(symprod_buf);

  return(0);
  
}

////////////////////////////
// check for required fields
//
// Returns 0 on success (all fields are present)
//        -1 on failure (some fields are missing)


int Server::_checkForRequiredFields(const Params *serverParams,
                                    const WxObs &obs)
  
{

  if (serverParams->required_fields.require_temperature) {
    if (obs.getTempC() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_dewpoint) {
    if (obs.getDewpointC() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_wind_speed) {
    if (obs.getWindSpeedMps() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_wind_direction) {
    if (obs.getWindDirnDegt() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_pressure) {
    if (obs.getPressureMb() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_ceiling) {
    if (obs.getCeilingKm() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_visibility) {
    if (obs.getVisibilityKm() < -9900) {
      return -1;
    }
  }

  if (serverParams->required_fields.require_rvr) {
    if (obs.getRvrKm() < -9900) {
      return -1;
    }
  }

  return 0;

} 
  
/////////////////////////
// add text labels

void Server::_addLabels(Params *serverParams,
			Symprod &prod,
			const WxObs &obs,
			int fcat_index)
  
{
  
  // Temperature

  double temp = obs.getTempC();

  // Put the temperature field into the right units for display.

  if (serverParams->display_temp == Params::DISPLAY_TEMP_F){
    if (temp != WxObs::missing) {
      temp = (9.0/5.0) * temp + 32.0;
    }
  } else if (serverParams->display_temp == Params::DISPLAY_TEMP_K){  
    if (temp != WxObs::missing) {
      temp = temp + 273.0;
    }
  }

  if (serverParams->temperature_label.do_draw &&
      temp != WxObs::missing) {
    char *color = serverParams->temperature_label.color;
    if (serverParams->temperature_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    
    if (serverParams->temps_to_1_digit){
      float pt = (float) ((int)rint(temp*10.0)) / 10.0;
      sprintf(text, "%g", pt);
    } else {
      sprintf(text, "%g", temp);
    }

    if (serverParams->temps_to_int){
      sprintf(text, "%d", (int)rint(temp));
    }

    if (_isVerbose){
      cerr << "Temperature point text : " << text << endl;
    }

    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->temperature_label.background_color,
		 serverParams->temperature_label.x_offset,
		 serverParams->temperature_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->temperature_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->temperature_label.horiz_align,
		 serverParams->temperature_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->temperature_label.font_name);
  }

  // Dew point

  double dewpoint = obs.getDewpointC();

  // Put the dewpoint temperature field into the right units for display.

  if (serverParams->display_temp == Params::DISPLAY_TEMP_F){
    if (dewpoint != WxObs::missing) {
      dewpoint = (9.0/5.0) * dewpoint + 32.0;
    }
  } else if (serverParams->display_temp == Params::DISPLAY_TEMP_K){  
    if (dewpoint != WxObs::missing) {
      dewpoint = dewpoint + 273.0;
    }
  }

  if (serverParams->dew_point_label.do_draw &&
      dewpoint != WxObs::missing) {
    char *color = serverParams->dew_point_label.color;
    if (serverParams->dew_point_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];

    if (serverParams->temps_to_1_digit){
      float pt = (float) ((int)rint(dewpoint*10.0)) / 10.0;
      sprintf(text, "%g", pt);
    } else {
      sprintf(text, "%g", dewpoint);
    }

    if (serverParams->temps_to_int){
      sprintf(text, "%d", (int)rint(dewpoint));
    }

    if (_isVerbose){
      cerr << "Dew point text : " << text << endl;
    }

    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->dew_point_label.background_color,
		 serverParams->dew_point_label.x_offset,
		 serverParams->dew_point_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->dew_point_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->dew_point_label.horiz_align,
		 serverParams->dew_point_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->dew_point_label.font_name);
  }

  // Relative Humidity 

  double rh = obs.getRhPercent();
  if (serverParams->humidity_label.do_draw &&
      rh != WxObs::missing) {
    char *color = serverParams->humidity_label.color;
    if (serverParams->humidity_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];

    if (serverParams->temps_to_1_digit){
      float pt = (float) ((int)rint(rh*10.0)) / 10.0;
      sprintf(text, "%g", pt);
    } else {
      sprintf(text, "%g", rh);
    }

    if (serverParams->temps_to_int){
      sprintf(text, "%d", (int)rint(rh));
    }

    if (_isVerbose){
      cerr << "Dew point text : " << text << endl;
    }

    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->humidity_label.background_color,
		 serverParams->humidity_label.x_offset,
		 serverParams->humidity_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->humidity_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->humidity_label.horiz_align,
		 serverParams->humidity_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->humidity_label.font_name);
  }

  // Pressure

  double pres = obs.getSeaLevelPressureMb();
  if (serverParams->pressure_label.do_draw &&
      pres != WxObs::missing) {
    int ipres = (int) (pres + 0.5);
    if (serverParams->pressure_as_3_digits) {
      if (pres >= 1000.0) {
	ipres -= 1000;
      }
    }
    char *color = serverParams->pressure_label.color;
    if (serverParams->pressure_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    sprintf(text, "%.3d", ipres);
    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->pressure_label.background_color,
		 serverParams->pressure_label.x_offset,
		 serverParams->pressure_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->pressure_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->pressure_label.horiz_align,
		 serverParams->pressure_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->pressure_label.font_name);
  }

  // Current weather - as string
  
  if (serverParams->current_weather_label.do_draw) {
    char *color = serverParams->current_weather_label.color;
    if (serverParams->current_weather_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    prod.addText(obs.getMetarWx().c_str(),
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->current_weather_label.background_color,
		 serverParams->current_weather_label.x_offset,
		 serverParams->current_weather_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->current_weather_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->current_weather_label.horiz_align,
		 serverParams->current_weather_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->current_weather_label.font_name);
  }

  // Current weather - as Bitfields
  
  if (serverParams->current_weather_type.do_draw) {
    char *color = serverParams->current_weather_type.color;

    if (serverParams->current_weather_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }

    prod.addText(obs.wxTypes2Str().c_str(),
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->current_weather_type.background_color,
		 serverParams->current_weather_type.x_offset,
		 serverParams->current_weather_type.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->current_weather_type.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->current_weather_type.horiz_align,
		 serverParams->current_weather_type.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->current_weather_type.font_name);
  }

  // Wind gust

  double windgust = obs.getWindGustMps();
  if (serverParams->wind_gust_label.do_draw &&
      windgust > 0 &&
      windgust != WxObs::missing) {
    char *color = serverParams->wind_gust_label.color;
    if (serverParams->wind_gust_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    sprintf(text, "G%d", (int) (windgust  * NMH_PER_MS + 0.5));
    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->wind_gust_label.background_color,
		 serverParams->wind_gust_label.x_offset,
		 serverParams->wind_gust_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->wind_gust_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->wind_gust_label.horiz_align,
		 serverParams->wind_gust_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->wind_gust_label.font_name);
  }

  // Ceiling

  double ceiling = obs.getCeilingKm();
  if (serverParams->ceiling_label.do_draw &&
      ceiling != WxObs::missing) {
    char *color = serverParams->ceiling_label.color;
    if (serverParams->ceiling_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    if (ceiling > 9.5) {
      text[0] = '\0';
    } else {
      switch (serverParams->ceiling_units) {
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
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->ceiling_label.background_color,
		 serverParams->ceiling_label.x_offset,
		 serverParams->ceiling_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->ceiling_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->ceiling_label.horiz_align,
		 serverParams->ceiling_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->ceiling_label.font_name);
  }

  // Visibility
  
  if (serverParams->visibility_label.do_draw) {
    char *color = serverParams->visibility_label.color;
    if (serverParams->visibility_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    double vis = obs.getVisibilityKm();
    if (vis < 0) {
      // convert from extinction coeff to visibility
      vis = -3.0 / vis;
    }
    if (vis > 20) {
      text[0] = '\0';
    } else {
      switch (serverParams->visibility_units) {
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
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->visibility_label.background_color,
		 serverParams->visibility_label.x_offset,
		 serverParams->visibility_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->visibility_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->visibility_label.horiz_align,
		 serverParams->visibility_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->visibility_label.font_name);
  }

  // precip rate

  if (serverParams->precip_rate_label.do_draw) {

    double precipRate = obs.getPrecipRateMmPerHr();

    if (precipRate > 0 &&
        precipRate != WxObs::missing) {
      char *color = serverParams->precip_rate_label.color;
      char text[64];
      sprintf(text, "%.1f", precipRate);
      prod.addText(text,
                   obs.getLatitude(), obs.getLongitude(), color,
                   serverParams->precip_rate_label.background_color,
                   serverParams->precip_rate_label.x_offset,
                   serverParams->precip_rate_label.y_offset,
                   (Symprod::vert_align_t)
                   serverParams->precip_rate_label.vert_align,
                   (Symprod::horiz_align_t)
                   serverParams->precip_rate_label.horiz_align,
                   serverParams->precip_rate_label.font_size,
                   Symprod::TEXT_NORM,
                   serverParams->precip_rate_label.font_name);
    }

  } // if (serverParams->precip_rate_label.do_draw) 
    
  // precip accum

  if (serverParams->precip_accum_label.do_draw) {

    const WxObsField &precipAccumField = obs.getPrecipLiquidMmField();
    char *color = serverParams->precip_accum_label.color;

    int count = 0;
    for (int ii = 0; ii < precipAccumField.getSize(); ii++) {
      
      double precipAccum = precipAccumField.getValue(ii);
      double accumPeriodSecs = precipAccumField.getQualifier(ii);

      if (serverParams->precip_accum_specify_duration_limits &&
          (accumPeriodSecs < serverParams->precip_accum_min_duration_secs ||
           accumPeriodSecs > serverParams->precip_accum_max_duration_secs)) {
        continue;
      }

      if (precipAccum >= 0 && precipAccum != WxObs::missing) {
        char text[64];
        if (serverParams->precip_accum_include_duration_text) {
          sprintf(text, "%g/%ghr", precipAccum, accumPeriodSecs / 3600.0);
        } else {
          sprintf(text, "%g", precipAccum);
        }
        prod.addText(text,
                     obs.getLatitude(), obs.getLongitude(), color,
                     serverParams->precip_accum_label.background_color,
                     serverParams->precip_accum_label.x_offset,
                     (serverParams->precip_accum_label.y_offset + 
                      count * serverParams->precip_accum_line_spacing_pixels),
                     (Symprod::vert_align_t)
                     serverParams->precip_accum_label.vert_align,
                     (Symprod::horiz_align_t)
                     serverParams->precip_accum_label.horiz_align,
                     serverParams->precip_accum_label.font_size,
                     Symprod::TEXT_NORM,
                     serverParams->precip_accum_label.font_name);
        count++;
      }

    } // ii

    // add circle to show location
    if (serverParams->station_posn_circle_radius > 0) {
      int detail_level = 0;
      if (serverParams->plot_unscaled) {
        detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
      }
      prod.addArc(obs.getLatitude(), obs.getLongitude(),
                  serverParams->station_posn_circle_radius,
                  serverParams->station_posn_circle_radius,
                  color, true,
                  0, 360.0, 0.0, 60,		     
                  Symprod::LINETYPE_SOLID, 1,
                  Symprod::CAPSTYLE_BUTT,
                  Symprod::JOINSTYLE_BEVEL,
                  Symprod::FILL_NONE,
                  0, detail_level);
    }

  } // if (serverParams->precip_accum_label.do_draw)

  // station name

  if (serverParams->station_name_label.do_draw) {
    char *color = serverParams->station_name_label.color;
    if (serverParams->station_name_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    int offset = (int) obs.getStationId().size() - serverParams->station_name_label_len;
    if (offset < 0) {
      offset = 0;
    }
    strcpy(text, obs.getStationId().c_str() + offset);
    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->station_name_label.background_color,
		 serverParams->station_name_label.x_offset,
		 serverParams->station_name_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->station_name_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->station_name_label.horiz_align,
		 serverParams->station_name_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->station_name_label.font_name);
  }

  // time

  if (serverParams->time_label.do_draw) {
    char *color = serverParams->time_label.color;
    if (serverParams->time_label.override_color_from_flight_cat &&
	fcat_index >= 0) {
      color = serverParams->_flight_category[fcat_index].color;
    }
    char text[64];
    sprintf(text, "%s", utimstr(obs.getObservationTime()));
    prod.addText(text,
		 obs.getLatitude(), obs.getLongitude(), color,
		 serverParams->time_label.background_color,
		 serverParams->time_label.x_offset,
		 serverParams->time_label.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->time_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->time_label.horiz_align,
		 serverParams->time_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->time_label.font_name);
  }

}

////////////////////////////////////////////////////////////////////
// _addWindBarb()
//
// Add a wind barb

void Server::_addWindBarb(Params *serverParams,
			  Symprod &prod,
			  const WxObs &obs,
			  int fcat_index)

{

  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  char *color = serverParams->wind_barb_color;
  if (serverParams->override_wind_barb_color_from_flight_cat &&
      fcat_index >= 0) {
    color = serverParams->_flight_category[fcat_index].color;
  }


  // get speed in desired units
  double speed_to_plot = 0;
  double windspd = obs.getWindSpeedMps();
  switch( serverParams->wind_units) {
	case Params::WIND_MS:
       speed_to_plot = windspd;
	break;

	case Params::WIND_KTS:
       speed_to_plot = windspd * NMH_PER_MS;
	break;

	case Params::WIND_MPH:
       speed_to_plot = windspd / MPH_TO_MS;
	break;

	case Params::WIND_KPH:
       speed_to_plot = windspd * MPERSEC_TO_KMPERHOUR;
	break;
  }

  // abort if wind value is too high

  if (speed_to_plot > 350.0) {
    return;
  }

  //
  // Color code by wind speed, if desired.
  //
  if (serverParams->override_wind_barb_color_from_wind_speed) {
    for (int k=0; k < serverParams->wind_speed_colors_n; k++){
      if (
	  (speed_to_plot >= serverParams->_wind_speed_colors[k].min) &&
	  (speed_to_plot <= serverParams->_wind_speed_colors[k].max)
	  ){
	color = serverParams->_wind_speed_colors[k].color;
	break;
      }
    }
  }

  //need this because we might want to use the color set from the filter
  //and that is a const char*
  const char* constColor = color;	
  if (_filter)
      constColor = _filter->getColor();

  // Symprod::wpt_t origin;
  // origin.lat = obs.getLatitude();
  // origin.lon = obs.getLongitude();
  int shaft_len = serverParams->wind_barb_shaft_len;

  if (serverParams->render_missing_ceiling_icon &&
      obs.getCeilingKm() < -9990) {

    // plot missing ceiling icon
    
    int nIconPts = serverParams->missing_ceiling_icon_n;
    double iconScale = serverParams->missing_ceiling_icon_scale;
    Symprod::ppt_t *iconPts = new Symprod::ppt_t[nIconPts];
    for (int ii = 0; ii < nIconPts; ii++) {
      iconPts[ii].x = (int)
        floor(serverParams->_missing_ceiling_icon[ii].x * iconScale + 0.5);
      iconPts[ii].y = (int)
        floor(serverParams->_missing_ceiling_icon[ii].y * iconScale + 0.5);
    }
    
    prod.addIconline(obs.getLatitude(), obs.getLongitude(),
                     nIconPts, iconPts,
                     serverParams->missing_ceiling_icon_color,		     
                     Symprod::LINETYPE_SOLID,
                     serverParams->missing_ceiling_icon_line_width,
                     Symprod::CAPSTYLE_BUTT,
                     Symprod::JOINSTYLE_BEVEL, false,
                     Symprod::FILL_NONE,
                     0, detail_level);
    
  } else {
  
    // add circle to show location
    if (serverParams->station_posn_circle_radius > 0) {
      prod.addArc(obs.getLatitude(), obs.getLongitude(),
                  serverParams->station_posn_circle_radius,
                  serverParams->station_posn_circle_radius,
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
  
  double dirrad = obs.getWindDirnDegt() * DEG_TO_RAD;
  double cosdir = cos(dirrad);
  double sindir = sin(dirrad);
  
  Symprod::ppt_t shaft[2];
  shaft[0].x = 0;
  shaft[0].y = 0;
  shaft[1].x = (int) floor(shaft_len * sindir + 0.5);
  shaft[1].y = (int) floor(shaft_len * cosdir + 0.5);
  
  prod.addIconline(obs.getLatitude(), obs.getLongitude(),
		   2, shaft,
		   constColor,		     
		   Symprod::LINETYPE_SOLID,
		   serverParams->wind_barb_line_width,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL, false,
		   Symprod::FILL_NONE,
		   0, detail_level);

  // add flags for each 50 kts

  double tick_rel_angle;
  if (obs.getLatitude() >= 0.0) {
    tick_rel_angle = serverParams->wind_ticks_angle_to_shaft;
  } else {
    tick_rel_angle = -serverParams->wind_ticks_angle_to_shaft;
  }
  double tick_rad = dirrad + tick_rel_angle * DEG_TO_RAD;
  double tick_len = serverParams->wind_barb_tick_len;
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
    prod.addIconline(obs.getLatitude(), obs.getLongitude(),
		     3, flag,
		     constColor,
		     Symprod::LINETYPE_SOLID,
		     serverParams->wind_barb_line_width,
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
    prod.addIconline(obs.getLatitude(), obs.getLongitude(),
		     2, full_tick,
		     constColor,		     
		     Symprod::LINETYPE_SOLID,
 		     serverParams->wind_barb_line_width,
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
    prod.addIconline(obs.getLatitude(), obs.getLongitude(),
		     2, half_tick,
		     constColor,		     
		     Symprod::LINETYPE_SOLID,
 		     serverParams->wind_barb_line_width,
		     Symprod::CAPSTYLE_BUTT,
		     Symprod::JOINSTYLE_BEVEL, false,
		     Symprod::FILL_NONE,
		     0, detail_level);

  } // if (speed_left >= 5)

}

/////////////////////////
// add flight category

void Server::_addFlightCat(Params *serverParams,
			   Symprod &prod,
			   const WxObs &obs,
			   int fcat_index)
  
{
  const char* constColor = serverParams->_flight_category[fcat_index].color;	
  if (_filter)
      constColor = _filter->getColor();

  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  if (fcat_index < 0) {
    double vis = obs.getVisibilityKm();
    if (vis < 0) {
      // convert from extinction coeff to visibility
      vis = -3.0 / vis;
    }
    if (_isVerbose) {
      cerr << "WARNING - no flight cat found." << endl;
      cerr << "  Station: " << obs.getStationId() << endl;
      cerr << "  Ceiling (km): " << obs.getCeilingKm() << endl;
      cerr << "  Visibility (km): " << vis << endl;
    }
    return;
  }
  
  double fractionCovered = -1.0;
  for (int ii = 0; ii < obs.getSkyObscSize(); ii++) {
    double fraction = obs.getSkyObscuration(ii);
    if (fraction > fractionCovered) {
      fractionCovered = fraction;
    }
  }
  for (int ii = 0; ii < obs.getWeatherTypeSize(); ii++) {
    wx_type_t wxType = obs.getWeatherType(ii);
    double fraction = 0.0;
    if (wxType == WxT_OVC) {
      fraction = 1.0;
    } else if (wxType == WxT_BKN) {
      fraction = 0.75;
    } else if (wxType == WxT_SCT) {
      fraction = 0.5;
    } else if (wxType == WxT_FEW) {
      fraction = 0.25;
    } else if (wxType == WxT_CLR) {
      fraction = 0.0;
    }
    if (fraction > fractionCovered) {
      fractionCovered = fraction;
    }
  }
  
  if (fractionCovered > 0.99) {
    
    // obscured
    
    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		0, 360.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);

  } else if (fractionCovered > 0.74) {
    
    // broken - 3/4 circle

    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		180.0, 450.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);
    
    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		90.0, 180.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);
    
  } else if (fractionCovered > 0.49) {

    // scattered - half circle
    
    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		270.0, 450.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);

    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		90.0, 270.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  } else if (fractionCovered > 0.24) {

    // few - quarter circle
    
    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		0.0, 90.0, 0.0, 60,
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_SOLID,
		0, detail_level);

    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		90.0, 360.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  } else if (fractionCovered > -0.01) {
    
    // clear
    
    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
		0, 360.0, 0.0, 60,		     
		Symprod::LINETYPE_SOLID, 1,
		Symprod::CAPSTYLE_BUTT,
		Symprod::JOINSTYLE_BEVEL,
		Symprod::FILL_NONE,
		0, detail_level);

  } else {
    
    // no cloud coverage mentioned
    
    prod.addArc(obs.getLatitude(), obs.getLongitude(),
		serverParams->flight_category_circle_radius,
		serverParams->flight_category_circle_radius,
		constColor, true,
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

void Server::_addHiddenText(Params *serverParams,
                            Symprod &prod,
                            const WxObs &obs,
                            int fcat_index)
  
{
  // add full METAR message as hidden text
  const char *foreground_color = serverParams->hidden_text_foreground_color;
  if (strlen(foreground_color) == 0){
    if (fcat_index >= 0) {
      foreground_color = serverParams->_flight_category[fcat_index].color;
    }
    if (_filter){
      foreground_color = _filter->getColor();
    }
  }

  prod.addText(obs.getMetarText().c_str(),
               obs.getLatitude(),
               obs.getLongitude(),
               foreground_color,
               serverParams->hidden_text_background_color,
               serverParams->hidden_text_x_offset,
               serverParams->hidden_text_y_offset,
               (Symprod::vert_align_t)
               serverParams->hidden_text_vert_align,
               (Symprod::horiz_align_t)
               serverParams->hidden_text_horiz_align,
               serverParams->hidden_text_font_size,
               Symprod::TEXT_NORM,
               serverParams->hidden_text_font_name,
               0, // Object ID
               Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
               Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
               Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS);
}

//////////////////////////////////
// find the flight category index
//
// Returns index on success, -1 on failure

int Server::_findFlightCatIndex(Params *serverParams,
				const WxObs &obs)
  
{
  
  // check for missing values (-9999)

  if (obs.getCeilingKm() < -9990 ||
      obs.getVisibilityKm() < -9990) {
    return -1;
  }

  double ceiling = obs.getCeilingKm() * 3048.0;
  double vis = obs.getVisibilityKm();
  
  for (int i = 0; i < serverParams->flight_category_n; i++) {
    if (ceiling >= serverParams->_flight_category[i].ceiling_threshold &&
	vis >= serverParams->_flight_category[i].visibility_threshold) {
      return i;
    }
  }

  return -1;

}

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
