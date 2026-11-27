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

#ifndef _FILEHANDLER_INC_
#define _FILEHANDLER_INC_

#include <string>
#include <vector>
#include <Mdv/DsMdvx.hh>

#include <euclid/TypeGrid.hh>

#include "Params.hh"
#include "ConstDefs.hh"
using namespace std;


class FileHandler
{

public:

  FileHandler();

  int ProcessPair(time_t ForecastTime,
		  time_t DataTime,
		  Params *P);

  ~FileHandler();

  unsigned long num_non, num_fail, num_false, num_success;


private:

  DsMdvx *truthMdv, *forecastMdv, *outMdv;

  float *Threshold(float *Grid, Params *P, int IsForecast,
		   float Bad, float Missing);

  float *Contingency(float *truthDetected, float *forecastDetected,
		   int Size, Params *P);

  int Clumper(float *t, Params *P);
  void clump_plane( ui08 *plane, int nx, int ny,
		    int threshold_byte, int min_npoints,
		    int Debug);


};

#endif































