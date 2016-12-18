#include <NcUtils/NcxxFloat.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxFloat  called netCDF::ncFloat
namespace netCDF {
  NcxxFloat ncFloat;
}

// constructor
NcxxFloat::NcxxFloat() : NcxxType(NC_FLOAT){
}

NcxxFloat::~NcxxFloat() {
}


// equivalence operator
bool NcxxFloat::operator==(const NcxxFloat & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
