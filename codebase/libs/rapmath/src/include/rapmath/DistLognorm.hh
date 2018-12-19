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
// DistLognorm.hh
//
// Normal distribution
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#ifndef DistLognorm_hh
#define DistLognorm_hh

#include <rapmath/Distribution.hh>

/////////////////////////////////////
// Normal distribution
// Derived class.

class DistLognorm : public Distribution {
  
public:
  
  // constructor

  DistLognorm();
  
  // destructor
  
  virtual ~DistLognorm();

  // set to use 3 params
  // default is to use 2 params

  void setUse3Params(bool state);
  void setLowerBound(double val);

  // perform a fit
  // values must have been set
  
  virtual int performFit();
  
  // get the pdf for a given x

  virtual double getPdf(double xx);
  
  // get the cdf for a given x
  
  virtual double getCdf(double xx);
  
  // get log moments and lower bound

  double getMeanLn() const { return _meanLn; }
  double getSdevLn() const { return _sdevLn; }
  double getVarianceLn() const { return _varianceLn; }
  double getLowerBound() const { return _lowerBound; }

protected:
private:

  bool _use3Params;
  
  double _meanLn; // mean of natural logs
  double _sdevLn; // sdev of natural logs
  double _varianceLn; // variance of natural logs
  double _lowerBound; // optional 3rd parameters
  
  virtual void _clearStats();

};

#endif
