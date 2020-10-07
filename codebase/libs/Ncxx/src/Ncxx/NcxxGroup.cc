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

#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxVar.hh>
#include <Ncxx/NcxxDim.hh>
#include <Ncxx/NcxxVlenType.hh>
#include <Ncxx/NcxxCompoundType.hh>
#include <Ncxx/NcxxOpaqueType.hh>
#include <Ncxx/NcxxGroupAtt.hh>
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
#include <Ncxx/NcxxCheck.hh>
using namespace std;

//  Global comparator operator ==============
// comparator operator
bool operator<(const NcxxGroup& lhs,const NcxxGroup& rhs)
{
  return false;
}

// comparator operator
bool operator>(const NcxxGroup& lhs,const NcxxGroup& rhs)
{
  return true;
}

/////////////////////////////////////////////

NcxxGroup::~NcxxGroup()
{
}

// Default constructor generates a null object.
NcxxGroup::NcxxGroup() :
        NcxxErrStr(),
        nullObject(true),
        myId(-1),
        _useProposedStandardName(false)
{
}


// constructor
NcxxGroup::NcxxGroup(const int groupId) :
        NcxxErrStr(),
        nullObject(false),
        myId(groupId),
        _useProposedStandardName(false)
{ 
}

// assignment operator
NcxxGroup& NcxxGroup::operator=(const NcxxGroup & rhs)
{
  if (&rhs == this) {
    return *this;
  }
  _errStr = rhs._errStr;
  nullObject = rhs.nullObject;
  myId = rhs.myId;
  _useProposedStandardName = rhs._useProposedStandardName;
  return *this;
}

// The copy constructor.
NcxxGroup::NcxxGroup(const NcxxGroup& rhs):
        nullObject(rhs.nullObject),
        myId(rhs.myId)
{
  _errStr = rhs._errStr;
  _useProposedStandardName = rhs._useProposedStandardName;
}


// equivalence operator
bool NcxxGroup::operator==(const NcxxGroup & rhs) const
{
  if(nullObject)
    return nullObject == rhs.nullObject;
  else
    return myId == rhs.myId;
}

//  !=  operator
bool NcxxGroup::operator!=(const NcxxGroup & rhs) const
{
  return !(*this == rhs);
}


// /////////////
// NcxxGroup-related methods
// /////////////

// Get the group name.
string NcxxGroup::getName(bool fullName) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getName on a Null group",
                      __FILE__, __LINE__);
  }
  string groupName;
  if(fullName){
    // return full name of group with foward "/" separarating sub-groups.
    size_t lenp;
    ncxxCheck(nc_inq_grpname_len(myId,&lenp), __FILE__, __LINE__);
    char* charName= new char[lenp+1];
    ncxxCheck(nc_inq_grpname_full(myId,&lenp,charName), __FILE__, __LINE__);
    groupName = charName;
    delete[] charName;
  }
  else {
    // return the (local) name of this group.
    char charName[NC_MAX_NAME+1];
    ncxxCheck(nc_inq_grpname(myId,charName), __FILE__, __LINE__);
    groupName = charName;
  }
  return groupName;
}

// returns true if this is the root group.
bool NcxxGroup::isRootGroup()  const{
  bool result = getName() == "/";
  return result;
}

// Get the parent group.
NcxxGroup NcxxGroup::getParentGroup() const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getParentGroup on a Null group",
                      __FILE__, __LINE__);
  }
  try {
    int parentId;
    ncxxCheck(nc_inq_grp_parent(myId,&parentId), __FILE__, __LINE__,
              "NcxxGroup::getParentGroup()", getName());
    NcxxGroup ncGroupParent(parentId);
    return ncGroupParent;
  }
  catch (NcxxEnoGrp& e) {
    // no group found, so return null group
    return NcxxGroup();
  }
}


// Get the group id.
int  NcxxGroup::getId() const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getId on a Null group",
                      __FILE__, __LINE__);
  }
  return myId;
}

// Get the number of NcxxGroup objects.
int NcxxGroup::getGroupCount(NcxxGroup::GroupLocation location) const {

  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getGroupCount on a Null group",
                      __FILE__, __LINE__);
  }

  // initialize group counter
  int ngroups=0;

  // record this group
  if(location == ParentsAndCurrentGrps || location == AllGrps) {
    ngroups ++;
  }

  // number of children in current group
  if(location == ChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    int numgrps;
    int* ncids=NULL;
    ncxxCheck(nc_inq_grps(getId(), &numgrps,ncids), __FILE__, __LINE__, getName());
    ngroups += numgrps;
  }

  // search in parent groups
  if(location == ParentsGrps || location == ParentsAndCurrentGrps || location == AllGrps ) {
    multimap<string,NcxxGroup> groups(getGroups(ParentsGrps));
    ngroups += groups.size();
  }


  // get the number of all children that are childreof children
  if(location == ChildrenOfChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    multimap<string,NcxxGroup> groups(getGroups(ChildrenOfChildrenGrps));
    ngroups += groups.size();
  }

  return ngroups;
}


// Get the set of child NcxxGroup objects.

multimap<std::string,NcxxGroup> NcxxGroup::getGroups(NcxxGroup::GroupLocation location) const 
{

  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getGroups on a Null group",
                      __FILE__, __LINE__);
  }

  // create a container to hold the NcxxGroup's.
  multimap<string,NcxxGroup> ncGroups;

  // record this group
  if(location == ParentsAndCurrentGrps || location == AllGrps) {
    ncGroups.insert(pair<const string,NcxxGroup>(getName(),*this));
  }

  // the child groups of the current group
  if(location == ChildrenGrps || location == AllChildrenGrps || location == AllGrps ) {
    // get the number of groups
    int groupCount = getGroupCount();
    if (groupCount){
      vector<int> ncids(groupCount);
      int* numgrps=NULL;
      // now get the id of each NcxxGroup and populate the ncGroups container.
      ncxxCheck(nc_inq_grps(myId, numgrps,&ncids[0]), __FILE__, __LINE__);
      for(int i=0; i<groupCount;i++){
        NcxxGroup tmpGroup(ncids[i]);
        ncGroups.insert(pair<const string,NcxxGroup>(tmpGroup.getName(),tmpGroup));
      }
    }
  }

  // search in parent groups.
  if(location == ParentsGrps ||
     location == ParentsAndCurrentGrps 
     || location == AllGrps ) {
    NcxxGroup tmpGroup(*this);
    if(!tmpGroup.isRootGroup()) {
      while(1) {
	const NcxxGroup parentGroup(tmpGroup.getParentGroup());
	if(parentGroup.isNull()) break;
	ncGroups.insert(pair<const string,NcxxGroup>(parentGroup.getName(),parentGroup));
	tmpGroup=parentGroup;
      }
    }
  }

  // search in child groups of the children
  if(location == ChildrenOfChildrenGrps ||
     location == AllChildrenGrps ||
     location == AllGrps ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(ChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxGroup> childGroups(it->second.getGroups(AllChildrenGrps));
      ncGroups.insert(childGroups.begin(),childGroups.end());
    }
  }

  return ncGroups;
}

