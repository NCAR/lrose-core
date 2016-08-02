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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: SeviriData.hh,v 1.11 2016/03/07 01:23:05 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	SeviriData
// 
// Author:	G. M. Cunning
// 
// Date:	Wed Jul 25 22:45:03 2007
// 
// Description:	This class handles reading the netcdf file and providing an 
//		interface to the data to objects that use the data.
// 
//		The grid starts in the upper left corner, and moves across
//		longitudes then latitudes. this will require som flipping 
//		to get into MDV frame of refererence.
//
//		The SEVIRI netcdf file contents are:
//		dimensions:
//		        lines = 4500 ;
//		        elems = 5972 ;
//		        bands = 1 ;
//		        auditCount = 2 ;
//		        auditSize = 80 ;
//		variables:
//		        int version ;
//		                version:long_name = "McIDAS area file version" ;
//		        int sensorID ;
//		                sensorID:long_name = "McIDAS sensor number" ;
//		        int imageDate ;
//		                imageDate:long_name = "image year and day of year" ;
//		                imageDate:units = "ccyyddd" ;
//		        int imageTime ;
//		                imageTime:long_name = "image time in UTC" ;
//		                imageTime:units = "hhmmss UTC" ;
//		        int startLine ;
//		                startLine:long_name = "starting image line" ;
//		                startLine:units = "satellite coordinates" ;
//		        int startElem ;
//		                startElem:long_name = "starting image element" ;
//		                startElem:units = "satellite coordinates" ;
//		        int numLines ;
//		                numLines:long_name = "number of lines" ;
//		        int numElems ;
//		                numElems:long_name = "number of elements" ;
//		        int dataWidth ;
//		                dataWidth:long_name = "number of bytes per source data point" ;
//		                dataWidth:units = "bytes/data point" ;
//		        int lineRes ;
//		                lineRes:long_name = "resolution of each pixel in line direction" ;
//		                lineRes:units = "km" ;
//		        int elemRes ;
//		                elemRes:long_name = "resolution of each pixel in element direction" ;
//		                elemRes:units = "km" ;
//		        int prefixSize ;
//		                prefixSize:long_name = "line prefix size" ;
//		                prefixSize:units = "bytes" ;
//		        int crDate ;
//		                crDate:long_name = "image creation year and day of year" ;
//		                crDate:units = "ccyyddd" ;
//		        int crTime ;
//		                crTime:long_name = "image creation time in UTC" ;
//		                crTime:units = "hhmmss UTC" ;
//		        int bands(bands) ;
//		                bands:long_name = "bands" ;
//		        char auditTrail(auditCount, auditSize) ;
//		                auditTrail:long_name = "audit trail" ;
//		        float data(bands, lines, elems) ;
//		                data:long_name = "data" ;
//		                data:type = "VISR" ;
//		                data:units = "brightness counts" ;
//		        float latitude(lines, elems) ;
//		                latitude:long_name = "latitude" ;
//		                latitude:units = "degrees" ;
//		        float longitude(lines, elems) ;
//		                longitude:long_name = "longitude" ;
//		                longitude:units = "degrees" ;
//
//// 


# ifndef    SEVIRI_DATA_H
# define    SEVIRI_DATA_H

// C++ include files
#include <string>
#include <iostream>
#include <vector>

// RAP include files
#include <euclid/Pjg.hh>
#include <toolsa/DateTime.hh>

// Local include files
#include "Params.hh"

using namespace std;

// forward declarations
class NcFile;
class NcDim;
class NcVar;
class Pjg;
class SeviriConverter;


class SeviriData {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  SeviriData();
  SeviriData(const SeviriData &);

  // destructor
  virtual ~SeviriData();

  SeviriData &operator=(const SeviriData &from);

  bool readFile(const string& file_path);

  void reset();

  void setParams(const Params* params) { _params = params; }

  //
  // access methods
  //
  const Pjg* getProjection() const { return _pjg; }

  const vector<int>& getBands() const { return _bandNums; }

  const vector<float*>& getData() const { return _bandData; }

  const DateTime& getImageTime() const { return _imageTime;  }

  const DateTime& getCreationTime() const { return _creationTime; }

  string getPathName() const { return _pathName; }

  bool process() const { return _processData; }

  void convertUnits(SeviriConverter* converter);

  void setMissing(float miss_val) { _missing = miss_val; }


  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  static const string _className;

  //
  // netcdf dimension names
  //
  static const char* LINES_DIM;
  static const char* ELEMS_DIM;
  static const char* BANDS_DIM;
  static const char* AUDIT_COUNT_DIM;
  static const char* AUDIT_SIZE_DIM;
  
  //
  // netcdf variable names
  //
  static const char* IMAGE_DATE_VAR;
  static const char* IMAGE_TIME_VAR;
  static const char* START_LINE_VAR;
  static const char* START_ELEM_VAR;
  static const char* NUM_LINES_VAR;
  static const char* NUM_ELEMS_VAR;
  static const char* DATA_WIDTH_VAR;
  static const char* LINE_RES_VAR;
  static const char* ELEM_RES_VAR;
  static const char* PREFIX_SIZE_VAR;
  static const char* CR_DATE_VAR;
  static const char* CR_TIME_VAR;
  static const char* BANDS_VAR;
  static const char* AUDIT_TRAIL_VAR;
  static const char* DATA_VAR;
  static const char* LATITUDE_VAR;
  static const char* LONGITUDE_VAR;

  //
  // netcdf variable units
  //
  static const char* RES_UNITS;
  static const char* GRID_UNITS;

  const Params* _params;

  NcFile* _ncFile;

  Pjg* _pjg;

  bool _processData;

  string _pathName;

  float _missing;

  // numElems variable
  int _numX; 

  // numLines variable
  int _numY;
  
  // _numX*_numY
  int _numPts;

  // number of bands -- bands dimension
  int _numBands;

  // elemRes
  float _dx;

  // lineRes
  float _dy;

  // number of bytes per data point -- we won't need this value, but grab it anyway
  int _dataWidth;

  // line prefix size -- we won't need this value, but grab it anyway
  int _prefixSize;

  // starting image line  -- we won't need this value, but grab it anyway
  int _startLine;

  // starting image element  -- we won't need this value, but grab it anyway
  int _startElem;

  // element resolution in km
  int _elemRes;
  
  // line resolution in km.
  int _lineRes;

  // combination of imageDate and imageTime 
  DateTime _imageTime; 

  // combination of crDate and crTime 
  DateTime _creationTime;


  //
  // right now only one band is in a SEVIRI netcdf file, but the definition allows
  // for multiple bands. The class will handle multple bands per file
  //
  vector<int> _bandNums;
  vector<float *> _bandData;


  /////////////////////
  // private methods //
  /////////////////////

  void _copy(const SeviriData &from);
  bool _getDimension(NcDim **dim, const char* id);
  bool _getVariable(NcVar **var, const char* id);
  void _setTime(int the_date, int the_time, 
		DateTime& datetime);
  void _setProjection(const NcVar* latitude, const NcVar* longitude);
  bool _processBand(int num);
  int _getBandInfoIndex(int band_num);

};

# endif     /* SEVIRI_DATA_H */
