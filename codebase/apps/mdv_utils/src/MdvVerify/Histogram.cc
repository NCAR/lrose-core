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
// Histogram.cc
//
// Histogram object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////

#include "Histogram.hh"
#include <iomanip>
#include <cmath>
using namespace std;

//////////////
// Constructor

Histogram::Histogram(const string &prog_name, const Params &params) :
        Comps(prog_name, params)
  
{

  _nTotal = 0.0;
  _sumx = 0.0;
  _sumx2 = 0.0;
  
  _nIntervals = _params.hist.n_intervals;
  _lowLimit = _params.hist.low_limit;
  _intervalSize = _params.hist.interval_size;
  
  _nPerInterval = new double[_nIntervals];
  _percentPerInterval = new double[_nIntervals];

}

/////////////
// destructor

Histogram::~Histogram()

{

  delete[] _nPerInterval;
  delete[] _percentPerInterval;

}

////////////////////////////
// update()
//
// Update stats struct
//
// Returns 0 on success, -1 on failure

void Histogram::update(const MdvxField &targetFld,
                       const MdvxField &truthFld)
  
{
  
  const Mdvx::field_header_t &targetFhdr = targetFld.getFieldHeader();
  
  int nPts = targetFhdr.nx * targetFhdr.ny * targetFhdr.nz;

  const fl32 *target = (fl32 *) targetFld.getVol();
  const fl32 *truth = (fl32 *) truthFld.getVol();

  for (int ii = 0; ii < nPts; ii++, target++, truth++) {

    fl32 truth_val = *truth;
    fl32 target_val = *target;
    
    if (target_val >= _params.target.min_data_value &&
        target_val <= _params.target.max_data_value) {
      
      int interval = (int)
	floor ((truth_val - _lowLimit) / _intervalSize);

      if (interval >= 0 && interval < _nIntervals) {
	
	_nTotal++;
	_nPerInterval[interval]++;
	_sumx += truth_val;
	_sumx2 += (truth_val * truth_val);
	
      } // if (interval ...

    } // if (target_val ...
    
  } // ii

}

//////////
// print()
//

void Histogram::print(ostream &out)

{

  double mean = _sumx / _nTotal;

  double var = ((_nTotal / (_nTotal - 1.0)) *
                ((_sumx2 / _nTotal) -
                 pow(_sumx / _nTotal, 2.0)));
  
  double sd;
  if (var < 0.0)
    sd = 0.0;
  else
    sd = sqrt(var);

  for (int ii = 0; ii < _nIntervals; ii++) {
    _percentPerInterval[ii] =
      100.0 * (double) _nPerInterval[ii] / (double) _nTotal;
  }
		   
  out << "mean : " <<  mean << endl;
  out << "sd   : " <<  sd << endl;
 
  out << endl;

  out << setw(10) << "lower";
  out << setw(10) << "upper";
  out << setw(10) << "%";
  out << endl;
  
  for (int ii = 0; ii < _nIntervals; ii++) {
    
    double lower = _lowLimit + ii * _intervalSize;
    double upper = lower + _intervalSize;
    
    out << setw(10) << lower;
    out << setw(10) << upper;
    out << setw(10) << _percentPerInterval[ii];
    out << endl;

  } // ii

}

