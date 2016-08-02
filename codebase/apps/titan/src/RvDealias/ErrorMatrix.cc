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
///////////////////////////////////////////////////////////////
// ErrorMatrix.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2003
//
///////////////////////////////////////////////////////////////
//
// ErrorMatrix computes the error matrix for a variable
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include "ErrorMatrix.hh"
using namespace std;

///////////////
// Constructor

ErrorMatrix::ErrorMatrix()
  
{
  _xySum = NULL;
  _xySumSq = NULL;
  _xyCount = NULL;
  _xyBias = NULL;
  _xySdev = NULL;
  setAxisVals(13, 0.0, 0.5, 26, 0.0, 2.0);
  setWidth2Limits(0.0, 1000.0);
  setBiasCscale(-2.5, 2.5, 0.5);
  setSdevCscale(0, 3.0, 0.3);
  setYAxisLabel("P1/P2 (db)");
  setXAxisLabel("W1");
  setOutputDir(".");

}

///////////////
// destructor

ErrorMatrix::~ErrorMatrix()

{
  _freeMatrix();
}

///////////////////
// set axis values

void ErrorMatrix::setAxisVals(int nx, double minx, double dx,
			      int ny, double miny, double dy)
  
{

  _nx = nx;
  _minx = minx;
  _dx = dx;

  _ny = ny;
  _miny = miny;
  _dy = dy;

  _allocMatrix();

}

//////////////////////////
// set limits for width 2

void ErrorMatrix::setWidth2Limits(double minWidth2, double maxWidth2)

{
  _minWidth2 = minWidth2;
  _maxWidth2 = maxWidth2;
}

////////////////////////////////////
// set limits for color scale values

void ErrorMatrix::setBiasCscale(double min, double max, double delta)

{
  _biasCscaleMin = min;
  _biasCscaleMax = max;
  _biasCscaleDelta = delta;
}

void ErrorMatrix::setSdevCscale(double min, double max, double delta)

{
  _sdevCscaleMin = min;
  _sdevCscaleMax = max;
  _sdevCscaleDelta = delta;
}

//////////////////////////////////////////
// add a value to the stats in the matrix

void ErrorMatrix::addToStats(double est, double truth,
			     double dbm1, double dbm2,
			     double w1, double w2)

{
  
  // check if width2 is within limits
  
  if (w2 < _minWidth2 || w2 >= _maxWidth2) {
    return;
  }
  
  // compute y index

  double dbmDiff = dbm1 - dbm2;
  int yIndex = int ((dbmDiff - _miny) / _dy + 0.5);
  if (yIndex < 0 || yIndex > _ny - 1) {
    return;
  }

  // compute x index

  int xIndex = int ((w1 - _minx) / _dx + 0.5);
  if (xIndex < 0 || xIndex > _nx - 1) {
    return;
  }

  // store accum data

  double error = est - truth;
  _xySum[yIndex][xIndex] += error;
  _xySumSq[yIndex][xIndex] += (error * error);
  _xyCount[yIndex][xIndex] += 1.0;

}

//////////////////////////////////////////
// compute the stats and write out

void ErrorMatrix::computeStatsAndWrite()

{
  _computeStats();
  _writeOut();
}

//////////////////////////////////////////
// compute the stats - bias and sdev

void ErrorMatrix::_computeStats()

{

  for (int iy = 0; iy < _ny; iy++) {

    for (int ix = 0; ix < _nx; ix++) {

      // compute mean and sdev

      double mean = 0.0;
      double sdev = 0.0;
      double nn = _xyCount[iy][ix];
      
      if (nn > 0) {
	mean = _xySum[iy][ix] / nn;
	double diff = (_xySumSq[iy][ix] / nn) - (mean * mean);
	if (diff > 0) {
	  sdev = sqrt(diff);
	}
      }

      _xyBias[iy][ix] = mean;
      _xySdev[iy][ix] = sdev;
      
    } // ix

  } // iy

}
  
