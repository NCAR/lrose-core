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

#include <Ncxx/NcxxVar.hh>
#include <Ncxx/NcxxVarAtt.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
#include <netcdf.h>
using namespace std;


//  Global comparator operator ==============
// comparator operator 
bool operator<(const NcxxVarAtt& lhs,const NcxxVarAtt& rhs)
{
  return false;
}

// comparator operator 
bool operator>(const NcxxVarAtt& lhs,const NcxxVarAtt& rhs)
{
  return true;
}


// assignment operator
NcxxVarAtt& NcxxVarAtt::operator=(const NcxxVarAtt & rhs)
{
  NcxxAtt::operator=(rhs);    // assign base class parts
  return *this;
}

//! The copy constructor.
NcxxVarAtt::NcxxVarAtt(const NcxxVarAtt& rhs): 
  NcxxAtt(rhs) // invoke base class copy constructor
{}


// Constructor generates a null object.
NcxxVarAtt::NcxxVarAtt() :
  NcxxAtt()  // invoke base class constructor
{}


// Constructor for an existing local attribute.
NcxxVarAtt::NcxxVarAtt(const NcxxGroup& grp, const NcxxVar& ncVar, const int index):
  NcxxAtt(false)
{
  groupId =  grp.getId();
  varId = ncVar.getId();
  // get the name of this attribute
  char attName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_attname(groupId,varId, index, attName),__FILE__,__LINE__);
  ncxxCheck(nc_inq_attname(groupId,varId,index,attName),__FILE__,__LINE__);
  myName = attName;
}

// Returns the NcxxVar parent object.
NcxxVar NcxxVarAtt::getParentVar() const {
  return NcxxVar(groupId,varId);
}
