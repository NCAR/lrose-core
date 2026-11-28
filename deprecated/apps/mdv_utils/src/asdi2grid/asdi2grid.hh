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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:14 $
 *   $Revision: 1.2 $
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
-------------------------------------------------------------------------------
asdi2grid.hh

Header file for asdi2grid.cc
-------------------------------------------------------------------------------
*/

#ifndef ASDI2GRID_HH
#define ASDI2GRID_HH


// SYSTEM INCLUDES
#include <string>
#include <vector>

// RAP LIBRARY INCLUDES
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

// LOCAL INCLUDES
#include "Params.hh"
#include "NcInput.hh"
#include "MdvOutput.hh"

class asdi2grid
{ 

public:

  asdi2grid(Params *P, const char *programName);
  
  ~asdi2grid();
  

  int run(vector<string> inputFileList, time_t startTime, time_t endTime);
  

private:
  
  Params *params;  

  const char *progName;

  NcInput  ncInput;

  MdvOutput *mdvOutput;
  MdvxProj proj;

  float lastLat[10000][42][42];
  float lastLon[10000][42][42];
  float lastAlt[10000][42][42];

  float *counts;

  int _readInputNetcdf(char *input_file);

  void _initProjection();

  int _processData(InputData_t *inputData);

  int _writeOutputNetcdf(time_t fileStartT, time_t fileEndT);

  int _writeOutputMdv(time_t fileStartT, time_t fileEndT);

  void _drawline(float alt, float lat, float lon, float alt2, float lat2, float lon2);

}; 

#endif /* ASDI2GRID_HH */
