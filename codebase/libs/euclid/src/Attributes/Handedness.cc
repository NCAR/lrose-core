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
#include <euclid/Handedness.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>
#include <cstdio>
using std::string;

/*----------------------------------------------------------------*/
Handedness::Handedness(Handedness::e_hand_t t)
{
  _type = t;
}

/*----------------------------------------------------------------*/
Handedness::Handedness()
{
  _type = UNKNOWN;
}

/*----------------------------------------------------------------*/
Handedness::~Handedness()
{
}

/*----------------------------------------------------------------*/
bool Handedness::operator==(const Handedness &l) const
{
  return (_type == l._type);
}

/*----------------------------------------------------------------*/
string Handedness::writeXml(void) const
{
  return TaXml::writeInt("Handedness", 0, (int)_type);
}

/*----------------------------------------------------------------*/
bool Handedness::readXml(const std::string &xml)
{
  *this = Handedness();

  int i;
  if (TaXml::readInt(xml, "Handedness", i))
  {
    LOG(ERROR) << "reading tag Handedness";
    return false;
  }
  _type = (e_hand_t)i;
  return true;
}

/*----------------------------------------------------------------*/
void Handedness::average(const Handedness &h1) 
{
  // what is average of left and right handedness? use this
  // unless no_handed and a1's handedess is not no_handed, then
  // use a1's handedness.
  if (_type == Handedness::NONE && h1._type != Handedness::NONE)
    _type = h1._type;
}

/*----------------------------------------------------------------*/
void Handedness::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
void Handedness::print(FILE *fp) const
{
  fprintf(fp, "%s", sprint().c_str());
}

/*----------------------------------------------------------------*/
string Handedness::sprint(void) const
{
  string ret;
  switch (_type)
  {
  case Handedness::LEFT:
    ret = "Left";
    break;
  case Handedness::RIGHT:
    ret = "Rght";
    break;
  case Handedness::NONE:
    ret = "None";
    break;
  default:
    ret = "Unkn";
    break;
  }
  return ret;
}


/*----------------------------------------------------------------*/
void Handedness::reverseHandedness(void)
{
  if (_type == Handedness::LEFT)
    _type = Handedness::RIGHT;
  else if (_type == Handedness::RIGHT)
    _type = Handedness::LEFT;
}
