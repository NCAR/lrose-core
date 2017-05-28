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
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <exception>
#include <string>
#include <iostream>

#ifndef NcxxExceptionClasses
#define NcxxExceptionClasses

//!  Exception classes.
/*!
  These exceptions are thrown if the netCDF-4 API encounters an error.
*/

/*! 
  Base object is thrown if a netCDF exception is encountered.  An unsatisfactory
  return from a call to one of the netCDF C-routines generates an exception
  using an object inheriting this class.  All other netCDF-related errors
  including those originating in the C++ binding, generates an NcxxException.
*/
class NcxxException : public std::exception {

public:

  NcxxException(std::string complaint, 
                std::string fileName,
                int lineNumber);

  NcxxException(int errorCode,
                std::string complaint,
                std::string fileName,
                int lineNumber);

  NcxxException(const NcxxException& e) throw();

  NcxxException& operator=(const NcxxException& e) throw();

  virtual ~NcxxException() throw();

  const char *what() const throw();
  const std::string whatStr() const throw();

  int errorCode() const throw();

private:

  std::string _whatMsg;
  int _ec;

};


/*!
  Thrown if the specified netCDF ID does not refer to an open netCDF
  dataset.
*/

class NcxxBadId : public NcxxException
{
public:
  NcxxBadId(std::string complaint,std::string file,int line);
};

/*! Thrown if too many netcdf files are open. */
class NcxxNFile : public NcxxException
{
public:
  NcxxNFile(std::string complaint,std::string file,int line);
};

/*! Thrown if, having set NC_NOCLOBBER, the specified dataset already exists. */
class NcxxExist : public NcxxException
{
public:
  NcxxExist(std::string complaint,std::string file,int line);
};

/*! Thrown if not a netCDF id.  */
class NcxxInvalidArg : public NcxxException
{
public:
  NcxxInvalidArg(std::string complaint,std::string file,int line);
};

/*! Thrown if invalid argument. */
class NcxxInvalidWrite : public NcxxException
{
public:
  NcxxInvalidWrite(std::string complaint,std::string file,int line);
};

/*! Thrown if operation not allowed in data mode. */
class NcxxNotInDefineMode : public NcxxException
{
public:
  NcxxNotInDefineMode(std::string complaint,std::string file,int line);
};

/*! Thrown if operation not allowed in defined mode. */
class NcxxInDefineMode : public NcxxException
{
public:
  NcxxInDefineMode(std::string complaint,std::string file,int line);
};

/*! 
  Index exceeds dimension bound.  Exception may be generated during operations
  to get or put netCDF variable data.  The exception is thrown if the specified
  indices were out of range for the rank of the specified variable. For example,
  a negative index or an index that is larger than the corresponding dimension
  length will cause an error.
*/
class NcxxInvalidCoords : public NcxxException
{
public:
  NcxxInvalidCoords(std::string complaint,std::string file,int line);
};

/*! Thrown if NC_MAX_DIMS is exceeded. */
class NcxxMaxDims : public NcxxException
{
public:
  NcxxMaxDims(std::string complaint,std::string file,int line);
};

/*! Thrown if std::string match to name is in use. */
class NcxxNameInUse : public NcxxException
{
public:
  NcxxNameInUse(std::string complaint,std::string file,int line);
};

/*! Thrown if attribute is not found. */
class NcxxNotAtt : public NcxxException
{
public:
  NcxxNotAtt(std::string complaint,std::string file,int line);
};

/*! Thrown if Nc_MAX_ATTRS is exceeded. */
class NcxxMaxAtts : public NcxxException
{
public:
  NcxxMaxAtts(std::string complaint,std::string file,int line);
};

/*! Thrown if not a valid netCDF data type. */
class NcxxBadType : public NcxxException
{
public:
  NcxxBadType(std::string complaint,std::string file,int line);
};

