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
/////////////////////////////////////////////////////////////////////////////
// Optical flow tracking.
//
// Courtesy of:
//   Alan Seed, BMRC, Bureau of Meteorology, Australia
//
// January 2003
//
// Class to manage the tracking of two images using the optical flow method
//
/////////////////////////////////////////////////////////////////////////////

#ifndef of_track_hh
#define of_track_hh

#include <cstdlib>
#include <cstdio>
#include <cstring>

#define PI 3.141592653589793115997963468544185161590576171875
#define MIN_RAINAREA 100
#define NMAX 500

class OFTrack {

public:

  OFTrack(int nLines, int nSamples, float outsideValue, int nGrids,
          char *sFileName, float mrt, int initOption) ;
  
  // outsideValue is a flag to show invalid data usually set to -999
  // but MUST be < 0
  // sFileName = the name of a scratch file to write to
  // initOption = 0 to initialise, 1 to open an existing file
  
  ~OFTrack();

  void track_image(float **thisMap, float **nextMap, int init);
  void track_image(float *thisMap, float *nextMap, int init );
  
  void advect_image(float *thisMap, float adFraction);
  void advect_image2(float **image, float adFraction);
  
  int track_1d(float **thisMap, float **nextMap);
  
  void get_image_advection(float *dLine, float *dSample);
  void set_velocity_1d(float dLine, float dSample);
  
  void get_velocity(float **vx, float **vy );
  void get_velocity(float *vx, float *vy);
  
  void set_velocity(float **vx, float **vy);
  void set_velocity(float *vx, float *vy);

private:

  float **dispX;
  float **dispY;
  float **dxs;
  float **dys;
  float **thisMapP;
  float **nextMapP; 
  float imageAdvectionX;
  float imageAdvectionY;
  int initAdvectionFlag;
  float threshold;
  int nGrids;
  int nLines, nSamples;
  float outsideValue;
  char tempVelFileName[256];
  
  void opt_flow_2(float **thisMap, float **nextMap,
                  int nLines, int nSamples,
                  float **dx, float **dy, int nGrids);
  
  void filter_image(float **image, float **temp, int size);
  
  // IN Number of iterations for Laplace Smoothing
  // IN The number of rows in the data
  // IN The number of cols in the data
  // IN & OUT The data to be smoothed

  void LSmooth_Data(int i4NumberIterations,
                    int i4NumberRows,
                    int i4NumberCols,
                    float** r4Data);
  
  void imageShift(float **in, int startLine, int startSample,
                  int nInLines, int nInSamples,
                  float dx, float dy, float **out,
                  int nOutLines, int nOutSamples, float **mask);
  
  void imageStretch(float **in, int startLine, int startSample,
                    int nInLines, int nInSamples,
                    float ds, float **out,
                    int nOutLines, int nOutSamples, float **mask);

  void imageRotate(float **in, int startLine, int startSample,
                   int nInLines, int nInSamples,
                   float dt, float **out,
                   int nOutLines, int nOutSamples, float **mask);

  size_t read_file(float **xVel, float **yVel,
                   float *dx, float *dy,
                   int size);

  size_t write_file(float **xVel, float **yVel,
                    float dx, float dy,
                    int size);

  void lubksb(float **a, int n, int *indx, float b[]);
  int  ludcmp(float **a, int n, int *indx, float *d);
  
  float calc_correlation(float *shift);

  void amoeba(float **p, float *y, int ndim, float ftol, int *nfunk);

  float amotry(float **p, float y[], float psum[], int ndim, int ihi, float fac);
  
  float **New2DArray(float outsideValue, int nLines, int nSamples);

};

#endif
