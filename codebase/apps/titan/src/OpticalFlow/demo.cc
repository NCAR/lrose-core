// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Ancilla Radar Quality Control System (ancilla)
// ** Copyright BOM (C) 2013
// ** Bureau of Meteorology, Commonwealth of Australia, 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from the BOM.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of the BOM nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%

/*------------------------------------------------------------------------------
 * Simple demo to exercise optical flow class extracted from Ancilla1
 *----------------------------------------------------------------------------*/

#include "optical_flow.h"
#include "array_utils.h"
#include "advection.h"
#include <netcdf.h>
#include <iostream>
#include <stdexcept>

using namespace ancilla;

array2<real> load_grid(const char* file, const char* variable);
void write_output(
      const char* file
    , const char* variable
    , const array2<real>& u
    , const array2<real>& v
    , const array2<real>& pred);

int main(int argc, const char* argv[])
{
  // sanity check arguments
  if (argc < 5 || argc > 10 || strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0)
  {
    std::cout 
      << "usage: ./demo lag1.nc lag0.nc varname output.nc [backgrnd] [threshold] [gain] [interp]\n"
      << "  lag1.nc   - netCDF file containing grid for time T-1\n"
      << "  lag0.nc   - netCDF file containing grid for time T\n"
      << "  varname   - name of variable containing grid to track in netCDF files\n"
      << "  out.nc    - name for output netCDF file\n"
      << "  backgrnd  - valid value used to replace missing data during tracking (def: 0.0)\n"
      << "  threshold - minimum valid value in inputs to track (must be >= 0, def: 0.0)\n"
      << "  gain      - premultiplier for input values (def: 1.0)\n"
      << "  interp    - whether to interpolate untracked areas 1=yes,0=no (def: 1)\n";
    return EXIT_SUCCESS;
  }

  try
  {
    // load the input grids
    auto lag1 = load_grid(argv[1], argv[3]);
    auto lag0 = load_grid(argv[2], argv[3]);

    // verify that the grid sizes match
    if (lag1.rows() != lag0.rows() || lag1.cols() != lag0.cols())
      throw std::runtime_error("input grid size mismatch");

    // initialize some arrays to hold our output vectors
    array2<real> adv_x(lag1.dims());
    array2<real> adv_y(lag1.dims());
    array_utils::zero(adv_x);
    array_utils::zero(adv_y);

    // initialize an optical flow tracker using default parameters
    // see header file for parameter descriptions
    optical_flow tracker(
          lag1.cols()
        , lag1.rows()
        , 0.5
        // tweak these to influence the core optical flow algorithm
        , 100
        , 5
        , 3
        , 5
        , 1.1
        );

    // get valid data threshold and gain
    real background = argc > 5 ? atof(argv[5]) : 0.0;
    real threshold = argc > 6 ? atof(argv[6]) : 0.0;
    real gain = argc > 7 ? atof(argv[7]) : 1.0;
    bool interp = argc > 8 ? atoi(argv[8]) : true;

    // track from lag1 to lag0
    // see optical_flow.h for detailed parameter documentation
    tracker.determine_velocities(
          lag1
        , lag0
        , adv_x
        , adv_y
        , false       // don't use adv_x/y as inputs (ie: reinitialize)

        // if input grids have no missing data and you want to track on every
        // pixel, remove all parameters below here
        , background  // replace untrackable (nan/masked) values with this value
        , threshold   // mask out values below this threshold before tracking
        , gain        // premultiplier, algorithm seems to like values in 0-255 range likely due
                      // to it's origins in image tracking

        // if you only want advection vectors for areas that had valid data
        // then set the below parameter to false (or remove all parameters below here)
        , interp      // interpolate tracking over masked out area?
        , 8           // these are the default values... see header
        , 0.05
        , 4.0
        , 3.0
        );

    // advect lag0 into a predicted next state
    array2<real> prediction(lag1.dims());
    advection::advect_field(adv_x, adv_y, background, lag0, prediction);

    // output the advection vectors to a file
    write_output(argv[4], argv[3], adv_x, adv_y, prediction);

    return EXIT_SUCCESS;
  }
  catch (std::exception& err)
  {
    std::cout << "fatal exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "fatal exception of unknown type";
    return EXIT_FAILURE;
  }
}

