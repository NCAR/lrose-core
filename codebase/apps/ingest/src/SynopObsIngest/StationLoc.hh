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

#include <string>
#include <ostream>
using namespace std;

class StationLoc {

public:

  void set(const string &icaoName,
           const string &name,
           const string &longName,
           const string &country,
           int id,
           double latitude, 
           double longitude,
           double heightM);

  const string getIcaoName() { return _icaoName; }
  const string getName() { return _name; }
  const string getLongName() { return _longName; }
  const string getCountry() { return _country; }
  int getId() { return _id; }
  double getLatitude() { return _latitude; }
  double getLongitude() { return _longitude; }
  double getHeightM() { return _heightM; }

  void print(ostream &out);

private:
  
  string _icaoName;
  string _name;
  string _longName;
  string _country;
  int _id;
  double _latitude;
  double _longitude;
  double _heightM;

};

      
   
   
   
