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
// BeamGeom.cc
//
// Beam geometry object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////

#include "BeamGeom.hh"
using namespace std;

/////////////////////////////////////////////////////////
// BeamGeom constructors
//
// When constructed, the beam count is set to 1, since a
// single beam is used to derive the geometry

BeamGeom::BeamGeom() :
        startRange(0.0),
        gateSpacing(0.0),
        angularRes(0.0),
        maxNGates(0),
        nBeams(1)
{

}

BeamGeom::BeamGeom(double start_range,
		   double gate_spacing) :
        startRange(start_range),
        gateSpacing(gate_spacing),
        angularRes(0.0),
        maxNGates(0),
        nBeams(1)
{

}

BeamGeom::BeamGeom(double start_range,
		   double gate_spacing,
                   double angular_res) :
        startRange(start_range),
        gateSpacing(gate_spacing),
        angularRes(angular_res),
        maxNGates(0),
        nBeams(1)
{

}

// print

void BeamGeom::print(ostream &out) const

{

  out << "== BEAM GEOM ==" << endl;
  out << "  startRange: " << startRange << endl;
  out << "  gateSpacing: " << gateSpacing << endl;
  out << "  angularRes: " << angularRes << endl;
  out << "  maxNGates: " << maxNGates << endl;
  out << "  nBeams: " << nBeams << endl;

}
