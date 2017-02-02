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

#include <NcxxUtils/Ncxx.hh>
#include <NcxxUtils/NcxxVarAtt.hh>
#include <NcxxUtils/NcxxDim.hh>
#include <NcxxUtils/NcxxVar.hh>
#include <NcxxUtils/NcxxGroup.hh>
#include <NcxxUtils/NcxxCheck.hh>
#include <NcxxUtils/NcxxException.hh>
#include<netcdf.h>
using namespace std;

//  Global comparator operator ==============
// comparator operator
bool operator<(const NcxxVar& lhs,const NcxxVar& rhs)
{
  return false;
}

// comparator operator
bool operator>(const NcxxVar& lhs,const NcxxVar& rhs)
{
  return true;
}

// assignment operator
NcxxVar& NcxxVar::operator=(const NcxxVar & rhs)

{
  if (&rhs == this) {
    return *this;
  }
  _errStr = rhs._errStr;
  nullObject = rhs.nullObject;
  myId = rhs.myId;
  groupId = rhs.groupId;
  return *this;
}

// The copy constructor.
NcxxVar::NcxxVar(const NcxxVar& rhs) :
        NcxxErrStr(),
        nullObject(rhs.nullObject),
        myId(rhs.myId),
        groupId(rhs.groupId)
{
  _errStr = rhs._errStr;
}


// equivalence operator
bool NcxxVar::operator==(const NcxxVar & rhs) const
{
  // simply check the netCDF id.
  return (myId == rhs.myId);
}

//  !=  operator
bool NcxxVar::operator!=(const NcxxVar & rhs) const
{
  return !(*this == rhs);
}

/////////////////

// Constructors and intialization

/////////////////

// Default constructor generates a null object.
NcxxVar::NcxxVar() :
        NcxxErrStr(),
        nullObject(true),
        myId(-1),
        groupId(-1)
{
}

// Constructor for a variable (must already exist in the netCDF file.)
NcxxVar::NcxxVar (const NcxxGroup& grp, const int& varId) :
        NcxxErrStr(),
        nullObject (false),
        myId (varId),
        groupId(grp.getId())
{
}



// Gets parent group.
NcxxGroup  NcxxVar::getParentGroup() const {
  return NcxxGroup(groupId);
}


// Get the variable id.
int  NcxxVar::getId() const {return myId;}

//////////////////////

//  Information about the variable type

/////////////////////


// Gets the NcxxType object with a given name.
NcxxType NcxxVar::getType() const {

  // if this variable has not been defined, return a NULL type
  if(isNull()) return NcxxType();

  // first get the typeid
  nc_type xtypep;
  ncxxCheck(nc_inq_vartype(groupId,myId,&xtypep),__FILE__,__LINE__);

  if(xtypep ==  ncxxByte.getId()    ) return ncxxByte;
  if(xtypep ==  ncxxUbyte.getId()   ) return ncxxUbyte;
  if(xtypep ==  ncxxChar.getId()    ) return ncxxChar;
  if(xtypep ==  ncxxShort.getId()   ) return ncxxShort;
  if(xtypep ==  ncxxUshort.getId()  ) return ncxxUshort;
  if(xtypep ==  ncxxInt.getId()     ) return ncxxInt;
  if(xtypep ==  ncxxUint.getId()    ) return ncxxUint;
  if(xtypep ==  ncxxInt64.getId()   ) return ncxxInt64;
  if(xtypep ==  ncxxUint64.getId()  ) return ncxxUint64;
  if(xtypep ==  ncxxFloat.getId()   ) return ncxxFloat;
  if(xtypep ==  ncxxDouble.getId()  ) return ncxxDouble;
  if(xtypep ==  ncxxString.getId()  ) return ncxxString;

  multimap<string,NcxxType>::const_iterator it;
  multimap<string,NcxxType> types(NcxxGroup(groupId).getTypes(NcxxGroup::ParentsAndCurrent));
  for(it=types.begin(); it!=types.end(); it++) {
    if(it->second.getId() == xtypep) return it->second;
  }
  // we will never reach here
  return true;
}








