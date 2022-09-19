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
// LayerStats.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////

#ifndef LayerStats_HH
#define LayerStats_HH

#include <vector>
#include <iostream>
#include <rapmath/DistNormal.hh>
#include "MomentData.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class LayerStats {
  
public:
  
  // constructor
  
  LayerStats(const Params &params, double min_ht, double max_ht);
  
  // destructor
  
  ~LayerStats();
  
  // clear data
  
  void clearData();

  // add a zdr value

  void addData(const MomentData &data);
  
  // print
  
  void print(ostream &out);
  
  // compute stats
  
  void computeStats();

  // compute global stats
  
  void computeGlobalStats();

  // get methods

  double getMinHt() const { return _minHt; }
  double getMeanHt() const { return _meanHt; }
  double getMaxHt() const { return _maxHt; }
  int getNValid() const { return _nValid; }
  int getGlobalNValid() const { return _globalNValid; }
  const vector<MomentData> &getMomentData() const { return _momentData; }
  const MomentData &getMean() const { return _mean; }
  const MomentData &getSdev() const { return _sdev; }
  const MomentData &getSum() const { return _sum; }
  const MomentData &getSumSq() const { return _sumSq; }
  const MomentData &getGlobalMean() const { return _globalMean; }
  const MomentData &getGlobalSdev() const { return _globalSdev; }

  DistNormal &getDist() { return _dist; }
  const DistNormal &getDist() const { return _dist; }

  DistNormal &getGlobalDist() { return _globalDist; }
  const DistNormal &getGlobalDist() const { return _globalDist; }

protected:
private:

  const Params &_params;
  double _minHt;
  double _meanHt;
  double _maxHt;
  vector<MomentData> _momentData;

  DistNormal _dist;
  DistNormal _globalDist;

  int _nValid;
  MomentData _mean;
  MomentData _sdev;
  MomentData _sum;
  MomentData _sumSq;

  int _globalNValid;
  MomentData _globalMean;
  MomentData _globalSdev;
  MomentData _globalSum;
  MomentData _globalSumSq;

  double _meanZdr, _sdevZdr;
  
  void _computeZdrmMeanSdev(double &mean, double &sdev);

};

#endif

