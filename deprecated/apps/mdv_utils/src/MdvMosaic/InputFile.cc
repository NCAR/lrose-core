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
//////////////////////////////////////////////////////////
// InputFile.cc : Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
//////////////////////////////////////////////////////////
//
// This module reads in files from the relevant input
// directory, computes the lookup table to convert this
// file to the output grid, and loads up the output
// grid with data from the file.
//
///////////////////////////////////////////////////////////

#include "InputFile.hh"
#include <toolsa/pjg.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>
#include <toolsa/str.h>
#include <mdv/mdv_read.h>
using namespace std;

//////////////
// Constructor

InputFile::InputFile(char *prog_name, MdvMosaic_tdrp_struct *params,
		     char *dir, int master,
		     mdv_grid_t *output_grid,
		     lat_lon_t *loc_array)

{

  _progName = STRdup(prog_name);
  _params = params;
  _dir = STRdup(dir);
  _master = master;
  _outputGrid = output_grid;
  _locArray = loc_array;
  memset(&_grid, 0, sizeof(mdv_grid_t));
  MDV_init_handle(&_handle);

  //MCP perhaps move this initialization into _computeXYLut if we can...?
  _xyLut = (long *) umalloc(2 * 2 * sizeof(long));
  _zLut = (int *) umalloc(_outputGrid->nz * sizeof(int));
	 
}

/////////////
// Destructor

InputFile::~InputFile()

{

  STRfree(_progName);
  STRfree(_dir);
  MDV_free_handle(&_handle);
  ufree(_xyLut);
  ufree(_zLut);

}

//////////////////////////////
// read in file for given time
//
// returns 0 on success, -1 on failure

int InputFile::read(time_t request_time)

{

  readSuccess = FALSE;

  if (_loadPath(request_time)) {
    return (-1);
  }
  
  if (_params->debug) {
    fprintf(stderr, "Reading input file '%s'\n", _path);
  } 

  if (MDV_read_all(&_handle, _path, MDV_INT8)) {
    return (-1);
  }

  // compute lookup table if grid differs from previous one

  if (memcmp(&_handle.grid, &_grid, sizeof(mdv_grid_t))) {
    _grid = _handle.grid;
    if (_params->debug) {
      fprintf(stderr, "====> Computing lookups\n");
    }
    _computeXYLookup();
    _computeZLookup();
  }

  // check the number of fields
  for (int i = 0; i < _params->field_list.len; i++) {
    int in_field = _params->field_list.val[i];
    if (in_field > _handle.master_hdr.n_fields -1) {
      fprintf(stderr, "ERROR - %s:InputFile::read\n", _progName);
      fprintf(stderr, "Field %d not available.\n", in_field);
      fprintf(stderr, "File %s has only %d fields\n",
	      _path, _handle.master_hdr.n_fields);
      fprintf(stderr, "Remember - field numbers are 0-based.\n");
      return (-1);
    }
  }

  readSuccess = TRUE;
  return (0);

}

//////////////////////////////////
// get min and max for given field
//
// Returns 0 on success, -1 on failure
//

int InputFile::getMinAndMax(int field_num, double *min_p, double *max_p)

{

  int min_byte = 256;
  int max_byte = -1;
  int missing_val = (int) _handle.fld_hdrs[field_num].missing_data_value;
  int bad_val = (int) _handle.fld_hdrs[field_num].bad_data_value;

  for (int iz = 0; iz < _grid.nz; iz++) {

    ui08 *datap = (ui08 *) _handle.field_plane[field_num][iz];

    for (int i = 0; i < _grid.nx * _grid.ny; i++, datap++) {
      ui08 bval = *datap;
      if (bval != missing_val && bval != bad_val) {
	min_byte = MIN(min_byte, bval);
	max_byte = MAX(max_byte, bval);
      }
    }

  } // iz

  if (min_byte > max_byte) {
    // no data found
    return (-1);
  }

  double scale = _handle.fld_hdrs[field_num].scale;
  double bias = _handle.fld_hdrs[field_num].bias;

  *min_p = min_byte * scale + bias;
  *max_p = max_byte * scale + bias;

  return(0);

}

/////////////////////////////////////
// update the start and end times
//

void InputFile::updateTimes(time_t *start_time_p, time_t *end_time_p)

{
  MDV_master_header_t *in_mhdr = &_handle.master_hdr;

  if (*start_time_p < 0) {
    *start_time_p = in_mhdr->time_begin;
  } else {
    *start_time_p = MIN((*start_time_p), in_mhdr->time_begin);
  }

  if (*end_time_p < 0) {
    *end_time_p = in_mhdr->time_end;
  } else {
    *end_time_p = MIN((*end_time_p), in_mhdr->time_end);
  }

}

