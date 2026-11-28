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
// Grib to MDV Manager
//
////////////////////////////////////////////////

#ifndef _GRIB2MDV
#define _GRIB2MDV

#include <string>
#include <map>
#include <vector>
#include <list>

#include <toolsa/utim.h>
#include <Mdv/Mdvx.hh>


#include "Params.hh"
#include "GribFile.hh"
using namespace std;

//
// Forward class declarations
//
class GribFile;
class GribRecord;
class InputPath;
class MdvxProj;
class MdvxField;
class OutputFile;

class Grib2Mdv {
public:
   
  Grib2Mdv (Params &params);
  ~Grib2Mdv();
   
  int init (int nFiles, char** fileList, bool printsummary );
  int getData();

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
  // Data ingest
  //
  GribFile *_GribFile;
  bool _printSummary;
  GFSrecord *_GribRecord;
  InputPath *_inputPath;
  fl32 _missingVal;

  //
  // list of GRIB fields to be processed
  //
  list<Params::out_field_t> _gribFields;
  list<Params::out_field_t>::iterator _field;

  //
  // Mdv output
  //
  Mdvx::field_header_t _fieldHeader;
  Mdvx::vlevel_header_t _vlevelHeader;

  bool _createMdv;
  vector<MdvxField*> _outputFields;
  OutputFile *_outputFile;


  int _mdvInit();

  void _clearMdvxFields(); 
  void _cleanup();
  void _addPlane(void *, int);
//  void _convertUnits(); 
  int _createFieldHdr(int nZLevels); 
  int _remapData(); 
  int _writeMdvFile();

  MemBuf *_data;
  void *_dataPtr;


};

#endif
