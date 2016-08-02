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

#include "Cidd_params.hh"
#include <string.h>
#include <malloc.h>

// Constructor

Cidd_params::Cidd_params(int argc, char **argv)

{

  // set programe name

  _progName = strdup("Cidd_params");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  OK = TRUE;

  // Instantiate params Classes - With Defaults
  _dpd_P = new Cdpd_P();
  _grid_P = new Cgrid_P();
  _gui_P = new Cgui_P();
  _map_P = new Cmap_P();
  _syprod_P = new Csyprod_P();
  _terrain_P = new Cterrain_P();
  _routes_P = new Croutes_P();

  return;
}

// destructor

Cidd_params::~Cidd_params()

{

  // free up

  delete(_dpd_P);
  delete(_grid_P);
  delete(_gui_P);
  delete(_map_P);
  delete(_syprod_P);

  delete(_args);
  free(_progName);
  
}

//////////////////////////////////////////////////
// Run

int Cidd_params::Run()
{

  printf("Running \n");
  // Just print out the file sections
  _dpd_P->print(stdout, PRINT_NORM);
  _grid_P->print(stdout, PRINT_NORM);
  _gui_P->print(stdout, PRINT_NORM);
  _map_P->print(stdout, PRINT_NORM);
  _syprod_P->print(stdout, PRINT_NORM);
  _terrain_P->print(stdout, PRINT_NORM);
  _routes_P->print(stdout, PRINT_NORM);

  return (0);

}
