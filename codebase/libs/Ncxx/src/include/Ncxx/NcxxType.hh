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

#include <string>
#include <netcdf.h>

#ifndef NcxxTypeClass
#define NcxxTypeClass


class NcxxGroup; // forward declaration to avoid cyclic reference.

/*! Base class inherited by NcOpaque, NcVlen, NcCompound and NcEnum classes. */
class NcxxType
{
  
public:
  
  /*!
    List of netCDF types that can be represented.
    The enumeration list contains the complete set of netCDF variable types. In addition, the type NC_TYPE
    is included. This enables the user to instantiate a netCDF type object without explcitly needing to know
    it precise type.
  */
  enum ncxxType
    {
      nc_BYTE     = NC_BYTE, 	//!< signed 1 byte integer
      nc_CHAR     = NC_CHAR,	//!< ISO/ASCII character
      nc_SHORT    = NC_SHORT, 	//!< signed 2 byte integer
      nc_INT      = NC_INT,	//!< signed 4 byte integer
      nc_FLOAT    = NC_FLOAT, 	//!< single precision floating point number
      nc_DOUBLE   = NC_DOUBLE, 	//!< double precision floating point number
      nc_UBYTE    = NC_UBYTE,	//!< unsigned 1 byte int
      nc_USHORT   = NC_USHORT,	//!< unsigned 2-byte int
      nc_UINT     = NC_UINT,	//!< unsigned 4-byte int
      nc_INT64    = NC_INT64,	//!< signed 8-byte int
      nc_UINT64   = NC_UINT64,	//!< unsigned 8-byte int
      nc_STRING   = NC_STRING, 	//!< string
      nc_VLEN     = NC_VLEN,   	//!< "NcVlen type"
      nc_OPAQUE   = NC_OPAQUE, 	//!< "NcOpaque type"
      nc_ENUM     = NC_ENUM, 	//!< "NcEnum type"
      nc_COMPOUND = NC_COMPOUND //!< "NcCompound type"
    };

  /*! Constructor generates a \ref isNull "null object". */
  NcxxType();

  /*!
    Constructor for a non-global type.
    This object describes the "essential" information for all netCDF types required by NcxxVar, NcxxAtt objects.
    New netCDF types can be added using the appropriate "add" method in the NcxxGroup object.
    \param grp    Parent NcxxGroup object.
    \param name   Name of this type.
  */
  NcxxType(const NcxxGroup& grp, const std::string& name);


  /*!
    Constructor for a non-global type.
    This object describes the "essential" information for all netCDF types required by NcxxVar, NcxxAtt objects.
    New netCDF types can be added using the appropriate "add" method in the NcxxGroup object.
    \param grp    Parent NcxxGroup object.
    \param id     type id
  */
  NcxxType(const NcxxGroup& grp, nc_type id);

  /*!
    Constructor for a global type
    This object describes the "essential" information for a netCDF global type.
    \param id     type id
  */
  NcxxType(nc_type id);

  /*! The copy constructor. */
  NcxxType(const NcxxType& rhs);

  /*! destructor  */
  virtual ~NcxxType() {}

  /*! equivalence operator */
  bool operator==(const NcxxType&) const;

  /*!  != operator */
  bool operator!=(const NcxxType &) const;

  // accessors to private data.
  /*! The netCDF Id of this type. */
  nc_type getId() const {return myId;}

  /*! Gets parent group. For an atomic type, returns a Null object.*/
  NcxxGroup getParentGroup() const;

  /*!
    The name of this type. For atomic types, the CDL type names are returned. These are as follows:
    - NcxxByte   String returned is "byte".
    - NcxxUbyte  String returned is "ubyte".
    - NcxxChar   String returned is "char".
    - NcxxShort  String returned is "short".
    - NcxxUshort String returned is "ushort".
    - NcxxInt    String returned is "int".
    - NcxxUint   String returned is "uint".
    - NcxxInt64  String returned is "int64".
    - NcxxUint64 String returned is "uint64".
    - NcxxFloat  String returned is "float".
    - NcxxDouble String returned is "double".
    - NcxxString String returned is "string".
  */
  std::string getName() const;

  /*!
    The size in bytes.
    This function will work on any type, including atomic and any user defined type, whether
    compound, opaque, enumeration, or variable length array.

  */
  size_t getSize() const;

  /*!
    The type class returned as enumeration type.
    Valid for all types, whether atomic or user-defined. User-defined types are returned as one of the following
    enumeration types: nc_VLEN, nc_OPAQUE, nc_ENUM, or nc_COMPOUND.
  */
  ncxxType getTypeClass() const;

  /*!
    Return a string containing the name of the enumerated type.  (ie one of the following strings:
    "nc_BYTE", "nc_CHAR", "nc_SHORT", "nc_INT", "nc_FLOAT", "nc_DOUBLE", "nc_UBYTE", "nc_USHORT",
    "nc_UINT", "nc_INT64", "nc_UINT64", "nc_STRING", "nc_VLEN", "nc_OPAQUE", "nc_ENUM", "nc_COMPOUND"
  */
  std::string getTypeClassName() const;

  /*! Returns true if this object is null (i.e. it has no contents); otherwise returns false. */
  bool isNull() const  {return nullObject;}

  /*! comparator operator  */
  friend bool operator<(const NcxxType& lhs,const NcxxType& rhs);

  /*! comparator operator  */
  friend bool operator>(const NcxxType& lhs,const NcxxType& rhs);

  // Check if this is a complex type
  // i.e. NC_VLEN, NC_OPAQUE, NC_ENUM, or NC_COMPOUND
  
  bool isComplex() const;

protected:

  /*! assignment operator  */
  NcxxType& operator=(const NcxxType& rhs);

  bool nullObject;

  /*! the type Id */
  nc_type myId;

  /*! the group Id */
  int groupId;

  /*! An ncid associated with a particular open file
    (returned from nc_open).
    This is required by many of the functions ncType uses,
    such as nc_inq_type */
  int g_fileId;

};

#endif
