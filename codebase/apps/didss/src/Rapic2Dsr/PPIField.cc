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
// PPIField.cc
//
// PPIField object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include "PPIField.hh"
#include "Beam.hh"
#include "sRadl.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

// Constructor

PPIField::PPIField(const Params &params) :
  _params(params)

{

  time = 0;
  maxGates = 0;
  rangeRes = 0;
  startRange = 0;

}

// destructor

PPIField::~PPIField()

{
  for (size_t ii = 0; ii < beams.size(); ii++) {
    delete (beams[ii]);
  }
}

// add a beam

void PPIField::addBeam(const Params &params,
		       const sRadl *radial, const ScanParams &sParams,
		       bool isBinary, double target_elev)

{
  Beam *beam = new Beam(params, radial, sParams, isBinary, target_elev);
  beams.push_back(beam);
  maxGates = MAX(maxGates, radial->data_size);
}

// print

void PPIField::print(ostream &out)

{
  out << "Field name: " << fieldName << endl;
  out << "Time: " << DateTime::str(time) << endl;
  out << "nBeams: " << beams.size() << endl;
  out << "maxGates: " << maxGates << endl;
  out << "startRange: " << startRange << endl;
  out << "rangeRes: " << rangeRes << endl;
 for (size_t i = 0; i < beams.size(); i++) {
    out << i << ": ";
    beams[i]->print(out);
  }
}

// print full

void PPIField::printFull(ostream &out)

{
  out << "PPIField::printFull" << endl;
  out << "Field name: " << fieldName << endl;
  out << "Time: " << DateTime::str(time) << endl;
  out << "nBeams: " << beams.size() << endl;
  out << "maxGates: " << maxGates << endl;
  out << "startRange: " << startRange << endl;
  out << "rangeRes: " << rangeRes << endl;
  for (size_t i = 0; i < beams.size(); i++) {
    beams[i]->printFull(out);
  }
}


// Remap refl data to same range resolution as vel data for Beijing data
// This assumes that original refl data has startRange = 0, rangeRes=1000 
// and vel data has startRange = 375, rangeRes = 250m. 
// This requires further work to make it more general. 
// (rjp 13 Sep 2006)
 
void PPIField::convertNexradResolution(int maxGatesLimit)
{

  Beam *new_beam = NULL;
  int ngates; 
  int numgates; 
  double index;

  //Set number of gates - assumes that rangeRes(refl) = 4*rangeRes(vel)
  //Need to set maxGates for refl to same as for vel. 
  ngates = 4*maxGates;
  if (ngates > maxGatesLimit) {
    ngates = maxGatesLimit;
  }

  for (int ii = 0; ii < (int) beams.size(); ii++) {
    //Create new_beam with ngates and val=0
    new_beam = new Beam(beams[ii],ngates);

    //Remap old beam to new beam with new rangeRes.  Note the 
    //number of gates in each old beam will vary. 
    numgates = 4*((beams[ii]->nGates))-1; 
    if (numgates > maxGatesLimit) {
      numgates = maxGatesLimit; 
    }

    for (int jj = 0; jj < numgates; jj++) {
      index = floor((jj + 2.0)/4.0);
      new_beam->vals[jj] = beams[ii]->vals[(int)index]; 
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      if (ii > 90 && ii < 100) {
	cerr << "NUMGATES: " << numgates << endl;
	cerr << "old beam: el: " << beams[ii]->elevation  
	     << ", az: " << beams[ii]->azimuth 
	     << ", ngates: " << beams[ii]->nGates << endl;
	cerr << "new beam: az: " << new_beam->azimuth 
	     << ", ngates: " << new_beam->nGates << endl;
      }
    }

    //Clear old beam and insert new beam
    delete beams[ii];
    beams[ii] = new_beam;

    //Set scan params explicitly.
    maxGates = ngates;
    rangeRes = 250;
    startRange = 375; 
  }
}