// Get the named child NcxxGroup object.

NcxxGroup NcxxGroup::getGroup(const string& name,
                              NcxxGroup::GroupLocation location) const
{
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getGroup on a Null group", 
                      __FILE__, __LINE__);
  }
  multimap<string,NcxxGroup> ncGroups(getGroups(location));
  pair<multimap<string,NcxxGroup>::iterator,multimap<string,NcxxGroup>::iterator> ret;
  ret = ncGroups.equal_range(name);
  if(ret.first == ret.second)
    return NcxxGroup();  // null group is returned
  else
    return ret.first->second;
}



// Get all NcxxGroup objects with a given name.
set<NcxxGroup> NcxxGroup::getGroups(const std::string& name,
                                    NcxxGroup::GroupLocation location) const {

  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getGroups on a Null group",
                      __FILE__, __LINE__);
  }

  // get the set of ncGroups in this group and above.
  multimap<std::string,NcxxGroup> ncGroups(getGroups(location));
  pair<multimap<string,NcxxGroup>::iterator,multimap<string,NcxxGroup>::iterator> ret;
  multimap<string,NcxxGroup>::iterator it;
  ret = ncGroups.equal_range(name);
  set<NcxxGroup> tmpGroup;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpGroup.insert(it->second);
  }
  return tmpGroup;

}

// Add a new child NcxxGroup object.
NcxxGroup NcxxGroup::addGroup(const string& name) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::addGroup on a Null group", 
                      __FILE__, __LINE__);
  }
  int new_ncid;
  ncxxCheck(nc_def_grp(myId,const_cast<char*> (name.c_str()),&new_ncid),
            __FILE__, __LINE__,
            "NcxxGroup::addGroup()", getName(), name);
  return NcxxGroup(new_ncid);
}



// /////////////
// NcxxVar-related accessors
// /////////////

// Get the number of NcxxVar objects in this group.
int NcxxGroup::getVarCount(NcxxGroup::Location location) const {
  
  // search in current group.
  NcxxGroup tmpGroup(*this);
  int nvars=0;
  // search in current group
  if((location == ParentsAndCurrent ||
      location == ChildrenAndCurrent ||
      location == Current ||
      location ==All) && !tmpGroup.isNull()) {
    ncxxCheck(nc_inq_nvars(tmpGroup.getId(), &nvars), 
              __FILE__, __LINE__,
              "NcxxGroup::getVarCount()", getName());
  }

  // search recursively in all parent groups.
  if(location == Parents ||
     location == ParentsAndCurrent
     || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      int nvarsp;
      ncxxCheck(nc_inq_nvars(tmpGroup.getId(), &nvarsp),
                __FILE__, __LINE__,
                "NcxxGroup::getVarCount", getName());
      nvars += nvarsp;
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recursively in all child groups
  if(location == ChildrenAndCurrent ||
     location == Children ||
     location == All) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      nvars += it->second.getVarCount(ChildrenAndCurrent);
    }
  }
  return nvars;
}

// Get the collection of NcxxVar objects.

multimap<std::string,NcxxVar> NcxxGroup::getVars(NcxxGroup::Location location) const 
{

  // create a container to hold the NcxxVar's.
  multimap<string,NcxxVar> ncVars;
  
  // search in current group.
  NcxxGroup tmpGroup(*this);
  if((location == ParentsAndCurrent ||
      location == ChildrenAndCurrent ||
      location == Current ||
      location ==All) && !tmpGroup.isNull()) {
    // get the number of variables.
    int varCount = getVarCount();
    if (varCount){
      // now get the name of each NcxxVar object and populate the ncVars container.
      int* nvars=NULL;
      vector<int> varids(varCount);
      ncxxCheck(nc_inq_varids(myId, nvars,&varids[0]), 
                __FILE__, __LINE__,
                "NcxxGroup::getVars()", getName());
      for(int i=0; i<varCount;i++){
        NcxxVar tmpVar(*this,varids[i]);
        ncVars.insert(pair<const string,NcxxVar>(tmpVar.getName(),tmpVar));
      }
    }
  }


  // search recursively in all parent groups.
  if(location == Parents ||
     location == ParentsAndCurrent ||
     location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      // get the number of variables
      int varCount = tmpGroup.getVarCount();
      if (varCount){
        // now get the name of each NcxxVar object and populate the ncVars container.
        int* nvars=NULL;
        vector<int> varids(varCount);
        ncxxCheck(nc_inq_varids(tmpGroup.getId(), nvars,&varids[0]), 
                  __FILE__, __LINE__,
                  "NcxxGroup::getVars()", getName());
        for(int i=0; i<varCount;i++){
          NcxxVar tmpVar(tmpGroup,varids[i]);
          ncVars.insert(pair<const string,NcxxVar>(tmpVar.getName(),tmpVar));
        }
      }
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recusively in all child groups.
  if(location == ChildrenAndCurrent ||
     location == Children ||
     location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxVar> vars=it->second.getVars(ChildrenAndCurrent);
      ncVars.insert(vars.begin(),vars.end());
    }
  }

  return ncVars;
}


// Get all NcxxVar objects with a given name.
set<NcxxVar> NcxxGroup::getVars(const string& name,NcxxGroup::Location location) const 
{
  // get the set of ncVars in this group and above.
  multimap<std::string,NcxxVar> ncVars(getVars(location));
  pair<multimap<string,NcxxVar>::iterator,multimap<string,NcxxVar>::iterator> ret;
  multimap<string,NcxxVar>::iterator it;
  ret = ncVars.equal_range(name);
  set<NcxxVar> tmpVar;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpVar.insert(it->second);
  }
  return tmpVar;
}


// Get the named NcxxVar object.
NcxxVar NcxxGroup::getVar(const string& name,NcxxGroup::Location location) const 
{
  multimap<std::string,NcxxVar> ncVars(getVars(location));
  pair<multimap<string,NcxxVar>::iterator,multimap<string,NcxxVar>::iterator> ret;
  ret = ncVars.equal_range(name);
  if(ret.first == ret.second) {
    // no matching netCDF variable found so return null object.
    return NcxxVar();
  } else {
    return ret.first->second;
  }
}

// Adds a new netCDF scalar variable.
NcxxVar NcxxGroup::addVar(const std::string& name, const NcxxType& ncType) const {
  return addVar(name, ncType, std::vector<NcxxDim>());
}

