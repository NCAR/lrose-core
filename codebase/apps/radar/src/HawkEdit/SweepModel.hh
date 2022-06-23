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
// SweepModel.hh
//
// SweepModel object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2021
//
///////////////////////////////////////////////////////////////
//
// Manage the sweep details, return sweep information
//
///////////////////////////////////////////////////////////////

#ifndef SweepModel_HH
#define SweepModel_HH

#include <cmath>
#include <string>
#include <vector>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include "DataModel.hh"

class SweepModel {
  
public:

  // constructor
  
  SweepModel(); // const Params &params);
  
  // destructor
  
  virtual ~SweepModel();

  // set from volume

  void set(); // const RadxVol &vol);
  
  // set the angle
  // size effect: sets the selected index

  //void setAngle(double selectedAngle);

  // get methods
  vector<double> *getSweepAngles();
  vector<int> *getSweepNumbers();
  //vector<double> *getSweepAngles(string filePath);

  size_t getNSweeps(); //  const  // _sweeps.size(); }

  double getSelectedAngle();

  int getSelectedSweepNumber();
  void setSelectedAngle(double value);
  void setSelectedNumber(int value);
  
private:
   // vector<double> somevalues = {100, 200, 300};

 
   // only use the sweep angle for display!!! 
   // the sweep angles are usually wonky in the data files
   float _selectedSweepAngle; 

   int _selectedSweepNumber;  

};

#endif

