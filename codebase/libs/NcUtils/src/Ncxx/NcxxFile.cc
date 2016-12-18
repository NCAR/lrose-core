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
}

// Constructor generates a null object.
NcxxFile::NcxxFile() :
    NcxxGroup()  // invoke base class constructor
{}

// constructor
NcxxFile::NcxxFile(const string& filePath, const FileMode fMode)
{
  open(filePath, fMode);
}

// open a file from path and mode
void NcxxFile::open(const string& filePath, const FileMode fMode)
{
  if (!nullObject)
    close();

  switch (fMode)
    {
    case NcxxFile::write:
      ncxxCheck(nc_open(filePath.c_str(), NC_WRITE, &myId),__FILE__,__LINE__);
      break;
    case NcxxFile::read:
      ncxxCheck(nc_open(filePath.c_str(), NC_NOWRITE, &myId),__FILE__,__LINE__);
      break;
    case NcxxFile::newFile:
      ncxxCheck(nc_create(filePath.c_str(), NC_NETCDF4 | NC_NOCLOBBER, &myId),__FILE__,__LINE__);
      break;
    case NcxxFile::replace:
      ncxxCheck(nc_create(filePath.c_str(), NC_NETCDF4 | NC_CLOBBER, &myId),__FILE__,__LINE__);
      break;
    }

  g_ncid = myId;

  nullObject=false;
}

// constructor with file type specified
NcxxFile::NcxxFile(const string& filePath, const FileMode fMode, const FileFormat fFormat )
{
  open(filePath, fMode, fFormat);
}

void NcxxFile::open(const string& filePath, const FileMode fMode, const FileFormat fFormat )
{
  if (!nullObject)
    close();

  int format;
  switch (fFormat)
    {
    case NcxxFile::classic:
	format = 0;
	break;
    case NcxxFile::classic64:
	format = NC_64BIT_OFFSET;
	break;
    case NcxxFile::nc4:
	format = NC_NETCDF4;
	break;
    case NcxxFile::nc4classic:
	format = NC_NETCDF4 | NC_CLASSIC_MODEL;
	break;
    }
  switch (fMode)
    {
    case NcxxFile::write:
      ncxxCheck(nc_open(filePath.c_str(), format | NC_WRITE, &myId),__FILE__,__LINE__);
      break;
    case NcxxFile::read:
      ncxxCheck(nc_open(filePath.c_str(), format | NC_NOWRITE, &myId),__FILE__,__LINE__);
      break;
    case NcxxFile::newFile:
      ncxxCheck(nc_create(filePath.c_str(), format | NC_NOCLOBBER, &myId),__FILE__,__LINE__);
      break;
    case NcxxFile::replace:
      ncxxCheck(nc_create(filePath.c_str(), format | NC_CLOBBER, &myId),__FILE__,__LINE__);
      break;
    }

  g_ncid = myId;
  nullObject=false;
}

// Synchronize an open netcdf dataset to disk
void NcxxFile::sync(){
  ncxxCheck(nc_sync(myId),__FILE__,__LINE__);
}

// Leave define mode, used for classic model
void NcxxFile::enddef() {
    ncxxCheck(nc_enddef(myId),__FILE__,__LINE__);
}
