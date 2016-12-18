#include <NcUtils/NcxxUbyte.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxUbyte  called netCDF::ncUbyte
namespace netCDF {
  NcxxUbyte ncUbyte;
}

// constructor
NcxxUbyte::NcxxUbyte() : NcxxType(NC_UBYTE){
}

NcxxUbyte::~NcxxUbyte() {
}


// equivalence operator
bool NcxxUbyte::operator==(const NcxxUbyte & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
