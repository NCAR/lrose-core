#include <NcUtils/NcxxFile.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxException.hh>
#include <NcUtils/NcxxByte.hh>
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

// Synchronize an open netcdf dataset to disk

void NcxxFile::sync()
{
  ncxxCheck(nc_sync(myId), __FILE__, __LINE__);
}

// Leave define mode, used for classic model

void NcxxFile::enddef()
{
  ncxxCheck(nc_enddef(myId), __FILE__, __LINE__);
}
