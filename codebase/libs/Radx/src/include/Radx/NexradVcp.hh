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
// NexradVcp.hh
//
// Stores the angles for a VCP
//
// Mike Dixon, EOL, NCAR, Boulder, CO
// Sept 2013
//
///////////////////////////////////////////////////////////////

#ifndef NexradVcp_HH
#define NexradVcp_HH

#include <vector>
#include <iostream>
using namespace std;

class NexradVcp {

public:
  
  /// Constructor
  
  NexradVcp(int num);

  /// Destructor
  
  ~NexradVcp();
  
  /// Print
  
  void print(ostream &out) const;

  /// get VCP num

  int getNum() const { return _num; }

  /// get number of fixed angles
  
  size_t getNFixedAngles() const { return _fixedAngles.size(); }

  /// get fixed angle list
  
  const vector<double> &getFixedAngles() const { return _fixedAngles; }
  
  /// get fixed angle closest to supplied value
  /// also set the sweep number
  
  double getClosestFixedAngle(double val, int &sweepNum) const;
  
protected:
private:

  int _num;
  vector<double> _fixedAngles;
  
}; /// NexradVcp

# endif


