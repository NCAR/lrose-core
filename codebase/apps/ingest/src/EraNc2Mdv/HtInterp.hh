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
///////////////////////////////////////////////////
// HtInterp - interpolate MDV data from pressure
// levels to ht levels
//
// Mike Dixon, EOL, NCAR, Boulder, CO
// April 2014
///////////////////////////////////////////////////
#ifndef _HT_INTERP_
#define _HT_INTERP_

//
// Forward class declarations
//

class MdvxField;
class DsMdvx;
#include "Params.hh"

class HtInterp {
  
public:

  typedef struct {
    double ht;
    int indexLower;
    int indexUpper;
    double ghtLower;
    double ghtUpper;
    double wtLower;
    double wtUpper;
  } interp_pt_t;
  
  HtInterp(Params *params);
 
  ~HtInterp();
  
  // interp vlevels onto heights msl

  int interpVlevelsToHeight(DsMdvx *mdvObj);
  
protected:
private:

  Params *_paramsPtr;

  // Add a pressure field which will then be interpolated

  void _addPressureField(DsMdvx *mdvObj,
                         const MdvxField *ghtFld);
  
  // Compute interpolation struct array
  
  void _computeInterpPts(const vector<double> &htsOut,
                         const MdvxField *ghtFld,
                         vector<interp_pt_t> &interpPts);
  
  // interpolate a field onto height levels

  void _interpField(MdvxField *fld, 
                    const vector<interp_pt_t> &interpPts,
                    const vector<double> &htsOut);
  
};

#endif
