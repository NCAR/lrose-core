#include <NcUtils/NcxxAtt.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <vector>

using namespace std;
using namespace netCDF;
  

// destructor  (defined even though it is virtual)
NcxxAtt::~NcxxAtt() {}

// assignment operator
NcxxAtt& NcxxAtt::operator=(const NcxxAtt& rhs)
{
  nullObject = rhs.nullObject;
  myName = rhs.myName;
  groupId = rhs.groupId;
  varId =rhs.varId;
  return *this;
}

// Constructor generates a null object.
NcxxAtt::NcxxAtt() : 
  nullObject(true) 
{}

// Constructor for non-null instances.
NcxxAtt::NcxxAtt(bool nullObject): 
  nullObject(nullObject)
{}

// The copy constructor.
NcxxAtt::NcxxAtt(const NcxxAtt& rhs) :
  nullObject(rhs.nullObject),
  myName(rhs.myName),
  groupId(rhs.groupId),
   varId(rhs.varId)
{}


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
netCDF::NcxxGroup  NcxxAtt::getParentGroup() const {
  return netCDF::NcxxGroup(groupId);
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

// Gets attribute length.
size_t  NcxxAtt::getAttLength() const{
  size_t lenp;
  ncxxCheck(nc_inq_attlen(groupId, varId, myName.c_str(), &lenp),__FILE__,__LINE__);
  return lenp;
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(string& dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());

  size_t att_len=getAttLength();
  char* tmpValues;
  tmpValues = (char *) malloc(att_len + 1);  /* + 1 for trailing null */

  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),tmpValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_text(groupId,varId,myName.c_str(),tmpValues),__FILE__,__LINE__);
  dataValues=string(tmpValues,att_len);
  free(tmpValues);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_text(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}


// Gets a netCDF variable attribute.
void NcxxAtt::getValues(unsigned char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_uchar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(signed char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_schar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_short(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_int(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_long(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(float* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_float(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(double* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_double(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(unsigned short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_ushort(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(unsigned int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_uint(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_longlong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(unsigned long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_ulonglong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(char** dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND) 
    ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_att_string(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

// Gets a netCDF variable attribute.
void NcxxAtt::getValues(void* dataValues) const {
  ncxxCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__);
}

