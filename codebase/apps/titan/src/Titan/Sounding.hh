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

  void setParams(const Params *params) { _params = params; }

  // retrieve temperature profile for a given time
  
  int retrieveTempProfile(time_t profileTime);

  // get the temperature profile

  const TempProfile &getProfile() { return _tempProfile; }

  // get model field from type or name
  // returns null on error
  // NOTE: error cannot happen if _checkModelFields() succeeded
  
  Params::model_field_t *getModelField(Params::model_field_type_t mftype);
  Params::model_field_t *getModelField(const string &fieldName);
  
  string getModelInputName(Params::model_field_type_t mftype);
  Params::model_field_type_t getModelTypeFromInputName(const string name);
  
private:
  
  // Singleton instance pointer

  static Sounding *_instance;
  
  //////////////////////////////////////////////////////////////////
  // Constructor -- private because this is a singleton object

  Sounding();

  // members

  const Params *_params;
  TempProfile _tempProfile;

  // model interpolation
  
  vector<Params::model_field_type_t> _modelFieldTypes;
  MdvxRemapInterp _modelRemap;
  DsMdvx _modelRawMdvx;
  DsMdvx _modelInterpMdvx;
  const DsMdvx *_radarMdvx;
  time_t _radarTime;
  
  int _readModel(const DsMdvx &radarMdvx);
  int _computeModelTempProfile();
  void _interpModelToRadarGrid();
  void _initModelFieldTypes();
  int _checkModelFields();
  string _modelFieldType2Str(Params::model_field_type_t mftype);

};

#endif
