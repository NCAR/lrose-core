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
#ifndef _DETECT_INC_
#define _DETECT_INC_


#include <Mdv/DsMdvx.hh>

#include "Params.hh"
using namespace std;

class Detect {

public:

  //
  // Constructor. Does naught.
  //
  Detect();

  //
  // Destructor. Does nowt.
  //
  ~Detect();

  //
  // Threshold the two fields, detect
  // storms, and send the output on.
  //
  int Threshold(MdvxField *truthField,
		MdvxField *forecastField,
		Params *P,
		int UrlIndex,
		time_t dataTime);

  private :

  void GetExtremes(float lat, float lon,
			   float radius,
			   Mdvx::field_header_t Fhdr,
			   int *Cx, int *Cy,
			   int *minx, int *maxx, 
			   int *miny, int *maxy);

  float GetDist(int Cx, int Cy, int ix, int iy,
			Mdvx::field_header_t Fhdr);


};

#endif





