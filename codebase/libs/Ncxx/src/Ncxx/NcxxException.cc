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

#include <Ncxx/NcxxException.hh>
#include <sstream>
#include <netcdf.h>
using namespace std;

// Default object thrown if a netCDF exception is encountered.

NcxxException::NcxxException(string complaint,
                             string fileName,
                             int lineNumber):
        _ec(0)
{
  try {
    std::ostringstream oss;
    oss << lineNumber;
    _whatMsg = complaint + "\nfile: " + fileName + "  line:" + oss.str();
  } catch(...) {
    _whatMsg.clear();
  }
}

NcxxException::NcxxException(int errorCode,
                             string complaint,
                             string fileName,
                             int lineNumber) :
        _ec(errorCode)
{
  try {
    std::ostringstream oss;
    oss << lineNumber;
    _whatMsg = complaint + "\nfile: " + fileName + "  line:" + oss.str();
  } catch(...) {
    _whatMsg.clear();
  }
}

NcxxException::NcxxException(const NcxxException& e) throw() :
        _ec(e._ec)
{
  try {
    _whatMsg = e._whatMsg;
  } catch(...) {
    _whatMsg.clear();
  }
}

NcxxException& NcxxException::operator=(const NcxxException& e) throw()
{
  if (this != &e){
    _ec = e._ec;
    _whatMsg.clear();
    try {
      _whatMsg = e._whatMsg;
    } catch(...) {
      _whatMsg.clear();
    }
  }
  return *this;
}

NcxxException::~NcxxException()throw() {
  _whatMsg.clear();
}


const char *NcxxException::what() const throw() {
  return _whatMsg.c_str();
}

const string NcxxException::whatStr() const throw() {
  return _whatMsg;
}

int NcxxException::errorCode() const throw() {
  return _ec;
}

// Thrown if the specified netCDF ID does not refer to an open netCDF dataset. 
NcxxBadId::NcxxBadId(string complaint,string file,int line) :
  NcxxException(NC_EBADID,complaint,file,line) { }


// Thrown if too many netcdf files are open.
NcxxNFile::NcxxNFile(string complaint,string file,int line) :
  NcxxException(NC_ENFILE,complaint,file,line) { }

// Thrown if, having set NC_NOCLOBBER, the specified dataset already exists. 
NcxxExist::NcxxExist(string complaint,string file,int line) :
  NcxxException(NC_EEXIST,complaint,file,line) { }

// Thrown if not a netCDF id.
NcxxInvalidArg::NcxxInvalidArg(string complaint,string file,int line) :
  NcxxException(NC_EINVAL,complaint,file,line) { }

// Thrown if invalid argument.
NcxxInvalidWrite::NcxxInvalidWrite(string complaint,string file,int line) :
  NcxxException(NC_EPERM,complaint,file,line) { }

// Thrown if operation not allowed in data mode.
NcxxNotInDefineMode::NcxxNotInDefineMode(string complaint,string file,int line) :
  NcxxException(NC_ENOTINDEFINE,complaint,file,line) { }

// Thrown if operation not allowed in defined mode.
NcxxInDefineMode::NcxxInDefineMode(string complaint,string file,int line) :
  NcxxException(NC_EINDEFINE,complaint,file,line) { }

// Index exceeds dimension bound
NcxxInvalidCoords::NcxxInvalidCoords(string complaint,string file,int line) :
  NcxxException(NC_EINVALCOORDS,complaint,file,line) { }

// Thrown if NC_MAX_DIMS is exceeded.
NcxxMaxDims::NcxxMaxDims(string complaint,string file,int line) :
  NcxxException(NC_EMAXDIMS,complaint,file,line) { }

// Thrown if string match to name is in use.
NcxxNameInUse::NcxxNameInUse(string complaint,string file,int line) :
  NcxxException(NC_ENAMEINUSE,complaint,file,line) { }

// Thrown if attribute is not found.
NcxxNotAtt::NcxxNotAtt(string complaint,string file,int line) :
  NcxxException(NC_ENOTATT,complaint,file,line) { }