/////////////////////////////////////
// load field data into output handle
//

void InputFile::mergeField(int in_field,
			   MDV_handle_t *outHandle, mdv_grid_t *outGrid,
			   int out_field, double out_scale, double out_bias)

{

  // compute lookup table for changing from input scale and bias
  // to output scale and bias

  MDV_field_header_t *in_fhdr = _handle.fld_hdrs + in_field;
  double in_scale = in_fhdr->scale;
  double in_bias = in_fhdr->bias;

  ui08 scaleLut[256];

  for (int i = 0; i < 256; i++) {
    double val = i * in_scale + in_bias;
    int outByte = (int) ((val - out_bias) / out_scale + 0.5);
    if (outByte >= 0 && outByte < 256) {
      scaleLut[i] = outByte;
    } else {
      scaleLut[i] = 0;
    }
  }

  // loop through output planes

  for (int iz = 0; iz < outGrid->nz; iz++) {

    int in_z = _zLut[iz];
    if (in_z < 0) {
      continue;
    }

    // set up pointers to planes and XY lookup table

    ui08 *inPlane = (ui08 *) _handle.field_plane[in_field][in_z];
    ui08 *outPlane = (ui08 *) outHandle->field_plane[out_field][iz];

    // How do we know this is the "correct" _xyLut lookup table?
    // Goes with current InputFile object?
    long *xylut = _xyLut;
    
    // loop through points in output plane, merging in the input
    // data if it exceeds the value already in the plane

    ui08 *outbp = outPlane;
 // for (int i = 0; i < _grid.nx * _grid.ny; i++, xylut++) {
 // for (int i = 0; i < out_lut_nx * out_lut_ny; i++, xylut++) {

    // Now loop through only the portion of the output grid where we have lut info
    for (int iy = _out_lut_miny_idx; iy < _out_lut_miny_idx + _out_lut_ny; iy++) {
      for (int ix = _out_lut_minx_idx; ix < _out_lut_minx_idx + _out_lut_nx; ix++, xylut++) {

        if (ix > 0 && ix < _outputGrid->nx &&
            iy > 0 && iy < _outputGrid->ny) {

          int in_idx = *xylut;
          if (in_idx >= 0)
          {
            int inb = inPlane[in_idx];
            int outb = scaleLut[inb];
  
            // Point to output array based on loop index
            int out_idx = (iy * _outputGrid->nx) + ix;
            outbp = outPlane + out_idx;

            if (*outbp < outb)
            {
              *outbp = outb;
            }
  
            // MCP test
            // *outbp = 73;

//          if (*outbp > 0) {
//            fprintf(stderr, "ix, iy, in_idx, out_idx, outbp: %d %d %d %d %d\n",
//                  ix, iy, in_idx, out_idx, *outbp);
//          }

          } // endif - in_idx >= 0

        } // endif - ix > 0...


      } // ix
    } // iy
  } // iz

}

//////////////////////////////////////////
// Load up the path for the requested time
//
// returns -1 on success, 0 on failure

int InputFile::_loadPath(time_t request_time)

{
  
  date_time_t rtime;
  rtime.unix_time = request_time;
  uconvert_from_utime(&rtime);
  
  // special case first - if this is the master input and we are using
  // FILE_TRIGGER then we can compute the path directly from the time
  // because the time will be the exact time of the required file

  if (_master && _params->trigger == FILE_TRIGGER) {

    sprintf(_path, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.mdv",
	    _dir, PATH_DELIM,
	    rtime.year, rtime.month, rtime.day, PATH_DELIM,
	    rtime.hour, rtime.min, rtime.sec);
	    
    return (0);
    
  }

  // search the directory for the first file before or at the request time
  // and load up _path

  if (_loadFirstBefore(request_time,
		       _params->trigger_time_margin)) {
    return (-1);
  }

  return (0);

}


////////////////////////////////////////////////////////////////////
// load latest data file before the given time, but within the
// given offset.
//
// Returns 0 on success, -1 on failure.

int InputFile::_loadFirstBefore(time_t request_time,
				int max_time_offset)

