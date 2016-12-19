#include <NcUtils/Ncxx.hh>
#include <NcUtils/NcxxFile.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxDim.hh>
#include <NcUtils/NcxxVar.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxException.hh>
#include <NcUtils/NcxxByte.hh>
#include <NcUtils/NcxxInt.hh>
#include <NcUtils/NcxxFloat.hh>
#include<iostream>
#include<string>
#include<sstream>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

int g_ncid = -1;

// destructor

NcxxFile::~NcxxFile()
{
  // destructor may be called due to an exception being thrown
  // hence throwing an exception from within a destructor
  // causes undefined behaviour! so just printing a warning message
  try
  {
    close();
  }
  catch (NcxxException &e)
  {
    cerr << e.what() << endl;
  }
}

void NcxxFile::close()
{
  if (!nullObject) {
    ncxxCheck(nc_close(myId),__FILE__,__LINE__);
    g_ncid = -1;
  }
  nullObject = true;
  _pathInUse.clear();
  _errStr.clear();
  _format = nc4;
  _mode = read;
}

// Default constructor generates a null object.

NcxxFile::NcxxFile() :
        NcxxGroup()  // invoke base class constructor
{
  close();
}

// constructor

NcxxFile::NcxxFile(const string& filePath, const FileMode fMode) :
        NcxxGroup()
{
  open(filePath, fMode);
}

// open a file from path and mode

