#include <NcUtils/NcxxOpaqueType.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxException.hh>
#include <netcdf.h>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Class represents a netCDF variable.
using namespace netCDF;
  
// assignment operator
NcxxOpaqueType& NcxxOpaqueType::operator=(const NcxxOpaqueType& rhs)
{
  // assign base class parts
  NcxxType::operator=(rhs);    
  return *this;
}
  
// assignment operator
NcxxOpaqueType& NcxxOpaqueType::operator=(const NcxxType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Opaque type
    if(getTypeClass() != NC_OPAQUE) 	throw NcxxException("The NcxxType object must be the base of an Opaque type.",__FILE__,__LINE__);
    // assign base class parts
    NcxxType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcxxOpaqueType::NcxxOpaqueType(const NcxxOpaqueType& rhs): 
  NcxxType(rhs)
{
}


// Constructor generates a null object.
NcxxOpaqueType::NcxxOpaqueType() :
  NcxxType()   // invoke base class constructor
{}


// constructor
NcxxOpaqueType::NcxxOpaqueType(const NcxxGroup& grp, const string& name) :
  NcxxType(grp,name)
{}
  
  
// constructor
NcxxOpaqueType::NcxxOpaqueType(const NcxxType& ncType) :
  NcxxType(ncType)
{
  // check the nctype object is the base of a Opaque type
  if(getTypeClass() != NC_OPAQUE) 	throw NcxxException("The NcxxType object must be the base of an Opaque type.",__FILE__,__LINE__);
}
  
// Returns the size of the opaque type in bytes.
size_t  NcxxOpaqueType::getTypeSize() const
{
  char* charName;
  charName=NULL;
  size_t sizep;
  ncxxCheck(nc_inq_opaque(groupId,myId,charName,&sizep),__FILE__,__LINE__);
  return sizep;
}
