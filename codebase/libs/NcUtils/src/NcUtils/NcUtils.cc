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
/////////////////////////////////////////////////////////////
// NcUtils.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////

#include <NcUtils/NcUtils.hh>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;

// initialize constants

const double NcUtils::LIGHT_SPEED = 2.99792458e8; // m/s
const double NcUtils::DegToRad = 0.01745329251994372;
const double NcUtils::RadToDeg = 57.29577951308092;

// missing values in meta-data

double NcUtils::missingMetaDouble = -9999.0;
float NcUtils::missingMetaFloat = -9999.0;
int NcUtils::missingMetaInt = -9999;
char NcUtils::missingMetaChar = -128;

// missing values in field data

NcUtils::fl64 NcUtils::missingFl64 = -9.0e33;
NcUtils::fl32 NcUtils::missingFl32 = -9.0e33f;
NcUtils::si32 NcUtils::missingSi32 = -2147483647;
NcUtils::si16 NcUtils::missingSi16 = -32768;
NcUtils::si08 NcUtils::missingSi08 = -128;

///////////////////////////////
// get byte width of data type

int NcUtils::getByteWidth(DataType_t dtype)

{

  switch (dtype) {
    case FL64:
      return sizeof(fl64);
    case FL32:
      return sizeof(fl32);
    case SI32:
      return sizeof(si32);
    case SI16:
      return sizeof(si16);
    case SI08:
    default:
      return sizeof(si08);
  }

}

///////////////////////////////////////////
// convert enums to strings and vice versa

string NcUtils::dataTypeToStr(DataType_t dtype)

{

  switch (dtype) {
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

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void NcUtils::addErrInt(string &errStr, string label,
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

void NcUtils::addErrDbl(string &errStr,
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

void NcUtils::addErrStr(string &errStr, string label,
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

string NcUtils::makeString(const char *text, int len)
  
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

void NcUtils::replaceSpacesWithUnderscores(string &str)

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

void NcUtils::printString(const string &label, const char *text,
                       int len, ostream &out)
  
{
  out << "  " << label << ": " << makeString(text, len) << endl;
}

/// compute sin and cos together

void NcUtils::sincos(double radians, double &sinVal, double &cosVal)

{
  
  double cosv, sinv, interval;

  // compute cosine
  
  cosv = cos(radians);
  cosVal = cosv;

  // compute sine magnitude

  sinv = sqrt(1.0 - cosv * cosv);
  
  // set sine sign from location relative to PI

  interval = floor(radians / M_PI);
  if (fabs(fmod(interval, 2.0)) == 0) {
    sinVal = sinv;
  } else {
    sinVal = -1.0 * sinv;
  }

}

