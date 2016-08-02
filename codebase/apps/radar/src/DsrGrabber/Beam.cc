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
///////////////////////////////////////////////////////////////
// Beam.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2007
//
///////////////////////////////////////////////////////////////
//
// Data for a beam - accumulation over multiple hits/gates
//
////////////////////////////////////////////////////////////////

#include "Beam.hh"
#include <toolsa/DateTime.hh>

// Constructor

Beam::Beam()
  
{

  time = 0;
  volNum = 0;
  tiltNum = 0;
  el = 0;
  az = 0;

}

Beam::Beam(double time,
	   int volNum,
	   int tiltNum,
	   double el,
	   double az,
	   const vector<double> &fields) :
  time(time),
  volNum(volNum),
  tiltNum(tiltNum),
  el(el),
  az(az),
  fields(fields)
  
{


}

// destructor

Beam::~Beam()

{

}

// print

void Beam::print(ostream &out)

{
  time_t ptime = (time_t) time;

  out << "Beam, time, vol, tilt, el, az, fields: "
      << DateTime::strm(ptime) << " "
      << volNum << " "
      << tiltNum << " "
      << el << " "
      << az << " ";

  for (int ii = 0; ii < (int) fields.size(); ii++) {
    out << fields[ii];
    if (ii == (int) fields.size() - 1) {
      out << endl;
    } else {
      out << " ";
    }
  }

}
