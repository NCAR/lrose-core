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
// Bdry2Axf.hh - class to convert SPDB boundaries from
// colide into meta data for the Olympics 2000 project.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
/////////////////////////////////////////////////////////////

#ifndef BDRY_2_AXF
#define BDRY_2_AXF

#include <rapformats/bdry.h>
#include <iostream>
#include <cstdio>
#include <toolsa/umisc.h>

#include "Params.hh"
using namespace std;

class Bdry2Axf {

  public :

  // Constructor - makes local copy of parrameters, opens output file.
  Bdry2Axf(Params *P, time_t ThisTime, int NumAtThisTime, int *LeadTimes);

  // Add a chunk to the output file.
  void Add(BDRY_product_t bdryProduct, int lineType, int LeadTime);

  // Close the output file, do a shuffle of latest data files.
  void Close();

  // Destructor - non-event.
  ~Bdry2Axf();

  //
  // Data.
  //
  int _Num;
  Params _params;
  time_t _Time;
  FILE *ofp;

  private :

  char TimeStr[256];
  char OutFile[MAX_PATH_LEN];
  int num;


};


#endif





