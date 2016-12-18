#include <NcUtils/NcxxInt64.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxInt64  called netCDF::ncInt64
namespace netCDF {
  NcxxInt64 ncInt64;
}

// constructor
NcxxInt64::NcxxInt64() : NcxxType(NC_INT64){
}

NcxxInt64::~NcxxInt64() {
}


// equivalence operator
bool NcxxInt64::operator==(const NcxxInt64 & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
