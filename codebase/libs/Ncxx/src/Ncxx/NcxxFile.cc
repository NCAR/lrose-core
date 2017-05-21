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
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//  Additional methods have been added to return error conditions. 
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxDim.hh>
#include <Ncxx/NcxxVar.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxException.hh>
#include <Ncxx/NcxxByte.hh>
#include <Ncxx/NcxxInt.hh>
#include <Ncxx/NcxxFloat.hh>
#include<iostream>
#include<string>
#include<sstream>
using namespace std;

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
  int ncFormat = 0;
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

