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
// ScaleSep.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// AUGUST 2014
//
////////////////////////////////////////////////////////////////////
//
// ScaleSep separates a radar image scene into different spatial
// scales, by applying a filter in the 2D FFT frequency domain
//
/////////////////////////////////////////////////////////////////////

#ifndef ScaleSep_H
#define ScaleSep_H

#include <string>
#include <Mdv/DsMdvxInput.hh>
#include <toolsa/TaArray.hh>
#include <fftw3.h>
#include "Args.hh"
#include "Params.hh"
class WorkingField;
class Fft2D;
using namespace std;

////////////////////////
// This class

class ScaleSep {
  
public:

  // constructor

  ScaleSep (int argc, char **argv);

  // destructor
  
  ~ScaleSep();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missing;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsMdvxInput _input;
  DsMdvx _inMdvx, _outMdvx;
  double _dbzMin;

  int _nz, _nx, _ny, _nxy;
  double _minx, _miny;
  double _dx, _dy;
  double _dxKm, _dyKm;

  int _prevNx, _prevNy;
  double _prevMinx, _prevMiny;
  double _prevDx, _prevDy;

  int _nxPad, _nyPad;
  int _nxPadded, _nyPadded, _nxyPadded;
  double _minxPadded, _minyPadded;

  WorkingField *_baseField;
  WorkingField *_basePadded;
  WorkingField *_spectrum;
  WorkingField *_specFilt;
  WorkingField *_filtered;
  WorkingField *_filtPadded;
  WorkingField *_filter;

  Fft2D *_fft;

  // checking timing performance

  struct timeval _timeA;

  // API

  int _doRead();
  int _processDataSet();
  void _loadBaseField();
  void _applyFilter();
  void _addFields();
  void _addField(const WorkingField *fld);
  int _doWrite();

  double _getHeight(int iz, const MdvxField &field);
  double _getDeltaHt(int iz, const MdvxField &field);

  void _printRunTime(const string& str, bool verbose = false);

  void _copyToPadded(const fl32 *normal, fl32 *padded);
  void _copyFromPadded(const fl32 *padded, fl32 *normal);

  void _init();
  void _allocFields();
  void _deleteFields();
  void _computeFilter();

};

#endif