/////////////////

// Information about Dimensions

/////////////////


// Gets the number of dimensions.
int NcxxVar::getDimCount() const
{
  // get the number of dimensions
  int dimCount;
  ncxxCheck(nc_inq_varndims(groupId,myId, &dimCount),__FILE__,__LINE__);
  return dimCount;
}

// Gets the set of Ncdim objects.
vector<NcxxDim> NcxxVar::getDims() const
{
  // get the number of dimensions
  int dimCount = getDimCount();
  // create a vector of dimensions.
  vector<NcxxDim> ncDims;
  if (dimCount){
    vector<int> dimids(dimCount);
    ncxxCheck(nc_inq_vardimid(groupId,myId, &dimids[0]),__FILE__,__LINE__);
    ncDims.reserve(dimCount);
    for (int i=0; i<dimCount; i++){
      NcxxDim tmpDim(getParentGroup(),dimids[i]);
      ncDims.push_back(tmpDim);
    }
  }
  return ncDims;
}


// Gets the i'th NcxxDim object.
NcxxDim NcxxVar::getDim(int i) const
{
  vector<NcxxDim> ncDims = getDims();
  if((size_t)i >= ncDims.size() || i < 0) throw NcxxException("Index out of range",__FILE__,__LINE__);
  return ncDims[i];
}


/////////////////

// Information about Attributes

/////////////////


// Gets the number of attributes.
int NcxxVar::getAttCount() const
{
  // get the number of attributes
  int attCount;
  ncxxCheck(nc_inq_varnatts(groupId,myId, &attCount),__FILE__,__LINE__);
  return attCount;
}

// Gets the set of attributes.
map<string,NcxxVarAtt> NcxxVar::getAtts() const
{
  // get the number of attributes
  int attCount = getAttCount();
  // create a container of attributes.
  map<string,NcxxVarAtt> ncAtts;
  for (int i=0; i<attCount; i++){
    NcxxVarAtt tmpAtt(getParentGroup(),*this,i);
    ncAtts.insert(pair<const string,NcxxVarAtt>(tmpAtt.getName(),tmpAtt));
  }
  return ncAtts;
}


// Gets attribute by name.
NcxxVarAtt NcxxVar::getAtt(const string& name) const
{
  map<string,NcxxVarAtt> attributeList = getAtts();
  map<string,NcxxVarAtt>::iterator myIter;
  myIter = attributeList.find(name);
  if(myIter == attributeList.end()){
    string msg("Attribute '"+name+"' not found");
    throw NcxxException(msg.c_str(),__FILE__,__LINE__);
  }
  return NcxxVarAtt(myIter->second);
}