/*! Thrown if an invalid dimension id or name. */
class NcxxBadDim : public NcxxException
{
public:
  NcxxBadDim(std::string complaint,std::string file,int line);
};

/*! Thrown if Nc_UNLIMITED is in the wrong index. */
class NcxxUnlimPos : public NcxxException
{
public:
  NcxxUnlimPos(std::string complaint,std::string file,int line);
};

/*! Thrown if NC_MAX_VARS is exceeded. */
class NcxxMaxVars : public NcxxException
{
public:
  NcxxMaxVars(std::string complaint,std::string file,int line);
};

/*! Thrown if variable is not found. */
class NcxxNotVar : public NcxxException
{
public:
  NcxxNotVar(std::string complaint,std::string file,int line);
};

/*! Thrown if the action is prohibited on the NC_GLOBAL varid. */
class NcxxGlobal : public NcxxException
{
public:
  NcxxGlobal(std::string complaint,std::string file,int line);
};

/*! Thrown if not a netCDF file. */
class NcxxNotNCF : public NcxxException
{
public:
  NcxxNotNCF(std::string complaint,std::string file,int line);
};

/*! Thrown if in FORTRAN, std::string is too short. */
class NcxxSts : public NcxxException
{
public:
  NcxxSts(std::string complaint,std::string file,int line);
};

/*! Thrown if NC_MAX_NAME is exceeded. */
class NcxxMaxName : public NcxxException
{
public:
  NcxxMaxName(std::string complaint,std::string file,int line);
};

/*! Thrown if NC_UNLIMITED size is already in use. */
class NcxxUnlimit : public NcxxException
{
public:
  NcxxUnlimit(std::string complaint,std::string file,int line);
};

/*! Thrown if nc_rec op when there are no record vars. */
class NcxxNoRecVars : public NcxxException
{
public:
  NcxxNoRecVars(std::string complaint,std::string file,int line);
};

/*! Thrown if attempt to convert between text and numbers. */
class NcxxCharConvert : public NcxxException
{
public:
  NcxxCharConvert(std::string complaint,std::string file,int line);
};

/*! Thrown if edge+start exceeds dimension bound. */
class NcxxEdge : public NcxxException
{
public:
  NcxxEdge(std::string complaint,std::string file,int line);
};

/*! Thrown if illegal stride. */
class NcxxStride : public NcxxException
{
public:
  NcxxStride(std::string complaint,std::string file,int line);
};

/*! Thrown if attribute or variable name contains illegal characters. */
class NcxxBadName : public NcxxException
{
public:
  NcxxBadName(std::string complaint,std::string file,int line);
};

/*! Thrown if math result not representable. */
class NcxxRange : public NcxxException
{
public:
  NcxxRange(std::string complaint,std::string file,int line);
};

/*! Thrown if memory allocation (malloc) failure. */
class NcxxNoMem : public NcxxException
{
public:
  NcxxNoMem(std::string complaint,std::string file,int line);
};

/*! Thrown if one or more variable sizes violate format constraints */
class NcxxVarSize : public NcxxException
{
public:
  NcxxVarSize(std::string complaint,std::string file,int line);
};

/*! Thrown if invalid dimension size. */
class NcxxDimSize : public NcxxException
{
public:
  NcxxDimSize(std::string complaint,std::string file,int line);
};

/*! Thrown if file likely truncated or possibly corrupted. */
class NcxxTrunc : public NcxxException
{
public:
  NcxxTrunc(std::string complaint,std::string file,int line);
};

/*! Thrown if an error was reported by the HDF5 layer. */
class NcxxHdfErr : public NcxxException
{
public:
  NcxxHdfErr(std::string complaint,std::string file,int line);
};

/*! Thrown if cannot read. */
class NcxxCantRead : public NcxxException
{
public:
  NcxxCantRead(std::string complaint,std::string file,int line);
};

/*! Thrown if cannot write. */
class NcxxCantWrite : public NcxxException
{
public:
  NcxxCantWrite(std::string complaint,std::string file,int line);
};

