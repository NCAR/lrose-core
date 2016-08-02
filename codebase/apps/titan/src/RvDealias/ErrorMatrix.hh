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
// ErrorMatrix.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2003
//
///////////////////////////////////////////////////////////////

#ifndef ErrorMatrix_hh
#define ErrorMatrix_hh

#include <string>
#include <vector>

using namespace std;

////////////////////////
// This class

class ErrorMatrix {
  
public:
  
  // constructor - initializes for given size
  
  ErrorMatrix();
  
  // destructor
  
  ~ErrorMatrix();

  // set labels

  void setVarName(const string &varName) {_varName = varName;}
  void setTopLabel(const string &topLabel) {_topLabel = topLabel;}
  void setXAxisLabel(const string &xAxisLabel) {_xAxisLabel = xAxisLabel;}
  void setYAxisLabel(const string &yAxisLabel) {_yAxisLabel = yAxisLabel;}

  // set output dir

  void setOutputDir(const string &outputDir) {_outputDir = outputDir;}
  
  // set axis values

  void setAxisVals(int nx, double minx, double dx,
		   int ny, double miny, double dy);

  // set limits for width 2

  void setWidth2Limits(double minWidth2, double maxWidth2);

  // set limits for color scale values

  void setBiasCscale(double min, double max, double delta);
  void setSdevCscale(double min, double max, double delta);

  // initialize matrix to zero

  void ZeroOutMatrix() { _initMatrix(); }
  
  // add a value to the stats in the matrix

  void addToStats(double est, double truth,
		  double dbm1, double dbm2,
		  double w1, double w2);
  
  // compute the stats and write out
  
  void computeStatsAndWrite();

protected:
  
private:

  string _varName;
  string _topLabel;
  string _xAxisLabel;
  string _yAxisLabel;
  string _outputDir;

  int _nx, _ny;
  double _minx, _dx;
  double _miny, _dy;
  
  double _minWidth2, _maxWidth2;

  double _biasCscaleMin, _biasCscaleMax, _biasCscaleDelta;
  double _sdevCscaleMin, _sdevCscaleMax, _sdevCscaleDelta;

  double **_xySum;
  double **_xySumSq;
  double **_xyCount;
  double **_xyBias;
  double **_xySdev;

  void _allocMatrix();
  void _initMatrix();
  void _freeMatrix();
  
  void _computeStats();

  int _writeOut();

  int _writeDataFile(string statName,
		     double **stat);

  int _writeTextFile(string statName,
		     double **stat);

};

#endif