// Thrown if Ncxx_MAX_ATTRS is exceeded.
NcxxMaxAtts::NcxxMaxAtts(string complaint,string file,int line) :
  NcxxException(NC_EMAXATTS,complaint,file,line) { }

// Thrown if not a valid netCDF data type.
NcxxBadType::NcxxBadType(string complaint,string file,int line) :
  NcxxException(NC_EBADTYPE,complaint,file,line) { }

// Thrown if an invalid dimension id or name.
NcxxBadDim::NcxxBadDim(string complaint,string file,int line) :
  NcxxException(NC_EBADDIM,complaint,file,line) { }

// Thrown if Nc_UNLIMITED is in the wrong index.
NcxxUnlimPos::NcxxUnlimPos(string complaint,string file,int line) :
  NcxxException(NC_EUNLIMPOS,complaint,file,line) { }

// Thrown if NC_MAX_VARS is exceeded.
NcxxMaxVars::NcxxMaxVars(string complaint,string file,int line) :
  NcxxException(NC_EMAXVARS,complaint,file,line) { }

// Thrown if variable is not found.
NcxxNotVar::NcxxNotVar(string complaint,string file,int line) :
  NcxxException(NC_ENOTVAR,complaint,file,line) { }

// Thrown if the action is prohibited on the NC_GLOBAL varid.
NcxxGlobal::NcxxGlobal(string complaint,string file,int line) :
  NcxxException(NC_EGLOBAL,complaint,file,line) { }

// Thrown if not a netCDF file.
NcxxNotNCF::NcxxNotNCF(string complaint,string file,int line) :
  NcxxException(NC_ENOTNC,complaint,file,line) { }

// Thrown if in FORTRAN, string is too short.
NcxxSts::NcxxSts(string complaint,string file,int line) :
  NcxxException(NC_ESTS,complaint,file,line) { }

// Thrown if NC_MAX_NAME is exceeded.
NcxxMaxName::NcxxMaxName(string complaint,string file,int line) :
  NcxxException(NC_EMAXNAME,complaint,file,line) { }

// Thrown if NC_UNLIMITED size is already in use.
NcxxUnlimit::NcxxUnlimit(string complaint,string file,int line) :
  NcxxException(NC_EUNLIMIT,complaint,file,line) { }

// Thrown if nc_rec op when there are no record vars.
NcxxNoRecVars::NcxxNoRecVars(string complaint,string file,int line) :
  NcxxException(NC_ENORECVARS,complaint,file,line) { }

// Thrown if attempt to convert between text and numbers.
NcxxCharConvert::NcxxCharConvert(string complaint,string file,int line) :
  NcxxException(NC_ECHAR,complaint,file,line) { }

// Thrown if edge+start exceeds dimension bound.
NcxxEdge::NcxxEdge(string complaint,string file,int line) :
  NcxxException(NC_EEDGE,complaint,file,line) { }

// Thrown if illegal stride.
NcxxStride::NcxxStride(string complaint,string file,int line) :
  NcxxException(NC_ESTRIDE,complaint,file,line) { }

// Thrown if attribute or variable name contains illegal characters.
NcxxBadName::NcxxBadName(string complaint,string file,int line) :
  NcxxException(NC_EBADNAME,complaint,file,line) { }

// Thrown if math result not representable.
NcxxRange::NcxxRange(string complaint,string file,int line) :
  NcxxException(NC_ERANGE,complaint,file,line) { }

// Thrown if memory allocation (malloc) failure.
NcxxNoMem::NcxxNoMem(string complaint,string file,int line) :
  NcxxException(NC_ENOMEM,complaint,file,line) { }

// Thrown if one or more variable sizes violate format constraints
NcxxVarSize::NcxxVarSize(string complaint,string file,int line) :
  NcxxException(NC_EVARSIZE,complaint,file,line) { }

