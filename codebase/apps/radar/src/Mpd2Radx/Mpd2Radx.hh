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
// Mpd2Radx.hh
//
// Mpd2Radx object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Nov 2021
// 
///////////////////////////////////////////////////////////////

#ifndef Mpd2Radx_HH
#define Mpd2Radx_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxVol.hh>
#include <physics/IcaoStdAtmos.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

class RadxFile;

using namespace std;

class Mpd2Radx {
  
public:

  // constructor
  
  Mpd2Radx (int argc, char **argv);

  // destructor
  
  ~Mpd2Radx();

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
  vector<string> _readPaths;

  IcaoStdAtmos _stdAtmos;

  DsMdvx _mdvx;
  MdvxField *_modelTempField, *_modelPresField;
  fl32 *_modelTempData, *_modelPresData;
  Mdvx::field_header_t _modelTempFhdr;
  Mdvx::vlevel_header_t _modelTempVhdr;
  MdvxProj _modelProj;

  int _nGates;
  double _gateSpacingKm;
  double _startRangeKm;

  // methods

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();

  int _processFile(const string &filePath);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  void _finalizeVol(RadxVol &vol);
  int _writeVol(RadxVol &vol);

  int _processMpdNcFile(const string &filePath);
  
  void _addEnvFields(const RadxVol &vol, RadxRay *ray);

  int _getModelData(time_t rayTime);

  int _setProfileFromModel(const RadxVol &vol,
                           RadxRay *ray,
                           Radx::fl32 *htMeters,
                           Radx::fl32 *tempK,
                           Radx::fl32 *presHpa);

};

#endif

