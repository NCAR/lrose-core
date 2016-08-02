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
/////////////////////////////////////////////////////////////
// InputFile.hh: Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
/////////////////////////////////////////////////////////////

#ifndef INPUTFILE_H
#define INPUTFILE_H

#include "MdvMosaic.hh"
#include <mdv/mdv_handle.h>
#include <mdv/mdv_grid.h>
using namespace std;

class InputFile {
  
public:

  InputFile(char *prog_name, MdvMosaic_tdrp_struct *params,
	    char *dir, int master,
	    mdv_grid_t *output_grid,
	    lat_lon_t *_loc_array);


  ~InputFile();

  // read in the relevant file
  int read(time_t request_time);

  // get min and max vals for a field
  int getMinAndMax(int field_num, double *min_p, double *max_p);
  
  // get handle
  MDV_handle_t *handle() { return (&_handle); }

  // merge field data
  void mergeField(int in_field,
		  MDV_handle_t *outHandle, mdv_grid_t *outGrid,
		  int out_field, double out_scale, double out_bias);

  // update the start and end times
  void updateTimes(time_t *start_time_p, time_t *end_time_p);

  // was latest read a success?
  int readSuccess; // set true if last read was a success

  // file path
  char *path() { return (_path); }

protected:
  
private:

  char *_progName;
  MdvMosaic_tdrp_struct *_params;

  char *_dir; // data directory

  mdv_grid_t _grid; // grid from latest file read

  mdv_grid_t *_outputGrid; // output grid params

  // set _master to TRUE if this is the master data set - i.e. the
  // first in the list
  int _master;

  MDV_handle_t _handle; // mdv file handle

  char _path[MAX_PATH_LEN]; // actual path used

  time_t _time; // actual time for data in file

  lat_lon_t *_locArray;

  long *_xyLut; // Lookup table in (x,y)
  int *_zLut;   // Lookup table in z

  int _out_lut_nx;
  int _out_lut_ny;

  int _out_lut_minx_idx;   // output minimum x index for current input grid
  int _out_lut_miny_idx;   // output minimum y index for current input grid

  int _out_lut_maxx_idx;   // output maximum x index for current input grid
  int _out_lut_maxy_idx;   // output maximum y index for current input grid

  // functions

  int _loadPath(time_t request_time);

  int _loadFirstBefore(time_t request_time,
		       int max_time_offset);

  void _computeXYLookup();
  void _computeZLookup();

};

#endif
