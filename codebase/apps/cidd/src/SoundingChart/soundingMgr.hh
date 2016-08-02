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
//
// Object that creates and stores the sounding objects.
//

#ifndef soundingMgr_H
#define soundingMgr_H

using namespace std;

#include "Params.hh"
#include <string>
#include <vector>
#include <rapformats/Sndg.hh>
#include <physics/physics.h>
#include <Spdb/sounding.h>
#include <Spdb/DsSpdb.hh>

using namespace std;

class soundingMgr {

public:

  //
  // Constructor. Reads the data and fills up the vector.
  // Determines which sounding to read.
  //
  soundingMgr(const Params &params,
              time_t startTime,
	      time_t endTime,
	      char *url,
	      int dataType,
	      char *label);
  
  //
  // Destructor
  //
  ~soundingMgr();

  //
  // Get the number of soundings.
  //
  int getNumSoundings();
  //
  // Get the nth sounding.
  //
  Sndg *getSoundingData(int n);
  //
  // Return the label for the point we used.
  //
  char *getLabel();
  //
  private :
  //
  //
  const Params &_params;
  vector <Sndg *> _soundings;
  char *_label;
  //
};

#endif

