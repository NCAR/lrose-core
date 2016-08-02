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
// Comps.cc
//
// Computations abstract base class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Comps.h"
#include <toolsa/str.h>
#include <rapmath/math_macros.h>

//////////////
// Constructor

Comps::Comps(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;
  _forecastGrid = NULL;
  _truthGrid = NULL;

}

/////////////
// destructor

Comps::~Comps()

{

  STRfree(_progName);
  _freeGrids();

}

////////////////////////////
// _loadGrids()
//
// Load up grids for computations
//
// Returns 0 on success, -1 on failure

int Comps::_loadGrids(char *forecast_file_path)

{

  _freeGrids();

  // get the file path for the truth data - if this returns
  // error, there is no truth data within the time margin of
  // the forecast data, so return early
  
  char truth_file_path[MAX_PATH_LEN];
  if (_getTruthPath(forecast_file_path, truth_file_path)) {
    return (-1);
  }

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Forecast file '%s'\n", forecast_file_path);
    fprintf(stderr, "Truth    file '%s'\n", truth_file_path);
  }

  // read in the grid data from the truth and forecast files
  
  _forecastGrid = new Grid(_progName, _params,
			   "forecast", forecast_file_path,
			   _params->forecast_field,
			   _params->forecast_ht);
  
  if (!_forecastGrid->OK) {
    _freeGrids();
    return (-1);
  }

  _truthGrid = new Grid(_progName, _params,
			"truth", truth_file_path,
			_params->truth_field,
			_params->truth_ht);

  if (!_truthGrid->OK) {
    _freeGrids();
    return (-1);
  }

  // check that the geometry is the same

  if (_params->check_grid_geom && _forecastGrid->checkGeom(_truthGrid)) {

    fprintf(stderr, "ERROR - %s:Comps:_loadGrids\n", _progName);
    fprintf(stderr, "Cartesian grids do not match\n\n");

    fprintf(stderr, "\nFile '%s'\n", forecast_file_path);
    _forecastGrid->printGeom(stderr, "  ");

    fprintf(stderr, "\nFile '%s'\n", truth_file_path);
    _truthGrid->printGeom(stderr, "  ");

    _freeGrids();

    return(-1);
    
  }

  return (0);

}

////////////////////////////
// _freeGrids()
//
// Frees up grids for computations
//

void Comps::_freeGrids()

{
  
  if (_forecastGrid != NULL) {
    delete (_forecastGrid);
    _forecastGrid = NULL;
  }

  if (_truthGrid != NULL) {
    delete (_truthGrid);
    _truthGrid = NULL;
  }

}

/////////////////
// _getTruthPath()
//
// Gets the path to the truth file.
// truth_file_path must point to memory allocated above with length
// of MAX_PATH_LEN.
//
// Returns 0 on success, -1 on failure.
//

int Comps::_getTruthPath(char *forecast_file_path,
			 char *truth_file_path)
  
{
  
  char dir_path[MAX_PATH_LEN];
  char *name, file_ext[16];
  char *last_delim;

  int file_found;

  int time_error;
  int iday, ndays;
  time_t mid_time;

  date_time_t ft;
  date_time_t start_dt, end_dt, *this_dt;

  /*
   * parse the forecast file path to get date and time
   */

  if ((last_delim = strrchr(forecast_file_path, '/')) == NULL) {
    
    fprintf(stderr, "ERROR - %s:Comps::_getTruthPath\n", _progName);
    fprintf(stderr, "File paths do not include the date directories\n");
    fprintf(stderr, "Example is '%s'\n", forecast_file_path);
    fprintf(stderr,
	    "Run the program from the forecast data directory or above\n");
    return(-1);

  }
  
  name = last_delim - 8;

  if (sscanf(name, "%4d%2d%2d/%2d%2d%2d.%s",
	     &ft.year, &ft.month, &ft.day,
	     &ft.hour, &ft.min, &ft.sec, file_ext) != 7) {

    fprintf(stderr, "ERROR - %s:Comps::_getTruthPath\n", _progName);
    fprintf(stderr, "Cannot process file path '%s'\n",
	    forecast_file_path);
    return(-1);

  }

  uconvert_to_utime(&ft);

  mid_time = ft.unix_time + _params->forecast_lead_time;
  start_dt.unix_time = mid_time - _params->file_time_margin;
  uconvert_from_utime(&start_dt);
  end_dt.unix_time = mid_time + _params->file_time_margin;
  uconvert_from_utime(&end_dt);

  time_error = LARGE_LONG;
  file_found = FALSE;

  if (start_dt.day != end_dt.day)
    ndays = 2;
  else
    ndays = 1;

  for (iday = 0; iday < ndays; iday++) {
      
    if (iday == 0)
      this_dt = &start_dt;
    else
      this_dt = &end_dt;
      
    /*
     * search through the data directory for a matching file in
     * the correct subdirectory
     */
    
    sprintf(dir_path, "%s/%.4d%.2d%.2d",
	    _params->truth_data_dir,
	    this_dt->year, this_dt->month, this_dt->day);
	
    /*
     * load file name of the closest scan to the requested time
     */
    
    if (_params->debug) {
      cerr << "searching truth dir: " << dir_path << endl;
    }

    if(_findBestFile(dir_path, truth_file_path,
		     start_dt.unix_time, end_dt.unix_time, this_dt,
		     mid_time, &time_error) == 0) {
      file_found = TRUE;
    }
    
  } /* iday */
  
  if (file_found) {
    return (0);
  } else {
    return (-1);
  }
    
}  

#include <dirent.h>
using namespace std;

////////////////////
// _findBestFile()
//
// Find the most appropriate file given the
// time frame requested
//

int Comps::_findBestFile (char *dir_path, char *file_path,
			  time_t start_time, time_t end_time,
			  date_time_t *dir_dt, time_t search_time,
			  int *time_error)

{
  
  char file_ext[16];
  int f_count;
  int time_diff;
  int hour, min, sec;
  
  date_time_t file_dt;
  
  struct dirent *dp;
  DIR *dirp;
  
  /*
   * open directory - return -1 if unable to open
   */
  
  if ((dirp = opendir(dir_path)) == NULL) {
    return (-1);
  }
  
  /*
   * Loop thru directory looking for the data file names
   */
  
  f_count = 0;
  
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.')
      continue;
      
    /*
     * check that the dir name is in the correct format
     */

    if (sscanf(dp->d_name, "%2d%2d%2d.%s",
	       &hour, &min, &sec, file_ext) != 4)
      continue;
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59)
      continue;

    /*
     * file name is in correct format. Therefore, accept it
     */
    
    file_dt.year = dir_dt->year;
    file_dt.month = dir_dt->month;
    file_dt.day = dir_dt->day;
    file_dt.hour = hour;
    file_dt.min = min;
    file_dt.sec = sec;
	
    uconvert_to_utime(&file_dt);
	
    if (file_dt.unix_time >= start_time &&
	file_dt.unix_time <= end_time) {
	  
      time_diff = abs ((int) (file_dt.unix_time - search_time));
	  
      if (time_diff < *time_error) {
	    
	*time_error = time_diff;
	sprintf(file_path, "%s%s%s",
		dir_path, PATH_DELIM,
		dp->d_name);
	f_count++;
	    
      } /*if (time_diff < *time_error) */
	  
    } /* if (file_dt.unix_time >= com->time_min ... */

  } /* dp */
  
  closedir(dirp);
  
  if(f_count == 0)
    return (-1);
  else
    return (0);
  
}


