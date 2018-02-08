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
// BufrProduct_gsi.hh
//
// BUFR Product
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2018
//
///////////////////////////////////////////////////////////////

#ifndef BufrProduct_gsi_HH
#define BufrProduct_gsi_HH

#include "BufrProductGeneric.hh"
#include <Radx/RadxBuf.hh>
#include <Radx/RadxTime.hh>
#include <cstdlib>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////
/// BASE CLASS FOR BUFR DATA ACCESS

class BufrProduct_gsi : public BufrProduct
{
  
public:

  /// Constructor
  
  BufrProduct_gsi();
  
  /// Destructor
  
  virtual ~BufrProduct_gsi();

  void ConstructDescriptor(string &desF, string &desX, 
       string &desY, string &value, string &des_fieldName, char *line);

  void addData(unsigned char value);

  bool StuffIt(unsigned short des, string fieldName, double value);
  bool StuffIt(unsigned short des, string name, string &value);

  double *decompressData();
  float *decompressDataFl32();

  void createSweep();
  virtual void trashReplicator();
  virtual void storeReplicator(unsigned int value);

   // a vector of pointers to vectors

  void printGenericStore();
  vector< vector<unsigned char> *> genericStore;  
  vector<unsigned char> *currentAccumulator;

  // local 
  vector<double> distanceFromAntennaUnitsOf125M;
  vector<double> dopplerMeanRadialVelocity;
  vector<double> dopplerVelocitySpectralWidth;

  // vector to accumulate new descriptors to add or define in table D format
  vector<string> descriptorsToDefine;

  unsigned int duration; // TODO: need getters and setters

  unsigned char des_f; 
  unsigned char des_x;
  unsigned char des_y;
  string desF;
  string desX;
  string desY;
  string des_fieldName; 
  int des_scale;
  string des_units;
  int des_referenceValue;
  int des_dataWidthBits;

};
#endif

