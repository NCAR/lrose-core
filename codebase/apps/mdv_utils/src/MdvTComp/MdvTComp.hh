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
// MdvTComp.hh
//
// MdvTComp object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////
//
// Performs temporal compositing on Mdv file data
//
///////////////////////////////////////////////////////////////

#ifndef MdvTComp_H
#define MdvTComp_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxInput.hh>
using namespace std;

////////////////////////
// This class

class MdvTComp {
  
public:

  // constructor

  MdvTComp (int argc, char **argv);

  // destructor
  
  ~MdvTComp();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsMdvxInput _input;

  int  _processNewData(DsMdvx &inMdvx, time_t &lastOutputTime);
  void _setupRead(DsMdvx &mdvx) const;
  void _prepareData(const Params::field_t &field, int nPointsField,
		    DsMdvx &working, DsMdvx &workingCounts) const;
  int _doComposite(DsMdvx &working, DsMdvx &workingCounts,
		   int nPointsField) const;
  void _accumulateComposite(const time_t pastFileTime, int nPointsField,
			    DsMdvx &past, DsMdvx &working,
			    DsMdvx &workingCounts) const;
  void _accumulateFieldComposite(const Params::field_t &field, int nPointsField,
				 DsMdvx &past, DsMdvx &working,
				 DsMdvx &workingCounts) const;
  void _finishAveraging(const Params::field_t &field, int nPointsField,
			DsMdvx &working, DsMdvx &workingCounts) const;
  void _prepareOutput(const Params::field_t &field, int nPointsField,
		      DsMdvx &working) const;
  bool _uniformFields(DsMdvx &working, int &n_points) const;
  
  bool _isWantedTime(const time_t &t) const;

};

#endif

