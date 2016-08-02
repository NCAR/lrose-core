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
// FixRadxPointing.hh
//
// FixRadxPointing object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#ifndef FixRadxPointing_H
#define FixRadxPointing_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <euclid/Rotate3d.hh>
class RadxVol;
class RadxRay;
class RadxFile;
class DoradeRadxFile;
using namespace std;

class FixRadxPointing {
  
public:

  // constructor
  
  FixRadxPointing (int argc, char **argv);

  // destructor
  
  ~FixRadxPointing();

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
  
  double _headingDeg;
  double _rollDeg;
  double _pitchDeg;
  Rotate3d::Matrix _rotMatrix;

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);
  void _setupWrite(RadxFile &file);
  int _writeVol(const RadxVol &vol);

  void _computeRollAndPitch();
  void _computeRotationMatrix();

  void _printTest();
  void _fixRay(RadxRay &ray);
  void _fixRayFixedAngle(RadxRay &ray);
  void _fixRayElevation(RadxRay &ray);
  void _fixRayAzimuth(RadxRay &ray);
  void _fixRayOrientation(RadxRay &ray);


};

#endif
