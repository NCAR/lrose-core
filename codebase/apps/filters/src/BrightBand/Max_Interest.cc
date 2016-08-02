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
// Max_Interest.cc:  Interest derived from maximum of two
//                       existing interest values
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include "Max_Interest.h"

using namespace std;

//////////////
// Constructor

Max_Interest::Max_Interest(const bool debug_flag) :
  Interest(debug_flag),
  field_used(0)
{
  if(_debug)
  {
    cerr << "Creating Max_Interest object" << endl;
  }
}


/////////////
// Destructor

Max_Interest::~Max_Interest()
   
{
   if (_debug)
     cerr << "Deleting Max_Interest object" << endl;

   delete [] field_used;
}


//////////////
// calcInterestFields()

bool Max_Interest::calcInterestFields(MdvxField *dbz_field,
				      vector< Template_Interest* > &template_interest)
{
  // Initialize the interest values

  int old_npoints = npoints;
  _initInterestFields(dbz_field);
  
  // create array which describes which field is max at each point

  if (npoints != old_npoints)
  {
    delete [] field_used;
    
    field_used = new int[npoints];
  }
  
  memset((void *) field_used,
	 (int) 0,
	 (size_t) (sizeof(int) * npoints));

  // compute interest

  for (int ipoint = 0; ipoint < npoints; ipoint++)
  {
    for (size_t i = 0; i < template_interest.size(); ++i)
    {
      fl32 template_interest_value =
	template_interest[i]->getInterestValue(ipoint);
      
      // If this is a missing value then we don't use it

      if (template_interest_value == _BAD_INTEREST_VALUE)
	continue;
      
      // If this is the first non-missing value encountered or if this
      // template has a higher interest value, save it

      if (interest_values[ipoint] == _BAD_INTEREST_VALUE ||
	  template_interest_value > interest_values[ipoint])
      {
	field_used[ipoint] = i + 1;
	interest_values[ipoint] = template_interest_value;
	ht[ipoint] = template_interest[i]->getHeightValue(ipoint);
      }
      
    } /* endfor - i */
    
  } /* endfor - ipoint */
   
  return true;
}
