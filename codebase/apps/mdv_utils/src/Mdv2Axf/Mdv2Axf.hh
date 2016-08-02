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

#ifndef _MDV2AXF_INC_
#define _MDV2AXF_INC_

#include <string>  
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <cstdio>



#include "Params.hh"
using namespace std;

class Mdv2Axf {

public:

  //
  // Constructor. Does nothing.
  //
  Mdv2Axf();

  //
  // Destructor. Does nothing.
  //
  ~Mdv2Axf();

  //
  // Open the output file. Writes a header.
  //
  int Begin(Mdvx::field_header_t Fhdr, time_t t, Params *P);

  //
  // Process a field.
  //
  int Process(time_t t, MdvxField *field, int GridNum, 
	      int PlaneNum, float MissingVal, float ConversionFactor,
	      char *FieldName);

  //
  // Close the output file.
  //
  int CloseOutput();


  private :

  FILE *AxfFp;
  char _OutFileName[MAX_PATH_LEN];
  char _LatestFileName[MAX_PATH_LEN];
  int  _NumLatestFiles;

};

#endif





