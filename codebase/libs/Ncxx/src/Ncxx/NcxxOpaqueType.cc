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

#include <Ncxx/NcxxOpaqueType.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxException.hh>
#include <netcdf.h>
using namespace std;

// Class represents a netCDF variable.
  
// assignment operator
NcxxOpaqueType& NcxxOpaqueType::operator=(const NcxxOpaqueType& rhs)
{
  // assign base class parts
  NcxxType::operator=(rhs);    
  return *this;
}
  
// assignment operator
NcxxOpaqueType& NcxxOpaqueType::operator=(const NcxxType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Opaque type
    if(getTypeClass() != NC_OPAQUE) 	throw NcxxException("The NcxxType object must be the base of an Opaque type.",__FILE__,__LINE__);
    // assign base class parts
    NcxxType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcxxOpaqueType::NcxxOpaqueType(const NcxxOpaqueType& rhs): 
  NcxxType(rhs)
{
}


// Constructor generates a null object.
NcxxOpaqueType::NcxxOpaqueType() :
  NcxxType()   // invoke base class constructor
{}


// constructor
NcxxOpaqueType::NcxxOpaqueType(const NcxxGroup& grp, const string& name) :
  NcxxType(grp,name)
{}
  
  
// constructor
NcxxOpaqueType::NcxxOpaqueType(const NcxxType& ncType) :
  NcxxType(ncType)
{
  // check the nctype object is the base of a Opaque type
  if(getTypeClass() != NC_OPAQUE) 	throw NcxxException("The NcxxType object must be the base of an Opaque type.",__FILE__,__LINE__);
}
  
// Returns the size of the opaque type in bytes.
size_t  NcxxOpaqueType::getTypeSize() const
{
  char* charName;
  charName=NULL;
  size_t sizep;
  ncxxCheck(nc_inq_opaque(groupId,myId,charName,&sizep),__FILE__,__LINE__);
  return sizep;
}
