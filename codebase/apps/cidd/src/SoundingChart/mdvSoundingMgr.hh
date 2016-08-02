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
// Object that creates and stores the mdvSounding objects.
//

#ifndef mdvSoundingMgr_H
#define mdvSoundingMgr_H

using namespace std;

#include <string>
#include <vector>
#include "mdvSounding.hh"

using namespace std;

class mdvSoundingMgr {

public:

  //
  // Constructor. Reads the data and fills up the vectors.
  //
  mdvSoundingMgr(time_t startTime,
		 time_t endTime,
		 string url,
		 double lat,
		 double lon,
		 vector <string> fieldNames);

  //
  // Destructor
  //
  ~mdvSoundingMgr();

  //
  // Get the number of soundings.
  //
  int getNumSoundings();
  //
  // Get data for the nth sounding.
  //
  double *getSoundingData(int n);
  //
  // Return the number of levels in the nth sounding.
  //
  int getSoundingNLevels(int n);
  //
  // Return the time of the nth sounding.
  //
  time_t getSoundingTime(int n);

  private :

  //
  vector <mdvSounding *> _mdvSoundings;
  //
};

#endif

