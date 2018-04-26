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
// NcfWrapper.cc
//
// NetCDF file wrapper
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2013
//
///////////////////////////////////////////////////////////////

#include "NcfWrapper.hh"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <toolsa/TaArray.hh>
using namespace std;

const double NcfWrapper::missingDouble = -9999.0;
const float NcfWrapper::missingFloat = -9999.0;
const int NcfWrapper::missingInt = -9999;
const unsigned char NcfWrapper::missingUchar = 0;

//////////////
// Constructor

NcfWrapper::NcfWrapper()
  
{

  _ncFormat = NcxxFile::classic;
  _ncFile = NULL;
  clear();

}

/////////////
// destructor

NcfWrapper::~NcfWrapper()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NcfWrapper::clear()
  
{
  clearErrStr();
  close();
  _pathInUse.clear();
}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int NcfWrapper::openRead(const string &path)
  
{

  close();
  _pathInUse = path;
  _ncFile = new NcxxFile(path, NcxxFile::read);

  // Check that constructor succeeded
  
  if (_ncFile == NULL || _ncFile->isNull()) {
    _addErrStr("ERROR - NcfWrapper::openRead");
    _addErrStr("  Cannot open file for reading, path: ", path);
    close();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
/// open netcdf file for writing
/// create error object so we can handle errors
/// set the netcdf format, before a write
/// format options are:
///   Classic - classic format (i.e. version 1 format)
///   Offset64Bits - 64-bit offset format
///   Netcdf4 - netCDF-4 using HDF5 format
///   Netcdf4Classic - netCDF-4 using HDF5 but only netCDF-3 calls
/// Returns 0 on success, -1 on failure

int NcfWrapper::openWrite(const string &path,
                          NcxxFile::FileFormat format) 

{
  
  close();
  _pathInUse = path;
  _ncFormat = format;
  _ncFile = new NcxxFile(path, NcxxFile::replace, _ncFormat);
  
  if (!_ncFile || !_ncFile->isNull()) {
    _addErrStr("ERROR - NcfWrapper::openWrite");
    _addErrStr("  Cannot open netCDF file for writing: ", path);
    close();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void NcfWrapper::close()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    delete _ncFile;
    _ncFile = NULL;
  }
  
}

///////////////////////////////////////////
// add string global attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addGlobAttr(const string &name, const string &val)
{
  try {
    _ncFile->putAtt(name, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addGlobalAttributes");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrStr("  val: ", val);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add int global attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addGlobAttr(const string &name, int val)
{
  NcxxInt xtype;
  try {
    _ncFile->putAtt(name, xtype, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add float global attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addGlobAttr(const string &name, float val)
{
  NcxxFloat xtype;
  try {
    _ncFile->putAtt(name, xtype, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// read a global attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::readGlobAttr(const string &name, string &val)
{
  NcxxGroupAtt att = _ncFile->getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  try {
    att.getValues(val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    _addErrStr("  Cannot read value as string");
    return -1;
  }
  return 0;
}

int NcfWrapper::readGlobAttr(const string &name, int &val)
{
  NcxxGroupAtt att = _ncFile->getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  int *vals = new int[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    _addErrStr("  Cannot read value as int");
    delete[] vals;
    return -1;
  }
  val = vals[0];
  delete[] vals;
  return 0;
}

int NcfWrapper::readGlobAttr(const string &name, float &val)
{
  NcxxGroupAtt att = _ncFile->getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  float *vals = new float[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    _addErrStr("  Cannot read value as float");
    delete[] vals;
    return -1;
  }
  val = vals[0];
  delete[] vals;
  return 0;
}

int NcfWrapper::readGlobAttr(const string &name, double &val)
{
  NcxxGroupAtt att = _ncFile->getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  double *vals = new double[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    _addErrStr("  Cannot read value as double");
    delete[] vals;
    return -1;
  }
  val = vals[0];
  delete[] vals;
  return 0;
}

///////////////////////////////////////////
// add string attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, const string &val)
{
  try {
    var.putAtt(name.c_str(), val.c_str());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add string var attr, name: ", name);
    _addErrStr("  val: ", val);
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add double attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, double val)
{
  try {
    NcxxType vtype(NcxxType::nc_DOUBLE);
    var.putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add double var attr, name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add float attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, float val)
{
  try {
    NcxxType vtype(NcxxType::nc_FLOAT);
    var.putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add float var attr, name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add int attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, int val)
{
  try {
    NcxxType vtype(NcxxType::nc_INT);
    var.putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add int var attr, name: ", name);
    _addErrDbl("  val: ", val, "%d");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add long attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, int64_t val)
{
  try {
    NcxxType vtype(NcxxType::nc_INT64);
    var.putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add int64_t var attr, name: ", name);
    _addErrDbl("  val: ", val, "%ld");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add short attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, short val)
{
  try {
    NcxxType vtype(NcxxType::nc_SHORT);
    var.putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add short var attr, name: ", name);
    _addErrDbl("  val: ", (int) val, "%d");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add ncbyte attribute
// Returns 0 on success, -1 on failure

int NcfWrapper::addAttr(NcxxVar &var, const string &name, unsigned char val)
{
  try {
    NcxxType vtype(NcxxType::nc_UBYTE);
    var.putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::addAttr");
    _addErrStr("  Cannot add byte var attr, name: ", name);
    _addErrDbl("  val: ", (int) val, "%d");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add a dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is updated

int NcfWrapper::addDim(NcxxDim &dim, const string &name, int size)
{
  if (size < 1) {
    dim = _ncFile->addDim(name);
  } else {
    dim = _ncFile->addDim(name, size);
  }
  if (dim.isNull()) {
    _addErrStr("ERROR - NcfWrapper::addDim");
    _addErrStr("  Cannot add dimension: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// read a dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is set

int NcfWrapper::readDim(const string &name, NcxxDim &dim)

{
  dim = _ncFile->getDim(name);
  if (dim.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readDim");
    _addErrStr("  Cannot read dimension, name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////
// Add scalar var
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NcfWrapper::addVar(NcxxVar &var,
                       const string &name, 
                       const string &standardName,
                       const string &longName,
                       NcxxType ncType, 
                       const string &units /* = "" */)
  
{
  
  vector<NcxxDim> dims; // 0 length - for scalar

  return addVar(var, name, standardName, longName,
                ncType, dims, units);

}

///////////////////////////////////////
// Add 1-D array var
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NcfWrapper::addVar(NcxxVar &var, 
                       const string &name, 
                       const string &standardName,
                       const string &longName,
                       NcxxType ncType, 
                       NcxxDim &dim, 
                       const string &units /* = "" */)
  
{
  
  vector<NcxxDim> dims;
  dims.push_back(dim);

  return addVar(var, name, standardName, longName,
                ncType, dims, units);

}

///////////////////////////////////////
// Add 2-D array var
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NcfWrapper::addVar(NcxxVar &var, 
                       const string &name,
                       const string &standardName,
                       const string &longName,
                       NcxxType ncType,
                       NcxxDim &dim0,
                       NcxxDim &dim1,
                       const string &units /* = "" */)
{

  vector<NcxxDim> dims;
  dims.push_back(dim0);
  dims.push_back(dim1);

  return addVar(var, name, standardName, longName,
                ncType, dims, units);
  
}

///////////////////////////////////////
// Add var in multiple-dimensions
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NcfWrapper::addVar(NcxxVar &var, 
                       const string &name,
                       const string &standardName,
                       const string &longName,
                       NcxxType ncType,
                       vector<NcxxDim> &dims,
                       const string &units /* = "" */)
{

  var = _ncFile->addVar(name, ncType, dims);
  nc_type vtype = ncType.getId();
  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::addVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", ncTypeToStr(vtype));
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  if (standardName.length() > 0) {
    addAttr(var, "standard_name", standardName);
  }

  if (longName.length() > 0) {
    addAttr(var, "long_name", longName);
  }

  if (units.length() > 0) {
    addAttr(var, "units", units);
  }
  
  _setFillvalue(var);

  return 0;

}

////////////////////////////////////////////////
// get the total number of values in a variable
// this is the product of the dimension sizes
// and is 1 for a scalar (i.e. no dimensions)

int64_t NcfWrapper::numVals(NcxxVar &var)
  
{

  std::vector<NcxxDim> dims = var.getDims();
  int64_t prod = 1;
  for (size_t ii = 0; ii < dims.size(); ii++) {
    prod *=  var.getDim(ii).getSize();
  }
  return prod;

}
  
/////////////////////////////////////
// read int variable, set var and val
// Returns 0 on success, -1 on failure

int NcfWrapper::readIntVar(NcxxVar &var, const string &name,
                           int &val, int missingVal, bool required)
  
{
  
  var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - NcfWrapper::readIntVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }
  }

  // check size
  
  if (numVals(var) < 1) {
    _addErrStr("ERROR - NcfWrapper::readIntVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVal(index, &val);
  
  return 0;
  
}

///////////////////////////////////
// read int variable, set val

int NcfWrapper::readIntVal(const string &name, 
                           int &val, int missingVal,
                           bool required)
  
{
  
  val = missingVal;
  
  NcxxVar var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size
  
  if (numVals(var) < 1) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readIntVal");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVal(index, &val);

  return 0;
  
}

///////////////////////////////////
// read float variable
// Returns 0 on success, -1 on failure

int NcfWrapper::readFloatVar(NcxxVar &var, const string &name,
                             float &val, 
                             float missingVal, bool required)

{
  
  var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - NcfWrapper::readIntVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }
  }

  // check size
  
  if (numVals(var) < 1) {
    _addErrStr("ERROR - NcfWrapper::readIntVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVal(index, &val);
  
  return 0;
  
}

///////////////////////////////////
// read float value

int NcfWrapper::readFloatVal(const string &name,
                             float &val,
                             float missingVal,
                             bool required)
  
{
  
  val = missingVal;
  
  NcxxVar var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size
  
  if (numVals(var) < 1) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readIntVal");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVal(index, &val);

  return 0;
  
}

///////////////////////////////////
// read double variable
// Returns 0 on success, -1 on failure

int NcfWrapper::readDoubleVar(NcxxVar &var, const string &name,
                              double &val, 
                              double missingVal, bool required)
  
{
  
  var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - NcfWrapper::readIntVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }
  }

  // check size
  
  if (numVals(var) < 1) {
    _addErrStr("ERROR - NcfWrapper::readIntVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVal(index, &val);
  
  return 0;
  
}

///////////////////////////////////
// read double value

int NcfWrapper::readDoubleVal(const string &name,
                              double &val,
                              double missingVal,
                              bool required)

{
  
  val = missingVal;
  
  NcxxVar var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size
  
  if (numVals(var) < 1) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readIntVal");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVal(index, &val);

  return 0;
  
}

///////////////////////////////////
// read a scalar string variable
// Returns 0 on success, -1 on failure

int NcfWrapper::readStringVar(NcxxVar &var, const string &name, string &val)

{
  
  // get var
  
  var = _ncFile->getVar(name);
  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readIntVar");
    _addErrStr("  Cannot read variable, name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  // check dimension
  
  std::vector<NcxxDim> dims = var.getDims();
  int64_t prod = 1;
  for (size_t ii = 0; ii < dims.size(); ii++) {
    prod *=  var.getDim(ii).getSize();
  }
  return prod;

  if (dims.size() != 1) {
    _addErrStr("ERROR - NcfWrapper::readStringVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable does not have 1 dimension");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  NcxxDim stringLenDim = dims[0];
  if (stringLenDim.isNull()) {
    _addErrStr("ERROR - NcfWrapper::readStringVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has NULL 0th dimension");
    _addErrStr("  should be a string length dimension");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_CHAR) {
    _addErrStr("ERROR - NcfWrapper::readStringVar");
    _addErrStr("  Incorrect variable type");
    _addErrStr("  expecting char");
    _addErrStr("  found: ", ncTypeToStr(vtype));
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  // load up data

  int stringLen = stringLenDim.getSize();
  char *cvalues = new char[stringLen+1];
  vector<size_t> index;
  index.push_back(stringLen);
  var.getVal(index, cvalues);

  // ensure null termination
  cvalues[stringLen] = '\0';
  val = cvalues;
  delete[] cvalues;

  return 0;

}

////////////////////////////////////////////////////////
// read int32 1D array
// Note - unsigned 32-bit values will wrap
// 64-bit values are not handled.
// returns 0 on success, -1 on failure

int NcfWrapper::readInt32Array(const string &name, 
                               vector<int32_t> &array,
                               int32_t missingVal,
                               bool required)
  
{

  array.clear();

  // val = missingVal;
  
  NcxxVar var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readInt32Array");
      _addErrStr("  Cannot read array, name: ", name);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size

  size_t nVals = numVals(var);
  if (nVals < 1) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readInt32Array");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  NcxxType varType = var.getType();
  switch (varType.getId()) {

    case NcxxType::nc_BYTE: {
      TaArray<int8_t> vals_;
      int8_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) vals[ii]);
      }
      break;
    }
    case NcxxType::nc_UBYTE: {
      TaArray<u_int8_t> vals_;
      u_int8_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_SHORT: {
      TaArray<int16_t> vals_;
      int16_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_USHORT: {
      TaArray<u_int16_t> vals_;
      u_int16_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_INT: {
      TaArray<int32_t> vals_;
      int32_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_UINT: {
      TaArray<u_int32_t> vals_;
      u_int32_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_FLOAT: {
      TaArray<float> vals_;
      float *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) floor(vals[ii] + 0.5));
      }
      break;
    }
    case  NcxxType::nc_DOUBLE: {
      TaArray<double> vals_;
      double *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int32_t) floor(vals[ii] + 0.5));
      }
      break;
    }
    case  NcxxType::nc_CHAR:
    case  NcxxType::nc_INT64:
    case  NcxxType::nc_UINT64:
    case  NcxxType::nc_STRING:
    case  NcxxType::nc_VLEN:
    case  NcxxType::nc_OPAQUE:
    case  NcxxType::nc_ENUM:
    case  NcxxType::nc_COMPOUND:
    default:
      _addErrStr("ERROR - NcfWrapper::_readInt32Array");
      _addErrStr("  Bad type for array: ", varType.getName());
      _addErrStr("  variable name: ", name);
      _addErrStr("  file: ", _pathInUse);
      return -1;

  } // switch

  // get the fill value, first using _FillValue and then missingValue

  NcxxVarAtt fillValAtt;
  try {
    fillValAtt = var.getAtt("_FillValue");
  }
  catch (NcxxException e) {
    try {
      fillValAtt = var.getAtt("missingValue");
    }
    catch (NcxxException e) {
    }
  }
  
  int32_t fillVal = -9999;
  if (!fillValAtt.isNull() && fillValAtt.getAttLength() > 0) {
    TaArray<int32_t> fillVals_;
    int32_t *fillVals = fillVals_.alloc(fillValAtt.getAttLength());
    fillValAtt.getValues(fillVals);
    fillVal = fillVals[0];
  }

  // set missing val

  for (size_t ii = 0; ii < nVals; ii++) {
    if (array[ii] == fillVal) {
      array[ii] = missingVal;
    }
  }

  return 0;
  
}

////////////////////////////////////////////////////////
// read int64 1D array
// Note - unsigned 64-bit values will wrap
// returns 0 on success, -1 on failure

int NcfWrapper::readInt64Array(const string &name, 
                               vector<int64_t> &array,
                               int64_t missingVal,
                               bool required)
  
{

  array.clear();

  // val = missingVal;
  
  NcxxVar var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readInt64Array");
      _addErrStr("  Cannot read array, name: ", name);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size

  size_t nVals = numVals(var);
  if (nVals < 1) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readInt64Array");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  NcxxType varType = var.getType();
  switch (varType.getId()) {

    case NcxxType::nc_BYTE: {
      TaArray<int8_t> vals_;
      int8_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case NcxxType::nc_UBYTE: {
      TaArray<u_int8_t> vals_;
      u_int8_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_SHORT: {
      TaArray<int16_t> vals_;
      int16_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_USHORT: {
      TaArray<u_int16_t> vals_;
      u_int16_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_INT: {
      TaArray<int32_t> vals_;
      int32_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_UINT: {
      TaArray<u_int32_t> vals_;
      u_int32_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_INT64: {
      TaArray<int64_t> vals_;
      int64_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_UINT64: {
      TaArray<u_int64_t> vals_;
      u_int64_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_FLOAT: {
      TaArray<float> vals_;
      float *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) floor(vals[ii] + 0.5));
      }
      break;
    }
    case  NcxxType::nc_DOUBLE: {
      TaArray<double> vals_;
      double *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((int64_t) floor(vals[ii] + 0.5));
      }
      break;
    }
    case  NcxxType::nc_CHAR:
    case  NcxxType::nc_STRING:
    case  NcxxType::nc_VLEN:
    case  NcxxType::nc_OPAQUE:
    case  NcxxType::nc_ENUM:
    case  NcxxType::nc_COMPOUND:
    default:
      _addErrStr("ERROR - NcfWrapper::_readInt64Array");
      _addErrStr("  Bad type for array: ", varType.getName());
      _addErrStr("  variable name: ", name);
      _addErrStr("  file: ", _pathInUse);
      return -1;

  } // switch

  // get the fill value, first using _FillValue and then missingValue

  NcxxVarAtt fillValAtt;
  try {
    fillValAtt = var.getAtt("_FillValue");
  }
  catch (NcxxException e) {
    try {
      fillValAtt = var.getAtt("missingValue");
    }
    catch (NcxxException e) {
    }
  }
  
  int64_t fillVal = -9999;
  if (!fillValAtt.isNull() && fillValAtt.getAttLength() > 0) {
    TaArray<int64_t> fillVals_;
    int64_t *fillVals = fillVals_.alloc(fillValAtt.getAttLength());
    fillValAtt.getValues(fillVals);
    fillVal = fillVals[0];
  }

  // set missing val

  for (size_t ii = 0; ii < nVals; ii++) {
    if (array[ii] == fillVal) {
      array[ii] = missingVal;
    }
  }

  return 0;
  
}

////////////////////////////////////////////////////////
// read double 1D array
// returns 0 on success, -1 on failure

int NcfWrapper::readDoubleArray(const string &name, 
                                vector<double> &array,
                                double missingVal,
                                bool required)
  
{

  array.clear();

  // val = missingVal;
  
  NcxxVar var = _ncFile->getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readDoubleArray");
      _addErrStr("  Cannot read array, name: ", name);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size

  size_t nVals = numVals(var);
  if (nVals < 1) {
    if (required) {
      _addErrStr("ERROR - NcfWrapper::_readDoubleArray");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  NcxxType varType = var.getType();
  switch (varType.getId()) {

    case NcxxType::nc_BYTE: {
      TaArray<int8_t> vals_;
      int8_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case NcxxType::nc_UBYTE: {
      TaArray<u_int8_t> vals_;
      u_int8_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_SHORT: {
      TaArray<int16_t> vals_;
      int16_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_USHORT: {
      TaArray<u_int16_t> vals_;
      u_int16_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_INT: {
      TaArray<int32_t> vals_;
      int32_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_UINT: {
      TaArray<u_int32_t> vals_;
      u_int32_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_INT64: {
      TaArray<int64_t> vals_;
      int64_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_UINT64: {
      TaArray<u_int64_t> vals_;
      u_int64_t *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_FLOAT: {
      TaArray<float> vals_;
      float *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back((double) vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_DOUBLE: {
      TaArray<double> vals_;
      double *vals = vals_.alloc(nVals);
      var.getVal(vals);
      for (size_t ii = 0; ii < nVals; ii++) {
        array.push_back(vals[ii]);
      }
      break;
    }
    case  NcxxType::nc_CHAR:
    case  NcxxType::nc_STRING:
    case  NcxxType::nc_VLEN:
    case  NcxxType::nc_OPAQUE:
    case  NcxxType::nc_ENUM:
    case  NcxxType::nc_COMPOUND:
    default:
      _addErrStr("ERROR - NcfWrapper::_readDoubleArray");
      _addErrStr("  Bad type for array: ", varType.getName());
      _addErrStr("  variable name: ", name);
      _addErrStr("  file: ", _pathInUse);
      return -1;

  } // switch

  // get the fill value, first using _FillValue and then missingValue

  NcxxVarAtt fillValAtt;
  try {
    fillValAtt = var.getAtt("_FillValue");
  }
  catch (NcxxException e) {
    try {
      fillValAtt = var.getAtt("missingValue");
    }
    catch (NcxxException e) {
    }
  }
  
  double fillVal = -9999.0;
  if (!fillValAtt.isNull() && fillValAtt.getAttLength() > 0) {
    TaArray<double> fillVals_;
    double *fillVals = fillVals_.alloc(fillValAtt.getAttLength());
    fillValAtt.getValues(fillVals);
    fillVal = fillVals[0];
  }

  // set missing val

  for (size_t ii = 0; ii < nVals; ii++) {
    if (fabs(array[ii] - fillVal) < 1.0e-5) {
      array[ii] = missingVal;
    }
  }

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
// write a scalar double variable
// Returns 0 on success, -1 on failure

int NcfWrapper::writeVar(NcxxVar &var, double val)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_DOUBLE) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Var type should be double, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    var.putVal(index, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Cannot write scalar double var, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a scalar float variable
// Returns 0 on success, -1 on failure

int NcfWrapper::writeVar(NcxxVar &var, float val)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_FLOAT) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Var type should be float, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    var.putVal(index, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Cannot write scalar float var, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a scalar int variable
// Returns 0 on success, -1 on failure

int NcfWrapper::writeVar(NcxxVar &var, int val)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_INT) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Var type should be int, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    var.putVal(index, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Cannot write scalar int var, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in dimension
// Returns 0 on success, -1 on failure

int NcfWrapper::writeVar(NcxxVar &var, const NcxxDim &dim, const void *data)
  
{
  return writeVar(var, dim, dim.getSize(), data);
}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in arguments
// Returns 0 on success, -1 on failure

int NcfWrapper::writeVar(NcxxVar &var, 
                         const NcxxDim &dim, size_t count, 
                         const void *data)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - ForayNcRadxFile::_writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  int iret = 0;
  vector<size_t> starts, counts;
  starts.push_back(0);
  counts.push_back(count);
  
  nc_type vtype = var.getType().getId();
  switch (vtype) {
    case NC_DOUBLE: {
      try {
        var.putVal(starts, counts, (double *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_INT: {
      try {
        var.putVal(starts, counts, (int *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_SHORT: {
      try {
        var.putVal(starts, counts, (short *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_UBYTE: {
      try {
        var.putVal(starts, counts, (unsigned char *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_FLOAT:
    default: {
      try {
        var.putVal(starts, counts, (float *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
  } // switch
  
  if (iret) {
    _addErrStr("ERROR - NcfWrapper::writeVar");
    _addErrStr("  Cannot write var, name: ", var.getName());
    _addErrStr("  Dim name: ", dim.getName());
    _addErrInt("  Count: ", count);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  } else {
    return 0;
  }

}


///////////////////////////////////////////////////////////////////////////
// write a string variable
// Returns 0 on success, -1 on failure

int NcfWrapper::writeStringVar(NcxxVar &var, const void *str)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::writeStringVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  std::vector<NcxxDim> dims = var.getDims();
  size_t nDims = dims.size();
  if (nDims < 1) {
    _addErrStr("ERROR - NcfWrapper::writeStringVar");
    _addErrStr("  var has no dimensions");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  if (nDims == 1) {

    // single dimension

    NcxxDim &dim0 = dims[0];
    if (dim0.isNull()) {
      _addErrStr("ERROR - NcfWrapper::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  dim 0 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize());
    try {
      var.putVal(starts, counts, (char *) str);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcfWrapper::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  file: ", _pathInUse);
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    return 0;
    
  } // if (nDims == 1)

  if (nDims == 2) {

    // two dimensions

    NcxxDim &dim0 = dims[0];
    if (dim0.isNull()) {
      _addErrStr("ERROR - NcfWrapper::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  dim 0 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    NcxxDim &dim1 = dims[1];
    if (dim1.isNull()) {
      _addErrStr("ERROR - NcfWrapper::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  dim 1 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize() * dim1.getSize());
    try {
      var.putVal(starts, counts, (char *) str);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcfWrapper::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  file: ", _pathInUse);
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    return 0;

  }

  // more than 2 is an error
  
  _addErrStr("ERROR - NcfWrapper::writeStringVar");
  _addErrStr("  Canont write var, name: ", var.getName());
  _addErrInt("  more than 2 dimensions: ", nDims);
  _addErrStr("  file: ", _pathInUse);
  return -1;

}

///////////////////////////////////////////////////////////////////////////
// compress a variable

int NcfWrapper::compressVar(NcxxVar &var, int compressionLevel)
{
  
  if (_ncFormat == NcxxFile::classic ||
      _ncFormat == NcxxFile::classic64) {
    // cannot compress
    return 0;
  }

  if (var.isNull()) {
    _addErrStr("ERROR - NcfWrapper::setVarCompression");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  int fileId = _ncFile->getId();
  int varId = var.getId();
  int shuffle = 0;
  
  if (nc_def_var_deflate(fileId, varId, shuffle,
                         true, compressionLevel)!= NC_NOERR) {
    _addErrStr("ERROR: FieldData::setCompression");
    _addErrStr("  Problem setting compression for var: ",
               var.getName());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  type: ", varTypeToStr(var));
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// convert type enum to string

string NcfWrapper::ncTypeToStr(nc_type nctype)
  
{
  
  switch (nctype) {
    case NC_DOUBLE:
      return "NC_DOUBLE";
    case NC_FLOAT:
      return "NC_FLOAT";
    case NC_INT:
      return "NC_INT";
    case NC_SHORT:
      return "NC_SHORT";
    case NC_UBYTE:
    default:
      return "NC_UBYTE";
  }
  
}

////////////////////////////////////////
// convert var type string

string NcfWrapper::varTypeToStr(const NcxxVar &var)
  
{
  nc_type vtype = var.getType().getId();
  return ncTypeToStr(vtype);
}

///////////////////////////////////////////
// get string representation of component

string NcfWrapper::asString(const NcxxAtt *att)
  
{
  
  string val;
  att->getValues(val);
  return val;

}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void NcfWrapper::_addErrInt(string label, int iarg, bool cr)
{
  _errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void NcfWrapper::_addErrDbl(string label, double darg,
                            string format, bool cr)

{
  _errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void NcfWrapper::_addErrStr(string label, string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// set fill value

void NcfWrapper::_setFillvalue(NcxxVar &var)

{

  nc_type vtype = var.getType().getId();
  if (vtype == NC_DOUBLE) {
    addAttr(var, "_fillValue", missingDouble);
    return;
  }
  if (vtype == NC_FLOAT) {
    addAttr(var, "_fillValue", (float) missingDouble);
    return;
  }
  if (vtype == NC_INT) {
    addAttr(var, "_fillValue", missingInt);
    return;
  }
  if (vtype == NC_LONG) {
    addAttr(var, "_fillValue", missingInt);
    return;
  }
#ifdef GOOD_FOR_64BIT
  // TODO - FIX LATER - DIXON
  // NOTE - FOR 64 bit can use the following instead of previous
  if (vtype == NC_LONG) {
    addAttr(var, "_fillValue", (long) missingInt);
    return;
  }
#endif
  if (vtype == NC_SHORT) {
    addAttr(var, "_fillValue", (short) missingInt);
    return;
  }
  if (vtype == NC_UBYTE) {
    addAttr(var, "_fillValue", missingUchar);
    return;
  }
  addAttr(var, "_fillValue", missingInt);
}
