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

#include <Ncxx/NcxxVlenType.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxException.hh>
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
#include <netcdf.h>
using namespace std;

// Class represents a netCDF variable.

// assignment operator
NcxxVlenType& NcxxVlenType::operator=(const NcxxVlenType& rhs)
{
  NcxxType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcxxVlenType& NcxxVlenType::operator=(const NcxxType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Opaque type
    if(getTypeClass() != NC_VLEN) 	throw NcxxException("The NcxxType object must be the base of an Vlen type.",__FILE__,__LINE__);
    // assign base class parts
    NcxxType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcxxVlenType::NcxxVlenType(const NcxxVlenType& rhs):   
  NcxxType(rhs)
{
}


// Constructor generates a null object.
NcxxVlenType::NcxxVlenType() :
  NcxxType()   // invoke base class constructor
{}

// constructor
NcxxVlenType::NcxxVlenType(const NcxxGroup& grp, const string& name) :
  NcxxType(grp,name)
{}
  
// constructor
NcxxVlenType::NcxxVlenType(const NcxxType& ncType): 
  NcxxType(ncType)
{
  // check the nctype object is the base of a Vlen type
  if(getTypeClass() != NC_VLEN) throw NcxxException("The NcxxType object must be the base of a Vlen type.",__FILE__,__LINE__);
}

// Returns the base type.
NcxxType NcxxVlenType::getBaseType() const
{
  char charName[NC_MAX_NAME+1];
  nc_type base_nc_typep;
  size_t datum_sizep;
  ncxxCheck(nc_inq_vlen(groupId,myId,charName,&datum_sizep,&base_nc_typep),__FILE__,__LINE__);
  switch (base_nc_typep) {
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
    return NcxxType(getParentGroup(),base_nc_typep);
  }
}