// Add a new netCDF variable.
NcxxVar NcxxGroup::addVar(const string& name, 
                          const string& typeName,
                          const string& dimName) const {
  ncxxCheckDefineMode(myId);

  // get an NcxxType object with the given type name.
  NcxxType tmpType(getType(typeName,NcxxGroup::ParentsAndCurrent));
  if(tmpType.isNull()) {
    throw NcxxNullType("Attempt to invoke NcxxGroup::addVar failed: "
                       "typeName must be defined in either the current "
                       "group or a parent group",
                       __FILE__, __LINE__);
  }

  // get a NcxxDim object with the given dimension name
  NcxxDim tmpDim(getDim(dimName,NcxxGroup::ParentsAndCurrent));
  if(tmpDim.isNull()) {
    throw NcxxNullDim("Attempt to invoke NcxxGroup::addVar failed: "
                      "dimName must be defined in either the current "
                      "group or a parent group", __FILE__, __LINE__);
  }

  // finally define a new netCDF  variable
  int varId;
  int dimId(tmpDim.getId());
  ncxxCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),1,&dimId,&varId),
            __FILE__, __LINE__,
            "NcxxGroup::addVar()", getName(), name);
  // return an NcxxVar object for this new variable
  return NcxxVar(*this,varId);
}


// Add a new netCDF variable.
NcxxVar NcxxGroup::addVar(const string& name,
                          const NcxxType& ncType,
                          const NcxxDim& ncDim) const {
  ncxxCheckDefineMode(myId);

  // check NcxxType object is valid
  if(ncType.isNull()) {
    throw NcxxNullType("Attempt to invoke NcxxGroup::addVar with a Null NcxxType object",
                       __FILE__, __LINE__);
  }
  NcxxType tmpType(getType(ncType.getName(),NcxxGroup::ParentsAndCurrent));
  if(tmpType.isNull()) {
    throw NcxxNullType("Attempt to invoke NcxxGroup::addVar failed: "
                       "NcxxType must be defined in either the "
                       "current group or a parent group",
                       __FILE__, __LINE__);
  }

  // check NcxxDim object is valid
  if(ncDim.isNull()) {
    throw NcxxNullDim("Attempt to invoke NcxxGroup::addVar with a Null NcxxDim object", 
                      __FILE__, __LINE__);
  }
  NcxxDim tmpDim(getDim(ncDim.getName(),NcxxGroup::ParentsAndCurrent));
  if(tmpDim.isNull()) {
    throw NcxxNullDim("Attempt to invoke NcxxGroup::addVar failed: "
                      "NcxxDim must be defined in either the "
                      "current group or a parent group",
                      __FILE__, __LINE__);
  }

  // finally define a new netCDF variable
  int varId;
  int dimId(tmpDim.getId());
  ncxxCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),1,&dimId,&varId),
            __FILE__, __LINE__,
            "NcxxGroup::addVar()", getName(), name);
  // return an NcxxVar object for this new variable
  return NcxxVar(*this,varId);
}


// Add a new netCDF multi-dimensional variable.
NcxxVar NcxxGroup::addVar(const string& name, const string& typeName, const vector<string>& dimNames) const {
  ncxxCheckDefineMode(myId);

  // get an NcxxType object with the given name.
  NcxxType tmpType(getType(typeName,NcxxGroup::ParentsAndCurrent));
  if(tmpType.isNull()) throw NcxxNullType("Attempt to invoke NcxxGroup::addVar failed: typeName must be defined in either the current group or a parent group", __FILE__, __LINE__);

  // get a set of NcxxDim objects corresponding to the given dimension names.
  vector<int> dimIds;
  dimIds.reserve(dimNames.size());
  for (size_t i=0; i<dimNames.size();i++){
    NcxxDim tmpDim(getDim(dimNames[i],NcxxGroup::ParentsAndCurrent));
    if(tmpDim.isNull()) throw NcxxNullDim("Attempt to invoke NcxxGroup::addVar failed: dimNames must be defined in either the current group or a parent group", __FILE__, __LINE__);
    dimIds.push_back(tmpDim.getId());
  }

  // finally define a new netCDF variable
  int varId;
  int *dimIdsPtr = dimIds.empty() ? 0 : &dimIds[0];
  ncxxCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),dimIds.size(), dimIdsPtr,&varId),
            __FILE__, __LINE__,
            "NcxxGroup::addVar()", getName(), name);
  // return an NcxxVar object for this new variable
  return NcxxVar(*this,varId);
}

// Add a new netCDF multi-dimensional variable.
NcxxVar NcxxGroup::addVar(const string& name, const NcxxType& ncType, const vector<NcxxDim>& ncDimVector) const {
  ncxxCheckDefineMode(myId);

  // check NcxxType object is valid
  if(ncType.isNull()) throw NcxxNullType("Attempt to invoke NcxxGroup::addVar with a Null NcxxType object", __FILE__, __LINE__);
  NcxxType tmpType(getType(ncType.getName(),NcxxGroup::ParentsAndCurrent));
  if(tmpType.isNull()) throw NcxxNullType("Attempt to invoke NcxxGroup::addVar failed: NcxxType must be defined in either the current group or a parent group", __FILE__, __LINE__);

  // check NcxxDim objects are valid
  vector<NcxxDim>::const_iterator iter;
  vector<int> dimIds;
  dimIds.reserve(ncDimVector.size());
  for (iter=ncDimVector.begin();iter < ncDimVector.end(); iter++) {
    if(iter->isNull()) throw NcxxNullDim("Attempt to invoke NcxxGroup::addVar with a Null NcxxDim object", __FILE__, __LINE__);
    NcxxDim tmpDim(getDim(iter->getName(),NcxxGroup::ParentsAndCurrent));
    if(tmpDim.isNull()) throw NcxxNullDim("Attempt to invoke NcxxGroup::addVar failed: NcxxDim must be defined in either the current group or a parent group", __FILE__, __LINE__);
    dimIds.push_back(tmpDim.getId());
  }

  // finally define a new netCDF variable
  int varId;
  int *dimIdsPtr = dimIds.empty() ? 0 : &dimIds[0];
  ncxxCheck(nc_def_var(myId,name.c_str(),tmpType.getId(),dimIds.size(), dimIdsPtr,&varId),
            __FILE__, __LINE__,
            "NcxxGroup::addVar()", getName(), name);
  // return an NcxxVar object for this new variable
  return NcxxVar(*this,varId);
}


// /////////////
// NcxxAtt-related methods
// /////////////

// Get the number of group attributes.
int NcxxGroup::getAttCount(NcxxGroup::Location location) const {

  // search in current group.
  NcxxGroup tmpGroup(*this);
  int ngatts=0;
  // search in current group
  if((location == ParentsAndCurrent || location == ChildrenAndCurrent || location == Current || location ==All) && !tmpGroup.isNull()) {
    ncxxCheck(nc_inq_natts(tmpGroup.getId(), &ngatts),
              __FILE__, __LINE__,
              "NcxxGroup::getAttCount()", getName());
  }

  // search recursively in all parent groups.
  if(location == Parents || location == ParentsAndCurrent || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      int ngattsp;
      ncxxCheck(nc_inq_natts(tmpGroup.getId(), &ngattsp), 
                __FILE__, __LINE__,
                "NcxxGroup::getAttCount()", getName());
      ngatts += ngattsp;
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recursively in all child groups
  if(location == ChildrenAndCurrent || location == Children || location == All) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      ngatts += it->second.getAttCount(ChildrenAndCurrent);
    }
  }

  return ngatts;
}

