#include <NcUtils/NcxxGroupAtt.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <netcdf.h>
using namespace std;


namespace netCDF {
  //  Global comparator operator ==============
  // comparator operator 
  bool operator<(const NcxxGroupAtt& lhs,const NcxxGroupAtt& rhs)
  {
    return false;
  }
  
  // comparator operator 
  bool operator>(const NcxxGroupAtt& lhs,const NcxxGroupAtt& rhs)
  {
    return true;
  }
}


using namespace netCDF;

// assignment operator
NcxxGroupAtt& NcxxGroupAtt::operator=(const NcxxGroupAtt & rhs)
{
  NcxxAtt::operator=(rhs);    // assign base class parts
  return *this;
}

//! The copy constructor.
NcxxGroupAtt::NcxxGroupAtt(const NcxxGroupAtt& rhs): 
  NcxxAtt(rhs)   // invoke base class copy constructor
{}


// Constructor generates a null object.
NcxxGroupAtt::NcxxGroupAtt() : 
  NcxxAtt()  // invoke base class constructor
{}

// equivalence operator (doesn't bother compaing varid's of each object).
bool NcxxGroupAtt::operator==(const NcxxGroupAtt & rhs)
{
  if(nullObject) 
    return nullObject == rhs.isNull();
  else
    return myName == rhs.myName && groupId == rhs.groupId;
}  

// Constructor for an existing global attribute.
NcxxGroupAtt::NcxxGroupAtt(const NcxxGroup& grp, const int index):
  NcxxAtt(false)
{
  groupId =  grp.getId();
  varId = NC_GLOBAL;
  // get the name of this attribute
  char attName[NC_MAX_NAME+1];
  ncxxCheck(nc_inq_attname(groupId,varId, index, attName),__FILE__,__LINE__);
  ncxxCheck(nc_inq_attname(groupId,varId,index,attName),__FILE__,__LINE__);
  myName = attName;
}

