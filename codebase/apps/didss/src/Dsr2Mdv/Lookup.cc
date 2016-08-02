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
// Lookup.cc
//
// Lookup class
//
// Handles the lookup tables.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#include "Lookup.hh"
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <Mdv/mdv/mdv_radar.h>
using namespace std;

//////////////
// Constructor

Lookup::Lookup(char *prog_name,
	       Dsr2Mdv_tdrp_struct *params)
  
{

  _progName = STRdup(prog_name);
  _params = params;

  // initialize index
  
  if (InitP2mdvIndex(&handle,
		     sizeof(handle),
		     _progName,
		     NULL, POLAR2MDV_LOOKUP_LABEL, NULL,
		     "Lookup::Lookup")) {
    OK = FALSE;
    return;
  }

  // read in lookup for first scan type

  _scanType = _params->lookups.val[0].scan_type;

  lookupTablePath = _params->lookups.val[0].lookup_table_path;
  if (ReadP2mdvLookup(&handle, lookupTablePath, "Lookup::Lookup")) {
    fprintf(stderr, "ERROR - %s:Lookup::Lookup\n", _progName);
    fprintf(stderr, "Cannot read lookup table '%s'\n", lookupTablePath);
    OK = FALSE;
    return;
  }
  if (_params->debug >= DEBUG_NORM) {
    fprintf(stderr, "Read in lookup table '%s'\n", lookupTablePath);
  }

  // read in clutter file

  if (_params->remove_clutter) {

    if (InitClutterIndex(&clut,
			 sizeof(clut_table_file_index_t),
			 _progName, NULL,
			 CLUTTER_TABLE_LABEL, NULL,
			 "Lookup::Lookup")) {
      OK = FALSE;
      return;
    }
    
    clutTablePath = _params->lookups.val[0].clutter_table_path;
    if (ReadClutterTable(&clut, clutTablePath,
			 "Lookup::Lookup")) {
      fprintf(stderr, "ERROR - %s:Lookup::Lookup\n", _progName);
      fprintf(stderr, "Cannot read clutter table '%s'\n", clutTablePath);
      OK = FALSE;
      return;
    }
    if (_params->debug >= DEBUG_NORM) {
      fprintf(stderr, "Read in clutter table '%s'\n", clutTablePath);
    }

  } // if (_params->remove_clutter)

  // initialize for elevation lookups

  _prevElev_handle = -1;
  _prevElev = 1000.0;

  OK = TRUE;

}

/////////////
// destructor

Lookup::~Lookup()

{
  
  STRfree(_progName);
  FreeP2mdvLookup(&handle, "Lookup::~Lookup");
  if (_params->remove_clutter) {
    FreeClutterTable(&clut, "Lookup::~Lookup");
  }

}

//////////////
// update()
//
// Updates the lookup table for the scan_type.
// If necessary, it reads in new table.
//
// Returns 0 on success, -1 on failure.
//
// Failure modes:
//  (a) scan_type not in list
//  (b) desired table will not read

int Lookup::update(int scan_type)

