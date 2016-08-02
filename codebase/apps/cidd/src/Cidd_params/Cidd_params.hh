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

#ifndef Cidd_params_H
#define Cidd_params_H

#include <tdrp/tdrp.h>

#include "Args.hh"

#include "Cdpd_P.hh"
#include "Cgrid_P.hh"
#include "Cgui_P.hh"
#include "Cmap_P.hh"
#include "Csyprod_P.hh"
#include "Croutes_P.hh"
#include "Cterrain_P.hh"

class Cidd_params {
  
public:

  // constructor

  Cidd_params (int argc, char **argv);

  // destructor
  
  ~Cidd_params();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  char *_progName;
  char *_paramsPath;
  Args *_args;

  // TDRP Parameter class instances 
  Cdpd_P *_dpd_P;
  Cgrid_P *_grid_P;
  Cgui_P *_gui_P;
  Cmap_P *_map_P;
  Csyprod_P *_syprod_P;
  Cterrain_P *_terrain_P;
  Croutes_P *_routes_P;

};

#endif
