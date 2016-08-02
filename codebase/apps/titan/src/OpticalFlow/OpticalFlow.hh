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
// OpticalFlow.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
///////////////////////////////////////////////////////////////
//
// OpticalFlow performs optical flow tracking on MDV fields
// separated in time.
//
// The objective is to estimate the 2D velocity of the field.
//
// Output is the original tracked field, plus the U,V components
// of the velocity.
//
// The optical flow code was provided courtesy of Alan Seed of the
// Australian Bureau or Meteorology.
//
////////////////////////////////////////////////////////////////

#ifndef OpticalFlow_H
#define OpticalFlow_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxInput.hh>
using namespace std;

////////////////////////
// This class

class OpticalFlow {
  
public:

  // constructor

  OpticalFlow (int argc, char **argv);

  // destructor
  
  ~OpticalFlow();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsMdvxInput _input;

  int _processTimeStep(DsMdvx &previous, DsMdvx &current);
  void _setupPrevRead(DsMdvx &mdvx);
  void _setupCurrRead(DsMdvx &mdvx);
  void _addVelocityFields(DsMdvx &mdvx,
                          MdvxField &trackingField,
                          fl32 *uu, fl32 *vv,
                          fl32 missingVal);
  MdvxField *_createField(DsMdvx &mdvx,
                          Mdvx::field_header_t fhdr,
                          Mdvx::vlevel_header_t vhdr,
                          fl32 *data,
                          const char *name,
                          const char *longName,
                          const char *units);
  
  int _copyFieldToOutput(DsMdvx &mdvx,
                         const string &inputName,
                         const string &outputName);
  
  int _writeResults(DsMdvx &out);

};

#endif

