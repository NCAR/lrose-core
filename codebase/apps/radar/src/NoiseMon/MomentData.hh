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
// MomentData.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////

#ifndef MomentData_HH
#define MomentData_HH
#include <ostream>

using namespace std;

////////////////////////
// This class

class MomentData {
  
public:
  
  MomentData();

  // set values to missing

  void initialize();

  // set values to zero

  void zeroOut();

  // accumulate values

  void add(const MomentData &val);

  // accumulate squared values
  
  void addSquared(const MomentData &val);

  // compute mean and standard deviation,
  // given sum and sum-squared objects
  
  static void computeMeanSdev(double count,
                              const MomentData &sum,
                              const MomentData &sum2,
                              MomentData &mean,
                              MomentData &sdev);

  void print(ostream &out) const;

  // double start; // used for computing fields offsets

  // public data

  double height;

  double snr;
  double snrhc;
  double snrhx;
  double snrvc;
  double snrvx;

  double dbm;
  double dbmhc;
  double dbmhx;
  double dbmvc;
  double dbmvx;

  double dbz;
  double vel;
  double width;

  double zdrm;

  double ldrh;
  double ldrv;

  double phidp;
  double rhohv;

  bool valid;

  static const double missingVal;

protected:
private:

  static void _computeMeanSdev(double count,
                               double sum,
                               double sum2,
                               double &mean,
                               double &sdev);
  
};

#endif

