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

#include <Ncxx/NcxxDim.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
#include <algorithm>
using namespace std;


//  Global comparator operator ==============
// comparator operator
bool operator<(const NcxxDim& lhs,const NcxxDim& rhs)
{
  return false;
}

// comparator operator
bool operator>(const NcxxDim& lhs,const NcxxDim& rhs)
{
  return true;
}

// assignment operator
NcxxDim& NcxxDim::operator=(const NcxxDim & rhs)
{
  nullObject = rhs.nullObject;
  myId = rhs.myId;
  groupId = rhs.groupId;
  return *this;
}

// The copy constructor.
NcxxDim::NcxxDim(const NcxxDim& rhs):
  nullObject(rhs.nullObject),
  myId(rhs.myId),
  groupId(rhs.groupId)
{}


// equivalence operator
bool NcxxDim::operator==(const NcxxDim& rhs) const
{
  if(nullObject)
    return nullObject == rhs.nullObject;
  else
    return myId == rhs.myId && groupId == rhs.groupId;
}

//  !=  operator
bool NcxxDim::operator!=(const NcxxDim & rhs) const
{
  return !(*this == rhs);
}


// Gets parent group.
NcxxGroup  NcxxDim::getParentGroup() const {
  return NcxxGroup(groupId);
}

// Constructor generates a null object.
NcxxDim::NcxxDim() :
  nullObject(true)
{}

// Constructor for a dimension (must already exist in the netCDF file.)
NcxxDim::NcxxDim(const NcxxGroup& grp, int dimId) :
  nullObject(false)
{
  groupId = grp.getId();
  myId = dimId;
}

// gets the size of the dimension, for unlimited, this is the current number of records.
size_t NcxxDim::getSize() const
{
  size_t dimSize;
  ncxxCheck(nc_inq_dimlen(groupId, myId, &dimSize),__FILE__,__LINE__);
  return dimSize;
}


// returns true if this dimension is unlimited.
bool NcxxDim::isUnlimited() const
{
  int numlimdims;
  int* unlimdimidsp=NULL;
  // get the number of unlimited dimensions
  ncxxCheck(nc_inq_unlimdims(groupId,&numlimdims,unlimdimidsp),__FILE__,__LINE__);
  if (numlimdims){
	  // get all the unlimited dimension ids in this group
	  vector<int> unlimdimid(numlimdims);
	  ncxxCheck(nc_inq_unlimdims(groupId,&numlimdims,&unlimdimid[0]),__FILE__,__LINE__);
	  vector<int>::iterator it;
	  // now look to see if this dimension is unlimited
	  it = find(unlimdimid.begin(),unlimdimid.end(),myId);
	  return it != unlimdimid.end();
  }
  return false;
}


// gets the name of the dimension.
string NcxxDim::getName() const
{
  char dimName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_dimname(groupId, myId, dimName),__FILE__,__LINE__);
  return string(dimName);
}

// renames this dimension.
void NcxxDim::rename(const string& name)
{
  ncxxCheck(nc_rename_dim(groupId, myId, name.c_str()),__FILE__,__LINE__);
}