// Get the collection of NcxxGroupAtt objects.
multimap<std::string,NcxxGroupAtt> NcxxGroup::getAtts(NcxxGroup::Location location) const {

  // create a container to hold the NcxxAtt's.
  multimap<string,NcxxGroupAtt> ncAtts;

  // search in current group.
  NcxxGroup tmpGroup(*this);
  if((location == ParentsAndCurrent || location == ChildrenAndCurrent || location == Current || location ==All) && !tmpGroup.isNull()) {
    // get the number of attributes
    int attCount = tmpGroup.getAttCount();
    // now get the name of each NcxxAtt and populate the ncAtts container.
    for(int i=0; i<attCount;i++){
      char charName[NC_MAX_NAME+1];
      ncxxCheck(nc_inq_attname(tmpGroup.getId(),NC_GLOBAL,i,charName), 
                __FILE__, __LINE__,
                "NcxxGroup::getAtts()", getName());
      NcxxGroupAtt tmpAtt(tmpGroup.getId(),i);
      ncAtts.insert(pair<const string,NcxxGroupAtt>(string(charName),tmpAtt));
    }
  }

  // search recursively in all parent groups.
  if(location == Parents || location == ParentsAndCurrent || location ==All) {
    tmpGroup=getParentGroup();
    while(!tmpGroup.isNull()) {
      // get the number of attributes
      int attCount = tmpGroup.getAttCount();
      // now get the name of each NcxxAtt and populate the ncAtts container.
      for(int i=0; i<attCount;i++){
        char charName[NC_MAX_NAME+1];
        ncxxCheck(nc_inq_attname(tmpGroup.getId(),NC_GLOBAL,i,charName), 
                  __FILE__, __LINE__,
                  "NcxxGroup::getAtts()", getName());
        NcxxGroupAtt tmpAtt(tmpGroup.getId(),i);
        ncAtts.insert(pair<const string,NcxxGroupAtt>(string(charName),tmpAtt));
      }
      // continue loop with the parent.
      tmpGroup=tmpGroup.getParentGroup();
    }
  }

  // search recusively in all child groups.
  if(location == ChildrenAndCurrent || location == Children  || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxGroupAtt> atts=it->second.getAtts(ChildrenAndCurrent);
      ncAtts.insert(atts.begin(),atts.end());
    }
  }

  return ncAtts;
}

// Get the named NcxxGroupAtt object.
NcxxGroupAtt NcxxGroup::getAtt(const std::string& name,NcxxGroup::Location location) const {
  multimap<std::string,NcxxGroupAtt> ncAtts(getAtts(location));
  pair<multimap<string,NcxxGroupAtt>::iterator,multimap<string,NcxxGroupAtt>::iterator> ret;
  ret = ncAtts.equal_range(name);
  if(ret.first == ret.second)
    // no matching groupAttribute so return null object.
    return NcxxGroupAtt();
  else
    return ret.first->second;
}

// Get all NcxxGroupAtt objects with a given name.
set<NcxxGroupAtt> NcxxGroup::getAtts(const string& name,NcxxGroup::Location location) const {
  // get the set of ncGroupAtts in this group and above.
  multimap<std::string,NcxxGroupAtt> ncAtts(getAtts(location));
  pair<multimap<string,NcxxGroupAtt>::iterator,multimap<string,NcxxGroupAtt>::iterator> ret;
  multimap<string,NcxxGroupAtt>::iterator it;
  ret = ncAtts.equal_range(name);
  set<NcxxGroupAtt> tmpAtt;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpAtt.insert(it->second);
  }
  return tmpAtt;
}




//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const string& dataValues) const {
  ncxxCheckDefineMode(myId);
  ncxxCheck(nc_put_att_text(myId,NC_GLOBAL,name.c_str(),dataValues.size(),dataValues.c_str()), 
            __FILE__, __LINE__,
            "NcxxGroup::putAtt(string)", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned char* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt(unsigned char*)", getName(), name);
  else
    ncxxCheck(nc_put_att_uchar(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt(unsigned char*)", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const signed char* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_schar(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, short datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_short(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, int datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_int(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, long datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_long(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, float datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_float(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, double datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_double(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, unsigned short datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_ushort(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, unsigned int datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_uint(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, long long datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_longlong(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, unsigned long long datumValue) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_ulonglong(myId,NC_GLOBAL,name.c_str(),type.getId(),1,&datumValue), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const short* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_short(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const int* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_int(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const long* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_long(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const float* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_float(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const double* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_double(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned short* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_ushort(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned int* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_uint(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const long long* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_longlong(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned long long* dataValues) const {
  ncxxCheckDefineMode(myId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  else
    ncxxCheck(nc_put_att_ulonglong(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
              __FILE__, __LINE__,
              "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}


//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, size_t len, const char** dataValues) const {
  ncxxCheckDefineMode(myId);
  ncxxCheck(nc_put_att_string(myId,NC_GLOBAL,name.c_str(),len,dataValues), 
            __FILE__, __LINE__,
            "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}

//  Creates a new NetCDF group attribute or if already exisiting replaces it.
NcxxGroupAtt NcxxGroup::putAtt(const string& name, const NcxxType& type, size_t len, const void* dataValues) const {
  ncxxCheckDefineMode(myId);
  ncxxCheck(nc_put_att(myId,NC_GLOBAL,name.c_str(),type.getId(),len,dataValues), 
            __FILE__, __LINE__,
            "NcxxGroup::putAtt()", getName(), name);
  // finally instantiate this attribute and return
  return getAtt(name);
}



// /////////////
// NcxxDim-related methods
// /////////////

// Get the number of NcxxDim objects.
int NcxxGroup::getDimCount(NcxxGroup::Location location) const {

  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getDimCount on a Null group", 
                      __FILE__, __LINE__);
  }

  // intialize counter
  int ndims=0;

  // search in current group
  if(location == Current ||
     location == ParentsAndCurrent ||
     location == ChildrenAndCurrent ||
     location == All ) {
    int ndimsp;
    ncxxCheck(nc_inq_ndims(getId(), &ndimsp), 
              __FILE__, __LINE__,
              "NcxxGroup::(getDimCount)", getName());
    ndims += ndimsp;
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ndims += it->second.getDimCount();
    }
  }

  // search in child groups.
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ndims += it->second.getDimCount();
    }
  }
  return ndims;
}


// Get the set of NcxxDim objects.
multimap<string,NcxxDim> NcxxGroup::getDims(NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getDims on a Null group", 
                      __FILE__, __LINE__);
  }
  // create a container to hold the NcxxDim's.
  multimap<string,NcxxDim> ncDims;

  // search in current group
  if(location == Current ||
     location == ParentsAndCurrent ||
     location == ChildrenAndCurrent ||
     location == All ) {
    int dimCount = getDimCount();
    if (dimCount){
      vector<int> dimids(dimCount);
      ncxxCheck(nc_inq_dimids(getId(), &dimCount, &dimids[0], 0), 
                __FILE__, __LINE__,
                "NcxxGroup::(getDims)", getName());
      // now get the name of each NcxxDim and populate the nDims container.
      for(int i=0; i<dimCount;i++){
        NcxxDim tmpDim(*this,dimids[i]);
        ncDims.insert(pair<const string,NcxxDim>(tmpDim.getName(),tmpDim));
      }
    }
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxDim> dimTmp(it->second.getDims());
      ncDims.insert(dimTmp.begin(),dimTmp.end());
    }
  }

  // search in child groups (makes recursive calls).
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxDim> dimTmp(it->second.getDims());
      ncDims.insert(dimTmp.begin(),dimTmp.end());
    }
  }

  return ncDims;
}



