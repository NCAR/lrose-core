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
// StationLoc.hh
//
// Station locations
//
/////////////////////////////////////////////////////////////

#include "StationLoc.hh"
using namespace std;

void StationLoc::set(const string &icaoName,
                     const string &name,
                     const string &longName,
                     const string &country,
                     int id,
                     double latitude, 
                     double longitude,
                     double heightM)

{

  _icaoName = icaoName;
  _name = name;
  _longName = longName;
  _country = country;
  _id = id;
  _latitude = latitude;
  _longitude = longitude;
  _heightM = heightM;

}

void StationLoc::print(ostream &out)

{

  out << "====== StationLoc =====" << endl;
  out << "  icaoName: " << _icaoName << endl;
  out << "  name: " << _name << endl;
  out << "  longName: " << _longName << endl;
  out << "  country: " << _country << endl;
  out << "  id: " << _id << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  out << "  heightM: " << _heightM << endl;
  out << "=======================" << endl;

}

