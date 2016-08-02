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
// Ncf2MdvField.hh
//
// Sue Dettling, Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008 
//
///////////////////////////////////////////////////////////////
//
// Ncf2MdvField class.
// Create an MdvxField from a CF NetCDF file
//
///////////////////////////////////////////////////////////////////////

#ifndef NCF2MDV_FIELD_HH
#define NCF2MDV_FIELD_HH

// #define USE_UDUNITS
#ifdef USE_UDUNITS
#include <udunits2.h>
#endif

#include <toolsa/TaArray.hh>
#include <Mdv/MdvxField.hh>
#include <netcdfcpp.h>
using namespace std;

////////////////////////
// 
// Ncf2MdvField object extracts data from a CF-compliant NetCDF file,
// and populates an MdvxField object with the data from the file.

class Ncf2MdvField {
  
public:
  
  // constructor

  Ncf2MdvField(bool debug,
               time_t validTime,
               int timeIndex,
               time_t forecastTime,
               int forecastDelta,
#ifdef USE_UDUNITS
               ut_system *uds,
#endif
               NcFile *ncFile, NcError *ncErr,
               NcVar *dataVar,
               NcDim *tDim, NcVar *tVar,
               NcDim *zDim, NcVar *zVar,
               NcDim *yDim, NcVar *yVar,
               NcDim *xDim, NcVar *xVar);
  
  // destructor

  ~Ncf2MdvField();
 
  // create MdvxField
  // pointer must be freed by calling routine
  // returns NULL on failure

  MdvxField *createMdvxField();

  // get the valid time

  time_t getValidTime() const { return _validTime; }

  // clear error string
  
  void clearErrStr();

  // Get the Error String. This has contents when an error is returned.
  
  string getErrStr() const { return _errStr; }

protected:  

  bool _debug;

  // Mdv field header and vlevel header, to be filled out
  
  time_t _validTime;
  int _timeIndex; // index into data for sets with multiple times
  Mdvx::field_header_t _fhdr;
  Mdvx::vlevel_header_t _vhdr;

#ifdef USE_UDUNITS  
  // unidata units handling
  
  ut_system *_uds;
#endif

  // netCDF file

  NcFile *_ncFile;
  NcError *_ncErr;

  // main data variable and type
  
  NcVar *_dataVar;
  NcType _dataType;
  TaArray<ui08> _data_;
  ui08 *_data;

  // dimensions and coordinate variables

  NcDim *_tDim;
  NcVar *_tVar;

  NcDim *_zDim;
  NcVar *_zVar;

  NcDim *_yDim;
  NcVar *_yVar;

  NcDim *_xDim;
  NcVar *_xVar;

  // projection

  Mdvx::projection_type_t _projType;
  MdvxProj _proj;
  string _projTypeStr;
  NcVar *_projVar;

  // error string

  string _errStr;
  
  // clear memory
  
  void _clear();

  // set the projection type information
  // returns 0 on success, -1 on failure
  
  int _setProjType();

  // set the projection information
  // returns 0 on success, -1 on failure
  
  int _setProjInfo();

  // set a particular projection parameter value
  // returns 0 on success, -1 on failure
  
  int _setProjParam(const string &param_name, double &param_val);

  // set a pair of projection parameter values
  // if only one value is available, both params are set the same
  // returns 0 on success, -1 on failure
  
  int _setProjParams(const string &param_name,
                     double &param_val_1,
                     double &param_val_2);

  // If the netCDF file was created from MDV, there will be some
  // parameters available from the field header
  
  int _setMdvSpecific();

  // Set names and units
  
  void _setNamesAndUnits();

  // Set dimensions of the grid
  // Returns 0 on success, -1 on failure

  int _setGridDimensions();
  
  // Set an X or Y axis
  // Returns 0 on success, -1 on failure
  
  int _setXYAxis(const string &axisName,
                 const NcVar *axisVar,
                 const string &latlonStdName,
                 const NcDim *axisDim,
                 int &nn,
                 double &minVal,
                 double &dVal);
  
  // Set Z axis
  // Returns 0 on success, -1 on failure

  void _setZAxis();

  // Set grid data
  // Returns 0 on success, -1 on failure

  int _setGridData();

  // set vals from attribute

  void _setSi32FromAttr(NcAtt *att, const string &requiredName, si32 &val);
  void _setFl32FromAttr(NcAtt *att, const string &requiredName, fl32 &val);
  void _setStrFromAttr(NcAtt *att, const string &requiredName, string &val);

  // get string from component
  
  string _asString(const NcTypedComponent *component, int index = 0) const;

  // get multiplier to convert to km

  double _getKmMult(const string &units) const;

private:

};

#endif
