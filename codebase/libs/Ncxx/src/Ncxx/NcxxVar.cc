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
#include <Ncxx/NcxxVarAtt.hh>
#include <Ncxx/NcxxDim.hh>
#include <Ncxx/NcxxVar.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxException.hh>
#include<netcdf.h>
using namespace std;

//  Global comparator operator ==============
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

/////////////////////////////////////
// Constructors and intialization
/////////////////////////////////////

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

///////////////////////////////////////////
//  Information about the variable type
///////////////////////////////////////////

// Gets the NcxxType object with a given name.
NcxxType NcxxVar::getType() const {

  // if this variable has not been defined, return a NULL type
  if(isNull()) return NcxxType();

  // first get the typeid
  nc_type xtypep;
  ncxxCheck(nc_inq_vartype(groupId, myId, &xtypep), 
            __FILE__, __LINE__,
            getName(), "getType()");
  
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
  multimap<string,NcxxType>
    types(NcxxGroup(groupId).getTypes(NcxxGroup::ParentsAndCurrent));
  for(it=types.begin(); it!=types.end(); it++) {
    if(it->second.getId() == xtypep) return it->second;
  }
  // we will never reach here
  return true;
}

/////////////////////////////////////
// Information about Dimensions
/////////////////////////////////////

// Gets the number of dimensions.
int NcxxVar::getDimCount() const
{
  // get the number of dimensions
  int dimCount;
  ncxxCheck(nc_inq_varndims(groupId, myId,  &dimCount),
            __FILE__, __LINE__,
            getName(), "getDimCount()");
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
    ncxxCheck(nc_inq_vardimid(groupId, myId,  &dimids[0]),
              __FILE__, __LINE__,
              getName(), "getDims()");
    ncDims.reserve(dimCount);
    for (int i=0; i<dimCount; i++) {
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
  if((size_t)i >= ncDims.size() || i < 0) {
    char errStr[4096];
    sprintf(errStr, "%s - index out of range: index = %d, size = %d",
            getDesc().c_str(), i, (int) ncDims.size());
    throw NcxxException(errStr, __FILE__, __LINE__);
  }
  return ncDims[i];
}

////////////////////////////////////
// Information about Attributes
////////////////////////////////////

// Gets the number of attributes.
int NcxxVar::getAttCount() const
{
  // get the number of attributes
  int attCount;
  ncxxCheck(nc_inq_varnatts(groupId, myId,  &attCount),
            __FILE__, __LINE__,
            getName(), "getAttCount()");
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
    throw NcxxException(msg, __FILE__, __LINE__);
  }
  return NcxxVarAtt(myIter->second);
}

