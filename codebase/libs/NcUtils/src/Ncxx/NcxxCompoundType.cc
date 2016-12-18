#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxCompoundType.hh>
#include <NcUtils/NcxxByte.hh>
#include <NcUtils/NcxxUbyte.hh>
#include <NcUtils/NcxxChar.hh>
#include <NcUtils/NcxxShort.hh>
#include <NcUtils/NcxxUshort.hh>
#include <NcUtils/NcxxInt.hh>
#include <NcUtils/NcxxUint.hh>
#include <NcUtils/NcxxInt64.hh>
#include <NcUtils/NcxxUint64.hh>
#include <NcUtils/NcxxFloat.hh>
#include <NcUtils/NcxxDouble.hh>
#include <NcUtils/NcxxString.hh>
#include <NcUtils/NcxxException.hh>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Class represents a netCDF variable.

// assignment operator
NcxxCompoundType& NcxxCompoundType::operator=(const NcxxCompoundType& rhs)
{
  NcxxType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcxxCompoundType& NcxxCompoundType::operator=(const NcxxType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of a Compound type
    if(getTypeClass() != nc_COMPOUND) 	throw NcxxException("The NcxxType object must be the base of a Compound type.",__FILE__,__LINE__);
    // assign base class parts
    NcxxType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcxxCompoundType::NcxxCompoundType(const NcxxCompoundType& rhs): 
  NcxxType(rhs)
{
}


// equivalence operator
bool NcxxCompoundType::operator==(const NcxxCompoundType& rhs)
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myId ==rhs.myId && groupId == rhs.groupId;
}  
  
// Constructor generates a null object.
NcxxCompoundType::NcxxCompoundType() : 
  NcxxType()   // invoke base class constructor
{}
  
// constructor
NcxxCompoundType::NcxxCompoundType(const NcxxGroup& grp, const string& name): 
  NcxxType(grp,name)
{
}

// constructor
// The copy constructor.
NcxxCompoundType::NcxxCompoundType(const NcxxType& rhs): 
  NcxxType()
{
  // assign base class parts
  NcxxType::operator=(rhs);
}
  
//  Inserts a named field.
void NcxxCompoundType::addMember(const string& memberName, const NcxxType& newMemberType,size_t offset)
{
  ncxxCheck(nc_insert_compound(groupId,myId,const_cast<char*>(memberName.c_str()),offset,newMemberType.getId()),__FILE__,__LINE__);
}



//  Inserts a named array field.
void NcxxCompoundType::addMember(const string& memberName, const NcxxType& newMemberType, size_t offset, const vector<int>& shape)
{
  if (!shape.empty())
    ncxxCheck(nc_insert_array_compound(groupId, myId,const_cast<char*>(memberName.c_str()), offset, newMemberType.getId(), shape.size(), const_cast<int*>(&shape[0])),__FILE__,__LINE__);
  else
    addMember(memberName, newMemberType, offset);
}



// Returns number of members in this NcxxCompoundType object.
size_t  NcxxCompoundType::getMemberCount() const
{
  size_t nfieldsp;
  ncxxCheck(nc_inq_compound_nfields(groupId,myId,&nfieldsp),__FILE__,__LINE__);
  return nfieldsp;
}
  

// Returns a NcxxType object for a single member. */
NcxxType NcxxCompoundType::getMember(int memberIndex) const 
{
  nc_type fieldtypeidp;
  ncxxCheck(nc_inq_compound_fieldtype(groupId,myId,memberIndex,&fieldtypeidp),__FILE__,__LINE__);
  switch (fieldtypeidp) {
  case NC_BYTE    : return ncByte;
  case NC_UBYTE   : return ncUbyte;
  case NC_CHAR    : return ncChar;
  case NC_SHORT   : return ncShort;
  case NC_USHORT  : return ncUshort;
  case NC_INT     : return ncInt;
  case NC_UINT    : return ncUint;  
  case NC_INT64   : return ncInt64; 
  case NC_UINT64  : return ncUint64;
  case NC_FLOAT   : return ncFloat;
  case NC_DOUBLE  : return ncDouble;
  case NC_STRING  : return ncString;
  default:  
    // this is a user defined type
    return NcxxType(getParentGroup(),fieldtypeidp);
  }
}

// Returns name of member field.
std::string NcxxCompoundType::getMemberName(int memberIndex) const
{
  char fieldName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_compound_fieldname(groupId,myId,memberIndex, fieldName),__FILE__,__LINE__);
  return std::string(fieldName);
}

// Returns index of named member field.
int NcxxCompoundType::getMemberIndex(const std::string& memberName) const{
  int memberIndex;
  ncxxCheck(nc_inq_compound_fieldindex(groupId,myId, memberName.c_str(),&memberIndex),__FILE__,__LINE__);
  return memberIndex;
}

// Returns the number of dimensions of a member with the given index.
int NcxxCompoundType::getMemberDimCount(int memberIndex) const 
{
  int ndimsp;
  ncxxCheck(nc_inq_compound_fieldndims(groupId,myId,memberIndex, &ndimsp),__FILE__,__LINE__);
  return ndimsp;
}
  
  
// Returns the shape of the given member.
vector<int> NcxxCompoundType::getMemberShape(int memberIndex) const 
{
  vector<int> dim_size;
  dim_size.resize(getMemberDimCount(memberIndex));
  if(!dim_size.empty())
    ncxxCheck(nc_inq_compound_fielddim_sizes(groupId,myId,memberIndex,&dim_size[0]),__FILE__,__LINE__);
  return dim_size;
}
 

// Returns the offset of the member with given index.
size_t NcxxCompoundType::getMemberOffset(const int index) const
{
  size_t offsetp;
  ncxxCheck(nc_inq_compound_fieldoffset(groupId,myId, index,&offsetp),__FILE__,__LINE__);
  return offsetp;
}
