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
// Clump.cc : Describes where clumps of interest values are
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Clump.h"

using namespace std;


//////////////
// Constructor

Clump::Clump(const double interest_threshold,
	     const double area_threshold_sq_km,
	     const bool debug_flag) :
  Interest(debug_flag),
  _clump_data(0),
  _threshold(interest_threshold),
  _areaThreshSqKm(area_threshold_sq_km),
  _num_big_clumps(0)
{
}

/////////////
// destructor

Clump::~Clump()
   
{
   if (_debug)
     cerr << "Deleting Clump object" << endl;

   delete [] _clump_data;
   
}

/////////////
// get the number of big clumps

int Clump::Get_num_big_clumps(void)
   
{
   return(_num_big_clumps);
}


/////////////
// compute clump field

void Clump::_clump_interest(const int min_npoints)
   
{
  int num_clumps = 0;
  int num_intervals = 0;
  Clump_order *clump_ptr = NULL;

  Row_hdr *Rowh = NULL;
  int Nrows_alloc = 0;
  Interval *Intervals = NULL;
  int N_intervals_alloc = 0;

  int N_ints_alloc_clump = 0;
  Clump_order *Clumps = NULL;	/* a set of clumps */
  Interval **Interval_order = NULL; 

  short row, begin, end;
  
  /*
   * Allocate Space for Row structs
   */

  EG_alloc_rowh(_dbzFieldHdr.ny, &Nrows_alloc, &Rowh);
  
  /*
   * Find all intervals in each row
   */
  
  num_intervals = EG_find_intervals_float(_dbzFieldHdr.ny, _dbzFieldHdr.nx,
					  _clump_data,
					  &Intervals,
					  &N_intervals_alloc, Rowh,
					  _threshold);
  
  /*
   * Allocate space for clump structs & make sure ID's are set to NULL_ID 
   */
  
  EG_alloc_clumps(num_intervals, &N_ints_alloc_clump,
		  &Clumps, &Interval_order);
  EG_reset_clump_id(Intervals, num_intervals);

  /*
   * Find all Clumps using the Intervals previously found
   */
  
  num_clumps = EG_rclump_2d(Rowh, _dbzFieldHdr.ny, TRUE, 1,
			    Interval_order, Clumps);

  cerr << "number of clumps = " << num_clumps << endl;
  
  /*
   * 0th Clump is not used and Clumps array is actually num_clumps+1 big
   */

  clump_ptr = &(Clumps[1]);

  /*
   * Set all interest values to zero, then go through all the clumps.
   * If a point is in a clump, set its interest to 1
   */

   memset((void *) interest_values,
          (int) 0,
          (size_t) sizeof(fl32) * npoints);

  /*
   * Loop through all Clumps
   */
  
  for(int i = 0; i < num_clumps; i++, clump_ptr++) 
  {
    if (clump_ptr->pts >= min_npoints) 
    {
       for (int j = 0; j < clump_ptr->size; j++)
       {
	  row = clump_ptr->ptr[j]->row_in_plane;
	  begin = clump_ptr->ptr[j]->begin;
	  end = clump_ptr->ptr[j]->end;
	  for (int k = begin; k <= end; k++)
          {
	     interest_values[k + row*_dbzFieldHdr.nx] = 1.0;
	  }
       }

       _num_big_clumps++;
    }

  }

  EG_free ((void *) Intervals);
  
  EG_free_rowh (&Nrows_alloc, &Rowh);
  EG_free_clumps (&N_ints_alloc_clump, &Clumps, &Interval_order);
  
  
}


//////////////
// calcInterestFields()

bool Clump::calcInterestFields(MdvxField *dbz_field,
			       fl32 *input_interest,
			       int *input_ht)
{
  // Initialize the interest fields

  int old_npoints = npoints;
  _initInterestFields(dbz_field);
  
  if (npoints != old_npoints)
  {
    delete [] _clump_data;
    
    _clump_data = new float[npoints];
  }
  
  for (int i = 0; i < npoints; i++)
  {
    _clump_data[i] = input_interest[i];
  }

  int min_npoints =
    (int) (_areaThreshSqKm /
	   (_dbzFieldHdr.grid_dx * _dbzFieldHdr.grid_dy) + 0.5);

  // initialize number of clumps to zero

  _num_big_clumps = 0;

  // compute the interest

  _clump_interest(min_npoints);

  // set the hts

  for (int ipoint = 0; ipoint < npoints; ipoint++)
  {
    ht[ipoint] = input_ht[ipoint];
  }

  return true;
}
