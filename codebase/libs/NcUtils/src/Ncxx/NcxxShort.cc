#include <NcUtils/NcxxShort.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxShort  called netCDF::ncShort
namespace netCDF {
  NcxxShort ncShort;
}

// constructor
NcxxShort::NcxxShort() : NcxxType(NC_SHORT){
}

NcxxShort::~NcxxShort() {
}


// equivalence operator
bool NcxxShort::operator==(const NcxxShort & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
