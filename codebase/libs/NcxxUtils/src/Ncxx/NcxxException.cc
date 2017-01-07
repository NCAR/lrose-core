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

#include <NcxxUtils/NcxxException.hh>
#include <sstream>
#include <netcdf.h>
using namespace std;

// Default object thrown if a netCDF exception is encountered.
/*NcxxException::NcxxException(const string& complaint,const char* fileName,int lineNumber)
  : what_msg(NULL)
  , ec(0)
{
	try{
		std::ostringstream oss;
		oss << lineNumber;
		what_msg = new std::string(complaint+"\nfile: "+fileName+"  line:"+oss.str());
	}catch(...){
		what_msg = NULL;
	}
}*/

NcxxException::NcxxException(const char* complaint,const char* fileName,int lineNumber)
  : what_msg(NULL)
  , ec(0)
{
  try{
    std::ostringstream oss;
    oss << lineNumber;
    what_msg = new std::string(complaint?complaint:"");
    what_msg->append("\nfile: ");
    what_msg->append(fileName);
    what_msg->append("  line:");
    what_msg->append(oss.str());
  }catch(...){
    what_msg = NULL;
  }
}

NcxxException::NcxxException(int errorCode, const char* complaint,const char* fileName,int lineNumber)
  : what_msg(NULL)
  , ec(errorCode)
{
  try{
    std::ostringstream oss;
    oss << lineNumber;
    what_msg = new std::string(complaint?complaint:"");
    what_msg->append("\nfile: ");
    what_msg->append(fileName);
    what_msg->append("  line:");
    what_msg->append(oss.str());
  }catch(...){
    what_msg = NULL;
  }
}

NcxxException::NcxxException(const NcxxException& e) throw()
	: what_msg(NULL)
  , ec(e.ec)
{
	try{
		what_msg = new std::string(*(e.what_msg));
	}catch(...){
		what_msg = NULL;
	}
}

NcxxException& NcxxException::operator=(const NcxxException& e) throw(){
	if (this != &e){
    ec = e.ec;
		delete what_msg;
		try{
			what_msg = new std::string(*(e.what_msg));
		}catch(...){
			what_msg = NULL;
		}
	}
	return *this;
}

NcxxException::~NcxxException()throw() {
	delete what_msg;
}


const char* NcxxException::what() const throw()
{
  return what_msg==NULL ? "" : what_msg->c_str();
}

int NcxxException::errorCode() const throw() {
  return ec;
}


// Thrown if the specified netCDF ID does not refer to an open netCDF dataset. 
NcxxBadId::NcxxBadId(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADID,complaint,file,line) { }


// Thrown if too many netcdf files are open.
NcxxNFile::NcxxNFile(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENFILE,complaint,file,line) { }

// Thrown if, having set NC_NOCLOBBER, the specified dataset already exists. 
NcxxExist::NcxxExist(const char* complaint,const char* file,int line) :
  NcxxException(NC_EEXIST,complaint,file,line) { }

// Thrown if not a netCDF id.
NcxxInvalidArg::NcxxInvalidArg(const char* complaint,const char* file,int line) :
  NcxxException(NC_EINVAL,complaint,file,line) { }

// Thrown if invalid argument.
NcxxInvalidWrite::NcxxInvalidWrite(const char* complaint,const char* file,int line) :
  NcxxException(NC_EPERM,complaint,file,line) { }

// Thrown if operation not allowed in data mode.
NcxxNotInDefineMode::NcxxNotInDefineMode(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOTINDEFINE,complaint,file,line) { }

// Thrown if operation not allowed in defined mode.
NcxxInDefineMode::NcxxInDefineMode(const char* complaint,const char* file,int line) :
  NcxxException(NC_EINDEFINE,complaint,file,line) { }

// Index exceeds dimension bound
NcxxInvalidCoords::NcxxInvalidCoords(const char* complaint,const char* file,int line) :
  NcxxException(NC_EINVALCOORDS,complaint,file,line) { }

