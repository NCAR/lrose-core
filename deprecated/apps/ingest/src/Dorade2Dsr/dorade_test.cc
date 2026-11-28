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
/////////////////////////////////////////////////////////////////////
// dorade_test.cc
//
// From peter.cc from Joe Vanandel
//

#include <dataport/port_types.h>
#include <dd_sweepfiles.hh>
#include <iostream>
#include <string>
using namespace std;

static string label(const char *label, int maxLen);

int main (int argc, char **argv)
{

  if (argc != 2) {
    cerr << "Usage: dorade_test inpath" << endl;
    return -1;
  }

  char *inpath = argv[1];

  cerr << "inpath: " << inpath << endl;

  // instantiate the dorade mapper

  dd_mapper * mapr = new dd_mapper();

  // instantiate an object for accessing an existing sweepfile

  dd_sweepfile_access * sac = new dd_sweepfile_access();

  // open sweepfile and read in the first ray (i.e. map it!)
  
  if( !sac->access_sweepfile( inpath, mapr ) ){ 
    cerr << "Cannot read in first ray, file: " << inpath << endl;
    return -1;
  }

  cerr << "mapr date: "
       << mapr->year() << "/"
       << mapr->month() << "/"
       << mapr->day() << " "
       << mapr->hour() << ":"
       << mapr->minute() << ":"
       << mapr->second() << endl;

  cerr << "proj_name: " << label(mapr->proj_name(), 20) << endl;
  cerr << "radar_name: " << label(mapr->radar_name(), 8) << endl;

  cerr << "volume_num: " << mapr->volume_num() << endl;
  cerr << "sweep_num: " << mapr->sweep_num() << endl;
  cerr << "new_vol: " << mapr->new_vol() << endl;
  cerr << "new_sweep: " << mapr->new_sweep() << endl;
  cerr << "new_ray: " << mapr->new_ray() << endl;
  cerr << "new_mpb: " << mapr->new_mpb() << endl;
  cerr << "volume_count: " << mapr->volume_count() << endl;
  cerr << "sweep_count: " << mapr->sweep_count() << endl;
  cerr << "total_ray_count: " << mapr->total_ray_count() << endl;
  cerr << "sweep_ray_count: " << mapr->sweep_ray_count() << endl;
  cerr << "sizeof_ray: " << mapr->sizeof_ray() << endl;
  cerr << "complete: " << mapr->complete() << endl;
  cerr << "found_ryib: " << mapr->found_ryib() << endl;
  cerr << "error_state: " << mapr->error_state() << endl;
  cerr << "swapped_data: " << mapr->swapped_data() << endl;
  cerr << "radar_type: " << mapr->radar_type() << endl;
  cerr << "radar_type_ascii: " << mapr->radar_type_ascii() << endl;
  cerr << "scan_mode: " << mapr->scan_mode() << endl;
  cerr << "scan_mode_mne: " << mapr->scan_mode_mne() << endl;
  cerr << "rays_in_sweep: " << mapr->rays_in_sweep() << endl;
  cerr << "latitude: " << mapr->latitude() << endl;
  cerr << "longitude: " << mapr->longitude() << endl;
  cerr << "altitude_km: " << mapr->altitude_km() << endl;
  cerr << "azimuth: " << mapr->azimuth() << endl;
  cerr << "elevation: " << mapr->elevation() << endl;
  cerr << "fixed_angle: " << mapr->fixed_angle() << endl;
  cerr << "roll: " << mapr->roll() << endl;
  cerr << "rotation_angle: " << mapr->rotation_angle() << endl;
  cerr << "number_of_cells: " << mapr->number_of_cells() << endl;
  cerr << "meters_to_first_cell: " << mapr->meters_to_first_cell() << endl;
  cerr << "meters_between_cells: " << mapr->meters_between_cells() << endl;
  cerr << "min_cell_spacing: " << mapr->min_cell_spacing() << endl;
  cerr << "meters_to_last_cell: " << mapr->meters_to_last_cell() << endl;
  cerr << "return_num_samples: " << mapr->return_num_samples(0) << endl;

  cerr << "num_fields: " << mapr->num_fields() << endl;
  for (int ii = 0; ii < mapr->num_fields(); ii++) {
    PARAMETER *fparam = mapr->param_ptr(mapr->field_name(ii));
    cerr << "Field: " << ii << " name: "
	 << label(fparam->parameter_name, 8) << endl;
    cerr << "  scale: " << fparam->parameter_scale << endl;
    cerr << "  bias: " << fparam->parameter_bias << endl;
    cerr << "  bad_data_flag: " << fparam->bad_data << endl;
    cerr << "  field type: " << fparam->binary_format << endl;
    cerr << "  field units: " << label(fparam->param_units, 8) << endl;
  }

  mapr->cfac->latitude_corr += 0.0024;
  mapr->cfac->longitude_corr += 0.0013;

  // the dorade headers are now publically available and
  // could be modified within the loop 

  // opens new sweepfile but does not write the first ray
  // this also assumes both the input field and the output field
  // are present in the source sweepfile
  // the output sweepfile name is derived from the first ray just as in the
  // translaters 
  
  // math functions expect double precision arguments
  // x**y is implementedd as "pow( x, y )" in C/C++ where x and y are double
  // precision variables
  //
  
  // loop through rays

  int azCount = 0;

  do {
    
    cerr << "time, count, azimuth, elevation: "
	 << mapr->year() << "/"
	 << mapr->month() << "/"
	 << mapr->day() << " "
	 << mapr->hour() << ":"
	 << mapr->minute() << ":"
	 << mapr->second() << ", "
	 << azCount << ", " << mapr->azimuth()
	 << ", " << mapr->elevation() << endl;

    int ncells = mapr->number_of_cells();
    cerr << "number_of_cells: " << ncells << endl;

    for (int ii = 0; ii < mapr->num_fields(); ii++) {

      PARAMETER *fparam = mapr->param_ptr(mapr->field_name(ii));

      double scale = fparam->parameter_scale;
      double bias = fparam->parameter_bias;

      switch (fparam->binary_format) {

      case 4:
	{
	  fl32 *vals = (fl32 *) mapr->raw_data_ptr(ii);
	  for (int jj = 0; jj < ncells; jj++) {
	    cerr << vals[jj] << " ";
	  }
	  cerr << endl;
	}
	break;
	  
      case 3:
	{
	  ui32 *vals = (ui32 *) mapr->raw_data_ptr(ii);
	  for (int jj = 0; jj < ncells; jj++) {
	    cerr << vals[jj] * scale + bias << " ";
	  }
	  cerr << endl;
	}
	break;
	  
      case 2:
	{
	  ui16 *vals = (ui16 *) mapr->raw_data_ptr(ii);
	  for (int jj = 0; jj < ncells; jj++) {
	    cerr << vals[jj] * scale + bias << " ";
	  }
	  cerr << endl;
	}
	break;
	  
      case 1:
      default:
	{
	  ui08 *vals = (ui08 *) mapr->raw_data_ptr(ii);
	  for (int jj = 0; jj < ncells; jj++) {
	    cerr << vals[jj] * scale + bias << " ";
	  }
	  cerr << endl;
	}
	break;
	  
      } // switch

      cerr << "Field: " << ii << " name: "
	   << label(fparam->parameter_name, 8) << endl;
      cerr << "  scale: " << fparam->parameter_scale << endl;
      cerr << "  bias: " << fparam->parameter_bias << endl;
      cerr << "  bad_data_flag: " << fparam->bad_data << endl;
      cerr << "  field type: " << fparam->binary_format << endl;
      cerr << "  field units: " << label(fparam->param_units, 8) << endl;

    }

    azCount++;
    
  } while (sac->next_ray() != LAST_RAY);

}

// c---------------------------------------------------------------------------

void dd_Testify(char * message)
{
    // sweepfiles codes need this routine
    int nn;
    char * mess = message;
    char str[512];

    // for now just use printf
    if(!mess || !(nn = strlen(mess))) {
	return;
    }
    if( nn > (int) sizeof(str) -1) {
	strncpy(str, mess, sizeof(str) -1);
	str[sizeof(str) -1] = '\0';
	mess = str;
    }
    printf("%s", mess);
}

// c---------------------------------------------------------------------------

static string label(const char *label, int maxLen)

{

  // null terminate

  char copy[maxLen + 1];
  memset(copy, 0, maxLen + 1);
  memcpy(copy, label, maxLen);
  
  // remove blanks

  int startPos = 0;
  for (int ii = 0; ii <= maxLen; ii++) {
    if (copy[ii] == ' ') {
      startPos++;
    } else {
      break;
    }
  }

  for (int ii = maxLen; ii >= 0; ii--) {
    if (copy[ii] == ' ') {
      copy[ii] = '\0';
    } else {
      break;
    }
  }

  return copy + startPos;

}



