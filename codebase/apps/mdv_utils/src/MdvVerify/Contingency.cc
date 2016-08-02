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
// Contingency.cc
//
// Contingency grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Contingency.hh"

//////////////
// Constructor

Contingency::Contingency(const string &prog_name, const Params &params) :
        Comps(prog_name, params)

{

  _nTarget = 0.0;
  _nTruth = 0.0;
  _nSuccess = 0.0;
  _nFailure = 0.0;
  _nFalseAlarm = 0.0;
  _nNonEvent = 0.0;

}

/////////////
// destructor

Contingency::~Contingency()

{

}

////////////////////////////
// update()
//
// Update contingency table
//
// Returns 0 on success, -1 on failure

void Contingency::update(const MdvxField &targetFld,
                         const MdvxField &truthFld)
  
{
  
  const Mdvx::field_header_t &targetFhdr = targetFld.getFieldHeader();
  
  int nPts = targetFhdr.nx * targetFhdr.ny * targetFhdr.nz;

  const fl32 *target = (fl32 *) targetFld.getVol();
  const fl32 *truth = (fl32 *) truthFld.getVol();

  double nTarget = 0.0;
  double nTruth = 0.0;
  double nSuccess = 0.0;
  double nFailure = 0.0;
  double nFalseAlarm = 0.0;
  double nNonEvent = 0.0;

  for (int ii = 0; ii < nPts; ii++, target++, truth++) {
    
    bool isTarget = false;
    if (*target >= _params.target.min_data_value &&
        *target <= _params.target.max_data_value) {
      isTarget = TRUE;
      nTarget++;
    }

    bool isTruth = false;
    if (*truth >= _params.truth.min_data_value &&
        *truth <= _params.truth.max_data_value) {
      isTruth = TRUE;
      nTruth++;
    }

    if (isTruth && isTarget) {
      nSuccess++;
    } else if (isTruth && !isTarget) {
      nFailure++;
    } else if (!isTruth && isTarget) {
      nFalseAlarm++;
    } else {
      nNonEvent++;
    }
    
  } // ii

  _nTarget += nTarget;
  _nTruth += nTruth;
  _nSuccess += nSuccess;
  _nFailure += nFailure;
  _nFalseAlarm += nFalseAlarm;
  _nNonEvent += nNonEvent;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> contingency data for this file set" << endl;
    _computeAndPrint(nTarget,
                     nTruth,
                     nSuccess,
                     nFailure,
                     nFalseAlarm,
                     nNonEvent,
                     cerr);
  }
  
}

///////////////////////
// print
//

void Contingency::print(ostream &out)
  
{

  out << "========== CONTINGENCY RESULTS FOR ALL FILES =============" << endl;

  _computeAndPrint(_nTarget,
                   _nTruth,
                   _nSuccess,
                   _nFailure,
                   _nFalseAlarm,
                   _nNonEvent,
                   out);

}
  
///////////////////////
// compute and print
//

void Contingency::_computeAndPrint(double nTarget,
                                   double nTruth,
                                   double nSuccess,
                                   double nFailure,
                                   double nFalseAlarm,
                                   double nNonEvent,
                                   ostream &out)
  
{
  
  double x = nSuccess;
  double y = nFailure;
  double z = nFalseAlarm;
  double w = nNonEvent;

  double pod_denom = x + y;
  double pod_no_denom = w + z;
  double far_denom = x + z;
  double csi_denom = x + y + z;
  double hss_denom =  (y * y) + (z * z) + (2.0 * x * w) + (y + z) * (x + w);
  
  double pod;
  if (pod_denom > 0)
    pod = x / pod_denom;
  else
    pod = 0.0;

  double pod_no;
  if (pod_no_denom > 0)
    pod_no = w / pod_no_denom;
  else 
    pod_no = 0.0;

  double far;
  if (far_denom > 0)
    far = z / far_denom;
  else
    far = 0.0;

  double csi;
  if (csi_denom > 0)
    csi = x / csi_denom;
  else
    csi = 0.0;

  double hss;
  if (hss_denom > 0)
    hss = (2.0 * (x * w - y * z)) / hss_denom;
  else
    hss = 0.0;

  double fcst_bias;
  if (nTruth > 0)
    fcst_bias = nTarget / nTruth;
  else
    fcst_bias = 0.0;
  
  out << "  n_target      : " << nTarget << endl;
  out << "  n_truth       : " << nTruth << endl;
  out << "  n_success     : " << nSuccess << endl;
  out << "  n_failure     : " << nFailure << endl;
  out << "  n_false_alarm : " << nFalseAlarm << endl;
  out << "  n_non_event   : " << nNonEvent << endl;
  
  out << endl;
  
  out << "  POD           : " << pod << endl;
  out << "  POD_NO        : " << pod_no << endl;
  out << "  FAR           : " << far << endl;
  out << "  CSI           : " << csi << endl;
  out << "  HSS           : " << hss << endl;
  out << "  FCST_BIAS     : " << fcst_bias << endl;
  
}