// Get the named NcxxDim object.
NcxxDim NcxxGroup::getDim(const string& name,NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getDim on a Null group",
                      __FILE__, __LINE__);
  }
  multimap<string,NcxxDim> ncDims(getDims(location));
  pair<multimap<string,NcxxDim>::iterator,multimap<string,NcxxDim>::iterator> ret;
  ret = ncDims.equal_range(name);
  if(ret.first == ret.second) {
    return NcxxDim(); // null group is returned
  } else {
    return ret.first->second;
  }
}


// Get all NcxxDim objects with a given name.
set<NcxxDim> NcxxGroup::getDims(const string& name,NcxxGroup::Location location) const {
  
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getDims on a Null group",
                      __FILE__, __LINE__);
  }
  // get the set of ncDims in this group and above.
  multimap<string,NcxxDim> ncDims(getDims(location));
  pair<multimap<string,NcxxDim>::iterator,multimap<string,NcxxDim>::iterator> ret;
  multimap<string,NcxxDim>::iterator it;
  ret = ncDims.equal_range(name);
  set<NcxxDim> tmpDim;
  for (it=ret.first; it!=ret.second; ++it) {
    tmpDim.insert(it->second);
  }
  return tmpDim;
}

// Add a new NcxxDim object.
NcxxDim NcxxGroup::addDim(const string& name, size_t dimSize) const {
  ncxxCheckDefineMode(myId);
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::addDim on a Null group",
                      __FILE__, __LINE__);
  }
  int dimId;
  ncxxCheck(nc_def_dim(myId,name.c_str(),dimSize,&dimId), 
            __FILE__, __LINE__,
            "NcxxGroup::addDim()", getName(), name);
  // finally return NcxxDim object for this new variable
  return NcxxDim(*this,dimId);
}

// Add a new NcxxDim object with unlimited size..
NcxxDim NcxxGroup::addDim(const string& name) const {
  ncxxCheckDefineMode(myId);
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::addDim on a Null group",
                      __FILE__, __LINE__);
  }
  int dimId;
  ncxxCheck(nc_def_dim(myId,name.c_str(),NC_UNLIMITED,&dimId), 
            __FILE__, __LINE__,
            "NcxxGroup::addDim()", getName(), name);
  // finally return NcxxDim object for this new variable
  return NcxxDim(*this,dimId);
}





// /////////////
// type-object related methods
// /////////////

// Gets the number of type objects.
int NcxxGroup::getTypeCount(NcxxGroup::Location location) const {

  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getTypeCount on a Null group",
                      __FILE__, __LINE__);
  }

  // intialize counter
  int ntypes=0;

  // search in current group
  if(location == Current ||
     location == ParentsAndCurrent ||
     location == ChildrenAndCurrent ||
     location == All ) {
    int ntypesp;
    int* typeidsp=NULL;
    ncxxCheck(nc_inq_typeids(getId(), &ntypesp,typeidsp), 
              __FILE__, __LINE__,
              "NcxxGroup::getTypeCount()", getName());
    ntypes+= ntypesp;
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount();
    }
  }

  // search in child groups.
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount();
    }
  }
  return ntypes;
}



// Gets the number of type objects with a given enumeration type.
int NcxxGroup::getTypeCount(NcxxType::ncxxType enumType, NcxxGroup::Location location) const {

  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getTypeCount on a Null group",
                      __FILE__, __LINE__);
  }

  // intialize counter
  int ntypes=0;

  // search in current group
  if(location == Current ||
     location == ParentsAndCurrent ||
     location == ChildrenAndCurrent ||
     location == All ) {
    int ntypesp;
    int* typeidsp=NULL;
    ncxxCheck(nc_inq_typeids(getId(), &ntypesp,typeidsp), 
              __FILE__, __LINE__,
              "NcxxGroup::getTypeCount()", getName());
    if (ntypesp){
      vector<int> typeids(ntypesp);
      ncxxCheck(nc_inq_typeids(getId(), &ntypesp,&typeids[0]), 
                __FILE__, __LINE__,
                "NcxxGroup::getTypeCount()", getName());
      for (int i=0; i<ntypesp;i++){
        NcxxType tmpType(*this,typeids[i]);
        if(tmpType.getTypeClass() == enumType) ntypes++;
      }
    }
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount(enumType);
    }
  }

  // search in child groups.
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      ntypes += it->second.getTypeCount(enumType);
    }
  }
  return ntypes;
}


// Gets the collection of NcxxType objects.
multimap<string,NcxxType> NcxxGroup::getTypes(NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getTypes on a Null group",
                      __FILE__, __LINE__);
  }
  // create a container to hold the NcxxType's.
  multimap<string,NcxxType> ncTypes;

  // search in current group
  if(location == Current || location == ParentsAndCurrent || location == ChildrenAndCurrent || location == All ) {
    int typeCount = getTypeCount();
    if (typeCount){
      vector<int> typeids(typeCount);
      ncxxCheck(nc_inq_typeids(getId(), &typeCount,&typeids[0]),
                __FILE__, __LINE__,
                "NcxxGroup::(getTypes)", getName());
      // now get the name of each NcxxType and populate the nTypes container.
      for(int i=0; i<typeCount;i++){
        NcxxType tmpType(*this,typeids[i]);
        ncTypes.insert(pair<const string,NcxxType>(tmpType.getName(),tmpType));
      }
    }
  }

  // search in parent groups.
  if(location == Parents || location == ParentsAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(ParentsGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxType> typeTmp(it->second.getTypes());
      ncTypes.insert(typeTmp.begin(),typeTmp.end());
    }
  }

  // search in child groups (makes recursive calls).
  if(location == Children || location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups(AllChildrenGrps));
    for (it=groups.begin();it!=groups.end();it++) {
      multimap<string,NcxxType> typeTmp(it->second.getTypes());
      ncTypes.insert(typeTmp.begin(),typeTmp.end());
    }
  }

  return ncTypes;
}


