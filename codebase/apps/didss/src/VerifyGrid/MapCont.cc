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
// MapCont.cc
//
// Contingency grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "MapCont.h"
#include "Map.h"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
using namespace std;

//////////////
// Constructor

MapCont::MapCont(char *prog_name, Params *params) :
  Cont(prog_name, params)

{

  // create Map objects

  _nMaps = params->MapList_n;
  _maps = (Map **) umalloc(_nMaps * sizeof(Map *));

  for (int i = 0; i < _nMaps; i++) {
    _maps[i] = new Map(_progName, _params,
                       _params->_MapList[i].filename,
                       _params->_MapList[i].percent_covered);
  }
  
  if (_params->output_region_spdb) {
    _vGrid = new VerGridRegion();
  }

}

/////////////
// destructor

MapCont::~MapCont()

{

  if (_params->output_region_spdb) {
    delete (_vGrid);
  }

  for (int i = 0; i < _nMaps; i++) {
    delete(_maps[i]);
  }

  ufree(_maps);

}

////////////////////////////
// update()
//
// Update contingency table
//
// Returns 0 on success, -1 on failure

int MapCont::update(char *forecast_file_path)

{
  
  // load up computation grids

  if (_loadGrids(forecast_file_path)) {
    return (-1);
  }

  // get the cont file based on the forecast file
  
  if (_params->output_scan_cont) {
    if (_openContFile(_forecastGrid->timeCent)) {
      return (-1);
    }
  }
  
  // if required, print out start of line for scan-by-scan
  // contingency data

  time_t forecast_time = _forecastGrid->timeCent;

  if (_params->output_scan_cont) {
    
    UTIMstruct time_struct;
    UTIMunix_to_date(forecast_time, &time_struct);

    fprintf(_scanContFile, "%ld %ld %ld %ld %ld %ld %g %g ",
	    time_struct.year, time_struct.month, time_struct.day,
	    time_struct.hour, time_struct.min, time_struct.sec,
	    (double) _params->forecast_lead_time / 60.0,
	    _params->forecast_level_lower);

    fflush(_scanContFile);

  }

  // load spdb header if required

  if (_params->output_region_spdb) {
    _vGrid->setHdr(_params->MapList_n,
		   forecast_time,
		   _params->forecast_lead_time,
		   _params->forecast_ht,
		   _params->forecast_level_lower,
		   _params->forecast_level_upper,
		   _params->truth_ht,
		   _params->truth_level_lower,
		   _params->truth_level_upper);
  }

  // compute contingency

  if (_params->debug >= Params::DEBUG_NORM) {
    _maps[0]->printHeader(stdout);
  }

  // Pass these data as effective circles to the SPDB server.

  for (int i = 0; i < _nMaps; i++) {
    _maps[i]->update(_forecastGrid, _truthGrid, &_cont);
    if (_params->output_region_spdb) {
      _vGrid->setData(i,
		      _maps[i]->_regionName,
		      _maps[i]->_latCentroid,
		      _maps[i]->_lonCentroid,
		      _maps[i]->_meanRadius,
		      _maps[i]->_percentCoveredForecast,
		      _maps[i]->_percentCoveredTruth);
    }
    if (_params->debug >= Params::DEBUG_NORM) {
      _maps[i]->printCont(stdout);
    }
  }

  // write spdb data if required

  if (_params->output_region_spdb) {

    _vGrid->to_BE();
    Spdb spdb;
    if (spdb.put(_params->region_spdb_dir,
                 SPDB_VERGRID_REGION_ID,
                 SPDB_VERGRID_REGION_LABEL,
                 1,
                 forecast_time,
                 forecast_time + _params->spdb_valid_period,
                 _vGrid->chunkLen(),
                 _vGrid->chunkPtr())) {
      fprintf(stderr, "ERROR - %s:MapCont::update()\n", _progName);
      fprintf(stderr, "Cannot put spdb data to directory '%s'\n",
	      _params->region_spdb_dir);
      cerr << spdb.getErrStr() << endl;
    }

    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Writing to SPDB, time %s\n", utimstr(forecast_time));
    }

  } // if (_params->output_region_spdb)

  if (_params->output_scan_cont) {
    fprintf(_scanContFile, "\n");
  }

  return (0);

}

//////////
// print()
//

void MapCont::print(FILE *out)

{

  _maps[0]->printHeader(stdout);
  for (int i = 0; i < _nMaps; i++) {
    _maps[i]->printCont(out);
  }

}