// Thrown if NC_MAX_DIMS is exceeded.
NcxxMaxDims::NcxxMaxDims(const char* complaint,const char* file,int line) :
  NcxxException(NC_EMAXDIMS,complaint,file,line) { }

// Thrown if string match to name is in use.
NcxxNameInUse::NcxxNameInUse(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENAMEINUSE,complaint,file,line) { }

// Thrown if attribute is not found.
NcxxNotAtt::NcxxNotAtt(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOTATT,complaint,file,line) { }

// Thrown if Ncxx_MAX_ATTRS is exceeded.
NcxxMaxAtts::NcxxMaxAtts(const char* complaint,const char* file,int line) :
  NcxxException(NC_EMAXATTS,complaint,file,line) { }

// Thrown if not a valid netCDF data type.
NcxxBadType::NcxxBadType(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADTYPE,complaint,file,line) { }

// Thrown if an invalid dimension id or name.
NcxxBadDim::NcxxBadDim(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADDIM,complaint,file,line) { }

// Thrown if Nc_UNLIMITED is in the wrong index.
NcxxUnlimPos::NcxxUnlimPos(const char* complaint,const char* file,int line) :
  NcxxException(NC_EUNLIMPOS,complaint,file,line) { }

// Thrown if NC_MAX_VARS is exceeded.
NcxxMaxVars::NcxxMaxVars(const char* complaint,const char* file,int line) :
  NcxxException(NC_EMAXVARS,complaint,file,line) { }

// Thrown if variable is not found.
NcxxNotVar::NcxxNotVar(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOTVAR,complaint,file,line) { }

// Thrown if the action is prohibited on the NC_GLOBAL varid.
NcxxGlobal::NcxxGlobal(const char* complaint,const char* file,int line) :
  NcxxException(NC_EGLOBAL,complaint,file,line) { }

// Thrown if not a netCDF file.
NcxxNotNCF::NcxxNotNCF(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOTNC,complaint,file,line) { }

// Thrown if in FORTRAN, string is too short.
NcxxSts::NcxxSts(const char* complaint,const char* file,int line) :
  NcxxException(NC_ESTS,complaint,file,line) { }

// Thrown if NC_MAX_NAME is exceeded.
NcxxMaxName::NcxxMaxName(const char* complaint,const char* file,int line) :
  NcxxException(NC_EMAXNAME,complaint,file,line) { }

// Thrown if NC_UNLIMITED size is already in use.
NcxxUnlimit::NcxxUnlimit(const char* complaint,const char* file,int line) :
  NcxxException(NC_EUNLIMIT,complaint,file,line) { }

// Thrown if nc_rec op when there are no record vars.
NcxxNoRecVars::NcxxNoRecVars(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENORECVARS,complaint,file,line) { }

// Thrown if attempt to convert between text and numbers.
NcxxCharConvert::NcxxCharConvert(const char* complaint,const char* file,int line) :
  NcxxException(NC_ECHAR,complaint,file,line) { }

// Thrown if edge+start exceeds dimension bound.
NcxxEdge::NcxxEdge(const char* complaint,const char* file,int line) :
  NcxxException(NC_EEDGE,complaint,file,line) { }

// Thrown if illegal stride.
NcxxStride::NcxxStride(const char* complaint,const char* file,int line) :
  NcxxException(NC_ESTRIDE,complaint,file,line) { }

// Thrown if attribute or variable name contains illegal characters.
NcxxBadName::NcxxBadName(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADNAME,complaint,file,line) { }

// Thrown if math result not representable.
NcxxRange::NcxxRange(const char* complaint,const char* file,int line) :
  NcxxException(NC_ERANGE,complaint,file,line) { }

// Thrown if memory allocation (malloc) failure.
NcxxNoMem::NcxxNoMem(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOMEM,complaint,file,line) { }

// Thrown if one or more variable sizes violate format constraints
NcxxVarSize::NcxxVarSize(const char* complaint,const char* file,int line) :
  NcxxException(NC_EVARSIZE,complaint,file,line) { }

