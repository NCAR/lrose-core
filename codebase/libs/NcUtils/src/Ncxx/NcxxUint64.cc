#include <NcUtils/NcxxUint64.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxUint64  called netCDF::ncUint64
namespace netCDF {
  NcxxUint64 ncUint64;
}

// constructor
NcxxUint64::NcxxUint64() : NcxxType(NC_UINT64){
}

NcxxUint64::~NcxxUint64() {
}


// equivalence operator
bool NcxxUint64::operator==(const NcxxUint64 & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