/////////////////////////

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const string& dataValues) const {
  ncxxCheckDefineMode(groupId);
  ncxxCheck(nc_put_att_text(groupId, myId, 
                            name.c_str(), dataValues.size(), dataValues.c_str()),
            __FILE__, __LINE__,
            "var", getName(), "putAtt(string)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type, size_t len,
                           const unsigned char* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId,
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(char*)");
  else
    ncxxCheck(nc_put_att_schar(groupId, myId,
                               name.c_str(), type.getId(), len, 
                               (const signed char*) dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(char*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type, size_t len,
                           const signed char* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(signed char*)");
  else
    ncxxCheck(nc_put_att_schar(groupId, myId, 
                               name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(signed char*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}



/////////////////////////////////
// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           short datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(short)");
  else
    ncxxCheck(nc_put_att_short(groupId, myId, 
                               name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(short)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           int datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(int)");
  else
    ncxxCheck(nc_put_att_int(groupId, myId, 
                             name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(int)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           long datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long)");
  else
    ncxxCheck(nc_put_att_long(groupId, myId, 
                              name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           float datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(float)");
  else
    ncxxCheck(nc_put_att_float(groupId, myId, 
                               name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(float)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           double datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(double)");
  else
    ncxxCheck(nc_put_att_double(groupId, myId, 
                                name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(double)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           unsigned short datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned short)");
  else
    ncxxCheck(nc_put_att_ushort(groupId, myId, 
                                name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned short)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           unsigned int datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned int)");
  else
    ncxxCheck(nc_put_att_uint(groupId, myId, 
                              name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned int)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           long long datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long long)");
  else
    ncxxCheck(nc_put_att_longlong(groupId, myId, 
                                  name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long long)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type, 
                           unsigned long long datumValue) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned long long)");
  else
    ncxxCheck(nc_put_att_ulonglong(groupId, myId, 
                                   name.c_str(), type.getId(), 1, &datumValue),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned long long)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


/////////////////////////////////

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const short* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(short*)");
  else
    ncxxCheck(nc_put_att_short(groupId, myId, 
                               name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(short*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const int* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(int*)");
  else
    ncxxCheck(nc_put_att_int(groupId, myId, 
                             name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(int*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const long* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long*)");
  else
    ncxxCheck(nc_put_att_long(groupId, myId, 
                              name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const float* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(float*)");
  else
    ncxxCheck(nc_put_att_float(groupId, myId, 
                               name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(float*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const double* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(double*)");
  else
    ncxxCheck(nc_put_att_double(groupId, myId, 
                                name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(double*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const unsigned short* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned short*)");
  else
    ncxxCheck(nc_put_att_ushort(groupId, myId, 
                                name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned short*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const unsigned int* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned int*)");
  else
    ncxxCheck(nc_put_att_uint(groupId, myId, 
                              name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned int*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const long long* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long long*)");
  else
    ncxxCheck(nc_put_att_longlong(groupId, myId, 
                                  name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(long long*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const unsigned long long* dataValues) const {
  ncxxCheckDefineMode(groupId);
  if (type.isComplex())
    ncxxCheck(nc_put_att(groupId, myId, 
                         name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned long long*)");
  else
    ncxxCheck(nc_put_att_ulonglong(groupId, myId, 
                                   name.c_str(), type.getId(), len, dataValues),
              __FILE__, __LINE__,
              "var", getName(), "putAtt(unsigned long long*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}


// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           size_t len,
                           const char** dataValues) const {
  ncxxCheckDefineMode(groupId);
  ncxxCheck(nc_put_att_string(groupId, myId, 
                              name.c_str(), len, dataValues), 
            __FILE__, __LINE__,
            "var", getName(), "putAtt(char**)");
  // finally instantiate this attribute and return
  return getAtt(name);
}

// Creates a new NetCDF variable attribute or if already exisiting replaces it.
NcxxVarAtt NcxxVar::putAtt(const string& name,
                           const NcxxType& type,
                           size_t len,
                           const void* dataValues) const {
  ncxxCheckDefineMode(groupId);
  ncxxCheck(nc_put_att(groupId,myId ,
                       name.c_str(), type.getId(), len, dataValues),
            __FILE__, __LINE__,
            "var", getName(), "putAtt(void*)");
  // finally instantiate this attribute and return
  return getAtt(name);
}




///////////////////////////////////
// Other Basic variable info
///////////////////////////////////

// The name of this variable.
string NcxxVar::getName() const{
  char charName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_varname(groupId, myId,  charName),
            __FILE__, __LINE__);
  return string(charName);
}

////////////////////
// Chunking details
////////////////////

// Sets chunking parameters.

void NcxxVar::setChunking(ChunkMode chunkMode,
                          vector<size_t>& chunkSizes) const {
  size_t *chunkSizesPtr = chunkSizes.empty() ? 0 : &chunkSizes[0];
  ncxxCheck(nc_def_var_chunking(groupId, myId, 
                                static_cast<int> (chunkMode), chunkSizesPtr),
            __FILE__, __LINE__, "var", getName(), "setChunking()");
}


// Gets the chunking parameters

void NcxxVar::getChunkingParameters(ChunkMode& chunkMode,
                                    vector<size_t>& chunkSizes) const {
  int chunkModeInt;
  chunkSizes.resize(getDimCount());
  size_t *chunkSizesPtr = chunkSizes.empty() ? 0 : &chunkSizes[0];
  ncxxCheck(nc_inq_var_chunking(groupId, myId,  &chunkModeInt, chunkSizesPtr),
            __FILE__, __LINE__, "var", getName(), "setChunkingParameters()");
  chunkMode = static_cast<ChunkMode> (chunkModeInt);
}

////////////////////
// Fill details
////////////////////

// Sets the fill parameters
void NcxxVar::setFill(bool fillMode,
                      void* fillValue) const {
  // If fillMode is enabled, check that fillValue has a legal pointer.
  if(fillMode && fillValue == NULL)
    throw NcxxException("FillMode was set to zero but fillValue has invalid pointer",
                        __FILE__, __LINE__);

  ncxxCheck(nc_def_var_fill(groupId, myId, static_cast<int> (!fillMode),fillValue),
            __FILE__, __LINE__, "var", getName(), "setFill()");
}

// Sets the fill parameters
void NcxxVar::setFill(bool fillMode,
                      const void* fillValue) const {
  setFill(fillMode,const_cast<void*>(fillValue));
}



// Gets the fill parameters
void NcxxVar::getFillModeParameters(bool& fillMode,
                                    void* fillValue) const {
  int fillModeInt;
  ncxxCheck(nc_inq_var_fill(groupId, myId, &fillModeInt,fillValue),
            __FILE__, __LINE__, "var", getName(), "getFillModeParameters()");
  fillMode= static_cast<bool> (fillModeInt == 0);
}


////////////////////////////////
// Compression details
////////////////////////////////


// Sets the compression parameters
void NcxxVar::setCompression(bool enableShuffleFilter, 
                             bool enableDeflateFilter, 
                             int deflateLevel) const {
  
  // Check that the deflate level is legal
  if(enableDeflateFilter & (deflateLevel < 0 || deflateLevel >9))
    throw NcxxException("The deflateLevel must be set between 0 and 9.",
                        __FILE__, __LINE__);
  
  ncxxCheck(nc_def_var_deflate(groupId, myId, 
                               static_cast<int> (enableShuffleFilter),
                               static_cast<int> (enableDeflateFilter),
                               deflateLevel),
            __FILE__, __LINE__, "var", getName(), "setCompression()");
}


// Gets the compression parameters
void NcxxVar::getCompressionParameters(bool& shuffleFilterEnabled,
                                       bool& deflateFilterEnabled,
                                       int& deflateLevel) const {

  int enableShuffleFilterInt;
  int enableDeflateFilterInt;
  ncxxCheck(nc_inq_var_deflate(groupId, myId, 
                               &enableShuffleFilterInt,
                               &enableDeflateFilterInt,
                               &deflateLevel),
            __FILE__, __LINE__, "var", getName(), "getCompressionParameters()");
  shuffleFilterEnabled =  static_cast<bool> (enableShuffleFilterInt);
  deflateFilterEnabled =  static_cast<bool> (enableDeflateFilterInt);
}



//////////////////////////////////
// Endianness details
//////////////////////////////////


// Sets the endianness of the variable.
void NcxxVar::setEndianness(EndianMode endianMode) const {

  ncxxCheck(nc_def_var_endian(groupId, myId, static_cast<int> (endianMode)),
            __FILE__, __LINE__, "var", getName(), "setEndianness(endianMode)");
}


// Gets the endianness of the variable.
NcxxVar::EndianMode NcxxVar::getEndianness() const {

  int endianInt;
  ncxxCheck(nc_inq_var_endian(groupId, myId, &endianInt),
            __FILE__, __LINE__, "var", getName(), "setEndianness()");
  return static_cast<EndianMode> (endianInt);
}



////////////////////

// Checksum details

////////////////////


// Sets the checksum parameters of a variable.
void NcxxVar::setChecksum(ChecksumMode checksumMode) const {
  ncxxCheck(nc_def_var_fletcher32(groupId, myId, static_cast<int> (checksumMode)),
            __FILE__, __LINE__, "var", getName(), "setChecksum()");
}


// Gets the checksum parameters of the variable.
NcxxVar::ChecksumMode NcxxVar::getChecksum() const {
  int checksumInt;
  ncxxCheck(nc_inq_var_fletcher32(groupId, myId, &checksumInt),
            __FILE__, __LINE__, "var", getName(), "getChecksum()");
  return static_cast<ChecksumMode> (checksumInt);
}


//////////////////////////////////////
//  renaming the variable
//////////////////////////////////////

void NcxxVar::rename( const string& newname ) const
{
  ncxxCheck(nc_rename_var(groupId, myId, newname.c_str()), 
            __FILE__, __LINE__, "var rename", getName(), newname);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  data writing
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Write a scalar into the netCDF variable.

// Write string scalar

void NcxxVar::putStringScalar(const string &dataVal) const {
  vector<size_t> index;
  index.push_back(0);
  putVal(index, dataVal);
}

// Write char scalar

void NcxxVar::putVal(char dataVal) const {
  putVal(&dataVal);
}

// Write an unsigned char scalar

void NcxxVar::putVal(unsigned char dataVal) const {
  putVal(&dataVal);
}

// Write a signed char scalar

void NcxxVar::putVal(signed char dataVal) const {
  putVal(&dataVal);
}

// Write a short scalar.

void NcxxVar::putVal(short dataVal) const {
  putVal(&dataVal);
}

// Write an unsigned short scalar

void NcxxVar::putVal(unsigned short dataVal) const {
  putVal(&dataVal);
}

// Write in int scalar

void NcxxVar::putVal(int dataVal) const {
  putVal(&dataVal);
}

// Write an unsigned int scalar

void NcxxVar::putVal(unsigned int dataVal) const {
  putVal(&dataVal);
}

// Write a long scalar

void NcxxVar::putVal(long dataVal) const {
  putVal(&dataVal);
}

// Write a long long scalar

void NcxxVar::putVal(long long dataVal) const {
  putVal(&dataVal);
}

// Write an unsigned long long scalar

void NcxxVar::putVal(unsigned long long dataVal) const {
  putVal(&dataVal);
}

// Write a float scalar

void NcxxVar::putVal(float dataVal) const {
  putVal(&dataVal);
}

// Write a double scalar

void NcxxVar::putVal(double dataVal) const {
  putVal(&dataVal);
}

///////////////////////////////////////////////////////////////////////////////

// Write the entire data into the netCDF variable.  This is the simplest
// interface to use for writing a value in a scalar variable or whenever all
// the values of a multidimensional variable can all be written at once. The
// values to be written are associated with the netCDF variable by assuming
// that the last dimension of the netCDF variable varies fastest in the C
// interface.

// Take care when using the simplest forms of this interface with record
// variables when you don't specify how many records are to be written. If you
// try to write all the values of a record variable into a netCDF file that has
// no record data yet (hence has 0 records), nothing will be
// written. Similarly, if you try to write all of a record variable but there
// are more records in the file than you assume, more data may be written to
// the file than you supply, which may result in a segmentation violation.

// \param
// dataValues The data values. The order in which the data will be
// written to the netCDF variable is with the last dimension of the specified
// variable varying fastest. If the type of data values differs from the netCDF
// variable type, type conversion will occur.  (However, no type conversion is
// carried out for variables using the user-defined data types: nc_Vlen,
// nc_Opaque, nc_Compound and nc_Enum.)

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(char**)");
  else
    ncxxCheck(nc_put_var_string(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(char**)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(char*)");
  else
    ncxxCheck(nc_put_var_text(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(char*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned char*)");
  else
    ncxxCheck(nc_put_var_uchar(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned char*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(signed char*)");
  else
    ncxxCheck(nc_put_var_schar(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(signed char*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(short*)");
  else
    ncxxCheck(nc_put_var_short(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(short*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(int*)");
  else
    ncxxCheck(nc_put_var_int(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(int*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(long*)");
  else
    ncxxCheck(nc_put_var_long(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(long*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(float*)");
  else
    ncxxCheck(nc_put_var_float(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(float*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(double*)");
  else
    ncxxCheck(nc_put_var_double(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(double*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned short*)");
  else
    ncxxCheck(nc_put_var_ushort(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned short*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned int*)");
  else
    ncxxCheck(nc_put_var_uint(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned int*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(long long*)");
  else
    ncxxCheck(nc_put_var_longlong(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(long long*)");
}

// Write the entire data into the netCDF variable.

void NcxxVar::putVal(const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned long long*)");
  else
    ncxxCheck(nc_put_var_ulonglong(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal(unsigned long long*)");
}

// Write the entire data into the netCDF variable with no data conversion.
void NcxxVar::putVal(const void* dataValues) const {
  ncxxCheckDataMode(groupId);
  ncxxCheck(nc_put_var(groupId, myId, dataValues),
            __FILE__, __LINE__,
            getDesc(), "putVal(void*)");
}


//////////////////////////////////////////////////////////////////////////////////////

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const string& datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex()) {
    throw NcxxException(getDesc() + " putVal() - user-defined type must be of type void",
                        __FILE__, __LINE__);
  } else {
    const char* tmpPtr = datumValue.c_str();
    ncxxCheck(nc_put_var1_string(groupId, myId, &index[0],&tmpPtr),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  }
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const unsigned char* datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex()) {
    throw NcxxException(getDesc() + " putVal() - user-defined type must be of type void",
                        __FILE__, __LINE__);
  } else {
    ncxxCheck(nc_put_var1_uchar(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  }
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const signed char* datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex()) {
    throw NcxxException(getDesc() + " putVal() - user-defined type must be of type void",
                        __FILE__, __LINE__);
  } else {
    ncxxCheck(nc_put_var1_schar(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  }
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const short datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_short(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const int datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_int(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const long datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_long(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const float datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_float(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const double datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_double(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const unsigned short datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_ushort(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const unsigned int datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_uint(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const long long datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_longlong(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const unsigned long long datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_var1(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_var1_ulonglong(groupId, myId, &index[0],&datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}

// Write a single datum value into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& index,
                     const char** datumValue) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex()) {
    throw NcxxException(getDesc() + " putVal() - user-defined type must be of type void",
                        __FILE__, __LINE__);
  } else {
    ncxxCheck(nc_put_var1_string(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  }
}

// Write a single datum value into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& index,
                     const void* datumValue) const {
  ncxxCheckDataMode(groupId);
  ncxxCheck(nc_put_var1(groupId, myId, &index[0],datumValue),
            __FILE__, __LINE__,
            getDesc(), "putVal()");
}


////////////////////

// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_text(groupId, myId, 
                               &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_uchar(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_schar(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_short(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_int(groupId, myId, 
                              &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_long(groupId, myId, 
                               &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_float(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_double(groupId, myId, 
                                 &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_ushort(groupId, myId, 
                                 &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_uint(groupId, myId, 
                               &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_longlong(groupId, myId, 
                                   &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_ulonglong(groupId, myId, 
                                    &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vara_string(groupId, myId, 
                                 &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write an array of values into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const void* dataValues) const {
  ncxxCheckDataMode(groupId);
  ncxxCheck(nc_put_vara(groupId, myId, 
                        &startp[0],&countp[0],dataValues),
            __FILE__, __LINE__,
            getDesc(), "putVal()");
}



////////////////////

// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_text(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_uchar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_schar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_short(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_int(groupId, myId, 
                              &startp[0],&countp[0],
                              &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_long(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_float(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_double(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_ushort(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_uint(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_longlong(groupId, myId, 
                                   &startp[0],&countp[0],
                                   &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_ulonglong(groupId, myId, 
                                    &startp[0],&countp[0],
                                    &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_vars_string(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a set of subsampled array values into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep, 
                     const void* dataValues) const {
  ncxxCheckDataMode(groupId);
  ncxxCheck(nc_put_vars(groupId, myId, 
                        &startp[0],&countp[0],
                        &stridep[0],dataValues),
            __FILE__, __LINE__,
            getDesc(), "putVal()");
}


////////////////////
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_text(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_uchar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_schar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_short(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_int(groupId, myId, 
                              &startp[0],&countp[0],
                              &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_long(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_float(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_double(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_ushort(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_uint(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_longlong(groupId, myId, 
                                   &startp[0],&countp[0],
                                   &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_ulonglong(groupId, myId, 
                                    &startp[0],&countp[0],
                                    &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  if (getType().isComplex())
    ncxxCheck(nc_put_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
  else
    ncxxCheck(nc_put_varm_string(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "putVal()");
}
// Write a mapped array section of values into the netCDF variable with no data conversion.
void NcxxVar::putVal(const vector<size_t>& startp,
                     const vector<size_t>&countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp,
                     const void* dataValues) const {
  ncxxCheckDataMode(groupId);
  ncxxCheck(nc_put_varm(groupId, myId, 
                        &startp[0],&countp[0],
                        &stridep[0],&imapp[0],dataValues),
            __FILE__, __LINE__,
            getDesc(), "putVal()");
}





// Data reading



// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_text(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_uchar(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(signed char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_schar(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_short(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_int(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_long(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(float* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_float(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}

// Reads the entire data of the netCDF variable.

void NcxxVar::getVal(double* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_double(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}

// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_ushort(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_uint(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_longlong(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(unsigned long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_ulonglong(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVal(char** dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var_string(groupId, myId, dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads the entire data of the netCDF variable with no data conversion.
void NcxxVar::getVal(void* dataValues) const {
  ncxxCheck(nc_get_var(groupId, myId, dataValues),
            __FILE__, __LINE__,
            getDesc(), "getVal()");
}



///////////

// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, char* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_text(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, unsigned char* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_uchar(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, signed char* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_schar(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, short* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_short(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, int* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_int(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, long* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_long(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, float* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_float(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, double* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_double(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, unsigned short* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_ushort(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, unsigned int* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_uint(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, long long* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_longlong(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable
void NcxxVar::getVal(const vector<size_t>& index, unsigned long long* datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_ulonglong(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& index, char** datumValue) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_var1_string(groupId, myId, &index[0],datumValue),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a single datum value of a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& index, void* datumValue) const {
  ncxxCheck(nc_get_var1(groupId, myId, &index[0],datumValue),
            __FILE__, __LINE__,
            getDesc(), "getVal()");
}



///////////

// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_text(groupId, myId, 
                               &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, unsigned char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_uchar(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, signed char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_schar(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_short(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_int(groupId, myId, 
                              &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_long(groupId, myId, 
                               &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, float* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_float(groupId, myId, 
                                &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, double* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_double(groupId, myId, 
                                 &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, unsigned short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_ushort(groupId, myId, 
                                 &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, unsigned int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_uint(groupId, myId, 
                               &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_longlong(groupId, myId, 
                                   &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, unsigned long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_ulonglong(groupId, myId, 
                                    &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, char** dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vara(groupId, myId, 
                          &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vara_string(groupId, myId, 
                                 &startp[0],&countp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads an array of values from  a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp, void* dataValues) const {
  ncxxCheck(nc_get_vara(groupId, myId, 
                        &startp[0],&countp[0],dataValues),
            __FILE__, __LINE__,
            getDesc(), "getVal()");
}


///////////

// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_text(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, unsigned char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_uchar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, signed char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_schar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_short(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_int(groupId, myId, 
                              &startp[0],&countp[0],
                              &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_long(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, float* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_float(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, double* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_double(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, unsigned short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_ushort(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, unsigned int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_uint(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_longlong(groupId, myId, 
                                   &startp[0],&countp[0],
                                   &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, unsigned long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_ulonglong(groupId, myId, 
                                    &startp[0],&countp[0],
                                    &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, char** dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_vars(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_vars_string(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a subsampled (strided) array section of values from a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep, void* dataValues) const {
  ncxxCheck(nc_get_vars(groupId, myId, 
                        &startp[0],&countp[0],
                        &stridep[0],dataValues),
            __FILE__, __LINE__,
            getDesc(), "getVal()");
}


///////////

// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_text(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, unsigned char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_uchar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, signed char* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_schar(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_short(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_int(groupId, myId, 
                              &startp[0],&countp[0],
                              &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_long(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, float* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_float(groupId, myId, 
                                &startp[0],&countp[0],
                                &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, double* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_double(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, unsigned short* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_ushort(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, unsigned int* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_uint(groupId, myId, 
                               &startp[0],&countp[0],
                               &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_longlong(groupId, myId, 
                                   &startp[0],&countp[0],
                                   &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, unsigned long long* dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_ulonglong(groupId, myId, 
                                    &startp[0],&countp[0],
                                    &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, char** dataValues) const {
  if (getType().isComplex())
    ncxxCheck(nc_get_varm(groupId, myId, 
                          &startp[0],&countp[0],
                          &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
  else
    ncxxCheck(nc_get_varm_string(groupId, myId, 
                                 &startp[0],&countp[0],
                                 &stridep[0],&imapp[0],dataValues),
              __FILE__, __LINE__,
              getDesc(), "getVal()");
}
// Reads a mapped array section of values from a netCDF variable with no data conversion.
void NcxxVar::getVal(const vector<size_t>& startp,
                     const vector<size_t>& countp,
                     const vector<ptrdiff_t>& stridep,
                     const vector<ptrdiff_t>& imapp, void* dataValues) const {
  ncxxCheck(nc_get_varm(groupId, myId, 
                        &startp[0],&countp[0],
                        &stridep[0],&imapp[0],dataValues),
            __FILE__, __LINE__,
            getDesc(), "getVal()");
}

///////////////////////////////////////////
// add string attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name,
                            const string &val)
{
  clearErrStr();
  try {
    putAtt(name.c_str(), val.c_str());
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add string var attr, name: ", name);
    addErrStr("  val: ", val);
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add double attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, double val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_DOUBLE);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add double var attr, name: ", name);
    addErrDbl("  val: ", val, "%g");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add float attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, float val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_FLOAT);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add float var attr, name: ", name);
    addErrDbl("  val: ", val, "%g");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add int attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, int val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_INT);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add int var attr, name: ", name);
    addErrDbl("  val: ", val, "%d");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add long attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, int64_t val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_INT64);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add int64_t var attr, name: ", name);
    addErrDbl("  val: ", val, "%ld");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add short attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, short val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_SHORT);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add short var attr, name: ", name);
    addErrDbl("  val: ", (int) val, "%d");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add ncbyte attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, signed char val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_BYTE);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add signed char var attr, name: ", name);
    addErrDbl("  val: ", (int) val, "%d");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
}

///////////////////////////////////////////
// add unsigned byte attribute
// Throws NcxxException on error

void NcxxVar::addScalarAttr(const string &name, unsigned char val)
{
  clearErrStr();
  try {
    NcxxType vtype(NcxxType::nc_UBYTE);
    putAtt(name.c_str(), vtype, 1, &val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::addAttr");
    addErrStr("  Cannot add unsigned byte var attr, name: ", name);
    addErrDbl("  val: ", (int) val, "%d");
    addErrStr("  var name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
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
// Throws NcxxException on error

void NcxxVar::write(double val)
  
{
  
  clearErrStr();
  
  if (isNull()) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  var is NULL");
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  nc_type vtype = getType().getId();
  if (vtype != NC_DOUBLE) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  Var type should be double, name: ", getName());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    putVal(index, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  Cannot write scalar double var, name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }

}

///////////////////////////////////////////////////////////////////////////
// write a scalar float variable
// Throws NcxxException on error

void NcxxVar::write(float val)
  
{
  
  clearErrStr();

  if (isNull()) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  var is NULL");
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  nc_type vtype = getType().getId();
  if (vtype != NC_FLOAT) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  Var type should be float, name: ", getName());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    putVal(index, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  Cannot write scalar float var, name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }

}

///////////////////////////////////////////////////////////////////////////
// write a scalar int variable
// Throws NcxxException on error

void NcxxVar::write(int val)
  
{
  
  clearErrStr();

  if (isNull()) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  var is NULL");
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  nc_type vtype = getType().getId();
  if (vtype != NC_INT) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  Var type should be int, name: ", getName());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  vector<size_t> index;
  index.push_back(0);
  try {
    putVal(index, val);
  } catch (NcxxException& e) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  Cannot write scalar int var, name: ", getName());
    addErrStr("  exception: ", e.what());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }

}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in dimension
// Throws NcxxException on error

void NcxxVar::write(const NcxxDim &dim,
                    const void *data)
  
{
  write(dim, dim.getSize(), data);
}

///////////////////////////////////////////////////////////////////////////
// write a 1-D vector variable
// number of elements specified in arguments
// Throws NcxxException on error

void NcxxVar::write(const NcxxDim &dim,
                    size_t count, 
                    const void *data)
  
{

  clearErrStr();

  if (isNull()) {
    addErrStr("ERROR - NcxxVar::write");
    addErrStr("  var is NULL");
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }

  vector<size_t> starts, counts;
  starts.push_back(0);
  counts.push_back(count);
  
  nc_type vtype = getType().getId();
  switch (vtype) {
    case NC_DOUBLE: {
      try {
        putVal(starts, counts, (double *) data);
      } catch (NcxxException& e) {
        addErrStr("  exception: ", e.what());
        throw NcxxException(getErrStr(), __FILE__, __LINE__);
      }
      break;
    }
    case NC_INT: {
      try {
        putVal(starts, counts, (int *) data);
      } catch (NcxxException& e) {
        addErrStr("  exception: ", e.what());
        throw NcxxException(getErrStr(), __FILE__, __LINE__);
      }
      break;
    }
    case NC_SHORT: {
      try {
        putVal(starts, counts, (short *) data);
      } catch (NcxxException& e) {
        addErrStr("  exception: ", e.what());
        throw NcxxException(getErrStr(), __FILE__, __LINE__);
      }
      break;
    }
    case NC_UBYTE: {
      try {
        putVal(starts, counts, (unsigned char *) data);
      } catch (NcxxException& e) {
        addErrStr("  exception: ", e.what());
        throw NcxxException(getErrStr(), __FILE__, __LINE__);
      }
      break;
    }
    case NC_FLOAT:
    default: {
      try {
        putVal(starts, counts, (float *) data);
      } catch (NcxxException& e) {
        addErrStr("  exception: ", e.what());
        throw NcxxException(getErrStr(), __FILE__, __LINE__);
      }
      break;
    }
  } // switch
  
}


///////////////////////////////////////////////////////////////////////////
// write a string variable
// Throws NcxxException on error

void NcxxVar::writeStrings(const void *str)
  
{

  clearErrStr();

  if (isNull()) {
    addErrStr("ERROR - NcxxVar::writeStrings");
    addErrStr("  var is NULL");
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }
  
  std::vector<NcxxDim> dims = getDims();
  size_t nDims = dims.size();
  if (nDims < 1) {
    addErrStr("ERROR - NcxxVar::writeStrings");
    addErrStr("  var has no dimensions");
    addErrStr("  var name: ", getName());
    throw NcxxException(getErrStr(), __FILE__, __LINE__);
  }

  if (nDims == 1) {

    // single dimension

    NcxxDim &dim0 = dims[0];
    if (dim0.isNull()) {
      addErrStr("ERROR - NcxxVar::writeStrings");
      addErrStr("  Cannot write var, name: ", getName());
      addErrStr("  dim 0 is NULL");
      throw NcxxException(getErrStr(), __FILE__, __LINE__);
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize());
    try {
      putVal(starts, counts, (char *) str);
      return;
    } catch (NcxxException& e) {
      addErrStr("ERROR - NcxxVar::writeStrings");
      addErrStr("  Cannot write var, name: ", getName());
      addErrStr("  exception: ", e.what());
      throw NcxxException(getErrStr(), __FILE__, __LINE__);
    }

  } // if (nDims == 1)

  if (nDims == 2) {

    // two dimensions

    NcxxDim &dim0 = dims[0];
    if (dim0.isNull()) {
      addErrStr("ERROR - NcxxVar::writeStrings");
      addErrStr("  Cannot write var, name: ", getName());
      addErrStr("  dim 0 is NULL");
      throw NcxxException(getErrStr(), __FILE__, __LINE__);
    }

    NcxxDim &dim1 = dims[1];
    if (dim1.isNull()) {
      addErrStr("ERROR - NcxxVar::writeStrings");
      addErrStr("  Cannot write var, name: ", getName());
      addErrStr("  dim 1 is NULL");
      throw NcxxException(getErrStr(), __FILE__, __LINE__);
    }

    vector<size_t> starts, counts;
    starts.push_back(0);
    counts.push_back(dim0.getSize() * dim1.getSize());
    try {
      putVal(starts, counts, (char *) str);
      return;
    } catch (NcxxException& e) {
      addErrStr("ERROR - NcxxVar::writeStrings");
      addErrStr("  Cannot write var, name: ", getName());
      addErrStr("  exception: ", e.what());
      throw NcxxException(getErrStr(), __FILE__, __LINE__);
    }

  } // if (nDims == 2) 
  
  // more than 2 is an error
  
  addErrStr("ERROR - NcxxVar::writeStrings");
  addErrStr("  Cannot write var, name: ", getName());
  addErrInt("  more than 2 dimensions: ", nDims);
  throw NcxxException(getErrStr(), __FILE__, __LINE__);

}

////////////////////////////////////////
// set default fill value, based on type
// throws NcxxException on error

void NcxxVar::setDefaultFillValue()

{

  nc_type vtype = getType().getId();
  if (vtype == NC_DOUBLE) {
    addScalarAttr("_FillValue", Ncxx::missingDouble);
    return;
  }
  if (vtype == NC_FLOAT) {
    addScalarAttr("_FillValue", Ncxx::missingFloat);
    return;
  }
  if (vtype == NC_INT) {
    addScalarAttr("_FillValue", Ncxx::missingInt);
    return;
  }
  if (vtype == NC_LONG) {
    addScalarAttr("_FillValue", /* (long) */ Ncxx::missingInt);
    return;
  }
  if (vtype == NC_SHORT) {
    addScalarAttr("_FillValue", (short) Ncxx::missingInt);
    return;
  }
  if (vtype == NC_UBYTE) {
    addScalarAttr("_FillValue", Ncxx::missingUchar);
    return;
  }
}

////////////////////////////////////////
// set meta fill value, based on type
// throws NcxxException on error

void NcxxVar::setMetaFillValue()

{

  nc_type vtype = getType().getId();
  if (vtype == NC_DOUBLE) {
    addScalarAttr("_FillValue", Ncxx::missingMetaDouble);
    return;
  }
  if (vtype == NC_FLOAT) {
    addScalarAttr("_FillValue", Ncxx::missingMetaFloat);
    return;
  }
  if (vtype == NC_INT) {
    addScalarAttr("_FillValue", Ncxx::missingMetaInt);
    return;
  }
  if (vtype == NC_LONG) {
    addScalarAttr("_FillValue", /* (long) */ Ncxx::missingMetaInt);
    return;
  }
  if (vtype == NC_SHORT) {
    addScalarAttr("_FillValue", (short) Ncxx::missingMetaInt);
    return;
  }
  if (vtype == NC_UBYTE) {
    addScalarAttr("_FillValue", Ncxx::missingMetaUchar);
    return;
  }
}

////////////////////////////////////////
// convert var type string

string NcxxVar::varTypeToStr() const
      
{
  nc_type vtype = getType().getId();
  return Ncxx::ncTypeToStr(vtype);
}

////////////////////////////////////////
// create the description string

string NcxxVar::getDesc() const
      
{

  string desc;
  desc += ("Var:" + getName());
  desc += (" type:" + varTypeToStr());

  for (int ii = 0; ii < getDimCount(); ii++) {
    NcxxDim dim = getDim(ii);
    char dimStr[256];
    sprintf(dimStr, " dim[%d]:%s-size:%ld",
            ii, dim.getName().c_str(), dim.getSize());
    desc += dimStr;
  }
  
  return desc;

}

