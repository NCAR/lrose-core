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
// Texture_Interest.cc:  Interest derived from texture of 
//                       existing interest values
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include <math.h>

#include "Texture_Interest.h"

using namespace std;

//
// Constants
//
const double Texture_Interest::_SCALE_TO_INTEREST = 40.0;
const double Texture_Interest::_MIN_INTEREST_LEVEL = 0.5;
const double Texture_Interest::_PERCENT_TO_EXCEED_MIN_LEVEL = 50;
const double Texture_Interest::_INTEREST_DIFF_THRESHOLD = 0.1;
const double Texture_Interest::_HT_DIFF_THRESHOLD = 2.0;

//////////////
// Constructor

Texture_Interest::Texture_Interest(const bool debug_flag) :
  Interest(debug_flag)
{
  if(_debug)
  {
    cerr << "Creating Texture_Interest object" << endl;
  }
}


/////////////
// Destructor

Texture_Interest::~Texture_Interest()
   
{
   if (_debug)
     cerr << "Deleting Texture_Interest object" << endl;
}


//////////////
// calcInterestFields()

bool Texture_Interest::calcInterestFields(MdvxField *dbz_field,
					  fl32 *input_interest,
					  int *input_ht)
{
  // Initialize the interest fields

  _initInterestFields(dbz_field);
  
  int count, nexceed;
  int min_num_to_exceed =
    int(25 * (_PERCENT_TO_EXCEED_MIN_LEVEL/100.0) + 0.5);

  // compute interest

  for (int iy = 2; iy < _dbzFieldHdr.ny - 2; iy++)
  {
    for (int ix = 2; ix < _dbzFieldHdr.nx - 2; ix++)
    {
      int data_index = ix+iy*_dbzFieldHdr.nx;
	    
      if (input_interest[data_index] >= _MIN_INTEREST_LEVEL)
      {
		
	count = 0;
	nexceed = 0;

	for (int ir = iy - 2; ir <= iy + 2; ir++)
	{
	  for (int ic = ix - 2; ic < ix + 2; ic++)
	  {
	    int int_index = ic+ir*_dbzFieldHdr.nx;
	    int nextc_index = (ic+1)+ir*_dbzFieldHdr.nx;
		      
	    if (input_interest[int_index] >= _MIN_INTEREST_LEVEL)
	    {
	      nexceed++;
	    }

	    if ((fabs(input_interest[nextc_index] -
		      input_interest[int_index]) >= 
		 _INTEREST_DIFF_THRESHOLD) ||
		(fabs(input_ht[nextc_index] -
		      input_ht[int_index]) >=
		 _HT_DIFF_THRESHOLD))
	    {
	      count++;
	    }
	  }

	  if (input_interest[(ix+2)+ir*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
	  {
	    nexceed++;
	  }
	  
	}

	for (int ic = ix - 2; ic <= ix + 2; ic++)
	{
	  for (int ir = iy - 2; ir < iy + 2; ir++)
	  {
	    int int_index = ic+ir*_dbzFieldHdr.nx;
	    int nextr_index = ic+(ir+1)*_dbzFieldHdr.nx;
		      
	    if ((fabs(input_interest[nextr_index] -
		      input_interest[int_index]) >
		 _INTEREST_DIFF_THRESHOLD ) ||
		(fabs(input_ht[nextr_index] - 
		      input_ht[int_index]) >= 
		 _HT_DIFF_THRESHOLD))
	    {
	      count++;
	    }
	  }
	}

	if (nexceed >= min_num_to_exceed)
	{
	  interest_values[data_index] = 1.0 - count/_SCALE_TO_INTEREST;
	}
	else
	{
	  interest_values[data_index] = 0.0;
	}
	
	ht[data_index] = input_ht[data_index];
		 
      }
      else
      {
	// if interest is low to begin with, even if the
	// values around point are fairly constant, assign
	// in interest value of zero
    
	interest_values[data_index] = 0.0;
	ht[data_index] = input_ht[data_index];
      }
      
    }
  }

  // Using a 5x5 grid centered on the point to assess how smooth the
  // data is around a given point, leaves the edges without interest
  // values.  This causes an edge effect in the final data, so remove
  // this effect by copying interest values closest to the edges out.
  // If the input interest values at the edges were low to begin with
  // set the interest to zero instead.

  for (int iy = 0; iy < 2; iy++)
  {
    // upper left corner
    for (int ix = 0; ix < 2; ix++)
    {
      int index = ix+iy*_dbzFieldHdr.nx;
	    
      if (input_interest[index] >= _MIN_INTEREST_LEVEL)
      {
	interest_values[index] = 
	  interest_values[2+2*_dbzFieldHdr.nx]; 
      }
      else
      {
	interest_values[index] = 0.0;
      }
      
      ht[index] = input_ht[index];
      
    }
	  
    // upper right corner
    for (int ix = _dbzFieldHdr.nx - 2; ix < _dbzFieldHdr.nx; ix++)
    {
      int index = ix+iy*_dbzFieldHdr.nx;
	    
      if (input_interest[index] >= _MIN_INTEREST_LEVEL)
      {
	interest_values[index] = 
	  interest_values[(_dbzFieldHdr.nx - 3)+2*_dbzFieldHdr.nx];
      }
      else
      {
	interest_values[index] = 0.0;
      }

      ht[index] = input_ht[index];
	     
    }
  }
      
  for (int iy = _dbzFieldHdr.ny - 2; iy < _dbzFieldHdr.ny; iy++)
  {
    // lower left corner
    for (int ix = 0; ix < 2; ix++)
    {
      int index = ix+iy*_dbzFieldHdr.nx;
	    
      if (input_interest[index] >= _MIN_INTEREST_LEVEL)
      {
	interest_values[index] = 
	  interest_values[2+(_dbzFieldHdr.ny - 3)*_dbzFieldHdr.nx];
      }
      else
      {
	interest_values[index] = 0.0;
      }

      ht[index] = input_ht[index];
	     
    }
	  
    // lower right corner
    for (int ix = _dbzFieldHdr.nx - 2; ix < _dbzFieldHdr.nx; ix++)
    {
      int index = ix+iy*_dbzFieldHdr.nx;
	    
      if (input_interest[index] >= _MIN_INTEREST_LEVEL)
      {
	interest_values[index] = 
	  interest_values[(_dbzFieldHdr.nx - 3)+(_dbzFieldHdr.ny - 3)*_dbzFieldHdr.nx];
      }
      else
      {
	interest_values[index] = 0.0;
      }

      ht[index] = input_ht[index];
	     
    }
  }

  for (int ix = 2; ix < _dbzFieldHdr.nx - 2; ix++)
  {
    // first row
    if (input_interest[ix+0*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[ix+0*_dbzFieldHdr.nx] = 
	interest_values[ix+2*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[ix+0*_dbzFieldHdr.nx] = 0.0;
    }

    ht[ix+0*_dbzFieldHdr.nx] = input_ht[ix+0*_dbzFieldHdr.nx];
	  
	  
    // second row
    if (input_interest[ix+1*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[ix+1*_dbzFieldHdr.nx] = 
	interest_values[ix+2*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[ix+1*_dbzFieldHdr.nx] = 0.0;
    }
	  
    ht[ix+1*_dbzFieldHdr.nx] = input_ht[ix+1*_dbzFieldHdr.nx];
	  
    // second to last row
    if (input_interest[ix+(_dbzFieldHdr.ny - 2)*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[ix+(_dbzFieldHdr.ny - 2)*_dbzFieldHdr.nx] =
	interest_values[ix+(_dbzFieldHdr.ny - 3)*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[ix+(_dbzFieldHdr.ny - 2)*_dbzFieldHdr.nx] = 0.0;
    }
	  
    ht[ix+(_dbzFieldHdr.ny-2)*_dbzFieldHdr.nx] =
      input_ht[ix+(_dbzFieldHdr.ny-2)*_dbzFieldHdr.nx];

    // last row
    if (input_interest[ix+(_dbzFieldHdr.ny - 1)*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[ix+(_dbzFieldHdr.ny - 1)*_dbzFieldHdr.nx] =
	interest_values[ix+(_dbzFieldHdr.ny - 3)*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[ix+(_dbzFieldHdr.ny - 1)*_dbzFieldHdr.nx] = 0.0;
    }	  

    ht[ix+(_dbzFieldHdr.ny-1)*_dbzFieldHdr.nx] =
      input_ht[ix+(_dbzFieldHdr.ny-1)*_dbzFieldHdr.nx];

  }

  for (int iy = 2; iy < _dbzFieldHdr.ny - 2; iy++)
  {
    // first column
    if (input_interest[0+iy*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[0+iy*_dbzFieldHdr.nx] = 
	interest_values[2+iy*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[0+iy*_dbzFieldHdr.nx] = 0.0;
    }
	  
    ht[0+iy*_dbzFieldHdr.nx] = input_ht[0+iy*_dbzFieldHdr.nx];
	  
    // second column
    if (input_interest[1+iy*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[1+iy*_dbzFieldHdr.nx] = 
	interest_values[2+iy*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[1+iy*_dbzFieldHdr.nx] = 0.0;
    }
	  
    ht[1+iy*_dbzFieldHdr.nx] = input_ht[1+iy*_dbzFieldHdr.nx];
	  
    // second to last column
    if (input_interest[(_dbzFieldHdr.nx - 2)+iy*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[(_dbzFieldHdr.nx - 2)+iy*_dbzFieldHdr.nx] = 
	interest_values[(_dbzFieldHdr.nx - 3)+iy*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[(_dbzFieldHdr.nx - 2)+iy*_dbzFieldHdr.nx] = 0.0;
    }
	  
    ht[(_dbzFieldHdr.nx-2)+iy*_dbzFieldHdr.nx] =
      input_ht[(_dbzFieldHdr.nx-2)+iy*_dbzFieldHdr.nx];
       
    // last column
    if (input_interest[(_dbzFieldHdr.nx - 1)+iy*_dbzFieldHdr.nx] >= _MIN_INTEREST_LEVEL)
    {
      interest_values[(_dbzFieldHdr.nx - 1)+iy*_dbzFieldHdr.nx] = 
	interest_values[(_dbzFieldHdr.nx - 3)+iy*_dbzFieldHdr.nx];
    }
    else
    {
      interest_values[(_dbzFieldHdr.nx - 1)+iy*_dbzFieldHdr.nx] = 0.0;
    }
	  
    ht[(_dbzFieldHdr.nx-1)+iy*_dbzFieldHdr.nx] =
      input_ht[(_dbzFieldHdr.nx-1)+iy*_dbzFieldHdr.nx];

    
  }
       
  return true;
}
