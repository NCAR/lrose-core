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
//
// Server.cc
// File Server object
//
///////////////////////////////////////////////////////////////

#include <vector>

#include <euclid/GridPoint.hh>
#include <tinyxml/tinyxml.h>

#include "Server.hh"

#include "BoundingBox.hh"
#include "CloudHandler.hh"
#include "JetStreamHandler.hh"
#include "TurbulenceHandler.hh"
#include "VolcanoHandler.hh"

using namespace std;



//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

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
		  initialParams->debug >= Params::DEBUG_VERBOSE)
{
  _setLocalParams((Params *)initialParams);
  
  return;
}


//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int Server::loadLocalParams( const string &paramFile, void **serverParams)
{
  const char *method_name = "Server::loadLocalParams()";

  Params  *localParams;
  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;

  if (_isDebug)
    cerr << "Loading new params from file: " << paramFile << endl;

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
			_isVerbose) != 0)
  {
    cerr << "ERROR - " << method_name << endl
	 << "Cannot load parameter file: " << paramFile << endl;

    delete localParams;

    return -1;
  }

  if (_isVerbose)
    localParams->print(stderr, PRINT_SHORT);

  if (!_setLocalParams(localParams))
    return -1;
  
  *serverParams = (void*)localParams;

  return 0;
}


//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

int Server::convertToSymprod(
  const void *params,
  const string &dir_path,
  const int prod_id,
  const string &prod_label,
  // See cvs/libs/Spdb/src/include/Spdb/Spdb_typedefs.hh:
  const Spdb::chunk_ref_t &chunk_ref,
  const Spdb::aux_ref_t &aux_ref,
  const void *spdb_data,
  const int spdb_len,
  MemBuf &symprod_buf)
{
  static const string method_name = "Server::convertToSymprod()";
  
  // Get a pointer to the parameters

  Params *serverParams = (Params *)params;
  
  int bugs = 0;
  // _isDebug is defined in libs/dsserver/src/include/dsserver/DsServer.hh
  if (_isDebug && bugs < 1) bugs = 1;
  if (_isVerbose && bugs < 5) bugs = 5;

  if (bugs >= 1) {
    cerr << endl << method_name << ": Server: entry" << endl;
    cerr << "  _isDebug: " << _isDebug << endl;
    cerr << "  _isVerbose: " << _isVerbose << endl;

    cerr << "  _horizLimitsSet: " << _horizLimitsSet << endl;
    cerr << "  _minLat: " << _minLat << endl;
    cerr << "  _maxLat: " << _maxLat << endl;
    cerr << "  _minLon: " << _minLon << endl;
    cerr << "  _maxLon: " << _maxLon << endl;
    cerr << "  _vertLimitsSet: " << _vertLimitsSet << endl;
    cerr << "  _minHt: " << _minHt << endl;
    cerr << "  _maxHt: " << _maxHt << endl;
  }

  BoundingBox bbox;
  
  if (_horizLimitsSet)
    bbox.setLimits(_minLat, _maxLat, _minLon, _maxLon);
  else
    bbox.setLimits(-90.0, 90.0, 0.0, 360.0);

  int irc = -1;

  try {
    if (serverParams->process_cloud &&
	(chunk_ref.data_type == SIGWX_CLOUD ||
	 prod_id == SPDB_WAFS_SIGWX_CLOUD_ID))
    {
      CloudHandler cloud_handler(serverParams, bugs);
      
      cloud_handler.convertToSymprod(_iconDefList,
				     dir_path,
				     prod_id,
				     prod_label,
				     chunk_ref,
				     aux_ref,
				     spdb_data,
				     spdb_len,
				     symprod_buf,
				     bbox);
      irc = 0;
    }
    else if (serverParams->process_jetstream &&
	     (chunk_ref.data_type == SIGWX_JETSTREAM ||
	      prod_id == SPDB_WAFS_SIGWX_JETSTREAM_ID))
    {
      JetStreamHandler jet_stream_handler(serverParams, bugs);
      
      jet_stream_handler.convertToSymprod(dir_path,
					  prod_id,
					  prod_label,
					  chunk_ref,
					  aux_ref,
					  spdb_data,
					  spdb_len,
					  symprod_buf,
					  bbox);
      irc = 0;
    }
    else if (serverParams->process_turbulence &&
	     (chunk_ref.data_type == SIGWX_TURBULENCE ||
	      prod_id == SPDB_WAFS_SIGWX_TURBULENCE_ID))
    {
      TurbulenceHandler turb_handler(serverParams, bugs);
      
      turb_handler.convertToSymprod(_iconDefList,
				    dir_path,
				    prod_id,
				    prod_label,
				    chunk_ref,
				    aux_ref,
				    spdb_data,
				    spdb_len,
				    symprod_buf,
				    bbox);
      irc = 0;
    }
    else if (serverParams->process_volcano &&
	     (chunk_ref.data_type == SIGWX_VOLCANO ||
	      prod_id == SPDB_WAFS_SIGWX_VOLCANO_ID))
    {
      VolcanoHandler volcano_handler(serverParams, bugs);
      
      volcano_handler.convertToSymprod(dir_path,
				       prod_id,
				       prod_label,
				       chunk_ref,
				       aux_ref,
				       spdb_data,
				       spdb_len,
				       symprod_buf,
				       bbox);
      irc = 0;
    }
  }
  catch (char * msg) {
    irc = -1;
    cerr << endl << "Error: " << method_name << ":" << endl;
    cerr << msg << endl << endl;
  }
  return irc;
} // end Server::convertToSymprod


/*********************************************************************
 * _setLocalParams()
 */

bool Server::_setLocalParams(Params *localParams)
{
  const char *method_name = "Server::_setLocalParams()";

  // Get icon_defs

  for (int ii = 0; ii < localParams->icon_defs_n; ii++)
  {
    // Create the list of icon points

    vector< GridPoint > point_list;
    char *x_string = strtok(localParams->_icon_defs[ii].icon_points, " ");
    char *y_string;

    if (x_string == (char *)0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in icon_points string for icon " <<
	localParams->_icon_defs[ii].icon_name << endl;
      cerr << "The string must contain at least 1 point" << endl;

      continue;
    }
    
    bool string_error = false;
    while (x_string != (char *)0)
    {
      // Get the string representing the Y coordinate of the icon point

      y_string = strtok(NULL, " ");
      if (y_string == (char *)0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error in icon_points string for icon " <<
	  localParams->_icon_defs[ii].icon_name << endl;
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

    if (string_error) continue;
    
    // Create the icon definition object and add it to our list

    string icon_name = localParams->_icon_defs[ii].icon_name;
    IconDef icon_def(icon_name, point_list);
    
    _iconDefList[icon_name] = icon_def;

  } // for ii

  return true;
}
