#include <NcUtils/NcxxChar.hh>
#include <netcdf.h>
using namespace netCDF;

// create an instance of NcxxChar  called netCDF::ncChar
namespace netCDF {
  NcxxChar ncChar;
}

// constructor
NcxxChar::NcxxChar() : NcxxType(NC_CHAR){
}

NcxxChar::~NcxxChar() {
}


// equivalence operator
bool NcxxChar::operator==(const NcxxChar & rhs)    {
  // simply check the netCDF id.
  return myId == rhs.myId;
}  
