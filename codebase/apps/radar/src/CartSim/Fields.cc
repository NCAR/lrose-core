// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file Fields.cc
 */

#include "Fields.hh"
#include <toolsa/LogMsg.hh>
using std::string;

//----------------------------------------------------------------
Params::Field_t Fields::fieldType(const std::string &name)
{
  Params::Field_t s;  

  if (name == "VEL")
  {
    s = Params::RADIAL_VEL;
  }
  else if (name == "VX")
  {
    s = Params::VX;
  }  
  else if (name == "VY")
  {
    s = Params::VY;
  }
  else if (name == "VZ")
  {
    s = Params::VZ;
  }
  else if (name == "DBZ")
  {
    s = Params::DBZ;
  }
  else if (name == "SNR")
  {
    s = Params::SNR;
  }
  else if (name == "SW")
  {
    s = Params::SW;
  }
  else if (name == "CLUTTER")
  {
    s = Params::CLUTTER;
  }
  else
  {
    LOGF(LogMsg::ERROR, "Bad input %s", name.c_str());
    s = Params::SW;
  }
  return s;
}

//----------------------------------------------------------------
string Fields::fieldName(const Params::Field_t f)
{
  string s;
  switch (f)
  {
  case Params::RADIAL_VEL:
    s = "VEL";
    break;
  case Params::VX:
    s = "VX";
    break;
  case Params::VY:
    s = "VY";
    break;
  case Params::VZ:
    s = "VZ";
    break;
  case Params::DBZ:
    s = "DBZ";
    break;
  case Params::SNR:
    s = "SNR";
    break;
  case Params::SW:
    s = "SW";
    break;
  case Params::CLUTTER:
    s = "CLUTTER";
    break;
  default:
    LOGF(LogMsg::ERROR, "Bad input %", (int)f);
    s = "UNKNOWN";
    break;
  }
  return s;
}

//----------------------------------------------------------------
string Fields::fieldUnits(const Params::Field_t f)
{
  string s;
  switch (f)
  {
  case Params::RADIAL_VEL:
    s = "m/s";
    break;
  case Params::VX:
    s = "m/s";
    break;
  case Params::VY:
    s = "m/s";
    break;
  case Params::VZ:
    s = "m/s";
    break;
  case Params::DBZ:
    s = "DB";
    break;
  case Params::SNR:
    s = "none";
    break;
  case Params::SW:
    s = "none";
    break;
  case Params::CLUTTER:
    s = "none";
    break;
  default:
    LOGF(LogMsg::ERROR, "Bad input %", (int)f);
    s = "UNKNOWN";
    break;
  }
  return s;
}

//----------------------------------------------------------------
double Fields::fieldMissingValue(const Params::Field_t f)
{
  double s;
  switch (f)
  {
  case Params::RADIAL_VEL:
    s = -99.99;
    break;
  case Params::VX:
    s = -99.99;
    break;
  case Params::VY:
    s = -99.99;
    break;
  case Params::VZ:
    s = -99.99;
    break;
  case Params::DBZ:
    s = -99.99;
    break;
  case Params::SNR:
    s = -99.99;
    break;
  case Params::SW:
    s = -99.99;
    break;
  case Params::CLUTTER:
    s = -99.99;
    break;
  default:
    LOGF(LogMsg::ERROR, "Bad input %", (int)f);
    s = -99.99;
    break;
  }
  return s;
}

