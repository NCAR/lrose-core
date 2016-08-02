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

/*********************************************************************
 * GridStandardDev: Class for calculating averages of grids.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>
#include <string.h>

#include <toolsa/str.h>

#include "GridStandardDev.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

GridStandardDev::GridStandardDev(const bool include_missing_in_avg,
				 const double missing_avg_value,
				 const bool debug_flag) :
  GridCalculator(debug_flag),
  _includeMissingInAvg(include_missing_in_avg),
  _missingAvgValue(missing_avg_value)
{
}


/*********************************************************************
 * Destructor
 */

GridStandardDev::~GridStandardDev()
{
}


/*********************************************************************
 * doCalculation() - Do the calculation on the given grids.  Put the
 *                   results in the given output grid.
 *
 * Note that this method assumes that all of the given grids have the
 * same projection, so the calling method must ensure that this is
 * true.
 *
 * Returns true on success, false on failure.
 */

bool GridStandardDev::doCalculation(vector< MdvxField* > input_fields,
				    MdvxField *stdDev_field)
{
  static const string method_name = "GridStandardDev::doCalculation()";
  
  // Make sure there are some fields to average

  if (input_fields.size() == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input fields to average" << endl;
    
    return false;
  }
  
  // Compute the averages

  Mdvx::field_header_t stdDev_field_hdr = stdDev_field->getFieldHeader();
  
  int grid_size = stdDev_field_hdr.nx * stdDev_field_hdr.ny * stdDev_field_hdr.nz;
  
  fl32 *sum_grid = new fl32[grid_size];
  fl32 *n_grid = new fl32[grid_size];
  fl32 *sumXMinusMeanSqrd_grid = new fl32[grid_size];
  
  memset(sum_grid, 0, grid_size * sizeof(fl32));
  memset(n_grid, 0, grid_size * sizeof(fl32));
  memset(sumXMinusMeanSqrd_grid, 0, grid_size * sizeof(fl32));
  
  vector< MdvxField* >::const_iterator field_iter;
  
  for (field_iter = input_fields.begin(); field_iter != input_fields.end();
       ++field_iter)
  {
    MdvxField *field = *field_iter;
    
    // Update the sum and n grids using the data from this field

    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    fl32 *data_ptr = (fl32 *)field->getVol();
    
    for (int i = 0; i < grid_size; ++i)
    {
      if (data_ptr[i] == field_hdr.missing_data_value ||
	  data_ptr[i] == field_hdr.bad_data_value)
      {
	if (_includeMissingInAvg)
	{
	  sum_grid[i] += _missingAvgValue;
	  ++n_grid[i];
	}
      }
      else
      {
	sum_grid[i] += data_ptr[i];
	++n_grid[i];
      }
      
    } /* endfor - i */
  } /* endfor - field */
  
  fl32 *stdDev_ptr = (fl32 *)stdDev_field->getVol();
  fl32 *avg_grid = new fl32[grid_size];

  for (int i = 0; i < grid_size; ++i)
  {
    if (n_grid[i] == 0.0)
      avg_grid[i] = stdDev_field_hdr.missing_data_value;
    else
      avg_grid[i] = sum_grid[i] / n_grid[i];
  }
  
  for (field_iter = input_fields.begin(); field_iter != input_fields.end();
       ++field_iter)
  {
    MdvxField *field = *field_iter;
    
    // Update the sum and n grids using the data from this field

    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    fl32 *data_ptr = (fl32 *)field->getVol();
    
    for (int i = 0; i < grid_size; ++i)
    {
      if (data_ptr[i] == field_hdr.missing_data_value ||
	  data_ptr[i] == field_hdr.bad_data_value)
      {
	if (_includeMissingInAvg)
	  sumXMinusMeanSqrd_grid[i] = ( _missingAvgValue - avg_grid[i] ) * ( _missingAvgValue - avg_grid[i] );
      }
      else
	sumXMinusMeanSqrd_grid[i] = ( data_ptr[i] - avg_grid[i] ) * ( data_ptr[i] - avg_grid[i] ) ;

    } /* endfor - i */
  } /* endfor - field */
  
  
  for (int i = 0; i < grid_size; ++i)
  {
    if (n_grid[i] == 0.0)
      stdDev_ptr[i] = stdDev_field_hdr.missing_data_value;
    else
      stdDev_ptr[i] = sqrt( sumXMinusMeanSqrd_grid[i] / ( n_grid[i] - 1 ) );
  }
  
  delete [] sum_grid;
  delete [] n_grid;
  delete [] avg_grid;
  delete [] sumXMinusMeanSqrd_grid;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
