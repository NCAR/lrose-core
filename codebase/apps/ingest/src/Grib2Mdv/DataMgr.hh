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
////////////////////////////////////////////////
// Data Manager
////////////////////////////////////////////////
#ifndef _DATA_MGR_
#define _DATA_MGR_

#include <string>
#include <map>
#include <vector>
#include <list>

#include <euclid/Pjg.hh>
#include <toolsa/utim.h>
#include <dataport/port_types.h>	// defines fl32

#include "InputStrategy.hh"
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class GribMgr;
class Ingester;
class MdvxProj;
class MdvxField;
class OutputFile;

class DataMgr {
public:
   
  DataMgr();
  ~DataMgr();
   
  bool init( Params &params );
  bool init( Params &params, const vector<string>& fileList);
  bool getData();

  //
  // Constants
  //
  static const int MAX_LINE;
  static const int MAX_NPTS;
  static const double M_TO_KM;
  static const double M_TO_100FT;
  static const double MPS_TO_KNOTS;
  static const double PASCALS_TO_MBARS;
  static const double KELVIN_TO_CELSIUS;
  static const double KG_TO_G;
  static const double PERCENT_TO_FRAC;

  /// Adjust the supplied value to something within bounds.
  /// The adjustment is accomplished by repeated subtraction or addition of the interval length.
  /// \param value the value which is out of range
  /// \param lowerBound adjust so value is >= this
  /// \param upperBound adjust so value is <= this
  /// \return a value between lowerBound and upperBound
  static fl32 wrapAdjust(fl32 value, fl32 lowerBound, fl32 upperBound);


private:
   
  //
  // Parameters
  //
  Params *_paramsPtr;

  //
  // Parameters which describe the grid on which the
  // grib data is projected - 
  //
  Pjg *_inputPjg; 
  GribMgr *_gribMgr;


  //
  // Data ingest
  //
  InputStrategy *_inputStrategy;
  Ingester *_ingester;
  fl32 _missingVal;

  //
  // Mdv output
  //
  bool _createMdv;
  vector<MdvxField*> _outputFields;
  OutputFile *_outputFile;


  bool _mdvInit();
  void _clearMdvxFields();
  void _convertUnits();
  bool _createMdvxFields();
  bool _remapData();

  /// Calculate fields that are derived from other fields.
  /// For example, wind speed and direction can be derived from u- and v-wind.
  void _deriveFields();

  /// Calculate the derived wind speed or direction from U and V.
  /// \param uWind the east-west component of the wind
  /// \param vWind the north-south component of the wind
  /// \param windField absolute wind speed or direction calculated and set in here
  /// \param physicsFunction calculates something from U and V.
  bool _calculateWindFunction(MdvxField *uWind, MdvxField *vWind,
    MdvxField *windField, double (&physicsFunction)(const double, const double));

  /// Find and return the matching level index.
  /// \param levelValue looking for this value
  /// \param levels[] array of level values in which to look
  /// \param numLevels size of the levels[] array
  /// \return the matching index, or -1 if not found
  int _matchLevel(fl32 levelValue, const fl32 levels[], int numLevels);

  /// Determine if the three fields are compatible (same level and size).
  /// \param uWind the east-west component of the wind
  /// \param vWind the north-south component of the wind
  /// \param windSpeed absolute wind speed calculated and set in here
  bool _checkFieldsCompatible(MdvxField *uWind, MdvxField *vWind, MdvxField *windSpeed);

  /// Some fields are used for deriving other fields.  Unless those fields
  /// are also requested explicity, we remove them here.
  void _removeNonRequestedFields();

  /// Determines if a field was requested in the output list.
  /// \param theField might be needed by derived field but not requested
  /// \return true if the field should be included in the output MDV file
  bool _isRequested(MdvxField *theField);

  bool _writeMdvFile();

};

#endif
