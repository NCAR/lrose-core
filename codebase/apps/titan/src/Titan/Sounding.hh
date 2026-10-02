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
//////////////////////////////////////////////////////////////////////////
// Sounding.hh
//
// Store vertical profile sounding
//
// EOL, NCAR, Boulder CO
//
// Feb 2018
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////
//
// This is implemented as a singleton.
// To initialize the object, get an instance and set the params.
//
//////////////////////////////////////////////////////////////////////////

#ifndef Sounding_HH
#define Sounding_HH

#include "Params.hh"
#include "InputMdv.hh"
#include <radar/TempProfile.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRemapInterp.hh>
using namespace std;

class Sounding

{

public:

  ~Sounding();
  
  ///////////////////////////////////////////////////////////
  // Inst() - Retrieve the singleton instance of this class.
  // To initialze, call inst() with the params class set
  // To use, call inst() without setting params.
  
  static Sounding &inst();

  // initialize by setting the params

  void setParams(const Params *params);

  // retrieve temperature profile for a given scan time
  
  int retrieveTempProfile(const DsMdvx &radarMdvx);

  // get the temperature profile
  
  const TempProfile &getProfile() { return _tempProfile; }

  // get temperature grid if model data was used
  // if success, missingVal is set
  // returns NULL if no 3D grid available
  
  const fl32* getTemp3D(fl32 &missingVal);
    
private:
  
  // Singleton instance pointer

  static Sounding *_instance;
  
  //////////////////////////////////////////////////////////////////
  // Constructor -- private because this is a singleton object

  Sounding();

  // members

  const Params *_params;
  
  time_t _radarTime;

  TempProfile _tempProfile;
  TempProfile _paramsProfile;
  TempProfile _spdbProfile;
  TempProfile _modelProfile;
  
  // model interpolation
  
  time_t _modelTime;
  DsMdvx _modelRawMdvx;
  DsMdvx _modelInterpMdvx;
  const DsMdvx *_radarMdvx;
  MdvxRemapInterp _modelRemap;
  bool _interpProjSet;
  MdvxProj _interpProj;
  vector<double> _interpVlevels;
  bool _modelTempAvail;

  // methods
  
  void _setFromParams();
  int _readSpdb(const DsMdvx &radarMdvx);
  int _readModel(const DsMdvx &radarMdvx);
  int _computeModelTempProfile();
  void _interpModelToRadarGrid();
  void _initModelFieldTypes();
  int _checkModelFields();
  
};

#endif
