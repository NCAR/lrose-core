#include <NcUtils/NcxxVarAtt.hh>
#include <NcUtils/NcxxDim.hh>
#include <NcUtils/NcxxVar.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxException.hh>
#include<netcdf.h>
using namespace std;
using namespace netCDF::exceptions;

namespace netCDF {
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
}

using namespace netCDF;

// assignment operator
NcxxVar& NcxxVar::operator=(const NcxxVar & rhs)
{
  nullObject = rhs.nullObject;
  myId = rhs.myId;
  groupId = rhs.groupId;
  return *this;
}

// The copy constructor.
NcxxVar::NcxxVar(const NcxxVar& rhs) :
  nullObject(rhs.nullObject),
  myId(rhs.myId),
  groupId(rhs.groupId)
{}


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

// Constructor generates a null object.
NcxxVar::NcxxVar() : nullObject(true),
                 myId(-1),
                 groupId(-1)
{}

// Constructor for a variable (must already exist in the netCDF file.)
NcxxVar::NcxxVar (const NcxxGroup& grp, const int& varId) :
  nullObject (false),
  myId (varId),
  groupId(grp.getId())
{}



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

  if(xtypep ==  ncByte.getId()    ) return ncByte;
  if(xtypep ==  ncUbyte.getId()   ) return ncUbyte;
  if(xtypep ==  ncChar.getId()    ) return ncChar;
  if(xtypep ==  ncShort.getId()   ) return ncShort;
  if(xtypep ==  ncUshort.getId()  ) return ncUshort;
  if(xtypep ==  ncInt.getId()     ) return ncInt;
  if(xtypep ==  ncUint.getId()    ) return ncUint;
  if(xtypep ==  ncInt64.getId()   ) return ncInt64;
  if(xtypep ==  ncUint64.getId()  ) return ncUint64;
  if(xtypep ==  ncFloat.getId()   ) return ncFloat;
  if(xtypep ==  ncDouble.getId()  ) return ncDouble;
  if(xtypep ==  ncString.getId()  ) return ncString;

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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
  NcxxType::ncType typeClass(type.getTypeClass());
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
void NcxxVar::putVar(const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_text(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_uchar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_schar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_short(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_int(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_long(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_float(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_double(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_ushort(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_uint(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_longlong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_ulonglong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable.
void NcxxVar::putVar(const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var_string(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Writes the entire data into the netCDF variable with no data conversion.
void NcxxVar::putVar(const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_var(groupId, myId,dataValues),__FILE__,__LINE__);
}


///////////////////
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const string& datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    {
      const char* tmpPtr = datumValue.c_str();
      ncxxCheck(nc_put_var1_string(groupId, myId,&index[0],&tmpPtr),__FILE__,__LINE__);
    }
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const unsigned char* datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_uchar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const signed char* datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_schar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const short datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_short(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const int datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_int(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const long datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_long(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const float datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_float(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const double datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_double(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const unsigned short datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_ushort(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const unsigned int datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_uint(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const long long datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_longlong(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const unsigned long long datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_ulonglong(groupId, myId,&index[0],&datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& index, const char** datumValue) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    throw NcxxException("user-defined type must be of type void",__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_var1_string(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Writes a single datum value into the netCDF variable with no data conversion.
void NcxxVar::putVar(const vector<size_t>& index, const void* datumValue) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}


////////////////////

// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_text(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_uchar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_schar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_short(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_int(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_long(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_float(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_double(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_ushort(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_uint(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_longlong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_ulonglong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vara_string(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Writes an array of values into the netCDF variable with no data conversion.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>& countp, const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}



////////////////////

// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_text(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_short(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_int(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_long(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_float(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_double(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_vars_string(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Writes a set of subsampled array values into the netCDF variable with no data conversion.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep,  const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}


////////////////////
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_text(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const signed char* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_short(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_int(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_long(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const float* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_float(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const double* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_double(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned short* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned int* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const unsigned long long* dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const char** dataValues) const {
  ncxxCheckDataMode(groupId);
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_put_varm_string(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Writes a mapped array section of values into the netCDF variable with no data conversion.
void NcxxVar::putVar(const vector<size_t>& startp, const vector<size_t>&countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, const void* dataValues) const {
    ncxxCheckDataMode(groupId);
    ncxxCheck(nc_put_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}





// Data reading



// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_text(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(unsigned char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_uchar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(signed char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_schar(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_short(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_int(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_long(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(float* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_float(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(double* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_double(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(unsigned short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_ushort(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(unsigned int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_uint(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_longlong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(unsigned long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_ulonglong(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable.
void NcxxVar::getVar(char** dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var_string(groupId, myId,dataValues),__FILE__,__LINE__);
}
// Reads the entire data of the netCDF variable with no data conversion.
void NcxxVar::getVar(void* dataValues) const {
    ncxxCheck(nc_get_var(groupId, myId,dataValues),__FILE__,__LINE__);
}



///////////

// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, char* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_text(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, unsigned char* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_uchar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, signed char* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_schar(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, short* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_short(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, int* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_int(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, long* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_long(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, float* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_float(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, double* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_double(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, unsigned short* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_ushort(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, unsigned int* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_uint(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, long long* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_longlong(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable
void NcxxVar::getVar(const vector<size_t>& index, unsigned long long* datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_ulonglong(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& index, char** datumValue) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_var1_string(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}
// Reads a single datum value of a netCDF variable with no data conversion.
void NcxxVar::getVar(const vector<size_t>& index, void* datumValue) const {
    ncxxCheck(nc_get_var1(groupId, myId,&index[0],datumValue),__FILE__,__LINE__);
}



///////////

// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_text(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_uchar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, signed char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_schar(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_short(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_int(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_long(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, float* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_float(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, double* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_double(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_ushort(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_uint(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_longlong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, unsigned long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_ulonglong(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, char** dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vara_string(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}
// Reads an array of values from  a netCDF variable with no data conversion.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, void* dataValues) const {
    ncxxCheck(nc_get_vara(groupId, myId,&startp[0],&countp[0],dataValues),__FILE__,__LINE__);
}


///////////

// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_text(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, signed char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_short(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_int(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_long(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, float* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_float(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, double* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_double(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, unsigned long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, char** dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_vars_string(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}
// Reads a subsampled (strided) array section of values from a netCDF variable with no data conversion.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, void* dataValues) const {
    ncxxCheck(nc_get_vars(groupId, myId,&startp[0],&countp[0],&stridep[0],dataValues),__FILE__,__LINE__);
}


///////////

// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_text(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_uchar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, signed char* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_schar(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_short(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_int(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_long(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, float* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_float(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, double* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_double(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned short* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN ||typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_ushort(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned int* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_uint(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_longlong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, unsigned long long* dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_ulonglong(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, char** dataValues) const {
  NcxxType::ncType typeClass(getType().getTypeClass());
  if(typeClass == NcxxType::nc_VLEN || typeClass == NcxxType::nc_OPAQUE || typeClass == NcxxType::nc_ENUM || typeClass == NcxxType::nc_COMPOUND)
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
  else
    ncxxCheck(nc_get_varm_string(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
// Reads a mapped array section of values from a netCDF variable with no data conversion.
void NcxxVar::getVar(const vector<size_t>& startp, const vector<size_t>& countp, const vector<ptrdiff_t>& stridep, const vector<ptrdiff_t>& imapp, void* dataValues) const {
    ncxxCheck(nc_get_varm(groupId, myId,&startp[0],&countp[0],&stridep[0],&imapp[0],dataValues),__FILE__,__LINE__);
}
