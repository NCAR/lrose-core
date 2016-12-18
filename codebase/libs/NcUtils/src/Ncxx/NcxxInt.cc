#include <NcUtils/NcxxInt.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxInt  called netCDF::ncInt
namespace netCDF {
  NcxxInt ncInt;
}

// constructor
NcxxInt::NcxxInt() : NcxxType(NC_INT){
}

NcxxInt::~NcxxInt() {
}


// equivalence operator
bool NcxxInt::operator==(const NcxxInt & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