// Thrown if invalid dimension size.
NcxxDimSize::NcxxDimSize(string complaint,string file,int line) :
  NcxxException(NC_EDIMSIZE,complaint,file,line) { }

// Thrown if file likely truncated or possibly corrupted.
NcxxTrunc::NcxxTrunc(string complaint,string file,int line) :
  NcxxException(NC_ETRUNC,complaint,file,line) { }

// Thrown if an error was reported by the HDF5 layer.
NcxxHdfErr::NcxxHdfErr(string complaint,string file,int line) :
  NcxxException(NC_EHDFERR,complaint,file,line) { }

// Thrown if cannot read.
NcxxCantRead::NcxxCantRead(string complaint,string file,int line) :
  NcxxException(NC_ECANTREAD,complaint,file,line) { }

// Thrown if cannot write.
NcxxCantWrite::NcxxCantWrite(string complaint,string file,int line) :
  NcxxException(NC_ECANTWRITE,complaint,file,line) { }

// Thrown if cannot create.
NcxxCantCreate::NcxxCantCreate(string complaint,string file,int line) :
  NcxxException(NC_ECANTCREATE,complaint,file,line) { }

// Thrown if file meta.
NcxxFileMeta::NcxxFileMeta(string complaint,string file,int line) :
  NcxxException(NC_EFILEMETA,complaint,file,line) { }

// Thrown if dim meta.
NcxxDimMeta::NcxxDimMeta(string complaint,string file,int line) :
  NcxxException(NC_EDIMMETA,complaint,file,line) { }

// Thrown if attribute meta.
NcxxAttMeta::NcxxAttMeta(string complaint,string file,int line) :
  NcxxException(NC_EATTMETA,complaint,file,line) { }

// Thrown if variable meta.
NcxxVarMeta::NcxxVarMeta(string complaint,string file,int line) :
  NcxxException(NC_EVARMETA,complaint,file,line) { }

// Thrown if no compound.
NcxxNoCompound::NcxxNoCompound(string complaint,string file,int line) :
  NcxxException(NC_ENOCOMPOUND,complaint,file,line) { }

// Thrown if attribute exists.
NcxxAttExists::NcxxAttExists(string complaint,string file,int line) :
  NcxxException(NC_EATTEXISTS,complaint,file,line) { }

// Thrown if attempting netcdf-4 operation on netcdf-3 file.
NcxxNotNc4::NcxxNotNc4(string complaint,string file,int line) :
  NcxxException(NC_ENOTNC4,complaint,file,line) { }

// Thrown if attempting netcdf-4 operation on strict nc3 netcdf-4 file.
NcxxStrictNc3::NcxxStrictNc3(string complaint,string file,int line) :
  NcxxException(NC_ESTRICTNC3,complaint,file,line) { }

// Thrown if bad group id.
NcxxBadGroupId::NcxxBadGroupId(string complaint,string file,int line) :
  NcxxException(NC_EBADGRPID,complaint,file,line) { }

// Thrown if bad type id.
NcxxBadTypeId::NcxxBadTypeId(string complaint,string file,int line) :
  NcxxException(NC_EBADTYPID,complaint,file,line) { }

// Thrown if bad field id.
NcxxBadFieldId::NcxxBadFieldId(string complaint,string file,int line) :
  NcxxException(NC_EBADFIELD,complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxUnknownName::NcxxUnknownName(string complaint,string file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxEnoGrp::NcxxEnoGrp(string complaint,string file,int line) :
  NcxxException(NC_ENOGRP,complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxNullGrp::NcxxNullGrp(string complaint,string file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxNullDim::NcxxNullDim(string complaint,string file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxNullType::NcxxNullType(string complaint,string file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if an operation to set the deflation, chunking, endianness, fill, compression, or checksum of a NcxxVar object is issued after a call to NcxxVar::getVar or NcxxVar::putVar.
NcxxElateDef::NcxxElateDef(string complaint,string file,int line) :
  NcxxException(NC_ELATEDEF,complaint,file,line) { }

