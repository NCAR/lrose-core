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
// DataSet.hh
//
// DataSet object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#ifndef DataSet_H
#define DataSet_H

#include <string>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <netcdf.hh>
#include "Params.hh"
#include "SunAngle.hh"

using namespace std;

////////////////////////
// This class

class DataSet {
  
public:

  // constructor

  DataSet (const Params &params,
	   const string &fileNameSubString,
	   const string &fieldName,
	   const string &fieldNameLong,
	   const string &units,
	   double dataScale,
	   double dataOffset,
	   int gridNx,
	   int gridNy,
	   double gridMinx,
	   double gridMiny,
	   double gridDx,
	   double gridDy,
	   const string &outputUrl);

  // destructor
  
  ~DataSet();

  // check if appropriate for given file path
  // Is considered appropriate if the file name
  // substring is contained in the file name, or if
  // the substring is zero-length.
  
  bool appropriateForFile(const char *input_path);

  // process this file
  
  int processFile(const char *input_path);

protected:
  
private:

  static const fl32 _missingFloat;

  const Params &_params;
  SunAngle _sunAngle;

  string _fileNameSubString;
  string _fieldName;
  string _fieldNameLong;
  string _units;
  double _dataScale;
  double _dataOffset;

  MdvxProj _proj;
  time_t _validTime, _imageTime;
  int _nx, _ny;
  double _minx, _miny;
  double _dx, _dy;
  double _lineRes, _elemRes;
  int _oversampleX, _oversampleY;
  int _dataWidth;
  string _outputUrl;

  int _inputNx, _inputNy;
  double _inputMinx, _inputMiny;
  double _inputDx, _inputDy;

  bool _inputIsRegularLatlon;
  bool _preserveLatlonGrid;
  vector<double> _meanLat, _meanLon;

  int _sensorId;

  int _setMasterHeader(NcFile &ncf, DsMdvx &mdvx);
  int _addDataField(NcFile &ncf, DsMdvx &mdvx);
  
  void _insertDataVal(float dataVal,
		      float lat, float lon,
		      int ix, int iy,
		      fl32 **outputData,
		      fl32 **distanceError,
		      fl32 **outputLat,
		      fl32 **outputLon);
  
  int _checkFile(NcFile &ncf);

  void _checkForRegularLatLonInput(int nLines, int nElems,
                                   const float *latArray,
                                   const float *lonArray);
    
  void _printFile(NcFile &ncf);
  void _printAtt(NcAtt *att);
  void _printVarVals(NcVar *var);

  int _writeZebraNetCDF(const string &dataSetName,
                        const string &fieldName,
                        const string &units,
                        fl32 **data);

};

#endif

