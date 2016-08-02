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
//   $Date: 2016/03/06 23:15:37 $
//   $Id: GridSummer.cc,v 1.2 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GridSummer: Class for calculating sums of grids.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>
#include <string.h>

#include <toolsa/str.h>

#include "GridSummer.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

GridSummer::GridSummer(const bool debug_flag) :
  GridCalculator(debug_flag)
{
}


/*********************************************************************
 * Destructor
 */

GridSummer::~GridSummer()
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

bool GridSummer::doCalculation(vector< MdvxField* > input_fields,
			       MdvxField *sum_field)
{
  static const string method_name = "GridSummer::doCalculation()";
  
  // Make sure there are some fields to sum

  if (input_fields.size() == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input fields to sum" << endl;
    
    return false;
  }
  
  // Compute the sums

  Mdvx::field_header_t sum_field_hdr = sum_field->getFieldHeader();
  
  int grid_size = sum_field_hdr.nx * sum_field_hdr.ny * sum_field_hdr.nz;
  fl32 *sum_ptr = (fl32 *)sum_field->getVol();
  
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
	continue;
      
      if (sum_ptr[i] == sum_field_hdr.missing_data_value ||
	  sum_ptr[i] == sum_field_hdr.bad_data_value)
	sum_ptr[i] = data_ptr[i];
      else
	sum_ptr[i] += data_ptr[i];
    } /* endfor - i */
  } /* endfor - field */
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
