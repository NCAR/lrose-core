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
// Fields.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include "Fields.hh"
#include <cmath>

const double Fields::missingDouble = -9999;

// constructor

Fields::Fields()

{

  initialize();

}

// Initialize to missing

void Fields::initialize()

{
  
  snr = missingDouble;
  dbm = missingDouble;
  dbz = missingDouble;
  vel = missingDouble;
  width = missingDouble;

  // clutter-filtered fields

  clut = missingDouble;
  dbzf = missingDouble;
  velf = missingDouble;
  widthf = missingDouble;

  // dual polarization fields

  zdr = missingDouble;
  zdrm = missingDouble;
  
  ldrh = missingDouble;
  ldrv = missingDouble;

  rhohv = missingDouble;
  phidp = missingDouble;
  kdp = missingDouble;

  snrhc = missingDouble;
  snrhx = missingDouble;
  snrvc = missingDouble;
  snrvx = missingDouble;

  dbmhc = missingDouble;
  dbmhx = missingDouble;
  dbmvc = missingDouble;
  dbmvx = missingDouble;

}

// Print

void Fields::print(ostream &out) const

{
  
  out << "snr: " << snr << endl;
  out << "dbm: " << dbm << endl;
  out << "dbz: " << dbz << endl;
  out << "vel: " << vel << endl;
  out << "width: " << width << endl;
  out << "clut: " << clut << endl;
  out << "dbzf: " << dbzf << endl;
  out << "velf: " << velf << endl;
  out << "widthf: " << widthf << endl;
  out << "zdr: " << zdr << endl;
  out << "zdrm: " << zdrm << endl;
  out << "ldrh: " << ldrh << endl;
  out << "ldrv: " << ldrv << endl;
  out << "rhohv: " << rhohv << endl;
  out << "phidp: " << phidp << endl;
  out << "kdp: " << kdp << endl;
  out << "snrhc: " << snrhc << endl;
  out << "snrhx: " << snrhx << endl;
  out << "snrvc: " << snrvc << endl;
  out << "snrvx: " << snrvx << endl;
  out << "dbmhc: " << dbmhc << endl;
  out << "dbmhx: " << dbmhx << endl;
  out << "dbmvc: " << dbmvc << endl;
  out << "dbmvx: " << dbmvx << endl;

}

