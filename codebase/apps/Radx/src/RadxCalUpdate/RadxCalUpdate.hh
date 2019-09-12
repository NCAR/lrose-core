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
// RadxCalUpdate.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Update the calibration in a Radx file.
// Also ajusts the DBZ fields accordingly.
// Optionally corrects the altitude for EGM errors.
//
////////////////////////////////////////////////////////////////

#ifndef RadxCalUpdate_HH
#define RadxCalUpdate_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <euclid/Rotate3d.hh>
#include <Radx/RadxRcalib.hh>
#include <radar/Egm2008.hh>
class RadxVol;
class RadxRay;
class RadxFile;
class DoradeRadxFile;
using namespace std;

class RadxCalUpdate {
  
public:

  // constructor
  
  RadxCalUpdate (int argc, char **argv);

  // destructor
  
  ~RadxCalUpdate();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // calibration

  RadxRcalib _calib;
  
  // altitude correction

  Egm2008 _egm;

  int _runFilelist();
  int _runArchive();
  int _runRealtime();

  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);

  void _setupWrite(RadxFile &file);
  int _writeVol(const RadxVol &vol);

  void _fixRay(RadxRay &ray);
  void _fixRayCalibration(RadxRay &ray);
  void _fixRayAltitude(RadxRay &ray);


};

#endif
