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
//
// CartBeamBlock.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2026
//
///////////////////////////////////////////////////////////////
//
// CartBeamBlock computes beam blockage for a specified radar
// type and location, and for a specified 3D Cartesian grid.
//
///////////////////////////////////////////////////////////////

#ifndef CartBeamBlock_HH
#define CartBeamBlock_HH

#include "Args.hh"
#include "Params.hh"
#include "BeamPowerPattern.hh"
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <rapformats/DsRadarParams.hh>
#include <radar/BeamHeight.hh>
#include <string>
#include <set>
using namespace std;

class DemProvider;
class BlockageCalc;

class CartBeamBlock {
  
public:

  // constructor
  
  CartBeamBlock (int argc, char **argv);

  // destructor
  
  ~CartBeamBlock();

  // run 

  int Run();

  // data members

  int OK;

  static const fl32 missingFl32;
  
protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // template file
  // provides headers, projection, radar location

  Mdvx _templateMdvx;
  MdvxField *_templateField;
  Mdvx::master_header_t _templateMhdr;
  Mdvx::field_header_t _templateFhdr;
  Mdvx::vlevel_header_t _templateVhdr;
  vector<double> _zKm;

  DsRadarParams _radarParams;
  double _radarLat, _radarLon, _radarHtKm;
  double _radarWavelengthCm;
  double _horizBeamWidthDeg, _vertBeamWidthDeg;
  
  MdvxProj _proj;
  double _sensorLat, _sensorLon, _sensorHtKm, _sensorHtM;
  double _minLat, _minLon, _maxLat, _maxLon;

  // digital terrain height data

  DemProvider *_dem;
  
  // computing blockage
  
  BeamHeight _beamHt;
  BeamPowerPattern *_pattern;
  BlockageCalc *_calc;
  vector<fl32> _blockage;

  // output file

  Mdvx _outMdvx;

  int _readGridTemplate(const string &path);
  int _readTemplateFile(const string &path);
  int _readDem(const string &path);

  int _computeBlockage();

  int _writeBlockage();
  void _setMasterHeader(Mdvx &mdvx);
  void _addBlockageField(Mdvx &mdvx);
  void _addTerrainField(Mdvx &mdvx);
  void _addHiResTerrainField(Mdvx &mdvx);
  
};

#endif

