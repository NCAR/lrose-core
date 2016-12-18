#include <NcUtils/NcxxByte.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxByte  called netCDF::ncByte
namespace netCDF {
  NcxxByte ncByte;
}

// constructor
NcxxByte::NcxxByte() : NcxxType(NC_BYTE){
}

NcxxByte::~NcxxByte() {
}

int NcxxByte::sizeoff(){char a;return sizeof(a);};


// equivalence operator
bool NcxxByte::operator==(const NcxxByte & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
