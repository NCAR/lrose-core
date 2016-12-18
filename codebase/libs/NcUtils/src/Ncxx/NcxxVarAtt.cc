#include <NcUtils/NcxxVar.hh>
#include <NcUtils/NcxxVarAtt.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <netcdf.h>
using namespace std;


namespace netCDF {
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
}


using namespace netCDF;


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