// Thrown if invalid dimension size.
NcxxDimSize::NcxxDimSize(const char* complaint,const char* file,int line) :
  NcxxException(NC_EDIMSIZE,complaint,file,line) { }

// Thrown if file likely truncated or possibly corrupted.
NcxxTrunc::NcxxTrunc(const char* complaint,const char* file,int line) :
  NcxxException(NC_ETRUNC,complaint,file,line) { }

// Thrown if an error was reported by the HDF5 layer.
NcxxHdfErr::NcxxHdfErr(const char* complaint,const char* file,int line) :
  NcxxException(NC_EHDFERR,complaint,file,line) { }

// Thrown if cannot read.
NcxxCantRead::NcxxCantRead(const char* complaint,const char* file,int line) :
  NcxxException(NC_ECANTREAD,complaint,file,line) { }

// Thrown if cannot write.
NcxxCantWrite::NcxxCantWrite(const char* complaint,const char* file,int line) :
  NcxxException(NC_ECANTWRITE,complaint,file,line) { }

// Thrown if cannot create.
NcxxCantCreate::NcxxCantCreate(const char* complaint,const char* file,int line) :
  NcxxException(NC_ECANTCREATE,complaint,file,line) { }

// Thrown if file meta.
NcxxFileMeta::NcxxFileMeta(const char* complaint,const char* file,int line) :
  NcxxException(NC_EFILEMETA,complaint,file,line) { }

// Thrown if dim meta.
NcxxDimMeta::NcxxDimMeta(const char* complaint,const char* file,int line) :
  NcxxException(NC_EDIMMETA,complaint,file,line) { }

// Thrown if attribute meta.
NcxxAttMeta::NcxxAttMeta(const char* complaint,const char* file,int line) :
  NcxxException(NC_EATTMETA,complaint,file,line) { }

// Thrown if variable meta.
NcxxVarMeta::NcxxVarMeta(const char* complaint,const char* file,int line) :
  NcxxException(NC_EVARMETA,complaint,file,line) { }

// Thrown if no compound.
NcxxNoCompound::NcxxNoCompound(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOCOMPOUND,complaint,file,line) { }

// Thrown if attribute exists.
NcxxAttExists::NcxxAttExists(const char* complaint,const char* file,int line) :
  NcxxException(NC_EATTEXISTS,complaint,file,line) { }

// Thrown if attempting netcdf-4 operation on netcdf-3 file.
NcxxNotNc4::NcxxNotNc4(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOTNC4,complaint,file,line) { }

// Thrown if attempting netcdf-4 operation on strict nc3 netcdf-4 file.
NcxxStrictNc3::NcxxStrictNc3(const char* complaint,const char* file,int line) :
  NcxxException(NC_ESTRICTNC3,complaint,file,line) { }

// Thrown if bad group id.
NcxxBadGroupId::NcxxBadGroupId(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADGRPID,complaint,file,line) { }

// Thrown if bad type id.
NcxxBadTypeId::NcxxBadTypeId(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADTYPID,complaint,file,line) { }

// Thrown if bad field id.
NcxxBadFieldId::NcxxBadFieldId(const char* complaint,const char* file,int line) :
  NcxxException(NC_EBADFIELD,complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxUnknownName::NcxxUnknownName(const char* complaint,const char* file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxEnoGrp::NcxxEnoGrp(const char* complaint,const char* file,int line) :
  NcxxException(NC_ENOGRP,complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxNullGrp::NcxxNullGrp(const char* complaint,const char* file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxNullDim::NcxxNullDim(const char* complaint,const char* file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if cannot find the field id.
NcxxNullType::NcxxNullType(const char* complaint,const char* file,int line) :
  NcxxException(complaint,file,line) { }

// Thrown if an operation to set the deflation, chunking, endianness, fill, compression, or checksum of a NcxxVar object is issued after a call to NcxxVar::getVar or NcxxVar::putVar.
NcxxElateDef::NcxxElateDef(const char* complaint,const char* file,int line) :
  NcxxException(NC_ELATEDEF,complaint,file,line) { }

