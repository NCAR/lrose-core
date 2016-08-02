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
/////////////////////////////////////////////////////////////
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <ctime>
#include <Mdv/DsMdvx.hh>

#include "mdvTimes.hh"

/**
 *
 * @file mdvTimes.cc
 *
 * Implementation of mdvTimes class.
 *
 * @author Niles Oien
 *
 */

//
// Constructor. See comments in mdvTimes.hh.
//
using namespace std;

  
mdvTimes::mdvTimes(string url,
		   bool isForecast,
		   time_t startTime,
		   time_t endTime){

  _numTimes = 0;
  _dataTimes.clear();

  //
  // Compile a time list.
  //
  DsMdvx Tlist;
  if (isForecast){
    Tlist.setTimeListModeGen(url, startTime, endTime);
  } else {
    Tlist.setTimeListModeValid(url, startTime, endTime);
  }

  Tlist.compileTimeList();

  vector<time_t> inTimes = Tlist.getTimeList();

  //
  // If we are not in forecast mode, then these are the times.
  // Set up the vector accordingly.
  //
  if (!(isForecast)){

    mdvTime_t dataTime;
    dataTime .leadTime = mdvTimes::undefinedLeadTime;

    vector<time_t>::iterator it;

    for (it = inTimes.begin(); it != inTimes.end(); it++){
      dataTime.genTime = *it;
      _dataTimes.push_back( dataTime );
      _numTimes++;
    }

    _currentPosition = _dataTimes.begin();
    return;
  }

  //
  // If we got to here, we have the non-trivial case - forecast data.
  //
  mdvTime_t dTime;

  vector<time_t>::iterator t;

  for (t = inTimes.begin(); t != inTimes.end(); t++){
    dTime.genTime = *t;
    //
    // Get list of valid times from this generation time.
    //  
    DsMdvx inTim;
    inTim.setTimeListModeForecast( url, *t);
    inTim.compileTimeList();
    
    vector <time_t> vtimes = inTim.getValidTimes();

    vector<time_t>::iterator v;

    for (v = vtimes.begin(); v != vtimes.end(); v++){
      dTime.leadTime = (int) *v - *t;
      _dataTimes.push_back( dTime );
      _numTimes++;
    }
  }
  
  _currentPosition = _dataTimes.begin();
  return;
}

//
// Return the number of times.
//
int mdvTimes::getNumTimes(){
  return _numTimes;
}


//
// Return the next mdvTimes::mdvTime_t struct.
//
mdvTimes::mdvTime_t mdvTimes::getNextTime(){

  mdvTime_t retVal = *_currentPosition;

  if (_currentPosition != _dataTimes.end())
    _currentPosition++;

  return retVal;

}


//
// Destructor. Minor memory management function.
//
mdvTimes::~mdvTimes(){

  _dataTimes.clear();
  return;

}
  
