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
// Nc3File.cc
//
// NetCDF file wrapper
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2017
//
///////////////////////////////////////////////////////////////

#include <Ncxx/Ncxx.hh>
#include <Ncxx/Nc3File.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

Nc3File::Nc3File()
  
{

  _ncFormat = NcFile::Classic;

  _ncFile = NULL;
  _err = NULL;

  clear();


}

/////////////
// destructor

Nc3File::~Nc3File()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void Nc3File::clear()
  
{
  clearErrStr();
  close();
  _pathInUse.clear();
}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int Nc3File::openRead(const string &path)
  
{

  close();
  _pathInUse = path;
  _ncFile = new NcFile(path.c_str(), NcFile::ReadOnly);
  
  // Check that constructor succeeded
  
  if (!_ncFile || !_ncFile->is_valid()) {
    _addErrStr("ERROR - Nc3File::openRead");
    _addErrStr("  File is not NetCDF, path: ", path);
    close();
    return -1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.

  if (_err == NULL) {
    _err = new NcError(NcError::silent_nonfatal);
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

int Nc3File::openWrite(const string &path,
                       NcFile::FileFormat format) 

{
  
  close();
  _pathInUse = path;
  _ncFormat = format;
  _ncFile = new NcFile(path.c_str(), NcFile::Replace, NULL, 0, _ncFormat);
  
  if (!_ncFile || !_ncFile->is_valid()) {
    _addErrStr("ERROR - Nc3File::openWrite");
    _addErrStr("  Cannot open netCDF file for writing: ", path);
    close();
    return -1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.
  
  if (_err == NULL) {
    _err = new NcError(NcError::silent_nonfatal);
  }

  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void Nc3File::close()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }
  
  if (_err) {
    delete _err;
    _err = NULL;
  }

}

///////////////////////////////////////////
// add string global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, const string &val)
{
  if (!_ncFile->add_att(name.c_str(), val.c_str())) {
    _addErrStr("ERROR - Nc3File::addGlobalAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrStr("  val: ", val);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add int global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, int val)
{
  if (!_ncFile->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add float global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, float val)
{
  if (!_ncFile->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add double global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, double val)
{
  if (!_ncFile->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add int[] global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, int len, int *vals)
{
  if (!_ncFile->add_att(name.c_str(), len, vals)) {
    _addErrStr("ERROR - Nc3File::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  n ints: ", len);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add float[] global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, int len, float *vals)
{
  if (!_ncFile->add_att(name.c_str(), len, vals)) {
    _addErrStr("ERROR - Nc3File::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  n floats: ", len);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add double[] global attribute
// Returns 0 on success, -1 on failure

int Nc3File::addGlobAttr(const string &name, int len, double *vals)
{
  if (!_ncFile->add_att(name.c_str(), len, vals)) {
    _addErrStr("ERROR - Nc3File::addGlobAttr");
    _addErrStr("  Cannot add global attr name: ", name);
    _addErrInt("  n doubles: ", len);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// read a global attribute
// Returns 0 on success, -1 on failure

int Nc3File::readGlobAttr(const string &name, string &val)
{
  NcAtt *att = _ncFile->get_att(name.c_str());
  if (att == NULL) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  val = asString(att);
  return 0;
}

int Nc3File::readGlobAttr(const string &name, int &val)
{
  NcAtt *att = _ncFile->get_att(name.c_str());
  if (att == NULL) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  string sval = asString(att);
  int ival;
  if (sscanf(sval.c_str(), "%d", &ival) != 1) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot interpret global attr as int");
    _addErrStr("  name: ", name);
    _addErrStr("  val: ", sval);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  val = ival;
  return 0;
}

int Nc3File::readGlobAttr(const string &name, float &val)
{
  NcAtt *att = _ncFile->get_att(name.c_str());
  if (att == NULL) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  string sval = asString(att);
  float fval;
  if (sscanf(sval.c_str(), "%g", &fval) != 1) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot interpret global attr as float");
    _addErrStr("  name: ", name);
    _addErrStr("  val: ", sval);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  val = fval;
  return 0;
}

int Nc3File::readGlobAttr(const string &name, double &val)
{
  NcAtt *att = _ncFile->get_att(name.c_str());
  if (att == NULL) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  string sval = asString(att);
  double dval;
  if (sscanf(sval.c_str(), "%lg", &dval) != 1) {
    _addErrStr("ERROR - Nc3File::readGlobAttr");
    _addErrStr("  Cannot interpret global attr as double");
    _addErrStr("  name: ", name);
    _addErrStr("  val: ", sval);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  val = dval;
  return 0;
}

///////////////////////////////////////////
// add string attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, const string &val)
{
  if (!var->add_att(name.c_str(), val.c_str())) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add string var attr, name: ", name);
    _addErrStr("  val: ", val);
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add double attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, double val)
{
  if (!var->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add double var attr, name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add float attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, float val)
{
  if (!var->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add float var attr, name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add int attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, int val)
{
  if (!var->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add int var attr, name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add long attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, long val)
{
  if (!var->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add long var attr, name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add short attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, short val)
{
  if (!var->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add short var attr, name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add ncbyte attribute
// Returns 0 on success, -1 on failure

int Nc3File::addAttr(NcVar *var, const string &name, ncbyte val)
{
  if (!var->add_att(name.c_str(), val)) {
    _addErrStr("ERROR - Nc3File::addAttr");
    _addErrStr("  Cannot add ncbyte var attr, name: ", name);
    _addErrInt("  val: ", val);
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add a dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is updated

int Nc3File::addDim(NcDim* &dim, const char *name, int size)
{
  if (size < 1) {
    dim = _ncFile->add_dim(name);
  } else {
    dim = _ncFile->add_dim(name, size);
  }
  if (dim == NULL) {
    _addErrStr("ERROR - Nc3File::addDim");
    _addErrStr("  Cannot add dimension: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// read a dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is set

int Nc3File::readDim(const string &name, NcDim* &dim)

{
  dim = _ncFile->get_dim(name.c_str());
  if (dim == NULL) {
    _addErrStr("ERROR - Nc3File::readDim");
    _addErrStr("  Cannot read dimension, name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////
// Add scalar  meta-data variable
// Returns 0 on success, -1 on failure
// Side effect: var is set

int Nc3File::addMetaVar(NcVar* &var,
                        const string &name, 
                        const string &standardName,
                        const string &longName,
                        NcType ncType, 
                        const string &units /* = "" */)
  
{
  
  var = addMetaVar(name, standardName, longName, ncType, units);
  if (var == NULL) {
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// Add scalar meta-data variable
// Returns var on success, NULL on failure

NcVar *Nc3File::addMetaVar(const string &name, 
                           const string &standardName,
                           const string &longName,
                           NcType ncType, 
                           const string &units /* = "" */)
  
{
  
  NcVar *var = _ncFile->add_var(name.c_str(), ncType);
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::addMetaVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", ncTypeToStr(ncType));
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return NULL;
  }
  
  if (standardName.length() > 0) {
    addAttr(var, "standard_name", standardName);
  }

  if (longName.length() > 0) {
    addAttr(var, "long_name", longName);
  }
  
  if (units.length() > 0 || ncType != ncChar) {
    addAttr(var, "units", units);
  }

  _setMetaFillvalue(var);

  return var;

}

///////////////////////////////////////
// Add 1-D array meta-data variable
// Returns 0 on success, -1 on failure
// Side effect: var is set

int Nc3File::addMetaVar(NcVar* &var, 
                        const string &name, 
                        const string &standardName,
                        const string &longName,
                        NcType ncType, 
                        NcDim *dim, 
                        const string &units /* = "" */)
{
  
  var = addMetaVar(name, standardName, longName, ncType, dim, units);
  if (var == NULL) {
    return -1;
  }

  return 0;

}

///////////////////////////////////////
// Add 1-D array  meta-data variable
// Returns var on success, NULL on failure

NcVar *Nc3File::addMetaVar(const string &name, 
                           const string &standardName,
                           const string &longName,
                           NcType ncType, 
                           NcDim *dim, 
                           const string &units /* = "" */)
{
  
  NcVar *var = _ncFile->add_var(name.c_str(), ncType, dim);
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::addMetaVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", ncTypeToStr(ncType));
    _addErrStr("  Dim: ", dim->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return NULL;
  }

  if (standardName.length() > 0) {
    addAttr(var, "standard_name", standardName);
  }

  if (longName.length() > 0) {
    addAttr(var, "long_name", longName);
  }

  if (units.length() > 0 || ncType != ncChar) {
    addAttr(var, "units", units);
  }
  
  _setMetaFillvalue(var);

  return var;

}

///////////////////////////////////////
// Add 2-D array  meta-data variable
// Returns 0 on success, -1 on failure
// Side effect: var is set

int Nc3File::addMetaVar(NcVar* &var, 
                        const string &name,
                        const string &standardName,
                        const string &longName,
                        NcType ncType,
                        NcDim *dim0,
                        NcDim *dim1,
                        const string &units /* = "" */)
{

  var = addMetaVar(name, standardName, longName, ncType, dim0, dim1, units);

  if (var == NULL) {
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////
// Add 2-D array  meta-data variable
// Returns var on success, NULL on failure

NcVar *Nc3File::addMetaVar(const string &name,
                           const string &standardName,
                           const string &longName,
                           NcType ncType,
                           NcDim *dim0,
                           NcDim *dim1,
                           const string &units /* = "" */)
{

  NcVar *var = _ncFile->add_var(name.c_str(), ncType, dim0, dim1);

  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::addMetaVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", ncTypeToStr(ncType));
    _addErrStr("  Dim0: ", dim0->name());
    _addErrStr("  Dim1: ", dim1->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return NULL;
  }
  
  if (standardName.length() > 0) {
    addAttr(var, "standard_name", standardName);
  }

  if (longName.length() > 0) {
    addAttr(var, "long_name", longName);
  }

  if (units.length() > 0 || ncType != ncChar) {
    addAttr(var, "units", units);
  }
  
  _setMetaFillvalue(var);

  return var;

}

/////////////////////////////////////
// read int variable, set var and val
// Returns 0 on success, -1 on failure

int Nc3File::readIntVar(NcVar* &var, const string &name,
                        int &val, int missingVal, bool required)
  
{
  
  var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - Nc3File::readIntVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      _addErrStr(_err->get_errmsg());
      return -1;
    }
  }

  // check size
  
  if (var->num_vals() < 1) {
    _addErrStr("ERROR - Nc3File::readIntVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  val = var->as_int(0);
  
  return 0;
  
}

///////////////////////////////////
// read int variable, set val

int Nc3File::readIntVal(const string &name, 
                        int &val, int missingVal,
                        bool required)
  
{
  
  val = missingVal;

  NcVar*var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - Nc3File::_readIntVal");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      _addErrStr(_err->get_errmsg());
    }
    return -1;
  }
  
  // check size
  
  if (var->num_vals() < 1) {
    if (required) {
      _addErrStr("ERROR - Nc3File::_readIntVal");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
      _addErrStr("  file: ", _pathInUse);
    }
    return -1;
  }

  val = var->as_int(0);
  
  return 0;
  
}

///////////////////////////////////
// read float variable
// Returns 0 on success, -1 on failure

int Nc3File::readFloatVar(NcVar* &var, const string &name,
                          float &val, 
                          float missingVal, bool required)

{
  
  var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - Nc3File::readFloatVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      _addErrStr(_err->get_errmsg());
      return -1;
    }
  }

  // check size
  
  if (var->num_vals() < 1) {
    _addErrStr("ERROR - Nc3File::readFloatVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  val = var->as_float(0);
  
  return 0;
  
}

///////////////////////////////////
// read float value

int Nc3File::readFloatVal(const string &name,
                          float &val,
                          float missingVal,
                          bool required)
  
{
  
  val = missingVal;
  
  NcVar* var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - Nc3File::readFloatVal");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_err->get_errmsg());
    }
    return -1;
  }
  
  // check size
  
  if (var->num_vals() < 1) {
    if (required) {
      _addErrStr("ERROR - Nc3File::readFloatVal");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
    }
    return -1;
  }

  val = var->as_float(0);
  
  return 0;
  
}

///////////////////////////////////
// read double variable
// Returns 0 on success, -1 on failure

int Nc3File::readDoubleVar(NcVar* &var, const string &name,
                           double &val, 
                           double missingVal, bool required)
  
{
  
  var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      val = missingVal;
      return 0;
    } else {
      _addErrStr("ERROR - Nc3File::readDoubleVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr("  file: ", _pathInUse);
      _addErrStr(_err->get_errmsg());
      return -1;
    }
  }

  // check size
  
  if (var->num_vals() < 1) {
    _addErrStr("ERROR - Nc3File::readDoubleVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no data");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  val = var->as_double(0);
  
  return 0;
  
}

///////////////////////////////////
// read double value

int Nc3File::readDoubleVal(const string &name,
                           double &val,
                           double missingVal,
                           bool required)

{
  
  val = missingVal;

  NcVar* var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - Nc3File::readDoubleVal");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_err->get_errmsg());
    }
    return -1;
  }

  // check size
  
  if (var->num_vals() < 1) {
    if (required) {
      _addErrStr("ERROR - Nc3File::readDoubleVal");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no data");
    }
    return -1;
  }

  val = var->as_double(0);
  
  return 0;
  
}

///////////////////////////////////
// read a scalar string variable
// Returns 0 on success, -1 on failure

int Nc3File::readStringVar(NcVar* &var, const string &name, string &val)

{

  // get var
  
  var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::readStringVar");
    _addErrStr("  Cannot read variable, name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }

  // check dimension
  
  if (var->num_dims() != 1) {
    _addErrStr("ERROR - Nc3File::readStringVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable does not have 1 dimension");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  NcDim *stringLenDim = var->get_dim(0);
  if (stringLenDim == NULL) {
    _addErrStr("ERROR - Nc3File::readStringVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has NULL 0th dimension");
    _addErrStr("  should be a string length dimension");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  NcType ntype = var->type();
  if (ntype != ncChar) {
    _addErrStr("ERROR - Nc3File::readStringVar");
    _addErrStr("  Incorrect variable type");
    _addErrStr("  expecting char");
    _addErrStr("  found: ", ncTypeToStr(ntype));
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  // load up data

  int stringLen = stringLenDim->size();
  char *cvalues = new char[stringLen+1];
  if (var->get(cvalues, stringLen)) {
    // ensure null termination
    cvalues[stringLen] = '\0';
    val = cvalues;
  } else {
    _addErrStr("ERROR - Nc3File::readStringVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }
  delete[] cvalues;

  return 0;

}


///////////////////////////////////////////////////////////////////////////
// write a scalar double variable
// Returns 0 on success, -1 on failure

int Nc3File::writeVar(NcVar *var, double val)
  
{
  
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  if (var->type() != ncDouble) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  var type should be double, name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  if (!var->put(&val, 1)) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  Cannot write scalar double var, name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////////////////////////////////
// write a scalar float variable
// Returns 0 on success, -1 on failure

int Nc3File::writeVar(NcVar *var, float val)
  
{
  
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  if (var->type() != ncFloat) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  var type should be float, name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  if (!var->put(&val, 1)) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  Cannot write scalar float var, name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////////////////////////////////
// write a scalar int variable
// Returns 0 on success, -1 on failure

int Nc3File::writeVar(NcVar *var, int val)
  
{
  
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  if (var->type() != ncInt) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  var type should be int, name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  if (!var->put(&val, 1)) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  Cannot write scalar int var, name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in dimension
// Returns 0 on success, -1 on failure

int Nc3File::writeVar(NcVar *var, const NcDim *dim, const void *data)
  
{
  return writeVar(var, dim, dim->size(), data);
}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in arguments
// Returns 0 on success, -1 on failure

int Nc3File::writeVar(NcVar *var, 
                      const NcDim *dim, size_t count, 
                      const void *data)
  
{
  
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::_writeVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  int iret = 0;
  
  switch (var->type()) {
    case ncDouble: {
      iret = !var->put((double *) data, count);
      break;
    }
    case ncFloat:
    default: {
      iret = !var->put((float *) data, count);
      break;
    }
    case ncInt: {
      iret = !var->put((int *) data, count);
      break;
    }
    case ncShort: {
      iret = !var->put((short *) data, count);
      break;
    }
    case ncByte: {
      iret = !var->put((ncbyte *) data, count);
      break;
    }
  } // switch

  if (iret) {
    _addErrStr("ERROR - Nc3File::writeVar");
    _addErrStr("  Cannot write var, name: ", var->name());
    _addErrStr("  Dim name: ", dim->name());
    _addErrInt("  Count: ", count);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  } else {
    return 0;
  }

}


///////////////////////////////////////////////////////////////////////////
// write a string variable
// Returns 0 on success, -1 on failure

int Nc3File::writeStringVar(NcVar *var, const void *str)
  
{
  
  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::writeStringVar");
    _addErrStr("  var is NULL");
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  
  int nDims = var->num_dims();
  if (nDims < 1) {
    _addErrStr("ERROR - Nc3File::writeStringVar");
    _addErrStr("  var has no dimensions");
    _addErrStr("  var name: ", var->name());
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  if (nDims == 1) {

    // single dimension

    NcDim *dim0 = var->get_dim(0);
    if (dim0 == NULL) {
      _addErrStr("ERROR - Nc3File::writeStringVar");
      _addErrStr("  Canont write var, name: ", var->name());
      _addErrStr("  dim 0 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    int iret = !var->put((char *) str, dim0->size());
  
    if (iret) {
      _addErrStr("ERROR - Nc3File::writeStringVar");
      _addErrStr("  Canont write var, name: ", var->name());
      _addErrStr(_err->get_errmsg());
      _addErrStr("  file: ", _pathInUse);
      return -1;
    } else {
      return 0;
    }

  }

  if (nDims == 2) {

    // two dimensions

    NcDim *dim0 = var->get_dim(0);
    if (dim0 == NULL) {
      _addErrStr("ERROR - Nc3File::writeStringVar");
      _addErrStr("  Canont write var, name: ", var->name());
      _addErrStr("  dim 0 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    NcDim *dim1 = var->get_dim(1);
    if (dim1 == NULL) {
      _addErrStr("ERROR - Nc3File::writeStringVar");
      _addErrStr("  Canont write var, name: ", var->name());
      _addErrStr("  dim 1 is NULL");
      _addErrStr("  file: ", _pathInUse);
      return -1;
    }

    int iret = !var->put((char *) str, dim0->size(), dim1->size());
    
    if (iret) {
      _addErrStr("ERROR - Nc3File::writeStringVar");
      _addErrStr("  Canont write var, name: ", var->name());
      _addErrInt("                    type: ", var->type());
      _addErrInt("                    is_valid: ", var->is_valid());
      _addErrStr("  file: ", _pathInUse);
      _addErrStr(_err->get_errmsg());
      return -1;
    } else {
      return 0;
    }

  }

  // more than 2 is an error
  
  _addErrStr("ERROR - Nc3File::writeStringVar");
  _addErrStr("  Canont write var, name: ", var->name());
  _addErrInt("  more than 2 dimensions: ", nDims);
  _addErrStr("  file: ", _pathInUse);
  return -1;

}

///////////////////////////////////////////////////////////////////////////
// compress a variable

int Nc3File::compressVar(NcVar *var, int compressionLevel)
{
  
  if (_ncFormat == NcFile::Netcdf4Classic ||
      _ncFormat == NcFile::Offset64Bits) {
    // cannot compress
    return 0;
  }

  if (var == NULL) {
    _addErrStr("ERROR - Nc3File::setVarCompression");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  int fileId = _ncFile->id();
  int varId = var->id();
  int shuffle = 0;
  
  if (nc_def_var_deflate(fileId, varId, shuffle,
                         true, compressionLevel)!= NC_NOERR) {
    _addErrStr("ERROR: FieldData::setCompression");
    _addErrStr("  Problem setting compression for var: ",
               var->name());
    _addErrStr("  file: ", _pathInUse);
    _addErrStr(_err->get_errmsg());
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// convert type enum to strings

string Nc3File::ncTypeToStr(NcType nctype)
  
{
  
  switch (nctype) {
    case ncDouble:
      return "ncDouble";
    case ncFloat:
      return "ncFloat";
    case ncInt:
      return "ncInt";
    case ncShort:
      return "ncShort";
    case ncByte:
    default:
      return "ncByte";
  }
  
}

///////////////////////////////////////////
// get string representation of component

string Nc3File::asString(const NcTypedComponent *component,
                         int index /* = 0 */)
  
{
  
  const char* strc = component->as_string(index);
  string strs(strc);
  delete[] strc;
  return strs;

}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void Nc3File::_addErrInt(string label, int iarg, bool cr)
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

void Nc3File::_addErrDbl(string label, double darg,
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

void Nc3File::_addErrStr(string label, string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// set fill value for metadata

void Nc3File::_setMetaFillvalue(NcVar *var)

{

  NcType vtype = var->type();
  if (vtype == ncDouble) {
    addAttr(var, "_FillValue", Ncxx::missingMetaDouble);
    return;
  }
  if (vtype == ncFloat) {
    addAttr(var, "_FillValue", Ncxx::missingMetaFloat);
    return;
  }
  if (vtype == ncInt) {
    addAttr(var, "_FillValue", Ncxx::missingMetaInt);
    return;
  }
  if (vtype == ncLong) {
    addAttr(var, "_FillValue", (long) Ncxx::missingMetaInt);
    return;
  }
  if (vtype == ncShort) {
    addAttr(var, "_FillValue", (short) Ncxx::missingMetaInt);
    return;
  }
  if (vtype == ncByte) {
    addAttr(var, "_FillValue", (ncbyte) Ncxx::missingMetaChar);
    return;
  }
  // no fill
  return;
}
