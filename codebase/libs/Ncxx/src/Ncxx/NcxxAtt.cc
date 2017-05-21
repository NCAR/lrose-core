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
#include <Ncxx/NcxxAtt.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxException.hh>
#include <ncException.h>
#include <Ncxx/NcxxCheck.hh>

using namespace std;
  
// destructor  (defined even though it is virtual)
NcxxAtt::~NcxxAtt() {}

// assignment operator
NcxxAtt& NcxxAtt::operator=(const NcxxAtt& rhs)
{
  if (&rhs == this) {
    return *this;
  }
  _errStr = rhs._errStr;
  nullObject = rhs.nullObject;
  myName = rhs.myName;
  groupId = rhs.groupId;
  varId =rhs.varId;
  return *this;
}

// Constructor generates a null object.
NcxxAtt::NcxxAtt() : 
        nullObject(true)
{
  _errStr.clear();
}

// Constructor for non-null instances.
NcxxAtt::NcxxAtt(bool nullObject): 
        nullObject(nullObject)
{
  _errStr.clear();
}

// The copy constructor.
NcxxAtt::NcxxAtt(const NcxxAtt& rhs) :
        nullObject(rhs.nullObject),
        myName(rhs.myName),
        groupId(rhs.groupId),
        varId(rhs.varId)
{
  _errStr = rhs._errStr;
}


// equivalence operator
bool NcxxAtt::operator==(const NcxxAtt & rhs) const
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myName == rhs.myName && groupId == rhs.groupId && varId == rhs.varId;
}  

//  !=  operator
bool NcxxAtt::operator!=(const NcxxAtt & rhs) const
{
  return !(*this == rhs);
}  

// Gets parent group.
NcxxGroup  NcxxAtt::getParentGroup() const {
  return NcxxGroup(groupId);
}
      

// Returns the attribute type.
NcxxType  NcxxAtt::getType() const{
  // get the identifier for the netCDF type of this attribute.
  nc_type xtypep;
  ncxxCheck(nc_inq_atttype(groupId,varId,myName.c_str(),&xtypep),__FILE__,__LINE__);
  if(xtypep <= 12)
    // This is an atomic type
    return NcxxType(xtypep);
  else
    // this is a user-defined type
  {
    // now get the set of NcxxType objects in this file.
    multimap<string,NcxxType> typeMap(getParentGroup().getTypes(NcxxGroup::ParentsAndCurrent));
    multimap<string,NcxxType>::iterator iter;
    // identify the Nctype object with the same id as this attribute.
    for (iter=typeMap.begin(); iter!= typeMap.end();iter++) {
      if(iter->second.getId() == xtypep) return iter->second;
    }
    // return a null object, as no type was identified.
    return NcxxType();
  }
}

// Gets attribute length - i.e. number of values in attribute.

size_t  NcxxAtt::getAttLength() const{
  size_t lenp;
  ncxxCheck(nc_inq_attlen(groupId, varId, myName.c_str(), &lenp),__FILE__,__LINE__);
  return lenp;
}

///////////////////////////////////////////////////////
// Gets a netCDF variable attribute.

void NcxxAtt::getValues(string& dataValues) const {

  NcxxType::ncxxType typeClass(getType().getTypeClass());

  size_t att_len=getAttLength();
  char* tmpValues = new char[att_len + 1];  /* + 1 for trailing null */

  if(typeClass == NcxxType::nc_VLEN ||
     typeClass == NcxxType::nc_OPAQUE ||
     typeClass == NcxxType::nc_ENUM ||
     typeClass == NcxxType::nc_COMPOUND) {
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),tmpValues),
              __FILE__, __LINE__);
  } else {
    ncxxCheck(nc_get_att_text(groupId,varId,myName.c_str(),tmpValues),
              __FILE__, __LINE__);
  }

  // find length to null

  size_t finalLen = 0;
  for (size_t ii = 0; ii < att_len; ii++) {
    if ((int) tmpValues[ii] == 0) {
      break;
    }
    finalLen = ii + 1;
  }

  // set return value

  dataValues=string(tmpValues,finalLen);

  // clean up

  delete[] tmpValues;

}

///////////////////////////////////////////////////////
// Gets a netCDF variable attribute.

void NcxxAtt::getValues(char* dataValues) const {

  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN ||
     typeClass == NcxxType::nc_OPAQUE ||
     typeClass == NcxxType::nc_ENUM ||
     typeClass == NcxxType::nc_COMPOUND) {
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),
              __FILE__, __LINE__);
  } else {
    ncxxCheck(nc_get_att_text(groupId,varId,myName.c_str(),dataValues),
              __FILE__, __LINE__);
  }

}


// Gets a netCDF variable attribute.

void NcxxAtt::getValues(unsigned char* dataValues) const {

  NcxxType::ncxxType typeClass(getType().getTypeClass());

  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_uchar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);

}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(signed char* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_schar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_short(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_int(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_long(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(float* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_float(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(double* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_double(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(unsigned short* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_ushort(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(unsigned int* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_uint(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_longlong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(unsigned long long* dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_ulonglong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.

void NcxxAtt::getValues(char** dataValues) const {
  NcxxType::ncxxType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_string(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(void* dataValues) const {
  ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

//////////////////////////////////////////////////////
// get values of different types loaded into vector
// vector is resized to hold attribute length
// returns 0 on success, -1 on failure
// On failure, getErrStr() holds error message.

int NcxxAtt::getValues(vector<char *> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<char> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<unsigned char> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<short> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<unsigned short> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<int> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<unsigned int> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<long> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<long long> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<unsigned long long> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<float> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}

int NcxxAtt::getValues(vector<double> &dataValues) const
{
  clearErrStr();
  size_t attLen = getAttLength();
  dataValues.resize(attLen);
  if (attLen < 1) {
    return -1;
  }
  try {
    getValues(&dataValues[0]);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxAtt::getValues");
    _addErrStr("  Cannot get values, attr name: ", myName);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  return 0;
}


///////////////////////////////////////////
// get string representation of attribute

std::string NcxxAtt::asString()
  
{
  std::string val;
  getValues(val);
  return val;
}

