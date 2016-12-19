#include <NcUtils/Ncxx.hh>

const double Ncxx::missingDouble = -9999.0;
const float Ncxx::missingFloat = -9999.0f;
const int Ncxx::missingInt = -9999;
const unsigned char Ncxx::missingUchar = -128;

////////////////////////////////////////
// convert type enum to string

string Ncxx::ncTypeToStr(nc_type nctype)
  
{
  
  switch (nctype) {
    case NC_DOUBLE:
      return "NC_DOUBLE";
    case NC_FLOAT:
      return "NC_FLOAT";
    case NC_INT:
      return "NC_INT";
    case NC_SHORT:
      return "NC_SHORT";
    case NC_UBYTE:
    default:
      return "NC_UBYTE";
  }
  
}


