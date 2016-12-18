#include <NcUtils/NcxxUshort.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxUshort  called netCDF::ncUshort
namespace netCDF {
  NcxxUshort ncUshort;
}

// constructor
NcxxUshort::NcxxUshort() : NcxxType(NC_USHORT){
}

NcxxUshort::~NcxxUshort() {
}


// equivalence operator
bool NcxxUshort::operator==(const NcxxUshort & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
