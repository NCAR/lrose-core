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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:11 $
//   $Id: NaturalNeighborInterp.cc,v 1.2 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NaturalNeighborInterp: Class implementing a natural neighbor
 *                        interpolation scheme.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>
#include <natgrid/natgrid.h>

#include "NaturalNeighborInterp.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

NaturalNeighborInterp::NaturalNeighborInterp(const bool debug_flag) :
  Interpolator(debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

NaturalNeighborInterp::~NaturalNeighborInterp(void)
{
}
  

/**********************************************************************
 * interpolate() - Interpolate the data to the grid in the given
 *                 MdvxField object.
 */

bool NaturalNeighborInterp::interpolate(const vector< DataPoint > points,
					MdvxField &field)
{
  static const string method_name = "NaturalNeighborInterp::interpolate()";
  
  // Gather all of the information needed for the natgrid call

  int num_obs = points.size();

  double *input_x_locs = new double[num_obs];
  double *input_y_locs = new double[num_obs];
  double *input_values = new double[num_obs];
  
  vector< DataPoint >::const_iterator point;
  int i;
  
  for (i = 0, point = points.begin(); point != points.end(); ++i, ++point)
  {
    input_x_locs[i] = point->getLongitude();
    input_y_locs[i] = point->getLatitude();
    input_values[i] = point->getValue();
  }
  
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  MdvxPjg proj(field_hdr);
  
  double *output_x_locs = new double[proj.getNx()];
  double *output_y_locs = new double[proj.getNy()];
  
  for (int i = 0; i < proj.getNx(); ++i)
    output_x_locs[i] = proj.getMinx() + (proj.getDx() * i);
  
  for (int i = 0; i < proj.getNy(); ++i)
    output_y_locs[i] = proj.getMiny() + (proj.getDy() * i);
  
  int error_code;
  
  // Call the interpolating routine

  if (_debug)
  {
    cerr << "   Interpolating data points:" << endl;
    for (int i = 0; i < num_obs; ++i)
      cerr << "        x = " << input_x_locs[i] << ", y = "
	   << input_y_locs[i] << ", val = "
	   << input_values[i] << endl;
  }
  
  c_nnsetr("nul", field_hdr.missing_data_value);
  c_nnseti("dup", 0);
  
  double *gridded_field = c_natgridd(num_obs, input_x_locs, input_y_locs,
				     input_values,
				     proj.getNx(), proj.getNy(),
				     output_x_locs, output_y_locs,
				     &error_code);
  
  if (error_code != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error interpolating data" << endl;
    cerr << "c_natgridd returned error code: " << error_code << endl;

    delete [] input_x_locs;
    delete [] input_y_locs;
    delete [] input_values;

    delete [] gridded_field;
    
    return false;
  }
  
  // Copy the gridded data to the MDV field

  int grid_size = proj.getNx() * proj.getNy();
  fl32 *output_grid = (fl32 *)field.getVol();
  
  for (int i = 0; i < grid_size; ++i)
    output_grid[i] = gridded_field[i];
  
  // Clean up memory

  delete [] input_x_locs;
  delete [] input_y_locs;
  delete [] input_values;
  delete [] gridded_field;
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
