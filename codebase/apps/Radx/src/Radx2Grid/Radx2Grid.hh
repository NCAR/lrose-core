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
// Radx2Grid.hh
//
// Radx2Grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// Radx2Grid reads moments from Radx-supported format files,
// interpolates onto a Cartesian grid, and writes out the
// results to Cartesian files in MDV or NetCDF.
//
///////////////////////////////////////////////////////////////

#ifndef Radx2Grid_HH
#define Radx2Grid_HH

#include "Args.hh"
#include "Params.hh"
#include "CartInterp.hh"
#include "PpiInterp.hh"
#include "PolarInterp.hh"
#include "ReorderInterp.hh"
#include "SatInterp.hh"
#include <string>
#include <deque>
#include <toolsa/TaArray.hh>
#include <radar/NoiseLocator.hh>
#include <Radx/RadxVol.hh>
#include <Mdv/MdvxProj.hh>
class RadxFile;
class RadxRay;
class RadxField;
using namespace std;

class Radx2Grid {
  
public:

  // constructor
  
  Radx2Grid (int argc, char **argv);

  // destructor
  
  ~Radx2Grid();

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

  // input data
  
  vector<string> _readPaths;
  RadxVol _readVol;
  bool _rhiMode;
  int _volNum;

  // interpolation fields
  
  vector<Interp::Field> _interpFields;
  vector<Interp::Ray *> _interpRays;

  // interpolation objects

  CartInterp *_cartInterp;
  PpiInterp *_ppiInterp;
  PolarInterp *_polarInterp;
  ReorderInterp *_reorderInterp;
  SatInterp *_satInterp;

  // censoring

  int _nWarnCensorPrint;

  // private methods

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);
  int _readFile(const string &filePath);
  void _checkFields(const string &filePath);
  void _loadInterpRays();
  void _censorInterpRay(Interp::Ray *interpRay);
  void _addGeometryFields();
  void _addTimeField();
  void _setupTransformFields();

  bool _isRhi();
  void _addBoundingSweeps();

  void _initInterpFields();
  void _allocCartInterp();
  void _allocPpiInterp();
  void _allocPolarInterp();
  void _allocReorderInterp();
  void _allocSatInterp();
    
  void _freeInterpRays();

};

#endif