void NcxxFile::open(const string& filePath, const FileMode fMode)
{

  if (!nullObject) {
    close();
  }

  switch (fMode) {
    case NcxxFile::write:
      _mode = write;
      ncxxCheck(nc_open(filePath.c_str(), NC_WRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::read:
      _mode = read;
      ncxxCheck(nc_open(filePath.c_str(), NC_NOWRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::newFile:
      _mode = newFile;
      ncxxCheck(nc_create(filePath.c_str(),
                          NC_NETCDF4 | NC_NOCLOBBER, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::replace:
      _mode = replace;
      ncxxCheck(nc_create(filePath.c_str(),
                          NC_NETCDF4 | NC_CLOBBER, &myId),
                __FILE__, __LINE__);
      break;
  }

  _pathInUse = filePath;
  g_ncid = myId;
  nullObject=false;

}

// constructor with file type specified

NcxxFile::NcxxFile(const string& filePath, 
                   const FileMode fMode,
                   const FileFormat fFormat )
{
  open(filePath, fMode, fFormat);
}

void NcxxFile::open(const string& filePath,
                    const FileMode fMode,
                    const FileFormat fFormat )
{

  if (!nullObject) {
    close();
  }

  _format = fFormat;
  int ncFormat;
  switch (fFormat)
  {
    case NcxxFile::classic:
      ncFormat = 0;
      break;
    case NcxxFile::classic64:
      ncFormat = NC_64BIT_OFFSET;
      break;
    case NcxxFile::nc4:
      ncFormat = NC_NETCDF4;
      break;
    case NcxxFile::nc4classic:
      ncFormat = NC_NETCDF4 | NC_CLASSIC_MODEL;
      break;
  }
  switch (fMode)
  {
    case NcxxFile::write:
      _mode = write;
      ncxxCheck(nc_open(filePath.c_str(), ncFormat | NC_WRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::read:
      _mode = read;
      ncxxCheck(nc_open(filePath.c_str(), ncFormat | NC_NOWRITE, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::newFile:
      _mode = newFile;
      ncxxCheck(nc_create(filePath.c_str(), ncFormat | NC_NOCLOBBER, &myId),
                __FILE__, __LINE__);
      break;
    case NcxxFile::replace:
      _mode = replace;
      ncxxCheck(nc_create(filePath.c_str(), ncFormat | NC_CLOBBER, &myId),
                __FILE__, __LINE__);
      break;
  }

  _pathInUse = filePath;
  g_ncid = myId;
  nullObject=false;

}

//////////////////////////////////////
// open file for reading
// Returns 0 on success, -1 on failure

int NcxxFile::openRead(const string &path)
  
{

  close();
  clearErrStr();

  try {
    open(path, read, nc4);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::openRead");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  File is not NetCDF");
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
/// open file for writing
/// set the netcdf format, before a write
/// format options are:
///   classic - classic format (i.e. version 1 format)
///   classic64 - 64-bit offset format
///   nc4 - using HDF5 format
///   nc4classic - netCDF-4 using HDF5 but only netCDF-3 calls
/// Returns 0 on success, -1 on failure

int NcxxFile::openWrite(const string &path,
                        NcxxFile::FileFormat format) 

{
  
  close();
  clearErrStr();
  
  try {
    open(path, write, format);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::openWrite");
    _addErrStr("  Cannot open file for writing: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////
// Synchronize an open netcdf dataset to disk

void NcxxFile::sync()
{
  ncxxCheck(nc_sync(myId), __FILE__, __LINE__);
}

//////////////////////////////////////////////
// Leave define mode, used for classic model

void NcxxFile::enddef()
{
  ncxxCheck(nc_enddef(myId), __FILE__, __LINE__);
}

///////////////////////////////////////////
// add string global attribute
// Returns 0 on success, -1 on failure

int NcxxFile::addGlobAttr(const string &name, const string &val)
{
  try {
    putAtt(name, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::addGlobalAttr");
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

int NcxxFile::addGlobAttr(const string &name, int val)
{
  NcxxInt xtype;
  try {
    putAtt(name, xtype, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::addGlobAttr");
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

int NcxxFile::addGlobAttr(const string &name, float val)
{
  NcxxFloat xtype;
  try {
    putAtt(name, xtype, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::addGlobAttr");
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

int NcxxFile::readGlobAttr(const string &name, string &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  try {
    att.getValues(val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  exception: ", e.what());
    _addErrStr("  Cannot read value as string");
    return -1;
  }
  return 0;
}

int NcxxFile::readGlobAttr(const string &name, int &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  int *vals = new int[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
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

int NcxxFile::readGlobAttr(const string &name, float &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  float *vals = new float[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
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

int NcxxFile::readGlobAttr(const string &name, double &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
    _addErrStr("  Cannot read global attr name: ", name);
    _addErrStr("  file: ", _pathInUse);
    _addErrStr("  no values supplied");
    return -1;
  }
  double *vals = new double[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxFile::readGlobAttr");
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
// add a dimension
// Returns 0 on success, -1 on failure
// Side effect: dim arg is updated

int NcxxFile::addDim(NcxxDim &dim, const string &name, int size)
{
  if (size < 1) {
    dim = NcxxGroup::addDim(name);
  } else {
    dim = NcxxGroup::addDim(name, size);
  }
  if (dim.isNull()) {
    _addErrStr("ERROR - NcxxFile::addDim");
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

int NcxxFile::readDim(const string &name, NcxxDim &dim)
  
{
  dim = getDim(name);
  if (dim.isNull()) {
    _addErrStr("ERROR - NcxxFile::readDim");
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

int NcxxFile::addVar(NcxxVar &var,
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

int NcxxFile::addVar(NcxxVar &var, 
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

int NcxxFile::addVar(NcxxVar &var, 
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

int NcxxFile::addVar(NcxxVar &var, 
                     const string &name,
                     const string &standardName,
                     const string &longName,
                     NcxxType ncType,
                     vector<NcxxDim> &dims,
                     const string &units /* = "" */)
{

  var = NcxxGroup::addVar(name, ncType, dims);
  nc_type vtype = ncType.getId();
  if (var.isNull()) {
    _addErrStr("ERROR - NcxxFile::addVar");
    _addErrStr("  Cannot add var, name: ", name);
    _addErrStr("  Type: ", Ncxx::ncTypeToStr(vtype));
    _addErrStr("  file: ", _pathInUse);
    return -1;
  }

  if (standardName.length() > 0) {
    var.addAttr("standard_name", standardName);
  }
  
  if (longName.length() > 0) {
    var.addAttr("long_name", longName);
  }

  if (units.length() > 0) {
    var.addAttr("units", units);
  }
  
  var.setDefaultFillvalue();

  return 0;

}

