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
// ConvectionFinder.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// MAY 2014
//
////////////////////////////////////////////////////////////////////
//
// ConvectionFinder finds convective regions in a Cartesian radar volume
//
/////////////////////////////////////////////////////////////////////

#ifndef ConvectionFinder_H
#define ConvectionFinder_H

#include <string>
#include <Mdv/DsMdvx.hh>
#include <toolsa/TaArray.hh>
#include <radar/ConvStrat.hh>
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class ConvectionFinder {
  
public:

  // constructor

  ConvectionFinder(const string &progName, const Params &params);

  // destructor
  
  ~ConvectionFinder();

  // run 

  int run(const DsMdvx &inMdvx, const MdvxField &dbzField);

  // get partition object

  const ConvStrat &getConvStrat() const { return _convStrat; }

protected:
  
private:

  static const fl32 _missing;
  
  string _progName;
  Params _params;
  ConvStrat _convStrat;
  
  void _addFields(const MdvxField &dbzField, DsMdvx &outMdvx);
  int _doWrite(const DsMdvx &inMdvx, const MdvxField &dbzField);

};

#endif
