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
// NetcdfCxxFile.cc
//
// NetCDF CXX file wrapper
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////

#include <NcUtils/NetcdfCxxFile.hh>
#include <NcUtils/NcUtils.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

NetcdfCxxFile::NetcdfCxxFile()
  
{

  _ncFormat = NcFile::classic;
  _ncFile = NULL;

  clear();


}

/////////////
// destructor

NetcdfCxxFile::~NetcdfCxxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void NetcdfCxxFile::clear()
  
{
  clearErrStr();
  close();
  _pathInUse.clear();
}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::openRead(const string &path)
  
{

  close();
  _pathInUse = path;
  _ncFile = new NcFile(path, NcFile::read);
  
  // Check that constructor succeeded
  
  if (!_ncFile || _ncFile->isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::openRead");
    _addErrStr("  File is not NetCDF, path: ", path);
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

int NetcdfCxxFile::openWrite(const string &path,
                             NcFile::FileFormat format) 

{
  
  close();
  _pathInUse = path;
  _ncFormat = format;
  _ncFile = new NcFile(path, NcFile::replace, _ncFormat);
  
  if (!_ncFile || !_ncFile->isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::openWrite");
    _addErrStr("  Cannot open netCDF file for writing: ", path);
    close();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void NetcdfCxxFile::close()
  
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

int NetcdfCxxFile::addGlobAttr(const string &name, const string &val)
{
  string sname = stripNulls(name);
  try {
    _ncFile->putAtt(sname, val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addGlobalAttributes");
    _addErrStr("  Cannot add global attr name: ", sname);
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

int NetcdfCxxFile::addGlobAttr(const string &name, int val)
{
  string sname = stripNulls(name);
  NcInt xtype;
  try {
    _ncFile->putAtt(sname, xtype, val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", sname);
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

int NetcdfCxxFile::addGlobAttr(const string &name, float val)
{
  string sname = stripNulls(name);
  NcFloat xtype;
  try {
    _ncFile->putAtt(sname, xtype, val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", sname);
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

int NetcdfCxxFile::readGlobAttr(const string &name, string &val)
{
  string sname = stripNulls(name);
  NcGroupAtt att = _ncFile->getAtt(sname);
  if (att.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  try {
    att.getValues(val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    _addErrStr("  Cannot read value as string");
    return -1;
  }
  val = stripNulls(val);
  return 0;
}

int NetcdfCxxFile::readGlobAttr(const string &name, int &val)
{
  string sname = stripNulls(name);
  NcGroupAtt att = _ncFile->getAtt(sname);
  if (att.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  int *vals = new int[nvals];
  try {
    att.getValues(vals);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
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

int NetcdfCxxFile::readGlobAttr(const string &name, float &val)
{
  string sname = stripNulls(name);
  NcGroupAtt att = _ncFile->getAtt(sname);
  if (att.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  float *vals = new float[nvals];
  try {
    att.getValues(vals);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
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

int NetcdfCxxFile::readGlobAttr(const string &name, double &val)
{
  string sname = stripNulls(name);
  NcGroupAtt att = _ncFile->getAtt(sname);
  if (att.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  double *vals = new double[nvals];
  try {
    att.getValues(vals);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, const string &val)
{
  string sname = stripNulls(name);
  try {
    var.putAtt(sname.c_str(), val.c_str());
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add string var attr, name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, double val)
{
  string sname = stripNulls(name);
  try {
    NcType vtype(NcType::nc_DOUBLE);
    var.putAtt(sname.c_str(), vtype, 1, &val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add double var attr, name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, float val)
{
  string sname = stripNulls(name);
  try {
    NcType vtype(NcType::nc_FLOAT);
    var.putAtt(sname.c_str(), vtype, 1, &val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add float var attr, name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, int val)
{
  string sname = stripNulls(name);
  try {
    NcType vtype(NcType::nc_INT);
    var.putAtt(sname.c_str(), vtype, 1, &val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add int var attr, name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, int64_t val)
{
  string sname = stripNulls(name);
  try {
    NcType vtype(NcType::nc_INT64);
    var.putAtt(sname.c_str(), vtype, 1, &val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add int64_t var attr, name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, short val)
{
  string sname = stripNulls(name);
  try {
    NcType vtype(NcType::nc_SHORT);
    var.putAtt(sname.c_str(), vtype, 1, &val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add short var attr, name: ", sname);
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

int NetcdfCxxFile::addAttr(NcVar &var, const string &name, unsigned char val)
{
  string sname = stripNulls(name);
  try {
    NcType vtype(NcType::nc_UBYTE);
    var.putAtt(sname.c_str(), vtype, 1, &val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::addAttr");
    _addErrStr("  Cannot add byte var attr, name: ", sname);
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

int NetcdfCxxFile::addDim(NcDim &dim, const string &name, int size)
{
  string sname = stripNulls(name);
  if (size < 1) {
    dim = _ncFile->addDim(sname);
  } else {
    dim = _ncFile->addDim(sname, size);
  }
  if (dim.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::addDim");
    _addErrStr("  Cannot add dimension: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// read a dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is set

int NetcdfCxxFile::readDim(const string &name, NcDim &dim)

{
  string sname = stripNulls(name);
  dim = _ncFile->getDim(sname);
  if (dim.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readDim");
    _addErrStr("  Cannot read dimension, name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////
// Add scalar var
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NetcdfCxxFile::addVar(NcVar &var,
                          const string &name, 
                          const string &standardName,
                          const string &longName,
                          NcType ncType, 
                          const string &units /* = "" */)
  
{
  
  vector<NcDim> dims; // 0 length - for scalar

  return addVar(var, name, standardName, longName,
                ncType, dims, units);

}

///////////////////////////////////////
// Add 1-D array var
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NetcdfCxxFile::addVar(NcVar &var, 
                          const string &name, 
                          const string &standardName,
                          const string &longName,
                          NcType ncType, 
                          NcDim &dim, 
                          const string &units /* = "" */)
  
{
  
  vector<NcDim> dims;
  dims.push_back(dim);

  return addVar(var, name, standardName, longName,
                ncType, dims, units);

}

///////////////////////////////////////
// Add 2-D array var
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NetcdfCxxFile::addVar(NcVar &var, 
                          const string &name,
                          const string &standardName,
                          const string &longName,
                          NcType ncType,
                          NcDim &dim0,
                          NcDim &dim1,
                          const string &units /* = "" */)
{

  vector<NcDim> dims;
  dims.push_back(dim0);
  dims.push_back(dim1);

  return addVar(var, name, standardName, longName,
                ncType, dims, units);
  
}

///////////////////////////////////////
// Add var in multiple-dimensions
// Returns 0 on success, -1 on failure
// Side effect: var is set

int NetcdfCxxFile::addVar(NcVar &var, 
                          const string &name,
                          const string &standardName,
                          const string &longName,
                          NcType ncType,
                          vector<NcDim> &dims,
                          const string &units /* = "" */)
{

  string sname = stripNulls(name);
  var = _ncFile->addVar(sname, ncType, dims);
  nc_type vtype = ncType.getId();
  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::addVar");
    _addErrStr("  Cannot add var, name: ", sname);
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

int64_t NetcdfCxxFile::numVals(NcVar &var)
  
{

  std::vector<NcDim> dims = var.getDims();
  int64_t prod = 1;
  for (size_t ii = 0; ii < dims.size(); ii++) {
    prod *=  var.getDim(ii).getSize();
  }
  return prod;

}
  
/////////////////////////////////////
// read int variable, set var and val
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::readIntVar(NcVar &var, const string &name,
                              int &val, int missingVal, bool required)
  
{
  
  string sname = stripNulls(name);
  var = _ncFile->getVar(sname);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
      _addErrStr("  Cannot read variable, name: ", sname);
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }
  }

  // check size
  
  if (numVals(var) < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
    _addErrStr("  variable name: ", sname);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVar(index, &val);
  
  return 0;
  
}

///////////////////////////////////
// read int variable, set val

int NetcdfCxxFile::readIntVal(const string &name, 
                              int &val, int missingVal,
                              bool required)
  
{
  
  string sname = stripNulls(name);
  val = missingVal;
  
  NcVar var = _ncFile->getVar(sname);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NetcdfCxxFile::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", sname);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size
  
  if (numVals(var) < 1) {
    if (required) {
      _addErrStr("ERROR - NetcdfCxxFile::_readIntVal");
      _addErrStr("  variable name: ", sname);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVar(index, &val);

  return 0;
  
}

///////////////////////////////////
// read float variable
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::readFloatVar(NcVar &var, const string &name,
                                float &val, 
                                float missingVal, bool required)

{
  
  string sname = stripNulls(name);
  var = _ncFile->getVar(sname);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
      _addErrStr("  Cannot read variable, name: ", sname);
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }
  }

  // check size
  
  if (numVals(var) < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
    _addErrStr("  variable name: ", sname);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVar(index, &val);
  
  return 0;
  
}

///////////////////////////////////
// read float value

int NetcdfCxxFile::readFloatVal(const string &name,
                                float &val,
                                float missingVal,
                                bool required)
  
{
  
  string sname = stripNulls(name);
  val = missingVal;
  
  NcVar var = _ncFile->getVar(sname);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NetcdfCxxFile::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", sname);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size
  
  if (numVals(var) < 1) {
    if (required) {
      _addErrStr("ERROR - NetcdfCxxFile::_readIntVal");
      _addErrStr("  variable name: ", sname);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVar(index, &val);

  return 0;
  
}

///////////////////////////////////
// read double variable
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::readDoubleVar(NcVar &var, const string &name,
                                 double &val, 
                                 double missingVal, bool required)
  
{
  
  string sname = stripNulls(name);
  var = _ncFile->getVar(sname);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
      _addErrStr("  Cannot read variable, name: ", sname);
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }
  }

  // check size
  
  if (numVals(var) < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
    _addErrStr("  variable name: ", sname);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVar(index, &val);
  
  return 0;
  
}

///////////////////////////////////
// read double value

int NetcdfCxxFile::readDoubleVal(const string &name,
                                 double &val,
                                 double missingVal,
                                 bool required)

{
  
  string sname = stripNulls(name);
  val = missingVal;
  
  NcVar var = _ncFile->getVar(sname);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NetcdfCxxFile::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", sname);
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }
  
  // check size
  
  if (numVals(var) < 1) {
    if (required) {
      _addErrStr("ERROR - NetcdfCxxFile::_readIntVal");
      _addErrStr("  variable name: ", sname);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  vector<size_t> index;
  index.push_back(0);
  var.getVar(index, &val);

  return 0;
  
}

///////////////////////////////////
// read a scalar string variable
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::readStringVar(NcVar &var, const string &name, string &val)

{
  
  // get var
  
  string sname = stripNulls(name);
  var = _ncFile->getVar(sname);
  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readIntVar");
    _addErrStr("  Cannot read variable, name: ", sname);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  // check dimension
  
  std::vector<NcDim> dims = var.getDims();
  int64_t prod = 1;
  for (size_t ii = 0; ii < dims.size(); ii++) {
    prod *=  var.getDim(ii).getSize();
  }
  return prod;

  if (dims.size() != 1) {
    _addErrStr("ERROR - NetcdfCxxFile::readStringVar");
    _addErrStr("  variable name: ", sname);
    _addErrStr("  variable does not have 1 dimension");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  NcDim stringLenDim = dims[0];
  if (stringLenDim.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::readStringVar");
    _addErrStr("  variable name: ", sname);
    _addErrStr("  variable has NULL 0th dimension");
    _addErrStr("  should be a string length dimension");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_CHAR) {
    _addErrStr("ERROR - NetcdfCxxFile::readStringVar");
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
  var.getVar(index, cvalues);

  // ensure null termination
  cvalues[stringLen] = '\0';
  val = cvalues;
  delete[] cvalues;

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a scalar double variable
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::writeVar(NcVar &var, double val)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_DOUBLE) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
    _addErrStr("  Var type should be double, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    var.putVar(index, val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
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

int NetcdfCxxFile::writeVar(NcVar &var, float val)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_FLOAT) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
    _addErrStr("  Var type should be float, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    var.putVar(index, val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
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

int NetcdfCxxFile::writeVar(NcVar &var, int val)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  nc_type vtype = var.getType().getId();
  if (vtype != NC_INT) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
    _addErrStr("  Var type should be int, name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    var.putVar(index, val);
  } catch (NcException& e) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
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

int NetcdfCxxFile::writeVar(NcVar &var, const NcDim &dim, const void *data)
  
{
  return writeVar(var, dim, dim.getSize(), data);
}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in arguments
// Returns 0 on success, -1 on failure

int NetcdfCxxFile::writeVar(NcVar &var, 
                            const NcDim &dim, size_t count, 
                            const void *data)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - ForayNcNcUtilsFile::_writeVar");
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
        var.putVar(starts, counts, (double *) data);
      } catch (NcException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_INT: {
      try {
        var.putVar(starts, counts, (int *) data);
      } catch (NcException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_SHORT: {
      try {
        var.putVar(starts, counts, (short *) data);
      } catch (NcException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_UBYTE: {
      try {
        var.putVar(starts, counts, (unsigned char *) data);
      } catch (NcException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_FLOAT:
    default: {
      try {
        var.putVar(starts, counts, (float *) data);
      } catch (NcException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
  } // switch
  
  if (iret) {
    _addErrStr("ERROR - NetcdfCxxFile::writeVar");
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

int NetcdfCxxFile::writeStringVar(NcVar &var, const void *str)
  
{
  
  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  std::vector<NcDim> dims = var.getDims();
  size_t nDims = dims.size();
  if (nDims < 1) {
    _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
    _addErrStr("  var has no dimensions");
    _addErrStr("  var name: ", var.getName());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  if (nDims == 1) {

    // single dimension

    NcDim &dim0 = dims[0];
    if (dim0.isNull()) {
      _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  dim 0 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize());
    try {
      var.putVar(starts, counts, (char *) str);
    } catch (NcException& e) {
      _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  file: ", _pathInUse);
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    return 0;
    
  } // if (nDims == 1)

  if (nDims == 2) {

    // two dimensions

    NcDim &dim0 = dims[0];
    if (dim0.isNull()) {
      _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  dim 0 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    NcDim &dim1 = dims[1];
    if (dim1.isNull()) {
      _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  dim 1 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize() * dim1.getSize());
    try {
      var.putVar(starts, counts, (char *) str);
    } catch (NcException& e) {
      _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
      _addErrStr("  Canont write var, name: ", var.getName());
      _addErrStr("  file: ", _pathInUse);
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    return 0;

  }

  // more than 2 is an error
  
  _addErrStr("ERROR - NetcdfCxxFile::writeStringVar");
  _addErrStr("  Canont write var, name: ", var.getName());
  _addErrInt("  more than 2 dimensions: ", nDims);
  _addErrStr("  file: ", _pathInUse);
  return -1;

}

///////////////////////////////////////////////////////////////////////////
// compress a variable

int NetcdfCxxFile::compressVar(NcVar &var, int compressionLevel)
{
  
  if (_ncFormat == NcFile::classic ||
      _ncFormat == NcFile::classic64) {
    // cannot compress
    return 0;
  }

  if (var.isNull()) {
    _addErrStr("ERROR - NetcdfCxxFile::setVarCompression");
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

string NetcdfCxxFile::ncTypeToStr(nc_type nctype)
  
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

string NetcdfCxxFile::varTypeToStr(const NcVar &var)
  
{
  nc_type vtype = var.getType().getId();
  return ncTypeToStr(vtype);
}

///////////////////////////////////////////
// get string representation of component

string NetcdfCxxFile::asString(const NcAtt *att)
  
{
  string val;
  att->getValues(val);
  return val;
}

///////////////////////////////////////////
// strip redundant null from string

string NetcdfCxxFile::stripNulls(const string &val)
  
{
  // find first null in string
  size_t len = val.size();
  for (size_t ii = 0; ii < val.size(); ii++) {
    if ((int) val[ii] == 0) {
      len = ii;
      break;
    }
  }
  return val.substr(0, len);
}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void NetcdfCxxFile::_addErrInt(string label, int iarg, bool cr)
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

void NetcdfCxxFile::_addErrDbl(string label, double darg,
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

void NetcdfCxxFile::_addErrStr(string label, string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// set fill value

void NetcdfCxxFile::_setFillvalue(NcVar &var)

{

  nc_type vtype = var.getType().getId();
  if (vtype == NC_DOUBLE) {
    addAttr(var, "_fillValue", NcUtils::missingFl64);
    return;
  }
  if (vtype == NC_FLOAT) {
    addAttr(var, "_fillValue", NcUtils::missingFl32);
    return;
  }
  if (vtype == NC_INT) {
    addAttr(var, "_fillValue", NcUtils::missingSi32);
    return;
  }
  if (vtype == NC_LONG) {
    addAttr(var, "_fillValue", NcUtils::missingSi32);
    return;
  }
  if (vtype == NC_SHORT) {
    addAttr(var, "_fillValue", (short) NcUtils::missingSi16);
    return;
  }
  if (vtype == NC_UBYTE) {
    addAttr(var, "_fillValue", NcUtils::missingSi08);
    return;
  }
  addAttr(var, "_fillValue", NcUtils::missingSi32);
}
