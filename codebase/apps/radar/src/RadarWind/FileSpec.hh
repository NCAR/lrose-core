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

#ifndef FILESPEC_H
#define FILESPEC_H

class FileSpec {

public:

string fpath;         // file path
double altitudeKmMsl;
double latitudeDeg;
double longitudeDeg;



FileSpec( string ppath)
{
  this->fpath = ppath;
  this->altitudeKmMsl = numeric_limits<double>::quiet_NaN();
  this->latitudeDeg = numeric_limits<double>::quiet_NaN();
  this->longitudeDeg = numeric_limits<double>::quiet_NaN();
} // end constructor



FileSpec(
  string ppath,
  double altKmMsl,
  double latDeg,
  double lonDeg)
{
  this->fpath = ppath;
  this->altitudeKmMsl = altKmMsl;
  this->latitudeDeg = latDeg;
  this->longitudeDeg = lonDeg;
} // end constructor





void print() {
  cout
    << "      fpath: " << fpath << endl
    << "      altitudeKmMsl: " << altitudeKmMsl << endl
    << "      latitudeDeg: " << latitudeDeg << endl
    << "      longitudeDeg: " << longitudeDeg << endl;
}

}; // end class

#endif
