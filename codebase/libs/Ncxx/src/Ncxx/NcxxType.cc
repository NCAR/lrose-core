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

#include <string>
#include <Ncxx/NcxxType.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
using namespace std;

extern int g_ncid;

//  Global comparator operator ==============
// comparator operator
bool operator<(const NcxxType& lhs,const NcxxType& rhs)
{
  return false;
}

// comparator operator
bool operator>(const NcxxType& lhs,const NcxxType& rhs)
{
  return true;
}

// assignment operator
NcxxType& NcxxType::operator=(const NcxxType & rhs)
{
  nullObject = rhs.nullObject;
  myId= rhs.myId;
  groupId = rhs.groupId;
  return *this;
}

// The copy constructor.
NcxxType::NcxxType(const NcxxType& rhs):
  nullObject(rhs.nullObject),
  myId(rhs.myId),
  groupId(rhs.groupId)
{}


// Constructor generates a null object.
NcxxType::NcxxType() :
  nullObject(true)
{}

// constructor
NcxxType::NcxxType(const NcxxGroup& grp, const string& name) :
  nullObject (false)
{
  groupId= grp.getId();
  NcxxType typTmp(grp.getType(name,NcxxGroup::ParentsAndCurrent));
  myId = typTmp.getId();
}

// constructor for a global type
NcxxType::NcxxType(nc_type id) :
  nullObject(false),
  myId(id),
  groupId(0)
{
}

// Constructor for a non-global type
NcxxType::NcxxType(const NcxxGroup& grp, nc_type id):
  nullObject(false),
  myId(id),
  groupId(grp.getId())
{}


// equivalence operator
bool NcxxType::operator==(const NcxxType & rhs) const
{
  if(nullObject)
    return nullObject == rhs.nullObject;
  else
    return groupId == rhs.groupId && myId == rhs.myId;
}

//  !=  operator
bool NcxxType::operator!=(const NcxxType & rhs) const
{
  return !(*this == rhs);
}

// Gets parent group.
NcxxGroup  NcxxType::getParentGroup() const {
  if(groupId == 0) return NcxxGroup(); else  return NcxxGroup(groupId);
}

// Returns the type name.
string  NcxxType::getName() const{
  char charName[NC_MAX_NAME+1];
  size_t *sizep=NULL;

  /* We cannot call nc_inq_type without a valid
     netcdf file ID (ncid), which is not *groupid*.
     Working around this for now. */

  ncxxCheck(nc_inq_type(g_ncid,myId,charName,sizep),__FILE__,__LINE__);
  return string(charName);

}

// Returns the size in bytes
size_t NcxxType::getSize() const {
  char* charName=NULL;
  size_t sizep;
  ncxxCheck(nc_inq_type(g_ncid,myId,charName,&sizep),__FILE__,__LINE__);
  return sizep;
}

// The type class returned as an enumeration type.
NcxxType::ncxxType NcxxType::getTypeClass() const{
  switch (myId) {
  case NC_BYTE    : return nc_BYTE;
  case NC_UBYTE   : return nc_UBYTE;
  case NC_CHAR    : return nc_CHAR;
  case NC_SHORT   : return nc_SHORT;
  case NC_USHORT  : return nc_USHORT;
  case NC_INT     : return nc_INT;
  case NC_UINT    : return nc_UINT;
  case NC_INT64   : return nc_INT64;
  case NC_UINT64  : return nc_UINT64;
  case NC_FLOAT   : return nc_FLOAT;
  case NC_DOUBLE  : return nc_DOUBLE;
  case NC_STRING  : return nc_STRING;
  default:
    // this is a user defined type
    // establish its type class, ie whether it is: NC_VLEN, NC_OPAQUE, NC_ENUM, or NC_COMPOUND.
    char* name=NULL;
    size_t* sizep=NULL;
    nc_type* base_nc_typep=NULL;
    size_t* nfieldsp=NULL;
    int classp;
    ncxxCheck(nc_inq_user_type(groupId,myId,name,sizep,base_nc_typep,nfieldsp,&classp),__FILE__,__LINE__);
    return static_cast<ncxxType>(classp);
  }
}

// The type class returned as a string.
string NcxxType::getTypeClassName() const{
  ncxxType typeClass=getTypeClass();
  switch (typeClass) {
  case nc_BYTE    : return string("nc_BYTE");
  case nc_UBYTE   : return string("nc_UBYTE");
  case nc_CHAR    : return string("nc_CHAR");
  case nc_SHORT   : return string("nc_SHORT");
  case nc_USHORT  : return string("nc_USHORT");
  case nc_INT     : return string("nc_INT");
  case nc_UINT    : return string("nc_UINT");
  case nc_INT64   : return string("nc_INT64");
  case nc_UINT64  : return string("nc_UINT64");
  case nc_FLOAT   : return string("nc_FLOAT");
  case nc_DOUBLE  : return string("nc_DOUBLE");
  case nc_STRING  : return string("nc_STRING");
  case nc_VLEN    : return string("nc_VLEN");
  case nc_OPAQUE  : return string("nc_OPAQUE");
  case nc_ENUM    : return string("nc_ENUM");
  case nc_COMPOUND: return string("nc_COMPOUND");
  }
  // we never get here!
  return "Dummy";
}

///////////////////////////////////////////////////////
// Check if this is a complex type
// i.e. NC_VLEN, NC_OPAQUE, NC_ENUM, or NC_COMPOUND

bool NcxxType::isComplex() const {

  switch (myId) {
    case NC_BYTE    : return false;
    case NC_UBYTE   : return false;
    case NC_CHAR    : return false;
    case NC_SHORT   : return false;
    case NC_USHORT  : return false;
    case NC_INT     : return false;
    case NC_UINT    : return false;
    case NC_INT64   : return false;
    case NC_UINT64  : return false;
    case NC_FLOAT   : return false;
    case NC_DOUBLE  : return false;
    case NC_STRING  : return false;
    default         : return true;
  }

}

