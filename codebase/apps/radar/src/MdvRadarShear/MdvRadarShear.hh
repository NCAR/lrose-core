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
// MdvRadarShear.hh
//
// MdvRadarShear object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// MdvRadarShear reads in radial velocity data in MDV polar
// radar format, computes the shear and writes out shear fields
// in MDV format.
//
///////////////////////////////////////////////////////////////

#ifndef MdvRadarShear_H
#define MdvRadarShear_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/TaArray.hh>
using namespace std;

////////////////////////
// This class

class MdvRadarShear {
  
public:

  // constructor

  MdvRadarShear (int argc, char **argv);

  // destructor
  
  ~MdvRadarShear();

  // run 

  int Run();

  // data members

  bool isOK;

  const static double missingDouble;
  const static fl32 missingFloat;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsMdvxInput _input;
  DsMdvx _inMdvx;
  
  /////////////////////////////////////////
  // input data

  int _nGates;
  int _nBeams;
  int _nTilts;
  int _nPointsVol;
  int _nPointsTilt;
  double _startRange;
  double _gateSpacing;
  double _startAz;
  double _deltaAz;
  vector<double> _elevs;
  bool _isRhi;
  double _rhiMinEl;
  double _rhiDeltaEl;

  // processing details

  int _nKernel;
  int _nKernelHalf;

  // range and cross-azimuth distance

  TaArray<double> _range_, _ss_;
  double *_range;
  double *_ss;

  // These are pointers into the input Mdvx object.
  // This memory is managed by the Mdvx class and should not be freed
  // by the calling class.

  fl32 *_dbzMdv;
  fl32 *_snrMdv;
  fl32 *_velMdv;
  fl32 *_censorMdv;

  fl32 _dbzMiss;
  fl32 _snrMiss;
  fl32 _velMiss;
  fl32 _censorMiss;

  //////////////////////////////
  // output fields for MDV volume

  TaArray<fl32> _radial_shear_;
  TaArray<fl32> _azimuthal_shear_;
  TaArray<fl32> _max_shear_;
  TaArray<fl32> _mean_vel_;

  fl32 *_radial_shear;
  fl32 *_azimuthal_shear;
  fl32 *_max_shear;
  fl32 *_mean_vel;

  // vectors and matrices for solving for the shear

  static const int nCoeff = 3;
  double *_b, *_x, *_w;
  double **_a, **_u, **_v;

  // methods

  void _setupRead();
  
  int _readNextVolume();
  
  bool _checkFields();
  
  int _setInputPointers();

  void _processVolume();
  
  void _computeShear(int ibeam, int igate,
                     fl32 **dbz, fl32 **vel, fl32 **snr,
                     double &radialShear, double &azimuthalShear, double &meanVel);

  void _fillOutput(DsMdvx &outMdvx);

  void _addField(DsMdvx &outMdvx,
		 const string &field_name,
		 const string &long_field_name,
		 const string &units,
		 int encoding_type,
		 const fl32 *data);
  
  int _echoInputFields(DsMdvx &outMdvx);
  
  int _writeOutput(DsMdvx &outMdvx);

  void _applyMedianFilter(fl32 *field, int fieldLen, int filterLen);

  static int _fl32Compare(const void *i, const void *j);

};

#endif