// Gets the collection of NcxxType objects with a given name.
set<NcxxType> NcxxGroup::getTypes(const string& name, NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getTypes on a Null group",
                      __FILE__, __LINE__);
  }
  // iterator for the multimap container.
  multimap<string,NcxxType>::iterator it;
  // return argument of equal_range: iterators to lower and upper bounds of the range.
  pair<multimap<string,NcxxType>::iterator,multimap<string,NcxxType>::iterator> ret;
  // get the entire collection of types.
  multimap<string,NcxxType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcxxType> tmpType;
  // get the set of NcxxType objects with a given name
  ret=types.equal_range(name);
  for (it=ret.first;it!=ret.second;it++) {
    tmpType.insert(it->second);
  }
  return tmpType;
}


// Gets the collection of NcxxType objects with a given data type.
set<NcxxType> NcxxGroup::getTypes(NcxxType::ncxxType enumType, NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getTypes on a Null group",
                      __FILE__, __LINE__);
  }
  // iterator for the multimap container.
  multimap<string,NcxxType>::iterator it;
  // get the entire collection of types.
  multimap<string,NcxxType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcxxType> tmpType;
  // get the set of NcxxType objects with a given data type
  for (it=types.begin();it!=types.end();it++) {
    if(it->second.getTypeClass() == enumType) {
      tmpType.insert(it->second);
    }
  }
  return(tmpType);
}


// Gets the collection of NcxxType objects with a given name and data type.
set<NcxxType> NcxxGroup::getTypes(const string& name, NcxxType::ncxxType enumType, NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getTypes on a Null group",
                      __FILE__, __LINE__);
  }
  // iterator for the multimap container.
  multimap<string,NcxxType>::iterator it;
  // return argument of equal_range: iterators to lower and upper bounds of the range.
  pair<multimap<string,NcxxType>::iterator,multimap<string,NcxxType>::iterator> ret;
  // get the entire collection of types.
  multimap<string,NcxxType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcxxType> tmpType;
  // get the set of NcxxType objects with a given name
  ret=types.equal_range(name);
  for (it=ret.first;it!=ret.second;it++) {
    if((*it).second.getTypeClass() == enumType) {
      tmpType.insert(it->second);
    }
  }
  return(tmpType);
}


// Gets the NcxxType object with a given name.
NcxxType NcxxGroup::getType(const string& name, NcxxGroup::Location location) const {
  if(isNull()) {
    throw NcxxNullGrp("Attempt to invoke NcxxGroup::getType on a Null group",
                      __FILE__, __LINE__);
  }
  if(name ==  "byte"    ) return ncxxByte;
  if(name ==  "ubyte"   ) return ncxxUbyte;
  if(name ==  "char"    ) return ncxxChar;
  if(name ==  "short"   ) return ncxxShort;
  if(name ==  "ushort"  ) return ncxxUshort;
  if(name ==  "int"     ) return ncxxInt;
  if(name ==  "uint"    ) return ncxxUint;
  if(name ==  "int64"   ) return ncxxInt64;
  if(name ==  "uint64"  ) return ncxxUint64;
  if(name ==  "float"   ) return ncxxFloat;
  if(name ==  "double"  ) return ncxxDouble;
  if(name ==  "string"  ) return ncxxString;

  // this is a user defined type
  // iterator for the multimap container.
  multimap<string,NcxxType>::iterator it;
  // return argument of equal_range: iterators to lower and upper bounds of the range.
  pair<multimap<string,NcxxType>::iterator,multimap<string,NcxxType>::iterator> ret;
  // get the entire collection of types.
  multimap<string,NcxxType> types(getTypes(location));
  // define STL set object to hold the result
  set<NcxxType> tmpType;
  // get the set of NcxxType objects with a given name
  ret=types.equal_range(name);
  if(ret.first == ret.second)
    return NcxxType();
  else
    return ret.first->second;
}


// Adds a new netCDF Enum type.
NcxxEnumType NcxxGroup::addEnumType(const string& name,NcxxEnumType::ncEnumType baseType) const {
  ncxxCheckDefineMode(myId);
  nc_type typeId;
  ncxxCheck(nc_def_enum(myId, baseType, name.c_str(), &typeId),
            __FILE__, __LINE__,
            "NcxxGroup::addEnumType()", getName(), name);
  NcxxEnumType ncTypeTmp(*this,name);
  return ncTypeTmp;
}


// Adds a new netCDF Vlen type.
NcxxVlenType NcxxGroup::addVlenType(const string& name,NcxxType& baseType) const {
  ncxxCheckDefineMode(myId);
  nc_type typeId;
  ncxxCheck(nc_def_vlen(myId,  const_cast<char*>(name.c_str()),baseType.getId(),&typeId),
            __FILE__, __LINE__,
            "NcxxGroup::addVlenType()", getName(), name);
  NcxxVlenType ncTypeTmp(*this,name);
  return ncTypeTmp;
}


// Adds a new netCDF Opaque type.
NcxxOpaqueType NcxxGroup::addOpaqueType(const string& name, size_t size) const {
  ncxxCheckDefineMode(myId);
  nc_type typeId;
  ncxxCheck(nc_def_opaque(myId, size,const_cast<char*>(name.c_str()), &typeId),
            __FILE__, __LINE__,
            "NcxxGroup::addOpaqueType()", getName(), name);
  NcxxOpaqueType ncTypeTmp(*this,name);
  return ncTypeTmp;
}

// Adds a new netCDF UserDefined type.
NcxxCompoundType NcxxGroup::addCompoundType(const string& name, size_t size) const {
  ncxxCheckDefineMode(myId);
  nc_type typeId;
  ncxxCheck(nc_def_compound(myId, size,const_cast<char*>(name.c_str()),&typeId),
            __FILE__, __LINE__,
            "NcxxGroup::addCompoundType()", getName(), name);
  NcxxCompoundType ncTypeTmp(*this,name);
  return ncTypeTmp;
}


// Get the collection of coordinate variables.
map<string,NcxxGroup> NcxxGroup::getCoordVars(NcxxGroup::Location location) const {
  map<string,NcxxGroup> coordVars;

  // search in current group and parent groups.
  NcxxGroup tmpGroup(*this);
  multimap<string,NcxxDim>::iterator itD;
  multimap<string,NcxxVar>::iterator itV;
  while(1) {
    // get the collection of NcxxDim objects defined in this group.
    multimap<string,NcxxDim> dimTmp(tmpGroup.getDims());
    multimap<string,NcxxVar> varTmp(tmpGroup.getVars());
    for (itD=dimTmp.begin();itD!=dimTmp.end();itD++) {
      string coordName(itD->first);
      itV = varTmp.find(coordName);
      if(itV != varTmp.end()) {
	coordVars.insert(pair<const string,NcxxGroup>(string(coordName),tmpGroup));
      }
    }
    //if(location != ParentsAndCurrent || location != All || tmpGroup.isRootGroup()) {
    if(location != ParentsAndCurrent ||  tmpGroup.isRootGroup()) {
      break;
    }
    // continue loop with the parent.
    tmpGroup=tmpGroup.getParentGroup();
  }

  // search in child groups (makes recursive calls).
  if(location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      map<string,NcxxGroup> coordVarsTmp=getCoordVars(ChildrenAndCurrent);
      coordVars.insert(coordVarsTmp.begin(),coordVarsTmp.end());
    }
  }

  return coordVars;
}

