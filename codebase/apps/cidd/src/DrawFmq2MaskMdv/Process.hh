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

#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>

#include "Params.hh"
using namespace std;

class Process {

public:

  //
  // Constructor
  //
  Process();


  //
  // Main method - run.
  //
  bool Derive(Params *TDRP_params, bool eraseOutside, time_t T,
	      double *boundingLats, double *boundingLons,
	      int numBounds);

  //
  // Destructor
  //
  ~Process();

  private :

  template <class T>
  void _updateGrid(T *data, unsigned char *grid_array,
		   int plane_size,
		   int bottom_level_num, int top_level_num,
		   bool erase_outside, T missing_data_value)
  {
    for (int i = 0; i < plane_size; ++i)
    {
      // If we should leave this point alone, continue on to the next point

      if ((erase_outside && grid_array[i] != 0) ||
	  (!erase_outside && grid_array[i] == 0))
	continue;
    
      // If we get here, we should set the grid point to missing in all of
      // the appropriate vertical levels

      for (int z = bottom_level_num; z <= top_level_num; ++z)
      {
	data[(z * plane_size) + i] = missing_data_value;
      } /* endfor - z */
    
    } /* endfor - i */
  }
  
};


