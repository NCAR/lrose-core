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

#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxCompoundType.hh>
#include <Ncxx/NcxxByte.hh>
#include <Ncxx/NcxxUbyte.hh>
#include <Ncxx/NcxxChar.hh>
#include <Ncxx/NcxxShort.hh>
#include <Ncxx/NcxxUshort.hh>
#include <Ncxx/NcxxInt.hh>
#include <Ncxx/NcxxUint.hh>
#include <Ncxx/NcxxInt64.hh>
#include <Ncxx/NcxxUint64.hh>
#include <Ncxx/NcxxFloat.hh>
#include <Ncxx/NcxxDouble.hh>
#include <Ncxx/NcxxString.hh>
#include <Ncxx/NcxxException.hh>

using namespace std;

// Class represents a netCDF variable.

// assignment operator
NcxxCompoundType& NcxxCompoundType::operator=(const NcxxCompoundType& rhs)
{
  NcxxType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcxxCompoundType& NcxxCompoundType::operator=(const NcxxType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of a Compound type
    if(getTypeClass() != nc_COMPOUND) 	throw NcxxException("The NcxxType object must be the base of a Compound type.",__FILE__,__LINE__);
    // assign base class parts
    NcxxType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcxxCompoundType::NcxxCompoundType(const NcxxCompoundType& rhs): 
  NcxxType(rhs)
{
}


// equivalence operator
bool NcxxCompoundType::operator==(const NcxxCompoundType& rhs)
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myId ==rhs.myId && groupId == rhs.groupId;
}  
  
// Constructor generates a null object.
NcxxCompoundType::NcxxCompoundType() : 
  NcxxType()   // invoke base class constructor
{}
  
// constructor
NcxxCompoundType::NcxxCompoundType(const NcxxGroup& grp, const string& name): 
  NcxxType(grp,name)
{
}

// constructor
// The copy constructor.
NcxxCompoundType::NcxxCompoundType(const NcxxType& rhs): 
  NcxxType()
{
  // assign base class parts
  NcxxType::operator=(rhs);
}
  
//  Inserts a named field.
void NcxxCompoundType::addMember(const string& memberName, const NcxxType& newMemberType,size_t offset)
{
  ncxxCheck(nc_insert_compound(groupId,myId,const_cast<char*>(memberName.c_str()),offset,newMemberType.getId()),__FILE__,__LINE__);
}



//  Inserts a named array field.
void NcxxCompoundType::addMember(const string& memberName, const NcxxType& newMemberType, size_t offset, const vector<int>& shape)
{
  if (!shape.empty())
    ncxxCheck(nc_insert_array_compound(groupId, myId,const_cast<char*>(memberName.c_str()), offset, newMemberType.getId(), shape.size(), const_cast<int*>(&shape[0])),__FILE__,__LINE__);
  else
    addMember(memberName, newMemberType, offset);
}



// Returns number of members in this NcxxCompoundType object.
size_t  NcxxCompoundType::getMemberCount() const
{
  size_t nfieldsp;
  ncxxCheck(nc_inq_compound_nfields(groupId,myId,&nfieldsp),__FILE__,__LINE__);
  return nfieldsp;
}
  

// Returns a NcxxType object for a single member. */
NcxxType NcxxCompoundType::getMember(int memberIndex) const 
{
  nc_type fieldtypeidp;
  ncxxCheck(nc_inq_compound_fieldtype(groupId,myId,memberIndex,&fieldtypeidp),__FILE__,__LINE__);
  switch (fieldtypeidp) {
  case NC_BYTE    : return ncxxByte;
  case NC_UBYTE   : return ncxxUbyte;
  case NC_CHAR    : return ncxxChar;
  case NC_SHORT   : return ncxxShort;
  case NC_USHORT  : return ncxxUshort;
  case NC_INT     : return ncxxInt;
  case NC_UINT    : return ncxxUint;  
  case NC_INT64   : return ncxxInt64; 
  case NC_UINT64  : return ncxxUint64;
  case NC_FLOAT   : return ncxxFloat;
  case NC_DOUBLE  : return ncxxDouble;
  case NC_STRING  : return ncxxString;
  default:  
    // this is a user defined type
    return NcxxType(getParentGroup(),fieldtypeidp);
  }
}

// Returns name of member field.
std::string NcxxCompoundType::getMemberName(int memberIndex) const
{
  char fieldName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_compound_fieldname(groupId,myId,memberIndex, fieldName),__FILE__,__LINE__);
  return std::string(fieldName);
}

// Returns index of named member field.
int NcxxCompoundType::getMemberIndex(const std::string& memberName) const{
  int memberIndex;
  ncxxCheck(nc_inq_compound_fieldindex(groupId,myId, memberName.c_str(),&memberIndex),__FILE__,__LINE__);
  return memberIndex;
}

// Returns the number of dimensions of a member with the given index.
int NcxxCompoundType::getMemberDimCount(int memberIndex) const 
{
  int ndimsp;
  ncxxCheck(nc_inq_compound_fieldndims(groupId,myId,memberIndex, &ndimsp),__FILE__,__LINE__);
  return ndimsp;
}
  
  
// Returns the shape of the given member.
vector<int> NcxxCompoundType::getMemberShape(int memberIndex) const 
{
  vector<int> dim_size;
  dim_size.resize(getMemberDimCount(memberIndex));
  if(!dim_size.empty())
    ncxxCheck(nc_inq_compound_fielddim_sizes(groupId,myId,memberIndex,&dim_size[0]),__FILE__,__LINE__);
  return dim_size;
}
 

// Returns the offset of the member with given index.
size_t NcxxCompoundType::getMemberOffset(const int index) const
{
  size_t offsetp;
  ncxxCheck(nc_inq_compound_fieldoffset(groupId,myId, index,&offsetp),__FILE__,__LINE__);
  return offsetp;
}
