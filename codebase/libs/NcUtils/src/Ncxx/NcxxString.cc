#include <NcUtils/NcxxString.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxString  called netCDF::ncString
namespace netCDF {
  NcxxString ncString;
}

// constructor
NcxxString::NcxxString() : NcxxType(NC_STRING){
}

NcxxString::~NcxxString() {
}


// equivalence operator
bool NcxxString::operator==(const NcxxString & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
