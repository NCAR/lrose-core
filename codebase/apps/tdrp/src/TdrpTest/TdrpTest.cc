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
// TdrpTest.cc
//
// TdrpTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// TdrpTest tests the C++ functionality of TDRP
//
///////////////////////////////////////////////////////////////

using namespace std;

#include "TdrpTest.hh"
#include <string.h>
#include <stdlib.h>

// Constructor

TdrpTest::TdrpTest(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = strdup("TdrpTest");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  return;

}

// destructor

TdrpTest::~TdrpTest()

{

  // free up

  delete(_params);
  delete(_args);
  free(_progName);
  
}

//////////////////////////////////////////////////
// Run

int TdrpTest::Run()
{

  // optionally modify selected parameters
  
  if (_params->modify_and_print) {
    _modifySelectedParams();
  }

  /*
   * print out table to params.out
   */
  
  fprintf(stdout, "\n\n");
  fprintf(stdout, "======================================\n");
  fprintf(stdout, "INIT PRINTOUT OF PARAMETERS FROM TABLE\n");
  fprintf(stdout, "======================================\n\n");
  _params->sync();
  _params->print(stdout, PRINT_SHORT);
  fprintf(stdout, "\n\n");
  fprintf(stdout, "======================================\n\n");

  // print out the parameters from user space
  
  _doPrintout(stdout);

  // print table to params.in

  FILE *out;
  if ((out = fopen("params.in", "w")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", _progName);
    fprintf(stderr,
	    "Cannot open file params.in for writing table.\n");
    return (-1);
  }
  _params->print(out, PRINT_SHORT);
  fclose(out);

  // change a few params from within the program
  
  _params->your_age = 22;
  
  if (_params->our_ages_n > 0) {
    _params->_our_ages[_params->our_ages_n - 1] = 27;
  }
  
  if (_params->item_count_n1 > 0 &&
      _params->item_count_n2 > 0) {
    _params->__item_count[_params->item_count_n1 - 1]
      [_params->item_count_n2 - 1] = 77;
  }
  
  if (_params->storm_volume_n > 0) {
    _params->_storm_volume[_params->storm_volume_n - 1] = 99.99;
  }
  
  if (_params->rain_accumulation_n1 > 0 &&
      _params->rain_accumulation_n2 > 0) {
    _params->__rain_accumulation[_params->rain_accumulation_n1 - 1]
      [_params->rain_accumulation_n2 - 1] = 3.3;
  }
  
  _params->mass_coefficient = 8.8e-8;
  _params->debug = (tdrp_bool_t) TRUE;
  
  if (_params->compute_length_n1 > 0 &&
      _params->compute_length_n2 > 0) {
    _params->__compute_length[_params->compute_length_n1 - 1]
      [_params->compute_length_n2 - 1] = (tdrp_bool_t) TRUE;
  }
  
  TDRP_str_replace(&_params->input_file_ext, "dob");
  
  if (_params->input_file_paths_n > 0) {
    TDRP_str_replace
      (&_params->_input_file_paths[_params->input_file_paths_n - 1],
       "New_file_path");
  }
    
  _params->grid.dy = 3.9;

  if (_params->surface_stations_n > 0) {
    _params->_surface_stations[_params->surface_stations_n - 1].lat =
      39.9998;
    _params->_surface_stations[_params->surface_stations_n - 1].gauge_make =
      Params::ETI;
  }

  if (_params->data_field_n > 0) {
    TDRP_str_replace(&_params->_data_field[_params->data_field_n - 1].name,
		     "Spectral width");
  }

  /*
   * realloc some arrays
   */
  
  _params->arrayRealloc("our_ages", 7);
  _params->_our_ages[5] = 55;
  _params->_our_ages[6] = 70;
  
  _params->array2DRealloc("rain_accumulation", 6, 8);
  _params->array2DRealloc("output_file_paths", 4, 3);
  _params->arrayRealloc("data_field", 3);

  TDRP_str_replace(&_params->__output_file_paths[2][2], "Test_path");
  
  _params->_data_field[2].scale = 1.1;
  _params->_data_field[2].bias = -25.0;
  TDRP_str_replace(&_params->_data_field[2].name, "Signal-to-noise");
  TDRP_str_replace(&_params->_data_field[2].units, "dB");
  
  /*
   * copy params back to table
   */
  
  _params->sync();

  /*
   * print out table to params.out
   */
  
  fprintf(stdout, "\n\n");
  fprintf(stdout, "=======================================\n");
  fprintf(stdout, "FINAL PRINTOUT OF PARAMETERS FROM TABLE\n");
  fprintf(stdout, "=======================================\n\n");
  _params->print(stdout, PRINT_SHORT);
  fprintf(stdout, "\n\n");
  fprintf(stdout, "=======================================\n\n");

  return (0);

}

//////////////////////////////////////////////////
// modify selected params

void TdrpTest::_modifySelectedParams()

{

  _params->your_age = 99;

  _params->arrayRealloc("our_ages", 7);
  _params->_our_ages[5] = 98;
  _params->_our_ages[6] = 97;

  _params->mass_coefficient = -88.88;
  _params->__length_factor[1][4] = -77.77;

  _params->_flags[4] = (tdrp_bool_t) false;

  TDRP_str_replace(&_params->input_file_ext, "nc");
  
}

//////////////////////////////////////////////////
// doPrintout

void TdrpTest::_doPrintout(FILE *out)
{

  int i, j;

  /*
   * print out the parameters
   */

  fprintf(out, "\n\n");
  fprintf(out, "PRINTOUT OF PARAMETERS FROM USER SPACE\n");
  fprintf(out, "======================================\n\n");

  /*
   * int parameters
   */
  
  fprintf(out, "your_age: %d\n", _params->your_age);
  fprintf(out, "\n");

  fprintf(out, "our_ages[%d]:\n",
	  _params->our_ages_n);
  for (i = 0; i < _params->our_ages_n; i++) {
    fprintf(out, " %d", _params->_our_ages[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "icon[%d][%d]:\n",
	  _params->icon_n1, _params->icon_n2);
  for (j = 0; j < _params->icon_n1; j++) {
    for (i = 0; i < _params->icon_n2; i++) {
      fprintf(out, " %d", _params->__icon[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * long parameters
   */
  
  fprintf(out, "number_of_radars: %ld\n", _params->number_of_radars);
  fprintf(out, "\n");

  fprintf(out, "days_in_month[%d]:\n",
	  _params->days_in_month_n);
  for (i = 0; i < _params->days_in_month_n; i++) {
    fprintf(out, " %ld", _params->_days_in_month[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "item_count[%d][%d]:\n",
	  _params->item_count_n1, _params->item_count_n2);
  for (j = 0; j < _params->item_count_n1; j++) {
    for (i = 0; i < _params->item_count_n2; i++) {
      fprintf(out, " %ld", _params->__item_count[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * float parameters
   */
  
  fprintf(out, "speed: %g\n", _params->speed);
  fprintf(out, "\n");

  fprintf(out, "storm_volume[%d]:\n",
	  _params->storm_volume_n);
  for (i = 0; i < _params->storm_volume_n; i++) {
    fprintf(out, " %g", _params->_storm_volume[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "rain_accumulation[%d][%d]:\n",
	  _params->rain_accumulation_n1, _params->rain_accumulation_n2);
  for (j = 0; j < _params->rain_accumulation_n1; j++) {
    for (i = 0; i < _params->rain_accumulation_n2; i++) {
      fprintf(out, " %g", _params->__rain_accumulation[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * double parameters
   */
  
  fprintf(out, "mass_coefficient: %g\n", _params->mass_coefficient);
  fprintf(out, "\n");

  fprintf(out, "speed_of_light: %g\n", _params->speed_of_light);
  fprintf(out, "\n");

  fprintf(out, "storm_mass[%d]:\n",
	  _params->storm_mass_n);
  for (i = 0; i < _params->storm_mass_n; i++) {
    fprintf(out, " %g", _params->_storm_mass[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "length_factor[%d][%d]:\n",
	  _params->length_factor_n1, _params->length_factor_n2);
  for (j = 0; j < _params->length_factor_n1; j++) {
    for (i = 0; i < _params->length_factor_n2; i++) {
      fprintf(out, " %g", _params->__length_factor[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * boolean parameters
   */
  
#define BOOL_STR(a) ((a)? "TRUE" : "FALSE")

  fprintf(out, "use_data: %s\n", BOOL_STR(_params->use_data));
  fprintf(out, "\n");

  fprintf(out, "allow_outliers[%d]:\n",
	  _params->allow_outliers_n);
  for (i = 0; i < _params->allow_outliers_n; i++) {
    fprintf(out, " %s", BOOL_STR(_params->_allow_outliers[i]));
  }
  fprintf(out, "\n\n");

  fprintf(out, "compute_length[%d][%d]:\n",
	  _params->compute_length_n1, _params->compute_length_n2);
  for (j = 0; j < _params->compute_length_n1; j++) {
    for (i = 0; i < _params->compute_length_n2; i++) {
      fprintf(out, " %s", BOOL_STR(_params->__compute_length[j][i]));
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  fprintf(out, "debug: %s\n", BOOL_STR(_params->debug));
  fprintf(out, "\n");

  fprintf(out, "flags[%d]:\n",
	  _params->flags_n);
  for (i = 0; i < _params->flags_n; i++) {
    fprintf(out, " %s", BOOL_STR(_params->_flags[i]));
  }
  fprintf(out, "\n\n");

  /*
   * string parameters
   */

  fprintf(out, "path_delim = %s\n", _params->path_delim);
  fprintf(out, "\n");
  
  fprintf(out, "input_file_ext = %s\n", _params->input_file_ext);
  fprintf(out, "\n");

  fprintf(out, "input_file_paths[%d]:\n",
	  _params->input_file_paths_n);
  for (i = 0; i < _params->input_file_paths_n; i++) {
    fprintf(out, " %s", _params->_input_file_paths[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "output_file_paths[%d][%d]:\n",
	  _params->output_file_paths_n1, _params->output_file_paths_n2);
  for (j = 0; j < _params->output_file_paths_n1; j++) {
    for (i = 0; i < _params->output_file_paths_n2; i++) {
      fprintf(out, " %s", _params->__output_file_paths[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  fprintf(out, "input_dir = %s\n", _params->input_dir);
  fprintf(out, "\n");

  /*
   * enum parameters
   */
  
#define ORIGIN_STR(a) (((a) == Params::BOTLEFT? "BOTLEFT") : \
		       ((a) == Params::TOPLEFT? "TOPLEFT") : \
		       ((a) == Params::BOTRIGHT? "BOTRIGHT") : "TOPRIGHT")

  fprintf(out, "data_origin[%d]:\n",
	  _params->data_origin_n);
  for (i = 0; i < _params->data_origin_n; i++) {
    switch (_params->_data_origin[i]) {
    case Params::BOTLEFT: fprintf(out, " BOTLEFT"); break;
    case Params::TOPLEFT: fprintf(out, " TOPLEFT"); break;
    case Params::BOTRIGHT: fprintf(out, " BOTRIGHT"); break;
    case Params::TOPRIGHT: fprintf(out, " TOPRIGHT"); break;
    }
  }
  fprintf(out, "\n\n");

#define MODE_STR(a) ((a) == Params::ARCHIVE? "ARCHIVE" : "REALTIME")

  fprintf(out, "mode[%d][%d]:\n",
	  _params->mode_n1, _params->mode_n2);
  for (j = 0; j < _params->mode_n1; j++) {
    for (i = 0; i < _params->mode_n2; i++) {
      fprintf(out, " %s", MODE_STR(_params->__mode[j][i]));
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * struct parameters
   */
  
  fprintf(out, "grid:\n");
  fprintf(out, "  {nx, ny, minx, miny, dx, dx} ="
	  "  {%ld, %ld, %g, %g, %g, %g}\n",
	  _params->grid.nx,
	  _params->grid.ny,
	  _params->grid.minx,
	  _params->grid.miny,
	  _params->grid.dx,
	  _params->grid.dy);
  fprintf(out, "\n");

#define GAUGE_STR(a) ((a) == Params::ETI? "ETI" : \
		      (a) == Params::GEONOR? "GEONOR" : "CAMPBELL")

  fprintf(out, "surface_stations: {\n");
  for (i = 0; i < _params->surface_stations_n; i++) {
    fprintf(out, "  {lat, lon, wind_sensor_ht, gauge_make, has_humidity} =\n");
    fprintf(out, "     {%g, %g, %g, %s, %s}\n",
	    _params->_surface_stations[i].lat,
	    _params->_surface_stations[i].lon,
	    _params->_surface_stations[i].wind_sensor_ht,
	    GAUGE_STR(_params->_surface_stations[i].gauge_make),
	    BOOL_STR(_params->_surface_stations[i].has_humidity));
  }
  fprintf(out, "}\n\n");
  
  fprintf(out, "data_field: {\n");
  for (i = 0; i < _params->data_field_n; i++) {
    fprintf(out, "  {scale, bias, nplanes, name, units} = "
	    "{%g, %g, %ld, %s, %s}\n",
	    _params->_data_field[i].scale,
	    _params->_data_field[i].bias,
	    _params->_data_field[i].nplanes,
	    _params->_data_field[i].name,
	    _params->_data_field[i].units);
  }
  fprintf(out, "}\n\n");
  
  return;

}





