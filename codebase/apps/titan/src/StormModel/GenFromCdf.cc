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
// GenFromCdf.cc
//
// GenFromCdf object - for generating from a specified CDF.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "GenFromCdf.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
using namespace std;

///////////////
// Constructor

GenFromCdf::GenFromCdf(int npdf, double *pdf_prob, double *pdf_vals,
		       char *label)

{

  OK = TRUE;
  _cdfVals = (double *) NULL;
  _cdfProb = (double *) NULL;
  _label = STRdup(label);

  // allocate
  
  _nCdf = npdf + 2;
  _cdfVals = (double *) umalloc(_nCdf * sizeof(double));
  _cdfProb = (double *) umalloc(_nCdf * sizeof(double));

  // load up vals - bin width is computed assuming equally sized bins

  for (int i = 1; i < _nCdf - 1; i++) {
    _cdfVals[i] = pdf_vals[i-1];
  }

  double bin_width = (pdf_vals[npdf-1] - pdf_vals[0]) / (double)(npdf - 1);
  _cdfVals[0] = _cdfVals[1] - bin_width / 2.0;
  _cdfVals[_nCdf-1] = _cdfVals[_nCdf-2] + bin_width / 2.0;
  
  // load up probabilities
  
  _cdfProb[0] = 0.0;
  _cdfProb[_nCdf-1] = 1.0;

  for (int i = 1; i < _nCdf - 1; i++) {
    _cdfProb[i] = _cdfProb[i-1] + pdf_prob[i-1];
    if (_cdfProb[i] > 1.0000001) {
      fprintf(stderr, "CDF for %s - cdf exceeds 1.0\n", _label);
      OK = FALSE;
    }
  }

  return;

}

/////////////
// destructor

GenFromCdf::~GenFromCdf()

{

  STRfree(_label);
  if (_cdfVals != NULL) {
    ufree(_cdfVals);
  }
  if (_cdfProb != NULL) {
    ufree(_cdfProb);
  }

}

//////////
// print()
//

void GenFromCdf::print(FILE *out)

{

  fprintf(out, "\n");
  fprintf(out, "CDF for %s\n", _label);
  for (int i = 0; i < _nCdf; i++) {
    fprintf(out, "  %10g : %10g\n", _cdfVals[i], _cdfProb[i]);
  }
  fprintf(out, "\n");
  fflush(out);

}

//////////////////////////////////////////////////
// Generate()
//
// Generate next start point.
//

double GenFromCdf::Generate()

{

  // Get a uniform variate

  double u;
  u = STATS_uniform_gen();

  // find the cdf segment which brackets the array

  int index;
  for (index = 1; index < _nCdf; index++) {
    if (u >= _cdfProb[index-1] && u <= _cdfProb[index]) {
      break;
    }
  }

  // interpolate

  double genVal;

  if (_cdfProb[index-1] == _cdfProb[index]) {

    genVal = (_cdfVals[index-1] + _cdfVals[index]) / 2.0;

  } else {

    double dprob = _cdfProb[index] - _cdfProb[index-1];
    double delta = u - _cdfProb[index-1];
    double fraction = delta / dprob;
    double bin_width = _cdfVals[index] - _cdfVals[index-1];
    genVal = _cdfVals[index-1] + fraction * bin_width;

  }

  return (genVal);

}