//-----------------------------------------------------------------------------
// below here is just helper code to read and write simple netCDF files
//-----------------------------------------------------------------------------

// read a netcdf variable using the same type as 'real' is typedef'ed to
template <class T>
int read_variable(int ncid, int varid, T* data);

template <>
int read_variable<float>(int ncid, int varid, float* data)
{
  return nc_get_var_float(ncid, varid, data);
}

template <>
int read_variable<double>(int ncid, int varid, double* data)
{
  return nc_get_var_double(ncid, varid, data);
}

// simple function to load a basic 2d grid from a netCDF file
array2<real> load_grid(const char* file, const char* variable)
{
  int ncid;
  if (nc_open(file, NC_NOWRITE, &ncid) != NC_NOERR)
    throw std::runtime_error("failed to open input file");

  try
  {
    int varid;
    if (nc_inq_varid(ncid, variable, &varid) != NC_NOERR)
      throw std::runtime_error("failed to open grid variable");

    int ndims;
    if (nc_inq_varndims(ncid, varid, &ndims) != NC_NOERR)
      throw std::runtime_error("failed to determine number of grid variable dimensions");
    if (ndims != 2)
      throw std::runtime_error("input grid variable is not 2D");

    int dimid[2];
    if (nc_inq_vardimid(ncid, varid, dimid) != NC_NOERR)
      throw std::runtime_error("failed to determine input grid dimension ids");

    size_t dims[2];
    if (   nc_inq_dimlen(ncid, dimid[0], &dims[0]) != NC_NOERR
        || nc_inq_dimlen(ncid, dimid[1], &dims[1]) != NC_NOERR)
      throw std::runtime_error("failed to determine input grid dimensions");

    array2<real> out(dims);
    if (read_variable(ncid, varid, out.data()) != NC_NOERR)
      throw std::runtime_error("failed to read grid variable contents");

    // apply any scale_factor and add_offset to bring data into native units
    double add_offset = 0.0, scale_factor = 1.0;
    nc_get_att_double(ncid, varid, "add_offset", &add_offset);
    nc_get_att_double(ncid, varid, "scale_factor", &scale_factor);
    array_utils::multiply_add(out, out, scale_factor, add_offset);

    return out;
  }
  catch (...)
  {
    nc_close(ncid);
    throw;
  }
}

// write a netcdf variable using the same type as 'real' is typedef'ed to
template <class T>
int write_variable(int ncid, int varid, const T* data);

template <>
int write_variable<float>(int ncid, int varid, const float* data)
{
  return nc_put_var_float(ncid, varid, data);
}

template <>
int write_variable<double>(int ncid, int varid, const double* data)
{
  return nc_put_var_double(ncid, varid, data);
}

// simple function to write the advection vectors out
// warning: simple hack - does almost no error checking
void write_output(
      const char* file
    , const char* variable
    , const array2<real>& u
    , const array2<real>& v
    , const array2<real>& pred)
{
  int ncid;
  if (nc_create(file, 0, &ncid) != NC_NOERR)
    throw std::runtime_error("failed to create output file");

  try
  {
    int dimid[2];
    if (   nc_def_dim(ncid, "y", u.rows(), &dimid[0]) != NC_NOERR
        || nc_def_dim(ncid, "x", u.cols(), &dimid[1]) != NC_NOERR)
      throw std::runtime_error("failed to define output dimensions");
    
    int varid[3];
    if (   nc_def_var(ncid, "advection_x", NC_FLOAT, 2, dimid, &varid[0]) != NC_NOERR
        || nc_def_var(ncid, "advection_y", NC_FLOAT, 2, dimid, &varid[1]) != NC_NOERR
        || nc_def_var(ncid, variable, NC_FLOAT, 2, dimid, &varid[2]) != NC_NOERR)
      throw std::runtime_error("failed to define output variables");

    if (nc_enddef(ncid) != NC_NOERR)
      throw std::runtime_error("failed to leave define mode");

    if (   write_variable(ncid, varid[0], u.data()) != NC_NOERR
        || write_variable(ncid, varid[1], v.data()) != NC_NOERR
        || write_variable(ncid, varid[2], pred.data()) != NC_NOERR)
      throw std::runtime_error("failed to write output variable contents");
  }
  catch (...)
  {
    nc_close(ncid);
    throw;
  }
}

