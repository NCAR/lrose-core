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
// TdrpSingleton.cc
//
// TdrpSingleton object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2019
//
///////////////////////////////////////////////////////////////
//
// TdrpSingleton tests the C++ singleton functionality of TDRP
//
///////////////////////////////////////////////////////////////

using namespace std;

#include "TdrpSingleton.hh"
#include <string.h>
#include <stdlib.h>

// Constructor

TdrpSingleton::TdrpSingleton(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = strdup("TdrpSingleton");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  Params *params = Params::Inst();
  _paramsPath = (char *) "unknown";
  if (params->loadFromArgs(argc, argv,
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

TdrpSingleton::~TdrpSingleton()

{

  // free up

  delete(_args);
  delete Params::Inst();
  free(_progName);
  
}

//////////////////////////////////////////////////
// Run

int TdrpSingleton::Run()
{

  // get params

  Params *params = Params::Inst();

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
  params->print(out, PRINT_SHORT);
  fclose(out);

  // change a few params from within the program
  
  params->your_age = 22;
  
  if (params->our_ages_n > 0) {
    params->_our_ages[params->our_ages_n - 1] = 27;
  }
  
  if (params->item_count_n1 > 0 &&
      params->item_count_n2 > 0) {
    params->__item_count[params->item_count_n1 - 1]
      [params->item_count_n2 - 1] = 77;
  }
  
  if (params->storm_volume_n > 0) {
    params->_storm_volume[params->storm_volume_n - 1] = 99.99;
  }
  
  if (params->rain_accumulation_n1 > 0 &&
      params->rain_accumulation_n2 > 0) {
    params->__rain_accumulation[params->rain_accumulation_n1 - 1]
      [params->rain_accumulation_n2 - 1] = 3.3;
  }
  
  params->mass_coefficient = 8.8e-8;
  params->debug = (tdrp_bool_t) TRUE;
  
  if (params->compute_length_n1 > 0 &&
      params->compute_length_n2 > 0) {
    params->__compute_length[params->compute_length_n1 - 1]
      [params->compute_length_n2 - 1] = (tdrp_bool_t) TRUE;
  }
  
  TDRP_str_replace(&params->input_file_ext, "dob");
  
  if (params->input_file_paths_n > 0) {
    TDRP_str_replace
      (&params->_input_file_paths[params->input_file_paths_n - 1],
       "New_file_path");
  }
    
  params->grid.dy = 3.9;

  if (params->surface_stations_n > 0) {
    params->_surface_stations[params->surface_stations_n - 1].lat =
      39.9998;
    params->_surface_stations[params->surface_stations_n - 1].gauge_make =
      Params::ETI;
  }

  if (params->data_field_n > 0) {
    TDRP_str_replace(&params->_data_field[params->data_field_n - 1].name,
		     "Spectral width");
  }

  /*
   * realloc some arrays
   */
  
  params->arrayRealloc("our_ages", 7);
  params->_our_ages[5] = 55;
  params->_our_ages[6] = 70;
  
  params->array2DRealloc("rain_accumulation", 6, 8);
  params->array2DRealloc("output_file_paths", 4, 3);
  params->arrayRealloc("data_field", 3);

  TDRP_str_replace(&params->__output_file_paths[2][2], "Test_path");
  
  params->_data_field[2].scale = 1.1;
  params->_data_field[2].bias = -25.0;
  TDRP_str_replace(&params->_data_field[2].name, "Signal-to-noise");
  TDRP_str_replace(&params->_data_field[2].units, "dB");
  
  /*
   * copy params back to table
   */
  
  params->sync();

  /*
   * print out table to params.out
   */
  
  fprintf(stdout, "\n\n");
  fprintf(stdout, "PRINTOUT OF PARAMETERS FROM TABLE\n");
  fprintf(stdout, "=================================\n\n");
  params->print(stdout, PRINT_SHORT);

  return (0);

}

//////////////////////////////////////////////////
// doPrintout

void TdrpSingleton::_doPrintout(FILE *out)
{

  // get params

  Params *params = Params::Inst();

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
  
  fprintf(out, "your_age: %d\n", params->your_age);
  fprintf(out, "\n");

  fprintf(out, "our_ages[%d]:\n",
	  params->our_ages_n);
  for (i = 0; i < params->our_ages_n; i++) {
    fprintf(out, " %d", params->_our_ages[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "icon[%d][%d]:\n",
	  params->icon_n1, params->icon_n2);
  for (j = 0; j < params->icon_n1; j++) {
    for (i = 0; i < params->icon_n2; i++) {
      fprintf(out, " %d", params->__icon[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * long parameters
   */
  
  fprintf(out, "number_of_radars: %ld\n", params->number_of_radars);
  fprintf(out, "\n");

  fprintf(out, "days_in_month[%d]:\n",
	  params->days_in_month_n);
  for (i = 0; i < params->days_in_month_n; i++) {
    fprintf(out, " %ld", params->_days_in_month[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "item_count[%d][%d]:\n",
	  params->item_count_n1, params->item_count_n2);
  for (j = 0; j < params->item_count_n1; j++) {
    for (i = 0; i < params->item_count_n2; i++) {
      fprintf(out, " %ld", params->__item_count[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * float parameters
   */
  
  fprintf(out, "speed: %g\n", params->speed);
  fprintf(out, "\n");

  fprintf(out, "storm_volume[%d]:\n",
	  params->storm_volume_n);
  for (i = 0; i < params->storm_volume_n; i++) {
    fprintf(out, " %g", params->_storm_volume[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "rain_accumulation[%d][%d]:\n",
	  params->rain_accumulation_n1, params->rain_accumulation_n2);
  for (j = 0; j < params->rain_accumulation_n1; j++) {
    for (i = 0; i < params->rain_accumulation_n2; i++) {
      fprintf(out, " %g", params->__rain_accumulation[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * double parameters
   */
  
  fprintf(out, "mass_coefficient: %g\n", params->mass_coefficient);
  fprintf(out, "\n");

  fprintf(out, "speed_of_light: %g\n", params->speed_of_light);
  fprintf(out, "\n");

  fprintf(out, "storm_mass[%d]:\n",
	  params->storm_mass_n);
  for (i = 0; i < params->storm_mass_n; i++) {
    fprintf(out, " %g", params->_storm_mass[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "length_factor[%d][%d]:\n",
	  params->length_factor_n1, params->length_factor_n2);
  for (j = 0; j < params->length_factor_n1; j++) {
    for (i = 0; i < params->length_factor_n2; i++) {
      fprintf(out, " %g", params->__length_factor[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  /*
   * boolean parameters
   */
  
#define BOOL_STR(a) ((a)? "TRUE" : "FALSE")

  fprintf(out, "use_data: %s\n", BOOL_STR(params->use_data));
  fprintf(out, "\n");

  fprintf(out, "allow_outliers[%d]:\n",
	  params->allow_outliers_n);
  for (i = 0; i < params->allow_outliers_n; i++) {
    fprintf(out, " %s", BOOL_STR(params->_allow_outliers[i]));
  }
  fprintf(out, "\n\n");

  fprintf(out, "compute_length[%d][%d]:\n",
	  params->compute_length_n1, params->compute_length_n2);
  for (j = 0; j < params->compute_length_n1; j++) {
    for (i = 0; i < params->compute_length_n2; i++) {
      fprintf(out, " %s", BOOL_STR(params->__compute_length[j][i]));
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  fprintf(out, "debug: %s\n", BOOL_STR(params->debug));
  fprintf(out, "\n");

  fprintf(out, "flags[%d]:\n",
	  params->flags_n);
  for (i = 0; i < params->flags_n; i++) {
    fprintf(out, " %s", BOOL_STR(params->_flags[i]));
  }
  fprintf(out, "\n\n");

  /*
   * string parameters
   */

  fprintf(out, "path_delim = %s\n", params->path_delim);
  fprintf(out, "\n");
  
  fprintf(out, "input_file_ext = %s\n", params->input_file_ext);
  fprintf(out, "\n");

  fprintf(out, "input_file_paths[%d]:\n",
	  params->input_file_paths_n);
  for (i = 0; i < params->input_file_paths_n; i++) {
    fprintf(out, " %s", params->_input_file_paths[i]);
  }
  fprintf(out, "\n\n");

  fprintf(out, "output_file_paths[%d][%d]:\n",
	  params->output_file_paths_n1, params->output_file_paths_n2);
  for (j = 0; j < params->output_file_paths_n1; j++) {
    for (i = 0; i < params->output_file_paths_n2; i++) {
      fprintf(out, " %s", params->__output_file_paths[j][i]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  
  fprintf(out, "input_dir = %s\n", params->input_dir);
  fprintf(out, "\n");

  /*
   * enum parameters
   */
  
#define ORIGIN_STR(a) (((a) == Params::BOTLEFT? "BOTLEFT") : \
		       ((a) == Params::TOPLEFT? "TOPLEFT") : \
		       ((a) == Params::BOTRIGHT? "BOTRIGHT") : "TOPRIGHT")

  fprintf(out, "data_origin[%d]:\n",
	  params->data_origin_n);
  for (i = 0; i < params->data_origin_n; i++) {
    switch (params->_data_origin[i]) {
    case Params::BOTLEFT: fprintf(out, " BOTLEFT"); break;
    case Params::TOPLEFT: fprintf(out, " TOPLEFT"); break;
    case Params::BOTRIGHT: fprintf(out, " BOTRIGHT"); break;
    case Params::TOPRIGHT: fprintf(out, " TOPRIGHT"); break;
    }
  }
  fprintf(out, "\n\n");

#define MODE_STR(a) ((a) == Params::ARCHIVE? "ARCHIVE" : "REALTIME")

  fprintf(out, "mode[%d][%d]:\n",
	  params->mode_n1, params->mode_n2);
  for (j = 0; j < params->mode_n1; j++) {
    for (i = 0; i < params->mode_n2; i++) {
      fprintf(out, " %s", MODE_STR(params->__mode[j][i]));
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
	  params->grid.nx,
	  params->grid.ny,
	  params->grid.minx,
	  params->grid.miny,
	  params->grid.dx,
	  params->grid.dy);
  fprintf(out, "\n");

#define GAUGE_STR(a) ((a) == Params::ETI? "ETI" : \
		      (a) == Params::GEONOR? "GEONOR" : "CAMPBELL")

  fprintf(out, "surface_stations: {\n");
  for (i = 0; i < params->surface_stations_n; i++) {
    fprintf(out, "  {lat, lon, wind_sensor_ht, gauge_make, has_humidity} =\n");
    fprintf(out, "     {%g, %g, %g, %s, %s}\n",
	    params->_surface_stations[i].lat,
	    params->_surface_stations[i].lon,
	    params->_surface_stations[i].wind_sensor_ht,
	    GAUGE_STR(params->_surface_stations[i].gauge_make),
	    BOOL_STR(params->_surface_stations[i].has_humidity));
  }
  fprintf(out, "}\n\n");
  
  fprintf(out, "data_field: {\n");
  for (i = 0; i < params->data_field_n; i++) {
    fprintf(out, "  {scale, bias, nplanes, name, units} = "
	    "{%g, %g, %ld, %s, %s}\n",
	    params->_data_field[i].scale,
	    params->_data_field[i].bias,
	    params->_data_field[i].nplanes,
	    params->_data_field[i].name,
	    params->_data_field[i].units);
  }
  fprintf(out, "}\n\n");
  
  return;

}





