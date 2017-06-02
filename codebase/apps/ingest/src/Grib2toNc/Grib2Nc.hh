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
// Grib2 to NetCDF Application Class
//
// Jason Craig  -  Jan 2015
////////////////////////////////////////////////

#ifndef _GRIB2NC
#define _GRIB2NC

#include <string>
#include <map>
#include <vector>
#include <list>
#include <Ncxx/Nc3File.hh>

#ifndef NOT_RAL
#include <dsdata/DsTrigger.hh>
#endif
#include <toolsa/utim.h>
#include <grib2/Grib2File.hh>
#include <grib2/Grib2Record.hh>

#include "Params.hh"
using namespace std;

#define MAX_NPTS 6000000
#define MAX_VLEVELS 122

//
// Forward class declarations
//
class Grib2File;
class GribRecord;
class NcOutput;

class Grib2Nc {
public:

  //
  // Constants
  //
  //static const int MAX_NPTS;
  //static const int MAX_VLEVELS;
  static const float FIELD_DATA_EPSILON;

  //
  // Typdefs
  //

  // Grid Information
  typedef struct {
    string ncfGridName;
    fl32 proj_origin_lat;
    fl32 proj_origin_lon;
    // start value, SW corner,
    // bottom plane (* scale)
    fl32 minx, miny;
    // cartesian spacing in each dirn (* scale)
    fl32 dx, dy;
    // number of points in each dirn    
    si32 nx, ny;
    // false easting and northing
    // false_easting is added to X to keep it positive
    fl32 false_easting;
    fl32 false_northing;
    // lambert conformal
    fl32 lat1;
    fl32 lat2;
    // polar stereographic
    fl32 tan_lon;
    fl32 central_scale;
    si32 pole_type; // 0 - POLE_NORTH, 1 - POLE_SOUTH
  } GridInfo;
  
  // Vertical Information
  typedef struct {
    string units;
    string longName;
    string standardName;
    string positive;
    int nz;
    float zData[MAX_VLEVELS];
  } VlevelInfo;
  
  // Field Information
  typedef struct {
    string name;
    string nameLong;
    string standardName;
    string units;
    string generatingCenter;
    string generatingProcess;
    float max_value;
    float min_value;  
    fl32 missing;
    fl32 secondaryMissing;
    GridInfo gridInfo;
    VlevelInfo vlevelInfo;
    Params::data_pack_t ncType;
    float addOffset;
    float scaleFactor;
  } FieldInfo;

  //
  // Methods
  //
   
  Grib2Nc (Params &params);
  ~Grib2Nc();
   
  int init (int nFiles, char** fileList, char* outputFile,
	    bool printVarList, bool printsummary , bool printsections);
  int getData();


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
  string _inputSuffix;
  string _inputSubstring;

#ifdef NOT_RAL
  vector< string > _file_list;
#else
  DsTrigger *_dataTrigger;
#endif

  //
  // list of GRIB fields to be processed
  //
  list<Params::out_field_t> _gribFields;
  list<Params::out_field_t>::iterator _field;

  //
  // Nc output
  //
  FieldInfo _fieldInfo;

  bool _reOrderNS_2_SN;
  bool _reOrderAdjacent_Rows;
  bool _reMapField;
  vector<FieldInfo*> _outputFields;
  NcOutput *_outputFile;

  int _mdvInit();
  int _setFieldInfo(); 
  int _writeOutput(time_t generateTime, long int forecastTime);

  void _sortByLevel(vector<Grib2::Grib2Record::Grib2Sections_t>::iterator begin, 
		    vector<Grib2::Grib2Record::Grib2Sections_t>::iterator end);
  void _reOrderNS2SN(fl32 *data, int numX, int numY);
  void _reOrderAdjacentRows(fl32 *data, int numX, int numY);
  fl32 *_reMapReducedOrGaussian(fl32 *data);
  void _setFieldNames(int paramsIndex);
  void _limitDataRange(int paramsIndex,fl32 *dataPtr);
  void _convertVerticalUnits(int paramsIndex);
  void _convertUnits(int paramsIndex,fl32 *dataPtr); 
  void _convertGribLevel2CFLevel(const string &GribLevel, const string &GribLevelUnits, const string &GribLevelLong);

  fl32 *_encode(fl32 *dataPtr, Params::data_pack_t output_encoding);
  void *_float32_to_int8(fl32 *inDataPtr);
  void *_float32_to_int16(fl32 *inDataPtr); 

};

#endif