{

  // if existing scan type is -1, the existing table is
  // used for all scan types

  if (_scanType == -1) {
    return (0);
  }

  // has scan type changed?

  if (_scanType == scan_type) {
    return (0);
  }

  // is requested scan type in list?
  
  int type_index = -1;
  for (int i = 0; i < _params->lookups.len; i++) {
    if (scan_type == _params->lookups.val[i].scan_type) {
      type_index = i;
      break;
    }
  }

  // if not in list, use first scan type available

  if (type_index < 0) {
    fprintf(stderr, "WARNING - %s:Lookup::update()\n", _progName);
    fprintf(stderr, "scan_type %d not supported\n", scan_type);
    fprintf(stderr, "Using table for scan_type %d\n",
	    _params->lookups.val[0].scan_type);
    type_index = 0;
  }

  // free up table

  FreeP2mdvLookup(&handle, "Lookup::update");

  // read in new table

  lookupTablePath = _params->lookups.val[type_index].lookup_table_path;
  if (ReadP2mdvLookup(&handle, lookupTablePath, "Lookup::update")) {
    fprintf(stderr, "ERROR - %s:Lookup::update\n", _progName);
    fprintf(stderr, "Cannot read lookup table '%s'\n", lookupTablePath);
    return (-1);
  }
  _scanType = _params->lookups.val[type_index].scan_type;
  if (_params->debug >= DEBUG_NORM) {
    fprintf(stderr, "Read in lookup table '%s'\n", lookupTablePath);
  }

  if (_params->remove_clutter) {

    // free up clutter table, read in new one
    
    FreeClutterTable(&clut, "Lookup::~Lookup");

    clutTablePath = _params->lookups.val[type_index].clutter_table_path;
    if (ReadClutterTable(&clut, clutTablePath,
			   "Lookup::update")) {
      fprintf(stderr, "ERROR - %s:Lookup::update\n", _progName);
      fprintf(stderr, "Cannot read clutter table '%s'\n", clutTablePath);
      return (-1);
    }
    if (_params->debug >= DEBUG_NORM) {
      fprintf(stderr, "Read in clutter table '%s'\n", clutTablePath);
    }

  } // if (_params->remove_clutter)

  return (0);
  
}

///////////////
// _checkGeom()
//
// Checks the lookup table geometry against the radar geometry
//

#define CHECK_DISTANCE 0.50
#define MIN_RATIO 0.999
#define MAX_RATIO 1.001

int Lookup::checkGeom(DsRadarMsg &radarMsg, int dbzFieldPos)

{

  int iret = 0;
  
  P2mdv_lookup_params_t *lookup_params = handle.lookup_params;
  MDV_radar_grid_t *grid = (MDV_radar_grid_t *) &handle.lookup_params->grid;
  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  radar_scan_table_t *scan_table = handle.scan_table;

  // check the distance between the data radar location
  // and the radar-to-cart lookup table radar location
  
  double data_lat = radarParams.latitude;
  double data_long = radarParams.longitude;
  double data_alt = radarParams.altitude;
  double table_lat = lookup_params->radar_latitude;
  double table_long = lookup_params->radar_longitude;

  if ( _params->override_radar_location ) {
    data_lat = _params->radar_location.latitude;
    data_long = _params->radar_location.longitude;
    data_alt = _params->radar_location.altitude;
  }

  double distance, theta;  
  PJGLatLon2RTheta(data_lat, data_long, table_lat, table_long,
		   &distance, &theta);
    
  if (distance > CHECK_DISTANCE) {
    
    fprintf(stderr, "\nERROR - %s:Resample::_checkGeom\n", _progName);
    fprintf(stderr, "Radar positions do not match.\n");
    fprintf(stderr, "Data radar latitude = %12.7g\n", data_lat);
    fprintf(stderr, "Radar to cart. table radar latitude = %12.7g\n",
	    table_lat);
    fprintf(stderr, "Data radar longitude = %12.7g\n", data_long);
    fprintf(stderr, "Lookup table radar longitude = %12.7g\n",
	    table_long);
    fprintf(stderr, "Lookup table '%s'\n", lookupTablePath);
    iret = -1;
    
  }
  
  double check_ratio = (data_alt / grid->radarz);
    
  if(check_ratio < MIN_RATIO || check_ratio > MAX_RATIO) {
    
    fprintf(stderr, "\nERROR - %s:Resample::_checkGeom\n", _progName);
    fprintf(stderr, "Radar altitude does not match.\n");
    fprintf(stderr, "Data radar altitude = %g\n",
	    radarParams.altitude);
    fprintf(stderr, "Lookup table radar altitude = %g\n",
	    grid->radarz);
    fprintf(stderr, "Lookup table '%s'\n", lookupTablePath);
    iret = -1;
    
  }
    
  if (fabs(radarParams.startRange - scan_table->start_range) > 0.001) {
    
    fprintf(stderr, "\nERROR - %s:Resample::_checkGeom\n", _progName);
    fprintf(stderr, "Start range does not match.\n");
    fprintf(stderr, "Radar volume startRange = %g\n",
	    radarParams.startRange);
    fprintf(stderr, "Lookup table start_range = %g\n",
	    scan_table->start_range);
    fprintf(stderr, "Lookup table '%s'\n", lookupTablePath);
    iret = -1;
      
  }
  
  if (_params->remove_clutter && dbzFieldPos >= 0) {
    if (_checkClutterGeom(radarMsg, dbzFieldPos)) {
      iret = -1;
    }
  }

  return (iret);

}