// Get the NcxxDim and NcxxVar object pair for a named coordinate variables.

void NcxxGroup::getCoordVar(string& coordVarName,
                            NcxxDim& ncDim,
                            NcxxVar& ncVar,
                            NcxxGroup::Location location) const {

  // search in current group and parent groups.
  multimap<string,NcxxDim>::iterator itD;
  NcxxGroup tmpGroup(*this);
  multimap<string,NcxxVar>::iterator itV;
  while(1) {
    // get the collection of NcxxDim objects defined in this group.
    multimap<string,NcxxDim> dimTmp(tmpGroup.getDims());
    multimap<string,NcxxVar> varTmp(tmpGroup.getVars());
    itD=dimTmp.find(coordVarName);
    itV=varTmp.find(coordVarName);
    if(itD != dimTmp.end() && itV != varTmp.end()) {
      ncDim=itD->second;
      ncVar=itV->second;
      return;
    }
    
    //if(location != ParentsAndCurrent || location != All || tmpGroup.isRootGroup()) {
    if (location != ParentsAndCurrent || tmpGroup.isRootGroup()) {
      break;
    }
    // continue loop with the parent.
    tmpGroup=tmpGroup.getParentGroup();
  }

  // search in child groups (makes recursive calls).
  if(location == ChildrenAndCurrent || location == All ) {
    multimap<string,NcxxGroup>::iterator it;
    multimap<string,NcxxGroup> groups(getGroups());
    for (it=groups.begin();it!=groups.end();it++) {
      getCoordVar(coordVarName,ncDim,ncVar,ChildrenAndCurrent);
      if(!ncDim.isNull()) break;
    }
  }

  if(ncDim.isNull()) {
    // return null objects as no coordinates variables were obtained.
    NcxxDim dimTmp;
    NcxxVar varTmp;
    ncDim=dimTmp;
    ncVar=varTmp;
    return;
  }

}

///////////////////////////////////////////
// add string global attribute
// Throws NcxxException on failure

