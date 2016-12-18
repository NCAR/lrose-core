#include <NcUtils/NcxxDim.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <algorithm>
using namespace std;


namespace netCDF {
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
}

using namespace netCDF;

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
