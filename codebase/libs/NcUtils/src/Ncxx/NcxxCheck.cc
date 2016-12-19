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
#include <ncException.h>
using namespace std;
using namespace netCDF::exceptions;

//  C++ API for netCDF4.
namespace netCDF
{
  // function checks error code and if necessary throws appropriate exception.
  void ncxxCheck(int retCode, const char* file, int line){
    if (retCode==NC_NOERR)
      return;

    const char* msg = 0;
    if (NC_ISSYSERR(retCode)){
      msg = std::strerror(retCode);
      msg = msg ? msg : "Unknown system error";
    }else{
      msg = nc_strerror(retCode);
    }

    switch(retCode) {
    case NC_EBADID          : throw NcBadId(msg,file,line);
    case NC_ENFILE          : throw NcNFile(msg,file,line);
    case NC_EEXIST          : throw NcExist(msg,file,line);
    case NC_EINVAL          : throw NcInvalidArg(msg,file,line);
    case NC_EPERM           : throw NcInvalidWrite(msg,file,line);
    case NC_ENOTINDEFINE    : throw NcNotInDefineMode(msg,file,line);
    case NC_EINDEFINE       : throw NcInDefineMode(msg,file,line);
    case NC_EINVALCOORDS    : throw NcInvalidCoords(msg,file,line);
    case NC_EMAXDIMS        : throw NcMaxDims(msg,file,line);
    case NC_ENAMEINUSE      : throw NcNameInUse(msg,file,line);
    case NC_ENOTATT         : throw NcNotAtt(msg,file,line);
    case NC_EMAXATTS        : throw NcMaxAtts(msg,file,line);
    case NC_EBADTYPE        : throw NcBadType(msg,file,line);
    case NC_EBADDIM         : throw NcBadDim(msg,file,line);
    case NC_EUNLIMPOS       : throw NcUnlimPos(msg,file,line);
    case NC_EMAXVARS        : throw NcMaxVars(msg,file,line);
    case NC_ENOTVAR         : throw NcNotVar(msg,file,line);
    case NC_EGLOBAL         : throw NcGlobal(msg,file,line);
    case NC_ENOTNC          : throw NcNotNCF(msg,file,line);
    case NC_ESTS            : throw NcSts(msg,file,line);
    case NC_EMAXNAME        : throw NcMaxName(msg,file,line);
    case NC_EUNLIMIT        : throw NcUnlimit(msg,file,line);
    case NC_ENORECVARS      : throw NcNoRecVars(msg,file,line);
    case NC_ECHAR           : throw NcChar(msg,file,line);
    case NC_EEDGE           : throw NcEdge(msg,file,line);
    case NC_ESTRIDE         : throw NcStride(msg,file,line);
    case NC_EBADNAME        : throw NcBadName(msg,file,line);
    case NC_ERANGE          : throw NcRange(msg,file,line);
    case NC_ENOMEM          : throw NcNoMem(msg,file,line);
    case NC_EVARSIZE        : throw NcVarSize(msg,file,line);
    case NC_EDIMSIZE        : throw NcDimSize(msg,file,line);
    case NC_ETRUNC          : throw NcTrunc(msg,file,line);

      // The following are specific netCDF4 errors.
    case NC_EHDFERR         : throw NcHdfErr(msg,file,line);
    case NC_ECANTREAD       : throw NcCantRead(msg,file,line);
    case NC_ECANTWRITE      : throw NcCantWrite(msg,file,line);
    case NC_ECANTCREATE     : throw NcCantCreate(msg,file,line);
    case NC_EFILEMETA       : throw NcFileMeta(msg,file,line);
    case NC_EDIMMETA        : throw NcDimMeta(msg,file,line);
    case NC_EATTMETA        : throw NcAttMeta(msg,file,line);
    case NC_EVARMETA        : throw NcVarMeta(msg,file,line);
    case NC_ENOCOMPOUND     : throw NcNoCompound(msg,file,line);
    case NC_EATTEXISTS      : throw NcAttExists(msg,file,line);
    case NC_ENOTNC4         : throw NcNotNc4(msg,file,line);
    case NC_ESTRICTNC3      : throw NcStrictNc3(msg,file,line);
    case NC_EBADGRPID       : throw NcBadGroupId(msg,file,line);
    case NC_EBADTYPID       : throw NcBadTypeId(msg,file,line);                       // netcdf.h file inconsistent with documentation!!
    case NC_EBADFIELD       : throw NcBadFieldId(msg,file,line);                     // netcdf.h file inconsistent with documentation!!
      //  case NC_EUNKNAME        : throw NcUnkownName("Cannot find the field id.",file,line);   // netcdf.h file inconsistent with documentation!!

    case NC_ENOGRP          : throw NcEnoGrp(msg,file,line);
    case NC_ELATEDEF        : throw NcElateDef(msg,file,line);

    default:
      throw NcException(retCode, msg, file, line);
    }
  }

  void ncxxCheckDefineMode(int ncid)
  {
    int status = nc_redef(ncid);
    if (status != NC_EINDEFINE) ncxxCheck(status, __FILE__, __LINE__);
  }

  void ncxxCheckDataMode(int ncid)
  {
    int status = nc_enddef(ncid);
    if (status != NC_ENOTINDEFINE) ncxxCheck(status, __FILE__, __LINE__);
  }
}
