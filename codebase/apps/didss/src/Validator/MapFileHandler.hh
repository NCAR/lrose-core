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

#ifndef _MAPFILEHANDLER_INC_
#define _MAPFILEHANDLER_INC_

#include <string>
#include <vector>

#include "Params.hh"
#include "Map.hh"
#include "Statistician.hh"
using namespace std;

class MapFileHandler
{

public:

  MapFileHandler();

  int ProcessPair(time_t ForecastTime,
		  time_t DataTime,
		  Params *P,
		  vector< Map* > & MapVec ,
		  vector< Statistician* > & StatsVec);

  ~MapFileHandler();



  unsigned long num_non, num_fail, num_false, num_success;

private:

  DsMdvx *truthMdv, *forecastMdv, *outMdv;

  float *Threshold(float *Grid, Params *P, int IsForecast,
		   float Bad, float Missing);

  int Contingency(float *truthDetected, float *forecastDetected,
		  Params *P, Map *M);

  int Clumper(float *t, Params *P);
  void clump_plane( ui08 *plane, int nx, int ny,
		    int threshold_byte, int min_npoints,
		    int Debug);


};

#endif































