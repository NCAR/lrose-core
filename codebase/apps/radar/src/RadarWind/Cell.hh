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

#ifndef CELL_H
#define CELL_H

class Cell {

public:

double ww;               // w wind
double vv;               // v wind
double uu;               // u wind

double meanNbrDbz;       // mean(dbz) over neighbors
double meanNbrNcp;       // mean(ncp) over neighbors
double meanNbrElevDeg;   // mean( elevation angle degrees) over neighbors
double meanNbrKeepDist;  // mean( distance from cell center) of keep nbrs
double meanNbrOmitDist;  // mean( distance from cell center) of omit nbrs
double conditionNumber;


Cell()
{
  this->ww = numeric_limits<double>::quiet_NaN();
  this->vv = numeric_limits<double>::quiet_NaN();
  this->uu = numeric_limits<double>::quiet_NaN();

  this->meanNbrDbz = numeric_limits<double>::quiet_NaN();
  this->meanNbrNcp = numeric_limits<double>::quiet_NaN();
  this->meanNbrElevDeg = numeric_limits<double>::quiet_NaN();
  this->meanNbrKeepDist = numeric_limits<double>::quiet_NaN();
  this->meanNbrOmitDist = numeric_limits<double>::quiet_NaN();
  this->conditionNumber = numeric_limits<double>::quiet_NaN();
} // end constructor


}; // end class

#endif