void NcxxGroup::addGlobAttr(const string &name, const string &val)
{
  try {
    putAtt(name, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::addGlobalAttr");
    addErrStr("  Cannot add global attr name: ", name);
    addErrStr("  val: ", val);
    addErrStr("  group: ", getName());
    addErrStr("  exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
}

///////////////////////////////////////////
// add int global attribute
// Throws NcxxException on failure

void NcxxGroup::addGlobAttr(const string &name, int val)
{
  try {
    putAtt(name, ncxxInt, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::addGlobAttr");
    addErrStr("  Cannot add global attr name: ", name);
    addErrInt("  val: ", val);
    addErrStr("  group: ", getName());
    addErrStr("  exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
}

///////////////////////////////////////////
// add float global attribute
// Throws NcxxException on failure

void NcxxGroup::addGlobAttr(const string &name, float val)
{
  try {
    putAtt(name, ncxxFloat, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::addGlobAttr");
    addErrStr("  Cannot add global attr name: ", name);
    addErrDbl("  val: ", val, "%g");
    addErrStr("  group: ", getName());
    addErrStr("  exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
}

///////////////////////////////////////////
// add double global attribute
// Throws NcxxException on failure

void NcxxGroup::addGlobAttr(const string &name, double val)
{
  try {
    putAtt(name, ncxxDouble, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::addGlobAttr");
    addErrStr("  Cannot add global attr name: ", name);
    addErrDbl("  val: ", val, "%lg");
    addErrStr("  group: ", getName());
    addErrStr("  exception: ", e.what());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
}

///////////////////////////////////////////
// read a global attribute
// Throws NcxxException on failure

void NcxxGroup::readGlobAttr(const string &name, string &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  try {
    att.getValues(val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  group: ", getName());
    addErrStr("  exception: ", e.what());
    addErrStr("  Cannot read value as string");
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
}

void NcxxGroup::readGlobAttr(const string &name, int &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  no values supplied");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  int *vals = new int[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  Cannot read value as int");
    addErrStr("  group: ", getName());
    delete[] vals;
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  val = vals[0];
  delete[] vals;
}

void NcxxGroup::readGlobAttr(const string &name, float &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  no values supplied");
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  float *vals = new float[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  Cannot read value as float");
    addErrStr("  group: ", getName());
    delete[] vals;
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  val = vals[0];
  delete[] vals;
}

void NcxxGroup::readGlobAttr(const string &name, double &val)
{
  NcxxGroupAtt att = getAtt(name);
  if (att.isNull()) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  size_t nvals = att.getAttLength();
  if (nvals < 1) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  no values supplied");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  double *vals = new double[nvals];
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readGlobAttr");
    addErrStr("  Cannot read global attr name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  Cannot read value as double");
    addErrStr("  group: ", getName());
    delete[] vals;
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  val = vals[0];
  delete[] vals;
}

//////////////////////////////////////////////
// Add scalar var
// Throws NcxxException on failure
// Side effect: var is set

NcxxVar NcxxGroup::addVar(const string &name, 
                          const string &standardName,
                          const string &longName,
                          NcxxType ncType, 
                          const string &units /* = "" */,
                          bool isMetadata /* = false */)
  
{
  
  vector<NcxxDim> dims; // 0 length - for scalar
  
  return addVar(name, standardName, longName,
                ncType, dims, units, isMetadata);
  
}

///////////////////////////////////////
// Add 1-D array var
// Throws NcxxException on failure
// Side effect: var is set

NcxxVar NcxxGroup::addVar(const string &name, 
                          const string &standardName,
                          const string &longName,
                          NcxxType ncType, 
                          NcxxDim &dim, 
                          const string &units /* = "" */,
                          bool isMetadata /* = false */)
  
{
  
  vector<NcxxDim> dims;
  dims.push_back(dim);
  
  return addVar(name, standardName, longName,
                ncType, dims, units, isMetadata);

}

///////////////////////////////////////
// Add 2-D array var
// Throws NcxxException on failure
// Side effect: var is set

NcxxVar NcxxGroup::addVar(const string &name,
                          const string &standardName,
                          const string &longName,
                          NcxxType ncType,
                          NcxxDim &dim0,
                          NcxxDim &dim1,
                          const string &units /* = "" */,
                          bool isMetadata /* = false */)
{
  
  vector<NcxxDim> dims;
  dims.push_back(dim0);
  dims.push_back(dim1);

  return addVar(name, standardName, longName,
                ncType, dims, units, isMetadata);
  
}

///////////////////////////////////////
// Add 3-D array var
// Throws NcxxException on failure
// Side effect: var is set

NcxxVar NcxxGroup::addVar(const string &name,
                          const string &standardName,
                          const string &longName,
                          NcxxType ncType,
                          NcxxDim &dim0,
                          NcxxDim &dim1,
                          NcxxDim &dim2,
                          const string &units /* = "" */,
                          bool isMetadata /* = false */)
{
  
  vector<NcxxDim> dims;
  dims.push_back(dim0);
  dims.push_back(dim1);
  dims.push_back(dim2);

  return addVar(name, standardName, longName,
                ncType, dims, units, isMetadata);
  
}

///////////////////////////////////////
// Add var in multiple-dimensions
// Throws NcxxException on failure
// Side effect: var is set

NcxxVar NcxxGroup::addVar(const string &name,
                          const string &standardName,
                          const string &longName,
                          NcxxType ncType,
                          vector<NcxxDim> &dims,
                          const string &units /* = "" */,
                          bool isMetadata /* = false */)
{

  NcxxVar var = addVar(name, ncType, dims);
  nc_type vtype = ncType.getId();
  if (var.isNull()) {
    addErrStr("ERROR - NcxxGroup::addVar");
    addErrStr("  Cannot add var, name: ", name);
    addErrStr("  Type: ", Ncxx::ncTypeToStr(vtype));
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  if (standardName.length() > 0) {
    if (_useProposedStandardName) {
      var.addScalarAttr("proposed_standard_name", standardName);
    } else {
      var.addScalarAttr("standard_name", standardName);
    }
  }
  
  if (longName.length() > 0) {
    var.addScalarAttr("long_name", longName);
  }
  
  if (units.length() > 0 || !isMetadata) {
    var.addScalarAttr("units", units);
  }
  
  if (isMetadata) {
    var.setMetaFillValue();
  } else {
    var.setDefaultFillValue();
  }

  return var;

}

/////////////////////////////////////
// read int variable, set var and val
// throws NcxxException on failure
// returns var

NcxxVar NcxxGroup::readIntVar(const string &name, int &val,
                              int missingVal, bool required)
  
{

  // init to missing

  val = missingVal;

  // get var

  NcxxVar var = getVar(name);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return var;
    } else {
      addErrStr("ERROR - NcxxGroup::readIntVar");
      addErrStr("  Cannot read variable, name: ", name);
      addErrStr("  group: ", getName());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }
  }

  // check size
  
  if (var.numVals() < 1) {
    addErrStr("ERROR - NcxxGroup::readIntVar");
    addErrStr("  variable name: ", name);
    addErrStr("  variable has no data");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  vector<int> vals;
  vals.resize(var.numVals());
  try {
    var.getVal(&vals[0]);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readIntVar");
    addErrStr("  cannot read variable, name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  val = vals[0];

  return var;

}

/////////////////////////////////////
// read float variable, set var and val
// throws NcxxException on failure
// returns var

NcxxVar NcxxGroup::readFloatVar(const string &name, float &val,
                                float missingVal, bool required)
  
{

  // init to missing

  val = missingVal;

  // get var

  NcxxVar var = getVar(name);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return var;
    } else {
      addErrStr("ERROR - NcxxGroup::readFloatVar");
      addErrStr("  Cannot read variable, name: ", name);
      addErrStr("  group: ", getName());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }
  }

  // check size
  
  if (var.numVals() < 1) {
    addErrStr("ERROR - NcxxGroup::readFloatVar");
    addErrStr("  variable name: ", name);
    addErrStr("  variable has no data");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  vector<float> vals;
  vals.resize(var.numVals());
  try {
    var.getVal(&vals[0]);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readFloatVar");
    addErrStr("  cannot read variable, name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  val = vals[0];

  return var;

}

/////////////////////////////////////
// read double variable, set var and val
// throws NcxxException on failure
// returns var

NcxxVar NcxxGroup::readDoubleVar(const string &name, double &val,
                                 double missingVal, bool required)
  
{

  // init to missing

  val = missingVal;

  // get var

  NcxxVar var = getVar(name);
  if (var.isNull()) {
    if (!required) {
      val = missingVal;
      return var;
    } else {
      addErrStr("ERROR - NcxxGroup::readDoubleVar");
      addErrStr("  Cannot read variable, name: ", name);
      addErrStr("  group: ", getName());
      throw(NcxxException(getErrStr(), __FILE__, __LINE__));
    }
  }

  // check size
  
  if (var.numVals() < 1) {
    addErrStr("ERROR - NcxxGroup::readDoubleVar");
    addErrStr("  variable name: ", name);
    addErrStr("  variable has no data");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  // get data

  vector<double> vals;
  vals.resize(var.numVals());
  try {
    var.getVal(&vals[0]);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readDoubleVar");
    addErrStr("  cannot read variable, name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  val = vals[0];

  return var;

}

///////////////////////////////////
// read a scalar char string variable
// throws NcxxException on failure
// returns var

NcxxVar NcxxGroup::readCharStringVar(const string &name,
                                     string &val)

{

  // init to missing

  val.clear();

  // get var
  
  NcxxVar var = getVar(name);
  if (var.isNull()) {
    addErrStr("ERROR - NcxxGroup::readCharStringVar");
    addErrStr("  Cannot read variable, name: ", name);
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  // check dimension
  
  if (var.getDimCount() != 1) {
    addErrStr("ERROR - NcxxGroup::readCharStringVar");
    addErrStr("  variable name: ", name);
    addErrStr("  variable does not have 1 dimension");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  NcxxDim stringLenDim = var.getDim(0);
  if (stringLenDim.isNull()) {
    addErrStr("ERROR - NcxxGroup::readCharStringVar");
    addErrStr("  variable name: ", name);
    addErrStr("  variable has NULL 0th dimension");
    addErrStr("  should be a string length dimension");
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }
  
  NcxxType ntype = var.getType();
  if (ntype != NcxxType::nc_CHAR) {
    addErrStr("ERROR - NcxxGroup::readCharStringVar");
    addErrStr("  Incorrect variable type");
    addErrStr("  expecting char");
    addErrStr("  found: ", Ncxx::ncxxTypeToStr(ntype));
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  // load up data

  int stringLen = stringLenDim.getSize();
  char *cvalues = new char[stringLen+1];

  try {
    var.getVal(cvalues);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxGroup::readCharStringVar");
    addErrStr("  cannot read variable, name: ", name);
    addErrStr("  exception: ", e.what());
    addErrStr("  group: ", getName());
    delete[] cvalues;
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  delete[] cvalues;

  return var;

}


///////////////////////////////////
// read a scalar string variable
// throws NcxxException on failure
// returns var

NcxxVar NcxxGroup::readScalarStringVar(const string &name,
                                       string &val)

{

  // init to missing

  val.clear();
  
  // get var
  
  NcxxVar var = getVar(name);
  if (var.isNull()) {
    addErrStr("ERROR - NcxxGroup::readCharStringVar");
    addErrStr("  Cannot read variable, name: ", name);
    addErrStr("  group: ", getName());
    throw(NcxxException(getErrStr(), __FILE__, __LINE__));
  }

  // get val

  char *charVal;
  var.getVal(&charVal);
  val = charVal;
  
  return var;

}


