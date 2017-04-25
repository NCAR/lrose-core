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
// Grib2 to MDV Manager
//
// Jason Craig
////////////////////////////////////////////////

#ifndef _GRIB2MDV
#define _GRIB2MDV

#include <string>
#include <map>
#include <vector>
#include <list>

#include <dsdata/DsTrigger.hh>
#include <toolsa/utim.h>
#include <Mdv/Mdvx.hh>
#include <grib2/Grib2File.hh>
#include <grib2/Grib2Record.hh>

#include "Params.hh"
#include "OutputFile.hh"
using namespace std;

//
// Forward class declarations
//
class Grib2File;
class GribRecord;
class MdvxProj;
class MdvxField;
class OutputFile;

class Grib2Mdv {
public:
   
  Grib2Mdv (Params &params);
  ~Grib2Mdv();
   
  int init (int nFiles, char** fileList, bool printVarList,
	    bool printsummary , bool printsections);
  int getData();

  //
  // Constants
  //
  static const int MAX_NPTS;

private:

  //
  // unit conversion constants
  //
  static const double M_TO_KM;
  static const double M_TO_FT;
  static const double M_TO_KFT;
  static const double M_TO_MI;
  static const double M_TO_100FT;
  static const double MPS_TO_KNOTS;
  static const double PASCALS_TO_MBARS;
  static const double KELVIN_TO_CELCIUS;
  static const double KG_TO_G;
  static const double PERCENT_TO_FRAC;
  static const double PASCAL_TO_HECTOPASCAL;
  static const double MM_S_TO_MM_HR;

  //
  // Parameters
  //
  Params *_paramsPtr;

  //
  // Data ingest
  //
  Grib2::Grib2File *_Grib2File;
  bool _printVarList;
  bool _printSummary;
  bool _printSections;
  Grib2::Grib2Record::Grib2Sections_t *_GribRecord;
  fl32 _missingVal;
  DsTrigger *_dataTrigger;
  string _inputSuffix;
  vector< string > _inputSubstrings;
  
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

  bool _reOrderNS_2_SN;
  bool _reOrderAdjacent_Rows;
  bool _reMapField;
  vector<MdvxField*> _outputFields;
  OutputFile *_outputFile;

  int _mdvInit();
  int _convertGribLevel2MDVLevel(const string &GribLevel);
  int _createFieldHdr(); 
  int _writeMdvFile(time_t generateTime, long int forecastTime);

  void _sortByLevel(vector<Grib2::Grib2Record::Grib2Sections_t>::iterator begin, 
		    vector<Grib2::Grib2Record::Grib2Sections_t>::iterator end);
  void _reOrderNS2SN(fl32 *data, int numX, int numY);
  void _reOrderAdjacentRows(fl32 *data, int numX, int numY);
  void _replaceAdditionalBadMissing(fl32 *data, Mdvx::field_header_t fhdr,
                                    bool use_bad_value, fl32 bad_value,
                                    bool use_missing_value, fl32 missing_value);
  fl32 *_reMapReducedOrGaussian(fl32 *data, Mdvx::field_header_t fhdr);
  void _setFieldNames(int paramsIndex);
  void _limitDataRange(int paramsIndex,fl32 *dataPtr);
  void _convertVerticalUnits(int paramsIndex);
  void _convertUnits(int paramsIndex,fl32 *dataPtr); 

  fl32 *_encode(fl32 *dataPtr, Params::encoding_type_t output_encoding);
  void *_float32_to_int8(fl32 *inDataPtr);
  void *_float32_to_int16(fl32 *inDataPtr); 

};

#endif
