#include <NcUtils/NcxxUint.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxUint  called netCDF::ncUint
namespace netCDF {
  NcxxUint ncUint;
}

// constructor
NcxxUint::NcxxUint() : NcxxType(NC_UINT){
}

NcxxUint::~NcxxUint() {
}


// equivalence operator
bool NcxxUint::operator==(const NcxxUint & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
