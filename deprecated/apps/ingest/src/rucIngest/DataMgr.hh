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
//
// $Id: DataMgr.hh,v 1.15 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////
#ifndef _DATA_MGR_
#define _DATA_MGR_

#include <string>
#include <map>
#include <vector>
#include <list>

#include <euclid/Pjg.hh>
#include <toolsa/utim.h>

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
  static const double KELVIN_TO_CELCIUS;
  static const double KG_TO_G;
  static const double PERCENT_TO_FRAC;

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
  void _limitRanges(); 
  void _convertUnits(); 
  bool _createMdvxFields(); 
  bool _remapData(); 
  bool _writeMdvFile();

};

#endif
