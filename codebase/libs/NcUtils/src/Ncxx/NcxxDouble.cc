#include <NcUtils/NcxxDouble.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxDouble  called netCDF::ncDouble
namespace netCDF {
  NcxxDouble ncDouble;
}

// constructor
NcxxDouble::NcxxDouble() : NcxxType(NC_DOUBLE){
}

NcxxDouble::~NcxxDouble() {
}


// equivalence operator
bool NcxxDouble::operator==(const NcxxDouble & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