//////////////////////////////////////////////////
// _checkClutterGeom()
//
// check that the lookup grif parameters match the 
// clutter tab;e grid

int Lookup::_checkClutterGeom(DsRadarMsg &radarMsg, int dbzFieldPos)

{

  int iret = 0;

  P2mdv_lookup_params_t *lookup_params = handle.lookup_params;

  if (memcmp(lookup_params,
	     &clut.table_params->lookup_params,
	     sizeof(P2mdv_lookup_params_t))) {
    
    fprintf(stderr, "\nERROR - %s:Resample::_checkCluterGeom\n",
	    _progName);
    fprintf(stderr,
	    "Lookup table params do not match clutter table params.\n");
    fprintf(stderr, "Lookup table '%s'\n", lookupTablePath);
    fprintf(stderr, "Clutter table '%s'\n", clutTablePath);
    fprintf(stderr,
	    "Generate a new clutter table using the current lookup table.\n");
    iret = -1;
  }
  
  /*
   * check that the scale and bias of the dbz field matches that used
   * to generate the clutter table. If not, adjust the
   * values accordingly.
   */
  
  double clutter_scale = clut.table_params->dbz_scale;
  double clutter_bias = clut.table_params->dbz_bias;

  const DsFieldParams *fparams = radarMsg.getFieldParams(dbzFieldPos);
  double dbz_scale = fparams->scale;
  double dbz_bias = fparams->bias;
  
  double scale_diff = fabs(clutter_scale - dbz_scale);
  double bias_diff = fabs(clutter_bias - dbz_bias);
  
  // if the scale or bias differ, adjust the clutter table
  // to use the same scale as the data

  clut_table_index_t **clut_index = clut.table_index;

  if (scale_diff > 0.01 || bias_diff > 0.01) {
    
    for (int ielev = 0; ielev < lookup_params->nelevations; ielev++) {
      
      for (int iaz = 0; iaz < lookup_params->nazimuths; iaz++) {
	
	clut_table_entry_t *clutter_entry = clut_index[ielev][iaz].u.entry;
	int nclut_points = clut_index[ielev][iaz].nclut_points;
	
	for (int ipoint = 0; ipoint < nclut_points;
	     ipoint++, clutter_entry++) {
	  
	  double clutter_dbz =
	    (double) clutter_entry->dbz * clutter_scale + clutter_bias;
	  
	  int data_dbz_val = (int) (((clutter_dbz - dbz_bias) /
				     dbz_scale) + 0.5);
	  
	  if (data_dbz_val < 0)
	    data_dbz_val = 0;
	  if (data_dbz_val > 255)
	    data_dbz_val = 255;
	  
	  clutter_entry->dbz = data_dbz_val;
	  
	} /* ipoint */
	
      } /* iaz */
      
    } /* ielev */
    
  } /* if (scale_diff....... */

  return (iret);

}
  
/////////////////////////////////////////////////////
// elev_handle()
// 
// Computes the elevation index for a given elevation
//

#define CHECK_DELTA 0.01

int Lookup::elev_handle (double elev)

{

  int index;
  fl32 *elev_limits;

  radar_scan_table_t *scan_table = handle.scan_table;
  elev_limits = scan_table->elev_limits;

  if (fabs(elev - _prevElev) < CHECK_DELTA) {

    // same elevation as before, return the same index

    return(_prevElev_handle);

  } else {
    
    /*
     * elev has changed, find new index
     */

    index = RadarScanTableAng2ElNum(scan_table, elev);
    _prevElev = elev;
    _prevElev_handle = index;
    
    return(index);

  }

}

