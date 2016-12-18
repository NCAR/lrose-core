#include <NcUtils/NcxxVlenType.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxException.hh>
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
#include <netcdf.h>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Class represents a netCDF variable.
using namespace netCDF;

// assignment operator
NcxxVlenType& NcxxVlenType::operator=(const NcxxVlenType& rhs)
{
  NcxxType::operator=(rhs);    // assign base class parts
  return *this;
}

// assignment operator
NcxxVlenType& NcxxVlenType::operator=(const NcxxType& rhs)
{
  if (&rhs != this) {
    // check the rhs is the base of an Opaque type
    if(getTypeClass() != NC_VLEN) 	throw NcxxException("The NcxxType object must be the base of an Vlen type.",__FILE__,__LINE__);
    // assign base class parts
    NcxxType::operator=(rhs);
  }
  return *this;
}

// The copy constructor.
NcxxVlenType::NcxxVlenType(const NcxxVlenType& rhs):   
  NcxxType(rhs)
{
}


// Constructor generates a null object.
NcxxVlenType::NcxxVlenType() :
  NcxxType()   // invoke base class constructor
{}

// constructor
NcxxVlenType::NcxxVlenType(const NcxxGroup& grp, const string& name) :
  NcxxType(grp,name)
{}
  
// constructor
NcxxVlenType::NcxxVlenType(const NcxxType& ncType): 
  NcxxType(ncType)
{
  // check the nctype object is the base of a Vlen type
  if(getTypeClass() != NC_VLEN) throw NcxxException("The NcxxType object must be the base of a Vlen type.",__FILE__,__LINE__);
}

// Returns the base type.
NcxxType NcxxVlenType::getBaseType() const
{
  char charName[NC_MAX_NAME+1];
  nc_type base_nc_typep;
  size_t datum_sizep;
  ncxxCheck(nc_inq_vlen(groupId,myId,charName,&datum_sizep,&base_nc_typep),__FILE__,__LINE__);
  switch (base_nc_typep) {
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
    return NcxxType(getParentGroup(),base_nc_typep);
  }
}
