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
///////////////////////////////////////////////////////////////
// AreaCompute.cc
//
// AreaCompute object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// AreaCompute tests the C++ functionality of TDRP
//
///////////////////////////////////////////////////////////////

#include "AreaCompute.hh"
#include <string.h>
#include <stdlib.h>

// Constructor

AreaCompute::AreaCompute(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = strdup("AreaCompute");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  return;

}

// destructor

AreaCompute::~AreaCompute()

{

  // free up

  delete(_params);
  delete(_args);
  free(_progName);
  
}

//////////////////////////////////////////////////
// Run

int AreaCompute::Run()
{

  // compute area

  switch (_params->shape) {
    
  case Params::SQUARE:
    _area = _params->size * _params->size;
    break;

  case Params::CIRCLE:
    _area = _params->size * _params->size * (3.14159 / 4.0);
    break;

  case Params::EQ_TRIANGLE:
    _area = _params->size * _params->size * (0.866 / 2.0);
    break;

  } // switch

  // debug message

  if (_params->debug) {
    fprintf(stderr, "Size is: %g\n", _params->size);
    switch (_params->shape) {
    case Params::SQUARE:
      fprintf(stderr, "Shape is SQUARE\n");
      break;
    case Params::CIRCLE:
      fprintf(stderr, "Shape is CIRCLE\n");
      break;
    case Params::EQ_TRIANGLE:
      fprintf(stderr, "Shape is EQ_TRIANGLE\n");
      break;
    } /* switch */
    fprintf(stderr, "Area is: %g\n", _area);
  }
  
  // write out the result
  
  _writeResults();

  return (0);

}

//////////////////////////////////////////////////
// writeResults

int AreaCompute::_writeResults()

{

  FILE *out;

  if ((out = fopen(_params->output_path, "w")) == NULL) {
    perror(_params->output_path);
    return (-1);
  }

  fprintf(out, "Size is: %g\n", _params->size);
  switch (_params->shape) {
  case Params::SQUARE:
    fprintf(out, "Shape is SQUARE\n");
    break;
  case Params::CIRCLE:
    fprintf(out, "Shape is CIRCLE\n");
    break;
  case Params::EQ_TRIANGLE:
    fprintf(out, "Shape is EQ_TRIANGLE\n");
    break;
  } /* switch */
  fprintf(out, "Area is: %g\n", _area);

  fclose(out);

  return (0);

}