{
  static char *routine_name = "getFirstBefore()";
  
  int file_found = FALSE;
  
  char file_ext[16];
  int time_diff;
  int min_time_diff;
  int hour, min, sec;
  
  time_t end_time = request_time - max_time_offset;
  
  date_time_t search_dt;
  date_time_t end_dir_dt;
  date_time_t file_dt;
  date_time_t dir_dt;
  
  char dir_path[MAX_PATH_LEN];
  
  struct dirent *dp;
  DIR *dirp;
  
  /*
   * Calculate the end_dir_dt structure.  This structure should contain
   * the same time information as the starting directory, but should have
   * the date information of the ending directory.  That way, we can
   * increment from the start time to this time using the number of
   * seconds in a day to hit each directory in between.
   */

  search_dt.unix_time = request_time;
  uconvert_from_utime(&search_dt);
  
  end_dir_dt.unix_time = end_time;
  uconvert_from_utime(&end_dir_dt);

  end_dir_dt.hour = search_dt.hour;
  end_dir_dt.min = search_dt.min;
  end_dir_dt.sec = search_dt.sec;
  uconvert_to_utime(&end_dir_dt);
  
  /*
   * Start in the directory that includes the search time and
   * process all of the directories (backwards) until we reach
   * the end time or find a file.  
   */

  time_t dir_time;
  
  for (dir_time = request_time; dir_time >= end_dir_dt.unix_time;
       dir_time -= SECS_IN_DAY)
  {
    dir_dt.unix_time = dir_time;
    uconvert_from_utime(&dir_dt);
    
    sprintf(dir_path, "%s%s%04d%02d%02d",
	    _dir, PATH_DELIM,
	    dir_dt.year, dir_dt.month, dir_dt.day);

    /*
     * Try to open the directory
     */
  
    if ((dirp = opendir(dir_path)) == NULL)
    {
      if (_params->debug)
      {
	fprintf(stderr,
		"ERROR: DsInputPath::%s\n", routine_name);
	fprintf(stderr,
		"Error openning directory <%s>\n",
		dir_path);
      }
    
      continue;
    }
  
    /*
     * Loop thru directory looking for the data file names
     */
  
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
      /*
       * exclude dir entries and files beginning with '.'
       */

      if (dp->d_name[0] == '.')
	continue;
      
      /*
       * check that the file name is in the correct format
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
    
      file_dt.year = dir_dt.year;
      file_dt.month = dir_dt.month;
      file_dt.day = dir_dt.day;
      file_dt.hour = hour;
      file_dt.min = min;
      file_dt.sec = sec;
	
      uconvert_to_utime(&file_dt);
	
      if (file_dt.unix_time <= request_time &&
	  file_dt.unix_time >= end_time)
      {
	time_diff = (int)(request_time - file_dt.unix_time);
	  
	if (time_diff <= max_time_offset &&
	    (!file_found ||
	     time_diff < min_time_diff))
	{
	  sprintf(_path, "%s%s%s",
		  dir_path, PATH_DELIM,
		  dp->d_name);

	  file_found = TRUE;
	  min_time_diff = time_diff;
	
	} /*if (time_diff < *time_error) */
	
      } /* if (file_dt.unix_time >= com->time_min ... */

    } /* dp */
  
    closedir(dirp);

    if (file_found)
    {
      _time = file_dt.unix_time;
      return(0);
    }
    
  } /* endfor - dir_time */
  
  _time = -1;
  return(-1);
  
}

////////////////////////////////////////////////////////////////////
// Compute XY lookup table

void InputFile::_computeXYLookup()

