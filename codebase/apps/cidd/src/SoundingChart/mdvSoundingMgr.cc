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

using namespace std;

#include "mdvSoundingMgr.hh"
#include "mdvSounding.hh"
#include <Mdv/DsMdvxTimes.hh>

using namespace std;


//
// Constructor. Reads the data and fills up the vectors.
//
mdvSoundingMgr::mdvSoundingMgr(time_t startTime,
			       time_t endTime,
			       string url,
			       double lat,
			       double lon,
			       vector <string> fieldNames){
  //
  _mdvSoundings.clear();
  //
  // Use a DsMdvxTimes object to get the data times.
  //
  DsMdvxTimes dTimes;
  dTimes.setArchive(url, startTime, endTime);

  //
  // Go through the times and exract a sounding from each.
  //
  time_t dataTime;
  while (0 == dTimes.getNext(dataTime)){
    mdvSounding *M = new mdvSounding(dataTime, url, lat, lon, fieldNames);
    if (M->isOK()){
      _mdvSoundings.push_back(M);
    }
  }

}

//
// Destructor
//
mdvSoundingMgr::~mdvSoundingMgr(){

  for (unsigned i=0; i < _mdvSoundings.size(); i++){
    delete _mdvSoundings[i];
  }

}
//
// Return the number of times found.
// 
int mdvSoundingMgr::getNumSoundings(){
  return _mdvSoundings.size();
}

//
// Return the number of levels in the nth sounding.
//
int mdvSoundingMgr::getSoundingNLevels(int n){
  return _mdvSoundings[n]->getNlevels();
}
//
// Get the data for the nth sounding.
//
double *mdvSoundingMgr::getSoundingData(int n){
  return _mdvSoundings[n]->getData();
}

//
// Get the time for the nth sounding.
//
time_t mdvSoundingMgr::getSoundingTime(int n){
  return _mdvSoundings[n]->getTime();
}

