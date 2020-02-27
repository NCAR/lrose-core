// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//  Additional methods have been added to return error conditions. 
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxPort.hh>

// missing values in meta-data

double Ncxx::missingMetaDouble = -9999.0;
float Ncxx::missingMetaFloat = -9999.0;
int Ncxx::missingMetaInt = -9999;
char Ncxx::missingMetaUchar = 0;
char Ncxx::missingMetaChar = -128;

// missing values in data

double Ncxx::missingDouble = -9999.0;
float Ncxx::missingFloat = -9999.0f;
int Ncxx::missingInt = -9999;
char Ncxx::missingChar = -128;
unsigned char Ncxx::missingUchar = 0;

// missing field values by portable type

Ncxx::fl64 Ncxx::missingFl64 = -9.0e33;
Ncxx::fl32 Ncxx::missingFl32 = -9.0e33f;
Ncxx::si32 Ncxx::missingSi32 = -2147483647;
Ncxx::si16 Ncxx::missingSi16 = -32768;
Ncxx::si08 Ncxx::missingSi08 = -128;

///////////////////////////////
// get byte width of data type

int Ncxx::getByteWidth(nc_type nctype)

{
  
  switch (nctype) {
    case NC_BYTE:
      return sizeof(char);
    case NC_CHAR:
      return sizeof(char);
    case NC_SHORT:
      return sizeof(short);
    case NC_INT:
      return sizeof(int);
    case NC_FLOAT:
      return sizeof(float);
    case NC_DOUBLE:
      return sizeof(double);
    case NC_UBYTE:
      return sizeof(unsigned char);
    case NC_USHORT:
      return sizeof(unsigned short);
    case NC_UINT:
      return sizeof(unsigned int);
    case NC_INT64:
      return sizeof(si64);
    case NC_UINT64:
      return sizeof(ui64);
    case NC_STRING:
      return sizeof(char);
    case NC_VLEN:
      return 0;
    case NC_OPAQUE:
      return 0;
    case NC_ENUM:
      return sizeof(int);
    case NC_COMPOUND:
    default:
      return 0;
  }

}

///////////////////////////////
// get byte width of data type

int Ncxx::getByteWidth(PortType_t ptype)

{

  switch (ptype) {
    case FL64:
      return sizeof(fl64);
    case FL32:
      return sizeof(fl32);
    case UI32:
      return sizeof(ui32);
    case SI32:
      return sizeof(si32);
    case UI16:
      return sizeof(ui16);
    case SI16:
      return sizeof(si16);
    case SI08:
    case UI08:
    default:
      return sizeof(si08);
  }

}

////////////////////////////////////////
// convert error enum to string

string Ncxx::ncErrToStr(int errtype)
  
{
  
  switch (errtype) {
    case NC_NOERR:
      return "NC_NOERR";
    case NC_EBADID:
      return "NC_EBADID - bad ncid";
    case NC_ENOTNC4:
      return "NC_ENOTNC4 - not a netCDF-4 file";
    case NC_ENOTVAR:
      return "NC_ENOTVAR - can't find this variable";
    case NC_ELATEDEF:
      return "NC_ELATEDEF - nc_enddef already called";
    case NC_ENOTINDEFINE:
      return "NC_ENOTINDEFINE - not in define mode";
    case NC_ESTRICTNC3:
      return "NC_ESTRICTNC3 - chunk size may be too big";
    case NC_EPERM:
      return "NC_EPERM - read-only mode, can't create object";
    default:
      return "NC_ERR_UNKNOWN";
  }

}

////////////////////////////////////////
// convert type enum to string

string Ncxx::ncTypeToStr(nc_type nctype)
  
{
  
  switch (nctype) {
    case NC_BYTE:
      return "NC_BYTE";
    case NC_CHAR:
      return "NC_CHAR";
    case NC_SHORT:
      return "NC_SHORT";
    case NC_INT:
      return "NC_INT";
    case NC_FLOAT:
      return "NC_FLOAT";
    case NC_DOUBLE:
      return "NC_DOUBLE";
    case NC_UBYTE:
      return "NC_UBYTE";
    case NC_USHORT:
      return "NC_USHORT";
    case NC_UINT:
      return "NC_UINT";
    case NC_INT64:
      return "NC_INT64";
    case NC_UINT64:
      return "NC_UINT64";
    case NC_STRING:
      return "NC_STRING";
    case NC_VLEN:
      return "NC_VLEN";
    case NC_OPAQUE:
      return "NC_OPAQUE";
    case NC_ENUM:
      return "NC_ENUM";
    case NC_COMPOUND:
      return "NC_COMPOUND";
    default:
      return "UNKNOWN";
  }

}

string Ncxx::ncxxTypeToStr(NcxxType nctype)
  
{
  return ncTypeToStr(nctype.getId());
}

///////////////////////////////////////////
// convert enums to strings

string Ncxx::portTypeToStr(PortType_t ptype)

{

  switch (ptype) {
    case FL64:
      return "fl64";
    case FL32:
      return "fl32";
    case SI32:
      return "si32";
    case SI16:
      return "si16";
    case SI08:
    default:
      return "si08";
  }
  
}

///////////////////////////////////////////
// strip redundant null from string

string Ncxx::stripNulls(const string &val)
  
{
  // find first null in string
  size_t len = val.size();
  for (size_t ii = 0; ii < val.size(); ii++) {
    if ((int) val[ii] == 0) {
      len = ii;
      break;
    }
  }
  return val.substr(0, len);
}



///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void Ncxx::addErrInt(string &errStr, string label,
                     int iarg, bool cr)
{
  errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  errStr += str;
  if (cr) {
    errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void Ncxx::addErrDbl(string &errStr,
                     string label, double darg,
                     string format, bool cr)
  
{
  errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  errStr += str;
  if (cr) {
    errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void Ncxx::addErrStr(string &errStr, string label,
                     string strarg, bool cr)

{
  errStr += label;
  errStr += strarg;
  if (cr) {
    errStr += "\n";
  }
}

/////////////////////////////
// make string from char text
// Ensure null termination

string Ncxx::makeString(const char *text, int len)
  
{

  char *copy = new char[len + 1];
  memcpy(copy, text, len);
  // force null termination
  copy[len] = '\0';
  // remove trailing spaces or non-printable characters
  for (int ii = len - 1; ii >= 0; ii--) {
    char cc = copy[ii];
    if (!isprint(cc) || isspace(cc)) {
      copy[ii] = '\0';
    } else {
      break;
    }
  }
  // convert to string
  string str(copy);
  delete[] copy;
  return str;

}

//////////////////////////////////////////////
// replace spaces in a string with underscores

void Ncxx::replaceSpacesWithUnderscores(string &str)

{

  for (size_t ii = 0; ii < str.size(); ii++) {
    if (str[ii] == ' ') {
      str[ii] = '_';
    }
  }

}

///////////////////////////
// safe print for char text
// Ensure null termination

void Ncxx::printString(const string &label, const char *text,
                       int len, ostream &out)
  
{
  out << "  " << label << ": " << makeString(text, len) << endl;
}