////////////////////////////////////////////
// write the error matrix out to ascii files

int ErrorMatrix::_writeOut()

{

  // make sure directory exists
  
  if (ta_makedir_recurse(_outputDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - ErrorMatrix::writeAsciiOutput" << endl;
    cerr << "  Cannot make output dir: " << _outputDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write the stats files as data

  if (_writeDataFile("bias", _xyBias)) {
    return -1;
  }
  if (_writeDataFile("sdev", _xySdev)) {
    return -1;
  }

  // write the stats files as readable text

  if (_writeTextFile("bias", _xyBias)) {
    return -1;
  }
  if (_writeTextFile("sdev", _xySdev)) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
// write an output file

int ErrorMatrix::_writeDataFile(string statName, double **stat)
  
{

  // compute output paths
  
  char path[MAX_PATH_LEN];
  double width2 = (_minWidth2 + _maxWidth2) / 2.0;
  sprintf(path, "%s%s%s_%s.w2_%.1f.dat",
	  _outputDir.c_str(), PATH_DELIM, _varName.c_str(), statName.c_str(),
	  width2);

  // open the file

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ErrorMatrix::_writeFile" << endl;
    cerr << "  Cannot open file for writing: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  fprintf(out, "# Moments retrieval stats from RvDealias\n");
  fprintf(out, "# Created: %s\n", DateTime::str().c_str());
  fprintf(out, "#\n");

  char label[128];
  sprintf(label, "%s of error of %s, W2 %.2f-%.2f",
	  statName.c_str(), _varName.c_str(),
	  _minWidth2, _maxWidth2);

  fprintf(out, "# Top label\n");
  fprintf(out, "%s\n", label);
  fprintf(out, "# X axis label\n");
  fprintf(out, "%s\n", _xAxisLabel.c_str());
  fprintf(out, "# Y axis label\n");
  fprintf(out, "%s\n", _yAxisLabel.c_str());
  
  // fprintf(out, "# Color scale label\n");
  // fprintf(out, "\n");

  fprintf(out, "# Color scale limits\n");
  if (statName.find("bias", 0) != string::npos) {
    fprintf(out, "%g %g\n", _biasCscaleMin, _biasCscaleMax);
  } else {
    fprintf(out, "%g %g\n", _sdevCscaleMin, _sdevCscaleMax);
  }
  
  fprintf(out, "# Number of colors\n");
  int nColors;
  if (statName.find("bias", 0) != string::npos) {
    nColors = (int)
      ((_biasCscaleMax - _biasCscaleMin) / _biasCscaleDelta + 0.5);
  } else {
    nColors = (int)
      ((_sdevCscaleMax - _sdevCscaleMin) / _sdevCscaleDelta + 0.5);
  }
  fprintf(out, "%d\n", nColors);
  
  fprintf(out, "# X axis values\n");
  for (int ix = 0; ix < _nx; ix++) {
    fprintf(out, "%g", _minx + ix * _dx);
    if (ix == _nx - 1) {
      fprintf(out, "\n");
    } else {
      fprintf(out, "  ");
    }
  }
  fprintf(out, "# Y axis values\n");
  for (int iy = 0; iy < _ny; iy++) {
    fprintf(out, "%g", _miny + iy * _dy);
    if (iy == _ny - 1) {
      fprintf(out, "\n");
    } else {
      fprintf(out, "  ");
    }
  }

  fprintf(out, "# value matrix\n");
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      fprintf(out, "%7.3f", stat[iy][ix]);
      if (ix == _nx - 1) {
	fprintf(out, "\n");
      } else {
	fprintf(out, "  ");
      }
    }
  }

  fclose(out);

  return 0;

}

////////////////////////////////////////////
// write a readable text file

int ErrorMatrix::_writeTextFile(string statName, double **stat)
  
{

  // compute output paths
  
  char path[MAX_PATH_LEN];
  double width2 = (_minWidth2 + _maxWidth2) / 2.0;
  sprintf(path, "%s%s%s_%s.w2_%.1f.txt",
	  _outputDir.c_str(), PATH_DELIM, _varName.c_str(), statName.c_str(),
	  width2);

  // open the file

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ErrorMatrix::_writeFile" << endl;
    cerr << "  Cannot open file for writing: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  string label = statName;
  label += " of error of ";
  label += _varName;

  fprintf(out, "############################################################\n");
  fprintf(out, "# Moments retrieval stats from RvDealias\n");
  fprintf(out, "# Created: %s\n", DateTime::str().c_str());
  fprintf(out, "# \n");
  fprintf(out, "# Statistic: %s\n", label.c_str());
  fprintf(out, "# W2 limits: %s\n", _topLabel.c_str());
  fprintf(out, "# \n");
  fprintf(out, "############################################################\n");

  // y axis label

  fprintf(out, "\n");
  fprintf(out, "%s\n", _yAxisLabel.c_str());
  
  // y axis plus data
  
  for (int iy = _ny - 1; iy >= 0; iy--) {
    fprintf(out, "%4g | ", _miny + iy * _dy);
    for (int ix = 0; ix < _nx; ix++) {
      fprintf(out, "%8.3f", stat[iy][ix]);
      if (ix == _nx - 1) {
	fprintf(out, "\n");
      } else {
	fprintf(out, " ");
      }
    }
    fprintf(out, "%4s | ", "n");
    for (int ix = 0; ix < _nx; ix++) {
      fprintf(out, "%8.0f", _xyCount[iy][ix]);
      if (ix == _nx - 1) {
	fprintf(out, "\n");
      } else {
	fprintf(out, " ");
      }
    }
  }

  // xaxis

  fprintf(out, "      ");
  for (int ix = 0; ix < _nx; ix++) {
    fprintf(out, "---------");
    if (ix == _nx - 1) {
      fprintf(out, "\n");
    }
  }
  fprintf(out, "%6s ", _xAxisLabel.c_str());
  for (int ix = 0; ix < _nx; ix++) {
    fprintf(out, "%8g", _minx + ix * _dx);
    if (ix == _nx - 1) {
      fprintf(out, "\n");
    } else {
      fprintf(out, " ");
    }
  }
  fprintf(out, "\n");
  fprintf(out, "\n");

  fclose(out);

  return 0;

}

////////////////////
// memory management

void ErrorMatrix::_allocMatrix()
{
  _freeMatrix();
  _xyBias = (double **) umalloc2(_ny, _nx, sizeof(double));
  _xySdev = (double **) umalloc2(_ny, _nx, sizeof(double));
  _xySum = (double **) umalloc2(_ny, _nx, sizeof(double));
  _xySumSq = (double **) umalloc2(_ny, _nx, sizeof(double));
  _xyCount = (double **) umalloc2(_ny, _nx, sizeof(double));
  _initMatrix();
}

void ErrorMatrix::_initMatrix()
{
  memset(*_xyBias, 0, _ny * _nx * sizeof(double));
  memset(*_xySdev, 0, _ny * _nx * sizeof(double));
  memset(*_xySum, 0, _ny * _nx * sizeof(double));
  memset(*_xySumSq, 0, _ny * _nx * sizeof(double));
  memset(*_xyCount, 0, _ny * _nx * sizeof(double));
}

void ErrorMatrix::_freeMatrix()
{
  if (_xyBias) {
    ufree2((void **) _xyBias);
  }
  if (_xySdev) {
    ufree2((void **) _xySdev);
  }
  if (_xySum) {
    ufree2((void **) _xySum);
  }
  if (_xySumSq) {
    ufree2((void **) _xySumSq);
  }
  if (_xyCount) {
    ufree2((void **) _xyCount);
  }
}