/*! Thrown if cannot create. */
class NcxxCantCreate : public NcxxException
{
public:
  NcxxCantCreate(std::string complaint,std::string file,int line);
};

/*! Thrown if file meta. */
class NcxxFileMeta : public NcxxException
{
public:
  NcxxFileMeta(std::string complaint,std::string file,int line);
};

/*! Thrown if dim meta. */
class NcxxDimMeta : public NcxxException
{
public:
  NcxxDimMeta(std::string complaint,std::string file,int line);
};

/*! Thrown if attribute meta. */
class NcxxAttMeta : public NcxxException
{
public:
  NcxxAttMeta(std::string complaint,std::string file,int line);
};

/*! Thrown if variable meta. */
class NcxxVarMeta : public NcxxException
{
public:
  NcxxVarMeta(std::string complaint,std::string file,int line);
};

/*! Thrown if no compound. */
class NcxxNoCompound : public NcxxException
{
public:
  NcxxNoCompound(std::string complaint,std::string file,int line);
};

/*! Thrown if attribute exists. */
class NcxxAttExists : public NcxxException
{
public:
  NcxxAttExists(std::string complaint,std::string file,int line);
};

/*! Thrown if attempting netcdf-4 operation on netcdf-3 file. */
class NcxxNotNc4 : public NcxxException
{
public:
  NcxxNotNc4(std::string complaint,std::string file,int line);
};

/*! Thrown if attempting netcdf-4 operation on strict nc3 netcdf-4 file. */
class NcxxStrictNc3 : public NcxxException
{
public:
  NcxxStrictNc3(std::string complaint,std::string file,int line);
};

/*! Thrown if bad group id. */
class NcxxBadGroupId : public NcxxException
{
public:
  NcxxBadGroupId(std::string complaint,std::string file,int line);
};

/*! Thrown if bad type id. */
class NcxxBadTypeId : public NcxxException
{
public:
  NcxxBadTypeId(std::string complaint,std::string file,int line);
};

/*! Thrown if bad field id. */
class NcxxBadFieldId : public NcxxException
{
public:
  NcxxBadFieldId(std::string complaint,std::string file,int line);
};

/*! Thrown if cannot find the field id. */
class NcxxUnknownName : public NcxxException
{
public:
  NcxxUnknownName(std::string complaint,std::string file,int line);
};

/*! Thrown if cannot return a netCDF group. */
class NcxxEnoGrp : public NcxxException
{
public:
  NcxxEnoGrp(std::string complaint,std::string file,int line);
};

/*! 
  Thrown if the requested operation is on a NULL group.
  This exception is thrown if an operation on a NcxxGroup object is requested
  which is empty. To test if the object is empty used NcxxGroup::isNull()
*/
class NcxxNullGrp : public NcxxException
{
public:
  NcxxNullGrp(std::string complaint,std::string file,int line);
};

/*! 
  Thrown if the requested operation is on a NULL type.
  This exception is thrown if an operation on a NcxxType object is requested
  which is empty. To test if the object is empty used NcxxType::isNull()
*/
class NcxxNullType : public NcxxException
{
public:
  NcxxNullType(std::string complaint,std::string file,int line);
};

/*! 
  Thrown if the requested operation is on a NULL dimension.  This exception is
  thrown if an operation on a NcxxDim object is requested which is empty. To
  test if the object is empty used NcxxDim::isNull()
*/
class NcxxNullDim : public NcxxException
{
public:
  NcxxNullDim(std::string complaint,std::string file,int line);
};

/*! 
  Thrown if an operation to set the chunking, endianness, fill of a NcxxVar
  object is issued after a call to NcxxVar::getVar or NcxxVar::putVar has been
  made.
*/
class NcxxElateDef : public NcxxException
{
public:
  NcxxElateDef(std::string complaint,std::string file,int line);
};

#endif