{

  // initialize projection geometry routines
  // based on 0,0 being at VIL grid center

  PJGflat_init(_grid.proj_origin_lat,
               _grid.proj_origin_lon,
               _grid.proj_params.flat.rotation);

  // Find the lat/lon of the corner cells of the input grid
  // Currently, the NIDS VIL grids have the origin in the center
  // of the grid, not in the lower left cell
 
  double in_ll_lat, in_ll_lon;
  double in_ul_lat, in_ul_lon;
  double in_ur_lat, in_ur_lon;
  double in_lr_lat, in_lr_lon;

  double out_lut_minx;
  double out_lut_miny;
  double out_lut_maxx;
  double out_lut_maxy;

  double in_x, in_y;

  // lower left corner
  in_x = _grid.minx;
  in_y = _grid.miny;
  PJGflat_xy2latlon(in_x, in_y, &in_ll_lat, &in_ll_lon);

  // upper left corner
  in_x = _grid.minx;
  in_y = _grid.miny + (_grid.ny * _grid.dy);
  PJGflat_xy2latlon(in_x, in_y, &in_ul_lat, &in_ul_lon);

  // upper right corner
  in_x = _grid.minx + (_grid.nx * _grid.dx);
  in_y = _grid.miny + (_grid.ny * _grid.dy);
  PJGflat_xy2latlon(in_x, in_y, &in_ur_lat, &in_ur_lon);

  // lower right corner
  in_x = _grid.minx;
  in_y = _grid.miny + (_grid.ny * _grid.dy);
  PJGflat_xy2latlon(in_x, in_y, &in_lr_lat, &in_lr_lon);

  // Find the extremes of the grid
  out_lut_minx = MIN(in_ll_lon, in_ul_lon);
  out_lut_miny = MIN(in_ll_lat, in_lr_lat);
  out_lut_maxx = MAX(in_lr_lon, in_ur_lon);
  out_lut_maxy = MAX(in_ul_lat, in_ur_lat);

  // Find the starting x, y indexes for this lut in output grid
  _out_lut_minx_idx = (int)rint((out_lut_minx - _outputGrid->minx) / _outputGrid->dx + 0.5);
  _out_lut_miny_idx = (int)rint((out_lut_miny - _outputGrid->miny) / _outputGrid->dy + 0.5);

  // Find the ending x, y indexes for this lut in output grid
  _out_lut_maxx_idx = (int)rint((out_lut_maxx - _outputGrid->minx) / _outputGrid->dx + 0.5);
  _out_lut_maxy_idx = (int)rint((out_lut_maxy - _outputGrid->miny) / _outputGrid->dy + 0.5);

  // Find the number of points in x, y
  if ( ((_out_lut_minx_idx == 0) && (_out_lut_maxx_idx == 0)) ||
       ((_out_lut_minx_idx == _outputGrid->nx - 1) && (_out_lut_maxx_idx == _outputGrid->nx - 1)) )
    _out_lut_nx = 0;
  else
    _out_lut_nx = _out_lut_maxx_idx - _out_lut_minx_idx + 1;

  if ( ((_out_lut_miny_idx == 0) && (_out_lut_maxy_idx == 0)) ||
       ((_out_lut_miny_idx == _outputGrid->ny - 1) && (_out_lut_maxy_idx == _outputGrid->ny - 1)) )
    _out_lut_ny = 0;
  else
    _out_lut_ny = _out_lut_maxy_idx - _out_lut_miny_idx + 1;


//printf("LUT_NX, NY minx miny maxx maxy: %d %d %d %d %d %d\n", _out_lut_nx, _out_lut_ny,
//       _out_lut_minx_idx, _out_lut_miny_idx, _out_lut_maxx_idx, _out_lut_maxy_idx);

  _xyLut = (long *) urealloc(_xyLut, _out_lut_nx * _out_lut_ny * sizeof(long));

  double lat, lon;

  // loop through points in current lut grid

  long *lut = _xyLut;
  for (int yy = 0; yy < _out_lut_ny; yy++)
  {
    for (int xx = 0; xx < _out_lut_nx; xx++)
    {
      lon = out_lut_minx + (xx * _outputGrid->dx);
      lat = out_lut_miny + (yy * _outputGrid->dy);

      double flat_xx, flat_yy;
      // This should give x,y based on input gride center point
      PJGflat_latlon2xy(lat, lon, &flat_xx, &flat_yy);
      
      // compute grid indicies for input grid

      int ix = (int) ((flat_xx - _grid.minx) / _grid.dx + 0.5);
      int iy = (int) ((flat_yy - _grid.miny) / _grid.dy + 0.5);

      // if within input grid, set lookup index, otherwise -1
  
      if (ix >= 0 && ix < _grid.nx &&
          iy >= 0 && iy < _grid.ny) {
        *lut = (iy * _grid.nx + ix);
      } else {
        *lut = -1;
      }

//    printf("INPUT: xx, yy, lon, lat OUTPUT: x, y, lut ");
//    printf("%3d %3d %9.5f %9.5f %4d %4d %8ld\n",
//           xx, yy, lon, lat, ix, iy, *lut);

      lut++;
    } // xx
  } // yy

}

////////////////////////////////////////////////////////////////////
// Compute Z lookup table

void InputFile::_computeZLookup()

{

  int nZ = _outputGrid->nz;

  // loop through levels

  int *lut = _zLut;
  for (int i = 0; i < nZ; i++, lut++) {

    if (_grid.nz == 1) {

      *lut = 0; // special case - single input plane
      
    } else {

      double ht = _outputGrid->minz + _outputGrid->dz * i;
      int ih = (int) ((ht - _grid.minz) / _grid.dz + 0.5);
      
      // if within grid, set lookup index, otherwise -1
      
      if (ih >= 0 && ih < _grid.nz) {
	*lut = ih;
      } else {
	*lut = -1;
      }

    } // if (_grid.nz == 1)

  } // i

}
