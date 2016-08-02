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
// RapicRay.cc
//
// RapicRay object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include <cstring>
#include "RapicRay.hh"
#include "ScanParams.hh"
#include "sRadl.hh"
using namespace std;

// Constructor

RapicRay::RapicRay(const sRadl *radial, const ScanParams &sParams,
                   bool isBinary, double target_elev)

{

  timeSecs = sParams.time;

  azimuth = radial->az / 10.0;
  if (!isBinary) {
    elevation = target_elev;
  } else {
    elevation = radial->el / 10.0;
  }
  nGates = radial->data_size;

  if (nGates == 0) {

    vals = NULL;

  } else {

    vals = new int[nGates];

    if (isBinary) {

      // Binary radial
      // accept the data just as it is

      for (int i = 0; i < nGates; i++) {
	vals[i] = radial->data[i];
      }

    } else {

      // Ascii radial

      if (strstr(sParams.field_name, "Refl")) {
	
	// dBZ data
	// scale from lookup table into target scale and bias
	
	double targetScale = 0.5;
	double targetBias = -32.0;

	for (int i = 0; i < nGates; i++) {
	  int sourceIndex =  radial->data[i];
	  if (sourceIndex == 0) {
	    vals[i] = 0;
	  } else {
	    if (sourceIndex > (int) sParams.dbz_levels.size() - 1) {
	      sourceIndex = (int) sParams.dbz_levels.size() - 1;
	    }
	    double dbz =
	      (sParams.dbz_levels[sourceIndex - 1] +
	       sParams.dbz_levels[sourceIndex]) / 2.0;
	    int targetIndex = (int) (((dbz - targetBias) / targetScale) + 0.5);
	    if (targetIndex < 0) {
	      targetIndex = 0;
	    } else if (targetIndex > 255) {
	      targetIndex = 255;
	    }
	    vals[i] = targetIndex;
	  }
	} // i
	
      } else if (strstr(sParams.field_name, "Vel")) {
	
	// Vel data
	// scale from source scale and bias into target scale and bias

	if (sParams.video_res == 256) {

	  // no rescaling necessary
	  for (int i = 0; i < nGates; i++) {
	    vals[i] = radial->data[i];
	  }

	} else {

	  double targetScale = sParams.nyquist / 127.0;
	  double targetBias = (-1.0 * sParams.nyquist * 128.0) / 127.0;
	  double sourceScale =
	    sParams.nyquist / (sParams.video_res / 2.0 - 1.0);
	  double sourceBias =
	    (-1.0 * sParams.nyquist * (sParams.video_res / 2.0)) /
	    (sParams.video_res / 2.0 - 1.0);

	  for (int i = 0; i < nGates; i++) {
	    if (radial->data[i] == 0) {
	      vals[i] = 0;
	    } else {
	      int sourceIndex =  radial->data[i];
	      double vel = sourceBias + sourceIndex * sourceScale;
	      int targetIndex =
		(int) (((vel - targetBias) / targetScale) + 0.5);
	      if (targetIndex < 0) {
		targetIndex = 0;
	      } else if (targetIndex > 255) {
		targetIndex = 255;
	      }
	      vals[i] = targetIndex;
	    } // if (radial->data[i] == 0)
	  } // i

	} // if (sParams.video_res == 256) 
	
      } // if (strstr(sParams.field_name, "Refl"))
    }
  }
}

// constructor for convertRangeResolution 
// rjp 13 Sep 2006

RapicRay::RapicRay(const RapicRay *beam, int ngates)

{

  vals = NULL;
  if (!beam) return;

  azimuth = beam->azimuth;
  elevation = beam->elevation;
  nGates = ngates;

  if (nGates ==0) {
    vals = NULL; 
  } else {
    vals = new int[nGates];
  }

  for (int i=0; i<nGates; i++) {
    vals[i]=0;
  }

}

// destructor

RapicRay::~RapicRay()

{

  if (vals) {
    delete[] vals;
  }

}

// print

void RapicRay::print(ostream &out)

{
  out << "el, az, nGates: "
      << elevation << ", "
      << azimuth << ", "
      << nGates << endl;
}

// print full

void RapicRay::printFull(ostream &out)

{
  print(out);
  for (int i = 0; i < nGates; i++) {
    out << vals[i] << " ";
  }
  out << endl;
}