/////////////////////////


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const string& dataValues) const {
  ncxxCheckDefineMode(groupId);
  ncxxCheck(nc_put_att_text(groupId,myId,name.c_str(),dataValues.size(),dataValues.c_str()),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned char* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_uchar(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const signed char* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_schar(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}



/////////////////////////////////
// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, short datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_short(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, int datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_int(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, long datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_long(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, float datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_float(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, double datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_double(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, unsigned short datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_ushort(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, unsigned int datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_uint(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, long long datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_longlong(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, unsigned long long datumValue) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_ulonglong(groupId,myId,name.c_str(),type.getId(),1,&datumValue),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


/////////////////////////////////

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const short* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_short(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const int* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_int(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const long* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_long(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const float* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_float(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const double* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_double(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned short* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_ushort(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned int* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_uint(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const long long* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_longlong(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const unsigned long long* dataValues) const {
  ncxxCheckDefineMode(groupId);
  NcxxType::ncxxType typeClass(type.getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_att(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_att_ulonglong(groupId,myId,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, size_t len, const char** dataValues) const {
  ncxxCheckDefineMode(groupId);
  ncxxCheck(nc_put_att_string(groupId,myId,name.c_str(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name, const NcxxType& type, size_t len, const void* dataValues) const {
  ncxxCheckDefineMode(groupId);
  ncxxCheck(nc_put_att(groupId,myId ,name.c_str(),type.getId(),len,dataValues),__FILE__,__LINE__);
  // finally instantiate this attribute and return
  return getAtt(name);
}




////////////////////

// Other Basic variable info

////////////////////

// The name of this variable.
string NcxxVar::getName() const{
  char charName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_varname(groupId, myId, charName),__FILE__,__LINE__);
  return string(charName);
}


////////////////////

// Chunking details

////////////////////


// Sets chunking parameters.
void NcxxVar::setChunking(ChunkMode chunkMode, vector<size_t>& chunkSizes) const {
  size_t *chunkSizesPtr = chunkSizes.empty() ? 0 : &chunkSizes[0];
  ncxxCheck(nc_def_var_chunking(groupId,myId,static_cast<int> (chunkMode), chunkSizesPtr),__FILE__,__LINE__);
}


// Gets the chunking parameters
void NcxxVar::getChunkingParameters(ChunkMode& chunkMode, vector<size_t>& chunkSizes) const {
  int chunkModeInt;
  chunkSizes.resize(getDimCount());
  size_t *chunkSizesPtr = chunkSizes.empty() ? 0 : &chunkSizes[0];
  ncxxCheck(nc_inq_var_chunking(groupId,myId, &chunkModeInt, chunkSizesPtr),__FILE__,__LINE__);
  chunkMode = static_cast<ChunkMode> (chunkModeInt);
}




////////////////////

// Fill details

////////////////////


// Sets the fill parameters
void NcxxVar::setFill(bool fillMode, void* fillValue) const {
  // If fillMode is enabled, check that fillValue has a legal pointer.
  if(fillMode && fillValue == NULL)
    throw NcxxException("FillMode was set to zero but fillValue has invalid pointer",__FILE__,__LINE__);

  ncxxCheck(nc_def_var_fill(groupId,myId,static_cast<int> (!fillMode),fillValue),__FILE__,__LINE__);
}

// Sets the fill parameters
void NcxxVar::setFill(bool fillMode,const void* fillValue) const {
  setFill(fillMode,const_cast<void*>(fillValue));
}



// Gets the fill parameters
void NcxxVar::getFillModeParameters(bool& fillMode, void* fillValue)const{

  int fillModeInt;
  ncxxCheck(nc_inq_var_fill(groupId,myId,&fillModeInt,fillValue),__FILE__,__LINE__);
  fillMode= static_cast<bool> (fillModeInt == 0);
}


////////////////////

// Compression details

////////////////////


// Sets the compression parameters
void NcxxVar::setCompression(bool enableShuffleFilter, bool enableDeflateFilter, int deflateLevel) const {

  // Check that the deflate level is legal
  if(enableDeflateFilter & (deflateLevel < 0 || deflateLevel >9))
    throw NcxxException("The deflateLevel must be set between 0 and 9.",__FILE__,__LINE__);

  ncxxCheck(nc_def_var_deflate(groupId,myId,
			     static_cast<int> (enableShuffleFilter),
			     static_cast<int> (enableDeflateFilter),
			     deflateLevel),__FILE__,__LINE__);
}


// Gets the compression parameters
void NcxxVar::getCompressionParameters(bool& shuffleFilterEnabled, bool& deflateFilterEnabled, int& deflateLevel) const {

  int enableShuffleFilterInt;
  int enableDeflateFilterInt;
  ncxxCheck(nc_inq_var_deflate(groupId,myId,
			     &enableShuffleFilterInt,
			     &enableDeflateFilterInt,
			     &deflateLevel),__FILE__,__LINE__);
  shuffleFilterEnabled =  static_cast<bool> (enableShuffleFilterInt);
  deflateFilterEnabled =  static_cast<bool> (enableDeflateFilterInt);
}



////////////////////

// Endianness details

////////////////////


// Sets the endianness of the variable.
void NcxxVar::setEndianness(EndianMode endianMode) const {

  ncxxCheck(nc_def_var_endian(groupId,myId,static_cast<int> (endianMode)),__FILE__,__LINE__);
}


// Gets the endianness of the variable.
NcxxVar::EndianMode NcxxVar::getEndianness() const {

  int endianInt;
  ncxxCheck(nc_inq_var_endian(groupId,myId,&endianInt),__FILE__,__LINE__);
  return static_cast<EndianMode> (endianInt);
}



////////////////////

// Checksum details

////////////////////


// Sets the checksum parameters of a variable.
void NcxxVar::setChecksum(ChecksumMode checksumMode) const {
  ncxxCheck(nc_def_var_fletcher32(groupId,myId,static_cast<int> (checksumMode)),__FILE__,__LINE__);
}


// Gets the checksum parameters of the variable.
NcxxVar::ChecksumMode NcxxVar::getChecksum() const {
  int checksumInt;
  ncxxCheck(nc_inq_var_fletcher32(groupId,myId,&checksumInt),__FILE__,__LINE__);
  return static_cast<ChecksumMode> (checksumInt);
}




////////////////////

//  renaming the variable

////////////////////

void NcxxVar::rename( const string& newname ) const
{
  ncxxCheck(nc_rename_var(groupId,myId,newname.c_str()),__FILE__,__LINE__);
}





////////////////////

//  data writing

////////////////////

// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_text(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_uchar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_schar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_short(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_int(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_long(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_float(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_double(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_ushort(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_uint(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_longlong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_ulonglong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVal(const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_string(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable with no data conversion.
void NcxxVar::putVal(const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
}


///////////////////
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const string& datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    {
      const char* tmpPtr = datumValue.c_str();
      ncxxCheck(nc_put_var1_string(groupId, myId,&index[0],&tmpPtr),__FILE__,__LINE__);
    }
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const unsigned char* datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_uchar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const signed char* datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_schar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const short datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_short(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const int datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_int(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const long datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_long(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const float datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_float(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const double datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_double(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const unsigned short datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_ushort(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const unsigned int datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_uint(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const long long datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_longlong(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const unsigned long long datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_ulonglong(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index, const char** datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_string(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& index, const void* datumValue) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}


////////////////////

// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_text(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_uchar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_schar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_short(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_int(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_long(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_float(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_double(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_ushort(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_uint(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_longlong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_ulonglong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_string(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>& countp, const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}



////////////////////

// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_text(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_short(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_int(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_long(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_float(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_double(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_string(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}


////////////////////
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_text(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_short(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_int(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_long(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_float(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_double(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_string(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}





// Data reading



// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_text(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_uchar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(signed char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_schar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_short(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_int(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_long(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(float* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_float(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(double* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_double(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_ushort(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_uint(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_longlong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_ulonglong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(char** dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_string(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable with no data conversion.
void NcxxVar::getVal(void* dataValues) const {
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
}



///////////

// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, char* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_text(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, unsigned char* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_uchar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, signed char* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_schar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, short* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_short(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, int* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_int(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, long* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_long(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, float* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_float(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, double* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_double(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, unsigned short* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_ushort(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, unsigned int* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_uint(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, long long* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_longlong(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable
void NcxxVar::getVal(const vector<size_t>& index, unsigned long long* datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_ulonglong(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, char** datumValue) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_string(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& index, void* datumValue) const {
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}



///////////

// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_text(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, unsigned char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_uchar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, signed char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_schar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_short(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_int(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_long(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, float* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_float(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, double* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_double(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, unsigned short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_ushort(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, unsigned int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_uint(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_longlong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, unsigned long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_ulonglong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, char** dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_string(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, void* dataValues) const {
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}


///////////

// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_text(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, signed char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_short(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_int(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_long(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, float* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_float(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, double* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_double(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, char** dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_string(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, void* dataValues) const {
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}


///////////

// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_text(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, signed char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_short(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_int(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_long(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, float* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_float(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, double* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_double(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN ||typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, char** dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_string(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, void* dataValues) const {
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}

///////////////////////////////////////////
// add string attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, const string &val)
{
  clearErrStr();
  try {
    putAtt(name.c_str(), val.c_str());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add string var attr, name: ", name);
    _addErrStr("  val: ", val);
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add double attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, double val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_DOUBLE);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add double var attr, name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add float attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, float val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_FLOAT);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add float var attr, name: ", name);
    _addErrDbl("  val: ", val, "%g");
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add int attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, int val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_INT);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add int var attr, name: ", name);
    _addErrDbl("  val: ", val, "%d");
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add long attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, int64_t val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_INT64);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add int64_t var attr, name: ", name);
    _addErrDbl("  val: ", val, "%ld");
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add short attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, short val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_SHORT);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add short var attr, name: ", name);
    _addErrDbl("  val: ", (int) val, "%d");
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

///////////////////////////////////////////
// add ncbyte attribute
// Returns 0 on success, -1 on failure

int NcxxVar::addAttr(const string &name, unsigned char val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_UBYTE);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::addAttr");
    _addErrStr("  Cannot add byte var attr, name: ", name);
    _addErrDbl("  val: ", (int) val, "%d");
    _addErrStr("  var name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

////////////////////////////////////////////////
// get the total number of values in a variable
// this is the product of the dimension sizes
// and is 1 for a scalar (i.e. no dimensions)

int64_t NcxxVar::numVals()
  
{

  std::vector<NcxxDim> dims = getDims();
  int64_t prod = 1;
  for (size_t ii = 0; ii < dims.size(); ii++) {
    prod *=  getDim(ii).getSize();
  }
  return prod;

}
  
///////////////////////////////////////////////////////////////////////////
// write a scalar double variable
// Returns 0 on success, -1 on failure

int NcxxVar::write(double val)
  
{
  
  clearErrStr();

  if (isNull()) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  nc_type vtype = getType().getId();
  if (vtype != NC_DOUBLE) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Var type should be double, name: ", getName());
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    putVal(index, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Cannot write scalar double var, name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a scalar float variable
// Returns 0 on success, -1 on failure

int NcxxVar::write(float val)
  
{
  
  clearErrStr();

  if (isNull()) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  nc_type vtype = getType().getId();
  if (vtype != NC_FLOAT) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Var type should be float, name: ", getName());
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    putVal(index, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Cannot write scalar float var, name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a scalar int variable
// Returns 0 on success, -1 on failure

int NcxxVar::write(int val)
  
{
  
  clearErrStr();

  if (isNull()) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  nc_type vtype = getType().getId();
  if (vtype != NC_INT) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Var type should be int, name: ", getName());
    return -1;
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    putVal(index, val);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Cannot write scalar int var, name: ", getName());
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in dimension
// Returns 0 on success, -1 on failure

int NcxxVar::write(const NcxxDim &dim,
                   const void *data)
  
{
  return write(dim, dim.getSize(), data);
}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in arguments
// Returns 0 on success, -1 on failure

int NcxxVar::write(const NcxxDim &dim,
                   size_t count, 
                   const void *data)
  
{

  clearErrStr();

  if (isNull()) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  var is NULL");
    return -1;
  }

  int iret = 0;
  vector<size_t> starts, counts;
  starts.push_back(0);
  counts.push_back(count);
  
  nc_type vtype = getType().getId();
  switch (vtype) {
    case NC_DOUBLE: {
      try {
        putVal(starts, counts, (double *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_INT: {
      try {
        putVal(starts, counts, (int *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_SHORT: {
      try {
        putVal(starts, counts, (short *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_UBYTE: {
      try {
        putVal(starts, counts, (unsigned char *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
    case NC_FLOAT:
    default: {
      try {
        putVal(starts, counts, (float *) data);
      } catch (NcxxException& e) {
        _addErrStr("  exception: ", e.what());
        iret = -1;
      }
      break;
    }
  } // switch
  
  if (iret) {
    _addErrStr("ERROR - NcxxVar::write");
    _addErrStr("  Cannot write var, name: ", getName());
    _addErrStr("  Dim name: ", dim.getName());
    _addErrInt("  Count: ", count);
    return -1;
  } else {
    return 0;
  }

}


///////////////////////////////////////////////////////////////////////////
// write a string variable
// Returns 0 on success, -1 on failure

int NcxxVar::writeStrings(const void *str)
  
{

  clearErrStr();

  if (isNull()) {
    _addErrStr("ERROR - NcxxVar::writeStrings");
    _addErrStr("  var is NULL");
    return -1;
  }
  
  std::vector<NcxxDim> dims = getDims();
  size_t nDims = dims.size();
  if (nDims < 1) {
    _addErrStr("ERROR - NcxxVar::writeStrings");
    _addErrStr("  var has no dimensions");
    _addErrStr("  var name: ", getName());
    return -1;
  }

  if (nDims == 1) {

    // single dimension

    NcxxDim &dim0 = dims[0];
    if (dim0.isNull()) {
      _addErrStr("ERROR - NcxxVar::writeStrings");
      _addErrStr("  Canont write var, name: ", getName());
      _addErrStr("  dim 0 is NULL");
      return -1;
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize());
    try {
      putVal(starts, counts, (char *) str);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxVar::writeStrings");
      _addErrStr("  Canont write var, name: ", getName());
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    return 0;
    
  } // if (nDims == 1)

  if (nDims == 2) {

    // two dimensions

    NcxxDim &dim0 = dims[0];
    if (dim0.isNull()) {
      _addErrStr("ERROR - NcxxVar::writeStrings");
      _addErrStr("  Canont write var, name: ", getName());
      _addErrStr("  dim 0 is NULL");
      return -1;
    }

    NcxxDim &dim1 = dims[1];
    if (dim1.isNull()) {
      _addErrStr("ERROR - NcxxVar::writeStrings");
      _addErrStr("  Canont write var, name: ", getName());
      _addErrStr("  dim 1 is NULL");
      return -1;
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize() * dim1.getSize());
    try {
      putVal(starts, counts, (char *) str);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxVar::writeStrings");
      _addErrStr("  Canont write var, name: ", getName());
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    return 0;

  }

  // more than 2 is an error
  
  _addErrStr("ERROR - NcxxVar::writeStrings");
  _addErrStr("  Canont write var, name: ", getName());
  _addErrInt("  more than 2 dimensions: ", nDims);
  return -1;

}

////////////////////////////////////////
// set default fill value, based on type

void NcxxVar::setDefaultFillvalue()

{

  nc_type vtype = getType().getId();
  if (vtype == NC_DOUBLE) {
    addAttr("_fillValue", Ncxx::missingDouble);
    return;
  }
  if (vtype == NC_FLOAT) {
    addAttr("_fillValue", Ncxx::missingFloat);
    return;
  }
  if (vtype == NC_INT) {
    addAttr("_fillValue", Ncxx::missingInt);
    return;
  }
  if (vtype == NC_LONG) {
    addAttr("_fillValue", /* (long) */ Ncxx::missingInt);
    return;
  }
  if (vtype == NC_SHORT) {
    addAttr("_fillValue", (short) Ncxx::missingInt);
    return;
  }
  if (vtype == NC_UBYTE) {
    addAttr("_fillValue", Ncxx::missingUchar);
    return;
  }
  addAttr("_fillValue", Ncxx::missingInt);
}

////////////////////////////////////////
// convert var type string

string NcxxVar::varTypeToStr()
      
{
  nc_type vtype = getType().getId();
  return Ncxx::ncTypeToStr(vtype);
}

