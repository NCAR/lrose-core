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

#include <cstring>
#include <netcdf.h>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxException.hh>
using namespace std;

////////////////////////
//  C++ API for netCDF4.

// function checks error code and if necessary throws appropriate exception.
// \param retCode Integer value returned by %netCDF C-routines.
// \param file    The name of the file from which this call originates.
// \param line    The line number in the file from which this call originates.
// \param context Optional extra context from the calling routine stack, to make
//                the exception message more meaningful

void ncxxCheck(int retCode, 
               string file, 
               int line, 
               std::string context1 /* = "" */,
               std::string context2 /* = "" */,
               std::string context3 /* = "" */)
  
{

  if (retCode == NC_NOERR) {
    return;
  }
  
  const char* msg = 0;
  if (NC_ISSYSERR(retCode)){
    msg = std::strerror(retCode);
    msg = msg ? msg : "Unknown system error";
  }else{
    msg = nc_strerror(retCode);
  }

  string complaint;
  if (context1.size() > 0) {
    complaint += context1;
    complaint += ", ";
  }
  if (context2.size() > 0) {
    complaint += context2;
    complaint += ", ";
  }
  if (context3.size() > 0) {
    complaint += context3;
    complaint += ", ";
  }
  complaint += msg;

  switch(retCode) {
    case NC_EBADID          : throw NcxxBadId(complaint, file, line);
    case NC_ENFILE          : throw NcxxNFile(complaint, file, line);
    case NC_EEXIST          : throw NcxxExist(complaint, file, line);
    case NC_EINVAL          : throw NcxxInvalidArg(complaint, file, line);
    case NC_EPERM           : throw NcxxInvalidWrite(complaint, file, line);
    case NC_ENOTINDEFINE    : throw NcxxNotInDefineMode(complaint, file, line);
    case NC_EINDEFINE       : throw NcxxInDefineMode(complaint, file, line);
    case NC_EINVALCOORDS    : throw NcxxInvalidCoords(complaint, file, line);
    case NC_EMAXDIMS        : throw NcxxMaxDims(complaint, file, line);
    case NC_ENAMEINUSE      : throw NcxxNameInUse(complaint, file, line);
    case NC_ENOTATT         : throw NcxxNotAtt(complaint, file, line);
    case NC_EMAXATTS        : throw NcxxMaxAtts(complaint, file, line);
    case NC_EBADTYPE        : throw NcxxBadType(complaint, file, line);
    case NC_EBADDIM         : throw NcxxBadDim(complaint, file, line);
    case NC_EUNLIMPOS       : throw NcxxUnlimPos(complaint, file, line);
    case NC_EMAXVARS        : throw NcxxMaxVars(complaint, file, line);
    case NC_ENOTVAR         : throw NcxxNotVar(complaint, file, line);
    case NC_EGLOBAL         : throw NcxxGlobal(complaint, file, line);
    case NC_ENOTNC          : throw NcxxNotNCF(complaint, file, line);
    case NC_ESTS            : throw NcxxSts(complaint, file, line);
    case NC_EMAXNAME        : throw NcxxMaxName(complaint, file, line);
    case NC_EUNLIMIT        : throw NcxxUnlimit(complaint, file, line);
    case NC_ENORECVARS      : throw NcxxNoRecVars(complaint, file, line);
    case NC_ECHAR           : throw NcxxCharConvert(complaint, file, line);
    case NC_EEDGE           : throw NcxxEdge(complaint, file, line);
    case NC_ESTRIDE         : throw NcxxStride(complaint, file, line);
    case NC_EBADNAME        : throw NcxxBadName(complaint, file, line);
    case NC_ERANGE          : throw NcxxRange(complaint, file, line);
    case NC_ENOMEM          : throw NcxxNoMem(complaint, file, line);
    case NC_EVARSIZE        : throw NcxxVarSize(complaint, file, line);
    case NC_EDIMSIZE        : throw NcxxDimSize(complaint, file, line);
    case NC_ETRUNC          : throw NcxxTrunc(complaint, file, line);

      // The following are specific netCDF4 errors.
    case NC_EHDFERR         : throw NcxxHdfErr(complaint, file, line);
    case NC_ECANTREAD       : throw NcxxCantRead(complaint, file, line);
    case NC_ECANTWRITE      : throw NcxxCantWrite(complaint, file, line);
    case NC_ECANTCREATE     : throw NcxxCantCreate(complaint, file, line);
    case NC_EFILEMETA       : throw NcxxFileMeta(complaint, file, line);
    case NC_EDIMMETA        : throw NcxxDimMeta(complaint, file, line);
    case NC_EATTMETA        : throw NcxxAttMeta(complaint, file, line);
    case NC_EVARMETA        : throw NcxxVarMeta(complaint, file, line);
    case NC_ENOCOMPOUND     : throw NcxxNoCompound(complaint, file, line);
    case NC_EATTEXISTS      : throw NcxxAttExists(complaint, file, line);
    case NC_ENOTNC4         : throw NcxxNotNc4(complaint, file, line);
    case NC_ESTRICTNC3      : throw NcxxStrictNc3(complaint, file, line);
    case NC_EBADGRPID       : throw NcxxBadGroupId(complaint, file, line);

      // netcdf.h file inconsistent with documentation!!
    case NC_EBADTYPID       : throw NcxxBadTypeId(complaint, file, line);

      // netcdf.h file inconsistent with documentation!!
    case NC_EBADFIELD       : throw NcxxBadFieldId(complaint, file, line);

      // netcdf.h file inconsistent with documentation!!
      //  case NC_EUNKNAME        : throw NcUnkownName("Cannot find the field id.",file,line);

    case NC_ENOGRP          : throw NcxxEnoGrp(complaint, file, line);
    case NC_ELATEDEF        : throw NcxxElateDef(complaint, file, line);

    default:
      throw NcxxException(retCode, complaint, file, line);
  }
}

// Function checks if the file (group) is in define mode.
// If not, it places it in the define mode.
// While this is automatically done by the underlying C API
// for netCDF-4 files, the netCDF-3 files still need this call.

void ncxxCheckDefineMode(int ncid, string context /* = "" */)
{
  int status = nc_redef(ncid);
  if (status != NC_EINDEFINE) ncxxCheck(status, __FILE__, __LINE__);
}

// Function checks if the file (group) is in data mode.
// If not, it places it in the data mode.
// While this is automatically done by the underlying C API
// for netCDF-4 files, the netCDF-3 files still need this call.

void ncxxCheckDataMode(int ncid, string context /* = "" */)
{
  int status = nc_enddef(ncid);
  if (status != NC_ENOTINDEFINE) ncxxCheck(status, __FILE__, __LINE__);
}
