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
// Regression.cc
//
// Regression object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////

#include "Regression.hh"
using namespace std;

//////////////
// Constructor

Regression::Regression(const string &prog_name, const Params &params) :
  Comps(prog_name, params)

{

}

/////////////
// destructor

Regression::~Regression()

{

}

/////////////
// print

void Regression::print(ostream &out)

{
  out << endl;
}

////////////////////////////
// update()
//
// Update regression data

void Regression::update(const MdvxField &targetFld,
                        const MdvxField &truthFld)
  
{
  
  const Mdvx::field_header_t &targetFhdr = targetFld.getFieldHeader();
  
  int nPts = targetFhdr.nx * targetFhdr.ny * targetFhdr.nz;

  const fl32 *target = (fl32 *) targetFld.getVol();
  const fl32 *truth = (fl32 *) truthFld.getVol();

  for (int ii = 0; ii < nPts; ii++, target++, truth++) {
    
    bool isTarget = false;
    if (*target >= _params.target.min_data_value &&
        *target <= _params.target.max_data_value) {
      isTarget = TRUE;
    }

    bool isTruth = false;
    if (*truth >= _params.truth.min_data_value &&
        *truth <= _params.truth.max_data_value) {
      isTruth = TRUE;
    }

    if (isTarget && isTruth) {
      cout << *target << " " << *truth << endl;
    }

  } // ii

}

