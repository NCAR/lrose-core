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
//
// Version 2.0
// included the 1d tracking as part of the class
//
// Version 1.5
// Modified the way that the tracking manages the case of isolated echos
//
// January 2003
// track_2d
// Class to manage the tracking of successive images
//
// Super seed soft where April 2001
//
// Optical Flow Algorithm following
//    Bab-Hadiashar, A, D. Suter, and R. Jarvis, 1996 (?),
// 2-D Motion extraction using an image interpolation technique,
//    SPIE Vol. 2564, 271-281.
// This algorithm gives the linear differential solution for
// I(x,y,t)=Ix * Ux + Iy * Uy + It
//
// This version allows for translation only (no rotation)
// assumes that outsideValue has been set the in calling program
//
// Numerical Recipes provided fourn, lucmp, lubksb, vector, matrix, ...

//---------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <toolsa/mem.h>

#include "OFTrack.hh"

#define SQR(a) (a)*(a)
#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
#define GET_PSUM \
					for (j=1;j<=ndim;j++) {\
					for (sum=0.0,i=1;i<=mpts;i++) sum += p[i][j];\
					psum[j]=sum;}
#define TINY 1.0e-20;
#define NMAX 500
#define NRANSI

//---------------------------------------------------------------------------

OFTrack::OFTrack(int nl, int ns, float ov, int ng,
                 char *velName, float mrt, int init)
  
{

  nLines      = nl;
  nSamples    = ns;
  outsideValue = ov;
  nGrids      = ng;
  strcpy(tempVelFileName, velName);
  threshold = mrt;

  imageAdvectionX = outsideValue;
  imageAdvectionY = outsideValue;
  initAdvectionFlag = 0;

  dispX =  New2DArray(outsideValue, nLines, nSamples);
  dispY =  New2DArray(outsideValue, nLines, nSamples);
  dxs   =  New2DArray(outsideValue, nGrids+2, nGrids+2);
  dys   =  New2DArray(outsideValue, nGrids+2, nGrids+2);

  // set up the scratch file
  // write zeros to the file if initialising
  if (init == 0) {
    size_t rStat = write_file(dxs, dys, imageAdvectionX, imageAdvectionY, nGrids+2);
    initAdvectionFlag = 0;
  }

  // else read in the previous values
  else {
    size_t rStat = read_file(dxs, dys, &imageAdvectionX, &imageAdvectionY, nGrids+2);
    initAdvectionFlag = 1;
  }
}

OFTrack::~OFTrack() {
  size_t rStat = write_file(dxs, dys, imageAdvectionX, imageAdvectionY, nGrids+2);
  ufree2((void **) dispX);
  ufree2((void **) dispY);
  ufree2((void **) dxs);
  ufree2((void **) dys);
}

void OFTrack::get_image_advection(float *dLine, float *dSample){
  *dLine   = imageAdvectionY;
  *dSample = imageAdvectionX;
}

void OFTrack::get_velocity(float *dxOut, float *dyOut) {
  int i4Line = 0;
  int i4Sample = 0;
  int i4StartLine = 0;
  int i4Loop = 0;
  for (i4Loop = 0; i4Loop < nLines*nSamples; i4Loop++) {
    if (i4Loop - i4StartLine == nSamples) {
      i4StartLine = i4Loop;
      i4Line++;
    }
    i4Sample = i4Loop - i4StartLine;
    dxOut[i4Loop] = dispX[i4Line][i4Sample];
    dyOut[i4Loop] = dispY[i4Line][i4Sample];
  }
}

void OFTrack::set_velocity(float *dxOut, float *dyOut) {
  int i4Line = 0;
  int i4Sample = 0;
  int i4StartLine = 0;
  int i4Loop = 0;
  for (i4Loop = 0; i4Loop < nLines*nSamples; i4Loop++) {
    if (i4Loop - i4StartLine == nSamples) {
      i4StartLine = i4Loop;
      i4Line++;
    }
    i4Sample = i4Loop - i4StartLine;
    dispX[i4Line][i4Sample] = dxOut[i4Loop];
    dispY[i4Line][i4Sample] = dyOut[i4Loop];
  }
  imageAdvectionX = 0;
  imageAdvectionY = 0;
}

// method to copy the contents of the two velocity arrays
void OFTrack::get_velocity(float **dxOut, float **dyOut){
  for (int iLine = 0; iLine < nLines; iLine++) {
    for (int iSample = 0; iSample < nSamples; iSample++) {
      dxOut[iLine][iSample] = dispX[iLine][iSample];
      dyOut[iLine][iSample] = dispY[iLine][iSample];
    }
  }
}

// method to copy the contents of the two velocity arrays
void OFTrack::set_velocity(float **dxOut, float **dyOut){
  for (int iLine = 0; iLine < nLines; iLine++) {
    for (int iSample = 0; iSample < nSamples; iSample++) {
      dispX[iLine][iSample] = dxOut[iLine][iSample];
      dispY[iLine][iSample] = dyOut[iLine][iSample];
    }
  }
  imageAdvectionX = 0;
  imageAdvectionY = 0;
}

// Laplace Smoothing
void OFTrack::LSmooth_Data(int i4NumberIterations,     // IN Number of iterations for Laplace Smoothing                  int i4NumberRows,                // IN The number of columns in the data
                         int i4NumberRows,                 // IN The number of rows in the data
	                 int i4NumberCols,                 // IN The number of cols in the data
	                 float** r4Data){            // IN & OUT The data to be smoothed
  //***00000000000000000000000000000000000000000000000000000000000000000000
  //
  // MODULE NAME: mcascade_41.cpp
  //
  // TYPE: void
  //
  // LANGUAGE: c++
  //
  // COMPILER OPTIONS:
  //
  //***11111111111111111111111111111111111111111111111111111111111111111111
  //
  // PURPOSE:
  //
  // To smooth the input field using Laplace Smoothing
  // Smoothed field is put into the outputdata array
  //
  //***22222222222222222222222222222222222222222222222222222222222222222222
  //
  //-----------------------------------------------------------------------
  //
  float **tempArray = New2DArray(0, i4NumberRows, i4NumberCols);
  for (int iter = 0; iter < i4NumberIterations; iter++) {
    for (int iRow = 0; iRow < i4NumberRows; iRow++) {
      for (int iCol = 0; iCol < i4NumberCols; iCol++) {
        if (iCol == 0 || iCol == i4NumberCols-1 || iRow == 0 || iRow == i4NumberRows-1) {
          tempArray[iRow][iCol] =  r4Data[iRow][iCol];
        }
        else {
          if (r4Data[iRow][iCol]   > outsideValue + 1  &&
               r4Data[iRow+1][iCol] > outsideValue + 1  &&
               r4Data[iRow-1][iCol] > outsideValue + 1  &&
               r4Data[iRow][iCol+1] > outsideValue + 1  &&
               r4Data[iRow][iCol-1] > outsideValue + 1) {
            tempArray[iRow][iCol] = r4Data[iRow][iCol] - 0.25*(4*r4Data[iRow][iCol] - r4Data[iRow-1][iCol] - r4Data[iRow+1][iCol] - r4Data[iRow][iCol-1] - r4Data[iRow][iCol+1]);
          }
          else
            tempArray[iRow][iCol] = r4Data[iRow][iCol];
        }
      }
    }

    for (int iRow = 0; iRow < i4NumberRows; iRow++) {
      for (int iCol = 0; iCol < i4NumberCols; iCol++) {
        r4Data[iRow][iCol] = tempArray[iRow][iCol];
      }
    }

  }
  ufree2((void **) tempArray);
}


void OFTrack::opt_flow_2(float **thisMap, float **nextMap, int nLines, int nSamples,
                        float **dx, float **dy, int nGrids){

  // ludcmp variables
  float b[128];
  int indx[128];
  int NP = 2;

  float**a = New2DArray(0, NP+1, NP+1);
  float d;

  // set the step size for the difference equations
  float h1 = -5;   // dx left
  float h2 = -h1;  // dx right
  float h3 = -5;   // dy down
  float h4 = -h3;  // dy up

  // and allocate memory for the difference images
  float **I1       = New2DArray(0, nLines, nSamples);
  float **I2       = New2DArray(0, nLines, nSamples);
  float **I3       = New2DArray(0, nLines, nSamples);
  float **I4       = New2DArray(0, nLines, nSamples);
  float **mask     = New2DArray(0, nLines, nSamples);
  float **thisMapT = New2DArray(0, nLines, nSamples);
  float **nextMapT = New2DArray(0, nLines, nSamples);

  int boxSizeX, boxSizeY;

  for (int iLine = 0; iLine < nLines; iLine++) {
    for (int iSample = 0; iSample < nSamples; iSample++) {
      thisMapT[iLine][iSample] = thisMap[iLine][iSample];
      nextMapT[iLine][iSample] = nextMap[iLine][iSample];
    }
  }

  LSmooth_Data(10, nLines, nSamples, nextMapT);
  LSmooth_Data(10, nLines, nSamples, thisMapT);

  for (int iLine = 0; iLine < nLines; iLine++) {
    for (int iSample = 0; iSample < nSamples; iSample++) {
      if (thisMapT[iLine][iSample] < threshold) thisMapT[iLine][iSample]=0;
      if (nextMapT[iLine][iSample] < threshold) nextMapT[iLine][iSample]=0;
    }
  }

  boxSizeX = nSamples/(nGrids+1);
  boxSizeY = nLines/(nGrids+1);
  int iStep = 2;

  // calculate the size of the local region (box) around each grid point
  int nBoxLines   = boxSizeY*iStep;
  int nBoxSamples = boxSizeX*iStep;
  int iGridLine, iGridSample;
  int startBoxLine, startBoxSample;
  for (iGridLine = 0; iGridLine <= nGrids-iStep+1; iGridLine++) {
    startBoxLine = iGridLine*boxSizeY;
    for (iGridSample = 0; iGridSample <= nGrids-iStep+1; iGridSample++) {
      startBoxSample = iGridSample*boxSizeX;

      // calculate the fraction of the box with non zeros
      float rainPixels = 0;
      for (int ia = 0; ia < nBoxLines; ia++) {
        for (int ib = 0; ib < nBoxSamples; ib++) {
          if (thisMapT[startBoxLine+ia][startBoxSample+ib] > 0) rainPixels++;
        }
      }

      // track if there are enough data in the box
      if (rainPixels > MIN_RAINAREA) {
        for (int ia = 0; ia < nBoxLines; ia++) {
          for (int ib = 0; ib < nBoxSamples; ib++) mask[ia][ib] = 0;
        }

        //generate the reference images
        imageShift(thisMapT, startBoxLine, startBoxSample, nLines, nSamples,
                   h1, 0, I1, nBoxLines, nBoxSamples, mask);

        imageShift(thisMapT, startBoxLine, startBoxSample, nLines, nSamples,
                   h2, 0, I2, nBoxLines, nBoxSamples, mask);

        imageShift(thisMapT, startBoxLine, startBoxSample, nLines, nSamples,
                   0, h3, I3, nBoxLines, nBoxSamples, mask);

        imageShift(thisMapT, startBoxLine, startBoxSample, nLines, nSamples,
                   0, h4, I4, nBoxLines, nBoxSamples, mask);

        // calculate the differences
        for (int ia = 0; ia < nBoxLines; ia++) {
          for (int ib = 0; ib < nBoxSamples; ib++) {
            if (mask[ia][ib] == 0) {
              I2[ia][ib] -= I1[ia][ib];
              I4[ia][ib] -= I3[ia][ib];
            }
            else {
              I2[ia][ib] = 0;
              I4[ia][ib] = 0;
            }
          }
        }

        // now set up the array of coefficients
        for (int ia = 1; ia <= NP; ia++) {
          for (int ib = 1; ib <= NP; ib++) a[ia][ib] = 0;
          b[ia] = 0;
        }

        for (int ia = 0; ia < nBoxLines; ia++) {
          for (int ib = 0; ib < nBoxSamples; ib++) {
            float diff =  (nextMapT[startBoxLine+ia][startBoxSample+ib]-thisMapT[startBoxLine+ia][startBoxSample+ib]);
            b[1] += I2[ia][ib]*diff;
            b[2] += I4[ia][ib]*diff;
          }
        }
        for (int ia = 1; ia <= NP; ia++) b[ia] *= 2;

        for (int ia = 0; ia < nBoxLines; ia++) {
          for (int ib = 0; ib < nBoxSamples; ib++) {
            a[1][1] += I2[ia][ib]*I2[ia][ib];
            a[1][2] += I2[ia][ib]*I4[ia][ib];
            a[2][2] += I4[ia][ib]*I4[ia][ib];
          }
        }

        a[2][1] = a[1][2];
        for (int ia = 1; ia <= NP; ia++) a[ia][1] /= h2;
        for (int ia = 1; ia <= NP; ia++) a[ia][2] /= h4;
        int status = ludcmp(a,NP,indx,&d);

        if (status != 0){
          lubksb(a,NP,indx,b);

          // now place the displacement in the grid points covered by this box
          dx[iGridLine][iGridSample] = b[1];
          dy[iGridLine][iGridSample] = b[2];
        }
      }
      else {
        // not enough data to track
        dx[iGridLine][iGridSample] = outsideValue;
        dy[iGridLine][iGridSample] = outsideValue;
      }
    }
  }

  // free up the memory
  ufree2((void **) a);
  ufree2((void **) I1);
  ufree2((void **) I2);
  ufree2((void **) I3);
  ufree2((void **) I4);
  ufree2((void **) mask);
  ufree2((void **) thisMapT);
  ufree2((void **) nextMapT);
}

//------------------------------------------------------------------------------
void	OFTrack::imageShift(float **in, int startLine, int startSample, int nInLines, int nInSamples,
                          float dx, float dy,
                          float **out, int nOutLines, int nOutSamples, float **mask){

  // function to shift the image dx, dy
  int outLine, outSample, inLine, inSample;
  for (outLine = 0; outLine < nOutLines; outLine++) {
    for (outSample = 0; outSample < nOutSamples; outSample++) {
      inLine = startLine+outLine-dy;
      inSample = startSample+outSample-dx;
      if (inLine >= 0 && inLine < nInLines && inSample >= 0 && inSample < nInSamples)
        out[outLine][outSample] = in[inLine][inSample];
      else {
        out[outLine][outSample] = 0;
        mask[outLine][outSample] = 1;
      }
    }
  }
}

//------------------------------------------------------------------------------
void	OFTrack::imageStretch(float **in, int startLine, int startSample, int nInLines, int nInSamples,
                            float ds, float **out, int nOutLines, int nOutSamples, float **mask){

  // function to stretch the image 1+ds
  int outLine, outSample, inLine, inSample;
  for (outLine = 0; outLine < nOutLines; outLine++) {
    for (outSample = 0; outSample < nOutSamples; outSample++) {
      inLine = startLine+(float)outLine/(1+ds);
      inSample = startSample+(float)outSample/(1+ds);
      if (inLine >= 0 && inLine < nInLines && inSample >= 0 && inSample < nInSamples)
        out[outLine][outSample] = in[inLine][inSample];
      else {
        out[outLine][outSample] = 0;
        mask[outLine][outSample] = 1;
      }
    }
  }
}

//------------------------------------------------------------------------------
void	OFTrack::imageRotate(float **in, int startLine, int startSample, int nInLines, int nInSamples,
                           float dt, float **out, int nOutLines, int nOutSamples, float **mask){

  // function to rotate the image about 0,0
  int outLine, outSample, inLine, inSample;
  for (outLine = 1; outLine <= nOutLines; outLine++) {
    for (outSample = 1; outSample <= nOutSamples; outSample++) {
      float r = sqrt(outSample*outSample+outLine*outLine);
      float theta = atan((float)outLine/(float)outSample);
      inLine = startLine+r*sin(theta-dt)-1;
      inSample = startSample+r*cos(theta-dt)-1;
      if (inLine >= 0 && inLine < nInLines && inSample >= 0 && inSample < nInSamples)
        out[outLine-1][outSample-1] = in[inLine][inSample];
      else {
        out[outLine-1][outSample-1] = 0;
        mask[outLine-1][outSample-1] = 1;
      }
    }
  }
}

void OFTrack::track_image(float *thisMap, float *nextMap, int init) {
  float **tempMapA = New2DArray(0, nLines, nSamples);
  float **tempMapB = New2DArray(0, nLines, nSamples);
  int i4Loop;
  int i4Sample;
  int i4Line = 0;
  int i4StartLine = 0;
  for (i4Loop = 0; i4Loop < nLines*nSamples; i4Loop++) {
    if (i4Loop - i4StartLine == nSamples) {
      i4StartLine = i4Loop;
      i4Line++;
    }
    i4Sample = i4Loop - i4StartLine;
    tempMapA[i4Line][i4Sample] = thisMap[i4Loop];
    tempMapB[i4Line][i4Sample] = nextMap[i4Loop];
  }
  track_image(tempMapA, tempMapB, init);
  ufree2((void **) tempMapA);
  ufree2((void **) tempMapB);
}

//------------------------------------------------------------------------------
void OFTrack::track_image(float **thisMap, float **nextMap, int init) {

  // function to track the displacement between two images

  // thisMap[iLine][iSample], 0 < iLine < nLines; 0 < iSample < nSamples at time t
  // nextMap[iLine][iSample], 0 < iLine < nLines; 0 < iSample < nSamples at time t+1
  // the values in thisMap and nextMap are modified by this function

  // results are returned on a grid of temporally smoothed displacements in dxs, dys
  // dxs[iGridY][iGridX], 0 < iGridY < nGridY+2; 0 < iGridX < nGridX+2
  // dys[iGridY][iGridX], 0 < iGridY < nGridY+2; 0 < iGridX < nGridX+2

  // the output dxs, dys grids have spacing
  // float gridLineStep   = (nLines-boxSize)/(nGridY-1);
  // float gridSampleStep = (nSamples-boxSize)/(nGridX-1);
  // lines   = 0, boxSize/2, boxSize/2+gridLineStep ... boxSize/2+(nGridY-1)*gridLineStep, nLines
  // samples = 0, boxSize/2, boxSize/2+gridSampleStep ... boxSize/2+(nGridX-1)*gridSampleStep, nSamples

  // these displacements are interpolated onto full size images using bilinear interpolation
  // dispX[iLine][iSample], 0 < iLine < nLines; 0 < iSample < nSamples
  // dispY[iLine][iSample], 0 < iLine < nLines; 0 < iSample < nSamples

  // boxSize is the size of the box used to determine the advection at each grid point should be >= 32
  // threshold is the rain/no rain threshold to be used- 0.1mm/h 15 dBZ work well enough
  // init is set to zero to force a reinitialisation of the temporal averaging

  // initialise the arrays if required
  thisMapP = thisMap;
  nextMapP = nextMap;

  if (init < 1) {
    initAdvectionFlag = 0;
    imageAdvectionX = outsideValue;
    imageAdvectionY = outsideValue;
    for (int iLine = 0; iLine < nGrids+2; iLine++) {
      for (int iSample = 0; iSample < nGrids+2; iSample++) {
        dxs[iLine][iSample] = outsideValue;
        dys[iLine][iSample] = outsideValue;
      }
    }
  }

  // get the mean advection for the entire image and add it to thisMap
  int status = track_1d(thisMap, nextMap);
  if (status == 0) { // no displacements were calculated so quit
    initAdvectionFlag = 0;
    imageAdvectionX = outsideValue;
    imageAdvectionY = outsideValue;
    for (int iLine = 0; iLine < nGrids+2; iLine++) {
      for (int iSample = 0; iSample < nGrids+2; iSample++) {
        dxs[iLine][iSample] = outsideValue;
        dys[iLine][iSample] = outsideValue;
      }
    }
    for (int iLine = 0; iLine < nLines; iLine++) {
      for (int iSample = 0; iSample < nSamples; iSample++) {
        dispX[iLine][iSample] = 0;
        dispY[iLine][iSample] = 0;
      }
    }
    return;
  }

  // start the 2d tracking
  float **dx = New2DArray(0,nGrids, nGrids);
  float **dy = New2DArray(0,nGrids, nGrids);
  float **t1 = New2DArray(0, nLines, nSamples);

  for (int iLine = 0; iLine < nLines; iLine++) {
    for (int iSample = 0; iSample < nSamples; iSample++) {
      t1[iLine][iSample] = thisMap[iLine][iSample];
    }
  }
  advect_image2(t1, 1.0);

  // calculate the optical flow
  opt_flow_2(t1, nextMap, nLines, nSamples, dx, dy, nGrids);

  // add the mean image displacement back onto the optical flow estimates
  // or replace invalid velocity with the mean velocity
  for (int iGridLine = 0; iGridLine < nGrids; iGridLine++) {
    for (int iGridSample = 0; iGridSample < nGrids; iGridSample++) {
      if (dx[iGridLine][iGridSample] < outsideValue+1)
        dx[iGridLine][iGridSample] = imageAdvectionX;
      else
        dx[iGridLine][iGridSample] += imageAdvectionX;

      if (dy[iGridLine][iGridSample] < outsideValue+1)
        dy[iGridLine][iGridSample] = imageAdvectionY;
      else
        dy[iGridLine][iGridSample] += imageAdvectionY;
    }
  }

  // finally smooth dispX and dispY in space
  LSmooth_Data(1, nGrids, nGrids, dx);
  LSmooth_Data(1, nGrids, nGrids, dy);

  // and then in time
  for (int iLine = 0; iLine < nGrids; iLine++) {
    for (int iSample = 0; iSample < nGrids; iSample++) {
      if (initAdvectionFlag == 0){
        dxs[iLine+1][iSample+1] = dx[iLine][iSample];
        dys[iLine+1][iSample+1] = dy[iLine][iSample];
      }
      else {
        if (dxs[iLine+1][iSample+1] > outsideValue+1 && dx[iLine][iSample] > outsideValue+1)
          dxs[iLine+1][iSample+1] = 0.4*dx[iLine][iSample] + 0.6*dxs[iLine+1][iSample+1];
        else
          dxs[iLine+1][iSample+1] = dx[iLine][iSample];

        if (dys[iLine+1][iSample+1] > outsideValue+1 && dy[iLine][iSample] > outsideValue+1)
          dys[iLine+1][iSample+1] = 0.4*dy[iLine][iSample] + 0.6*dys[iLine+1][iSample+1];
        else
          dys[iLine+1][iSample+1] = dy[iLine][iSample];
      }
    }
  }

  // add the extra rows, cols for bilinear interpolation
  for (int iSample = 1; iSample <= nGrids; iSample++) {
    dxs[0][iSample] = dxs[1][iSample];
    dxs[nGrids+1][iSample] = dxs[nGrids][iSample];
    dys[0][iSample] = dys[1][iSample];
    dys[nGrids+1][iSample] = dys[nGrids][iSample];
  }

  for (int iLine = 1; iLine <= nGrids; iLine++) {
    dxs[iLine][0] = dxs[iLine][1];
    dxs[iLine][nGrids+1] = dxs[iLine][nGrids];
    dys[iLine][0] = dys[iLine][1];
    dys[iLine][nGrids+1] = dys[iLine][nGrids];
  }

  dxs[0][0] = dxs[1][1];
  dys[0][0] = dys[1][1];

  dxs[0][nGrids+1] = dxs[1][nGrids];
  dys[0][nGrids+1] = dys[1][nGrids];

  dxs[nGrids+1][nGrids+1] = dxs[nGrids][nGrids];
  dys[nGrids+1][nGrids+1] = dys[nGrids][nGrids];

  dxs[nGrids+1][0] = dxs[nGrids][1];
  dys[nGrids+1][0] = dys[nGrids][1];

  // write these arrays to file
  int rStat = write_file(dxs, dys, imageAdvectionX, imageAdvectionY, nGrids+2);

  // and interpolate onto the full image
  float gridLineStep   = nLines/(nGrids+1);
  float gridSampleStep = nSamples/(nGrids+1);

  int *x1a = new int [nGrids+2];
  int *x2a = new int [nGrids+2];
  for (int ia = 1; ia <= nGrids; ia++) x1a[ia] = ia*gridSampleStep;
  for (int ia = 1; ia <= nGrids; ia++) x2a[ia] = ia*gridLineStep;
  x1a[0] = 0;
  x2a[0] = 0;
  x1a[nGrids+1] = nSamples;
  x2a[nGrids+1] = nLines;
  float u,t;
  for (int iGridLine = 0; iGridLine < nGrids+1; iGridLine++) {
    for (int iGridSample = 0; iGridSample < nGrids+1; iGridSample++) {
      for (int iLine = x2a[iGridLine]; iLine < x2a[iGridLine+1]; iLine++) {
        u = (float)(iLine-x2a[iGridLine])/(float)(x2a[iGridLine+1] - x2a[iGridLine]);
        for (int iSample = x1a[iGridSample]; iSample < x1a[iGridSample+1]; iSample++) {
          t = (float)(iSample-x1a[iGridSample])/(float)(x1a[iGridSample+1]-x1a[iGridSample]);
          if (dxs[iGridLine][iGridSample] > outsideValue+1 &&
               dxs[iGridLine][iGridSample+1] > outsideValue+1 &&
               dxs[iGridLine+1][iGridSample+1] > outsideValue+1 &&
               dxs[iGridLine+1][iGridSample] > outsideValue+1) {
            dispX[iLine][iSample] = (1-t)*(1-u)*dxs[iGridLine][iGridSample] +
              t*(1-u)*dxs[iGridLine][iGridSample+1] +
              t*u*dxs[iGridLine+1][iGridSample+1] +
              (1-t)*u*dxs[iGridLine+1][iGridSample];
          }
          else {
            // use the nearest grid point
            int nGridLine, nGridSample;

            if (u < 0.5)
              nGridLine = iGridLine;
            else
              nGridLine = iGridLine+1;

            if (t < 0.5)
              nGridSample = iGridSample;
            else
              nGridSample = iGridSample+1;

            dispX[iLine][iSample] = dxs[nGridLine][nGridSample];
          }

          if (dys[iGridLine][iGridSample] > outsideValue+1 &&
               dys[iGridLine][iGridSample+1] > outsideValue+1 &&
               dys[iGridLine+1][iGridSample+1] > outsideValue+1 &&
               dys[iGridLine+1][iGridSample] > outsideValue+1) {


            dispY[iLine][iSample] = (1-t)*(1-u)*dys[iGridLine][iGridSample] +
              t*(1-u)*dys[iGridLine][iGridSample+1] +
              t*u*dys[iGridLine+1][iGridSample+1] +
              (1-t)*u*dys[iGridLine+1][iGridSample];
          }
          else {
            // use the nearest grid point
            int nGridLine, nGridSample;

            if (u < 0.5)
              nGridLine = iGridLine;
            else
              nGridLine = iGridLine+1;

            if (t < 0.5)
              nGridSample = iGridSample;
            else
              nGridSample = iGridSample+1;

            dispY[iLine][iSample] = dys[nGridLine][nGridSample];

          }
        }
      }
    }
  }

  if (dx  != NULL) ufree2((void **) dx);
  if (dy  != NULL) ufree2((void **) dy);
  ufree2((void **) t1);
  delete [] x1a;
  delete [] x2a;
}

void OFTrack::advect_image(float *thisMap, float adFraction) {
  float **tempMapA = New2DArray(0, nLines, nSamples);
  int i4Loop;
  int i4Sample;
  int i4Line      = 0;
  int i4StartLine = 0;
  for (i4Loop = 0; i4Loop < nLines*nSamples; i4Loop++) {
    if (i4Loop - i4StartLine == nSamples) {
      i4StartLine = i4Loop;
      i4Line++;
    }
    i4Sample = i4Loop - i4StartLine;
    tempMapA[i4Line][i4Sample] = thisMap[i4Loop];
  }

  advect_image2(tempMapA, adFraction);

  i4Line      = 0;
  i4StartLine = 0;
  for (i4Loop = 0; i4Loop < nLines*nSamples; i4Loop++) {
    if (i4Loop - i4StartLine == nSamples) {
      i4StartLine = i4Loop;
      i4Line++;
    }
    i4Sample = i4Loop - i4StartLine;
    thisMap[i4Loop] = tempMapA[i4Line][i4Sample];
  }
  ufree2((void **) tempMapA);
}

//------------------------------------------------------------------------------
void	OFTrack::advect_image2 (float **image, float adFraction){
  // function to advect image by adFraction*dispX, adFraction*dispY
  // Copes better with non-integer displacements, by taking a weighted average of four source pixels
  // Corrects for numerical diffusion introduced by the averaging process

  if (imageAdvectionX < outsideValue+1) return; // no valid tracking fields
  float **temp  = New2DArray(0, nLines, nSamples);

  // loop through and fill with the nearest neighbour
  float startLineD, startSampleD;
  int startLine, startSample;
  float nLinesMinus1 = (float) (nLines-1);
  float nSamplesMinus1 = (float) (nSamples-1);
  float fractionLine, fractionSample, value;
  float outsideValuePlusOne = outsideValue+1;
  int iLine, iSample;

  for (iLine = 0; iLine < nLines; iLine++) {
    for (iSample = 0; iSample < nSamples; iSample++) {
      // the pixel probably has an advection that is not too different from the advection at the target
      startLineD = ((float) iLine) -dispY[iLine][iSample]*adFraction;
      startSampleD = ((float) iSample) -dispX[iLine][iSample]*adFraction;

      if (startLineD >= 0 && startSampleD >= 0 && startLineD < nLinesMinus1 && startSampleD < nSamplesMinus1) {
        // All of the source area is within the defined area
        temp[iLine][iSample] = 0.0;

        // Top left pixel
        startLine = (int) startLineD;
        startSample = (int) startSampleD;
        value = image[startLine][startSample];
        if (value > outsideValuePlusOne) {
          fractionLine = 1.0+((float) startLine)-startLineD;
          fractionSample = 1.0+((float) startSample)-startSampleD;
          temp[iLine][iSample] += value*fractionSample*fractionLine;
        } else {
          temp[iLine][iSample] = outsideValue;
        }

        // Top right pixel
        ++startSample;
        value = image[startLine][startSample];
        if (temp[iLine][iSample] > outsideValuePlusOne && value > outsideValuePlusOne) {
          fractionSample = 1.0-fractionSample;
          temp[iLine][iSample] += value*fractionSample*fractionLine;
        } else {
          temp[iLine][iSample] = outsideValue;
        }

        // Bottom right pixel
        ++startLine;
        value = image[startLine][startSample];
        if (temp[iLine][iSample] > outsideValuePlusOne && value > outsideValuePlusOne) {
          fractionLine = 1.0-fractionLine;
          temp[iLine][iSample] += value*fractionSample*fractionLine;
        } else {
          temp[iLine][iSample] = outsideValue;
        }

        // Bottom left pixel
        --startSample;
        value = image[startLine][startSample];
        if (temp[iLine][iSample] > outsideValuePlusOne && value > outsideValuePlusOne) {
          fractionSample = 1.0-fractionSample;
          temp[iLine][iSample] += value*fractionSample*fractionLine;
        } else {
          temp[iLine][iSample] = outsideValue;
        }
      } else {
        // Some or all of the source area is outside the original image
        temp[iLine][iSample] = outsideValue;
      }
    }
  }

  for (iLine = 0; iLine < nLines; iLine++) {
    for (iSample = 0; iSample < nSamples; iSample++) {
      image[iLine][iSample] = temp[iLine][iSample];
    }
  }
  ufree2((void **) temp);
}

int OFTrack::track_1d(float **thisMap, float **nextMap) {
  // method to estimate the advection between maps returned as dSample, dLine
  // returns 0 on failure, 1 on success

  int ip, ia, np, iLine, iSample;
  float tx, ty;
  np = 2;
  float **translation = New2DArray(0, np+2, np+1);

  // starting points for the translation
  if (imageAdvectionX > outsideValue+1) {
    translation[1][1] = imageAdvectionX;
    translation[2][1] = imageAdvectionX-5;
    translation[3][1] = imageAdvectionX+5;
  }
  else {
    translation[1][1] = 0;
    translation[2][1] = -15;
    translation[3][1] = 15;
  }

  if (imageAdvectionY > outsideValue+1) {
    translation[1][2] = imageAdvectionY + 10;
    translation[2][2] = imageAdvectionY-7;
    translation[3][2] = imageAdvectionY;
  }
  else {
    translation[1][2] = 15;
    translation[2][2] = -15;
    translation[3][2] = 0;
  }

  float y[6], x[6];
  for (ip =1; ip <= np+1; ++ip) {
    for (ia = 1; ia <= np; ++ia) x[ia] = translation[ip][ia];
    y[ip] = calc_correlation(x);
  }
  int nIterations;

  amoeba(translation, y, np, 1.0E-12, &nIterations);
  if (nIterations < 500) {
    float bestFit = y[1];
    tx = translation[1][1];
    ty = translation[1][2];

    for (ia = 1; ia <= np+1; ++ia) {
      if (y[ia] < bestFit) {
        tx = translation[ia][1];
        ty = translation[ia][2];
        bestFit = y[ia];
      }
    }

    if (bestFit > 0.75) {  // bestFit = 1-r^2
      for (ip = 1; ip <= np+1; ++ip) delete [] translation[ip];
      delete [] translation;
      imageAdvectionX = outsideValue;
      imageAdvectionY = outsideValue;
      for (int iLine = 0; iLine < nLines; iLine++) {
        for (int iSample = 0; iSample < nSamples; iSample++) {
          dispX[iLine][iSample] = 0;
          dispY[iLine][iSample] = 0;
        }
      }
      return 0;
    }

    if (imageAdvectionY > outsideValue+1  && imageAdvectionX > outsideValue+1) {
      imageAdvectionX = 0.2*tx + 0.8*imageAdvectionX;
      imageAdvectionY = 0.2*ty + 0.8*imageAdvectionY;
    }
    else {
      imageAdvectionX = tx;
      imageAdvectionY = ty;
    }
    ufree2((void **) translation);

    for (int iLine = 0; iLine < nLines; iLine++) {
      for (int iSample = 0; iSample < nSamples; iSample++) {
        dispX[iLine][iSample] = imageAdvectionX;
        dispY[iLine][iSample] = imageAdvectionY;
      }
    }
  }
  else {
    imageAdvectionX = outsideValue;
    imageAdvectionY = outsideValue;
    for (int iLine = 0; iLine < nLines; iLine++) {
      for (int iSample = 0; iSample < nSamples; iSample++) {
        dispX[iLine][iSample] = 0;
        dispY[iLine][iSample] = 0;
      }
    }
    return 0;
  }
  return 1;
}

// read the velocity data, returns -1 on file open error, bytes read otherwise
size_t  OFTrack::read_file(float **xVel, float **yVel, float *dx, float *dy, int size) {
  size_t nBytes = 0;
  FILE *inFile = fopen (tempVelFileName, "rb");
  if (inFile == NULL) return nBytes;

  size_t status = fread(dx, 4, 1, inFile);
  nBytes += status;

  status = fread(dy, 4, 1, inFile);
  nBytes += status;
  for (int iLine = 0; iLine < size; iLine++) {
    status = fread(xVel[iLine], 4, size, inFile);
    nBytes += status;
  }
  for (int iLine = 0; iLine < size; iLine++) {
    status = fread(yVel[iLine], 4, size, inFile);
    nBytes += status;
  }
  fclose (inFile);

  return nBytes;
}

//-----------------------------------------------------------------------------
size_t OFTrack::write_file(float **xVel, float **yVel, float dx, float dy, int size) {
  size_t nBytes = 0;
  FILE *inFile = fopen(tempVelFileName, "wb");
  if (inFile == NULL) return nBytes;

  float temp[8];
  temp[0] = dx;
  temp[1] = dy;
  size_t status = fwrite(temp, 4, 2, inFile);
  nBytes += status;

  for (int iLine = 0; iLine < size; iLine++) {
    status = fwrite(xVel[iLine], 4, size, inFile);
    nBytes += status;
  }

  for (int iLine = 0; iLine < size; iLine++) {
    status = fwrite(yVel[iLine], 4, size, inFile);
    nBytes += status;
  }

  fclose(inFile);
  return nBytes;

}

// method to set the velocity field
void OFTrack::set_velocity_1d(float dLine, float dSample){
  imageAdvectionX = dSample;
  imageAdvectionY = dLine;
  for (int iLine = 0; iLine < nLines; iLine++) {
    for (int iSample = 0; iSample < nSamples; iSample++) {
      dispX[iLine][iSample] = imageAdvectionX;
      dispY[iLine][iSample] = imageAdvectionY;
    }
  }
}

float OFTrack::calc_correlation(float *shift) {

  int outLine, outSample, inLine, inSample;
  int mapSize = nLines * nSamples;
  double sx  = 0.0;
  double sxx = 0.0;
  double sy  = 0.0;
  double syy = 0.0;
  double sxy = 0.0;
  double n   = 0.0;

  for (inLine = 0; inLine < nLines; inLine++) {
    for (inSample = 0; inSample < nSamples; inSample++) {
      outSample = inSample + shift[1];
      outLine   = inLine  + shift[2];
      if (outLine >= 0 && outLine < nLines &&
           outSample >= 0 && outSample < nSamples){
        if (nextMapP[outLine][outSample] > outsideValue+1 && thisMapP[inLine][inSample] > outsideValue+1) {
          sx  += nextMapP[outLine][outSample];
          sy  += thisMapP[inLine][inSample];
          sxx += nextMapP[outLine][outSample]*nextMapP[outLine][outSample];
          syy += thisMapP[inLine][inSample]*thisMapP[inLine][inSample];
          sxy += thisMapP[inLine][inSample]*nextMapP[outLine][outSample];
          n++;
        }
      }
    }
  }

  double rsq;
  double top    = (n*sxy-sx*sy)*(n*sxy-sx*sy);
  double bottom = (n*sxx-sx*sx)*(n*syy-sy*sy);

  if (bottom > 0.0)
    rsq = 1.0 - top/bottom;
  else
    rsq = 1.0;

  return (float)rsq;

}

void OFTrack::amoeba(
	float **p,          // INOUT Series of points used in evalulating the function
	float *y,           // INOUT Value of the function at the points p
	int ndim,           // IN The number of dimensions
	float ftol,         // IN Fractional convergence tolerance
	int *nfunk          // OUT The number of function evaluations required for convergence
       ) {
  //***00000000000000000000000000000000000000000000000000000000000000000000
  //
  // MODULE NAME: optical_flow.cpp
  //
  // TYPE: void
  //
  // LANGUAGE: c++
  //
  // COMPILER OPTIONS:
  //
  //***11111111111111111111111111111111111111111111111111111111111111111111
  //
  // PURPOSE:
  //
  // To find the minimum of a function, and thereby the optimal translation
  // vector between two smoothed radar analyses
  //
  //***22222222222222222222222222222222222222222222222222222222222222222222
  //
  // CHANGE HISTORY
  //
  // VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
  //
  // 1.00         A Programmer    dd/mm/yy     Initial version
  // 1.01         A Programmer    dd/mm/yy     Change to ....
  // 1.02         etc....
  //
  //***33333333333333333333333333333333333333333333333333333333333333333333
  //
  // REVIEW HISTORY
  //
  // VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
  //
  // 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
  // 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
  // 1.02         etc....
  //
  //***44444444444444444444444444444444444444444444444444444444444444444444
  //
  // STRUCTURE:
  //
  //
  // NOTES:
  //
  // Multidimensional minimisation of the function funk(x) where x[1,,ndim] is a vector in ndim
  // dimensions, by the downhill simplex method of Nelder and Mead.  The matrix p[1..ndim+1]
  // [1..ndim] is input.  Its ndim+1 rows are ndim-dimensional vectors which are the vertices of
  // the starting simplex.  Also input is the vector y[1..ndim+1], whose components must be pre-
  // initialised to the values of funk evaluated at the ndim+1 vertices (rows) of p; and ftol the
  // fractional convergence tolerance to be achieved in the function value (n.b.!).  On output, p and
  // y will have been reset to ndim+1 new points all within ftol of a minimum function value, and
  // nfunk gives the function of function evaluations taken.
  //
  // Note: Has been fixed to only run with the function calc_correlation
  //
  // REFERENCES:
  //
  // Numerical recipes
  //
  //***55555555555555555555555555555555555555555555555555555555555555555555
  //
  // RETURN CODES
  //
  // None
  //
  //***66666666666666666666666666666666666666666666666666666666666666666666
  //
  // FILES USED:
  //
  // None
  //
  //***77777777777777777777777777777777777777777777777777777777777777777777
  //
  // DYNAMIC ARRAYS ALLOCATED
  //
  // None
  //
  //***88888888888888888888888888888888888888888888888888888888888888888888
  //
  // CLASS-WIDE VARIABLES USED:
  //
  // None
  //
  // STANDARD LIBRARIES USED:
  //
  // None
  //
  //***99999999999999999999999999999999999999999999999999999999999999999999
  //
  //
  int i,ihi,ilo,inhi,j,mpts=ndim+1;
  float rtol,sum,temp,ysave,ytry,*psum;

  psum = new float [ndim+1];
  *nfunk=0;
  GET_PSUM
    for (;;) {
      ilo=1;
      ihi = y[1]>y[2] ? (inhi=2,1) : (inhi=1,2);
      for (i=1;i<=mpts;i++) {
        if (y[i] <= y[ilo]) ilo=i;
        if (y[i] > y[ihi]) {
          inhi=ihi;
          ihi=i;
        } else if (y[i] > y[inhi] && i != ihi) inhi=i;
      }

      if (fabs(y[ihi]) + fabs(y[ilo]) == 0)
        rtol = 0;
      else
        rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo]));

      if (rtol < ftol) {
        SWAP(y[1],y[ilo])
          for (i=1;i<=ndim;i++) SWAP(p[1][i],p[ilo][i])
                                  break;
      }
      if (*nfunk >= NMAX) return;
      *nfunk += 2;
      ytry=amotry(p,y,psum,ndim,ihi,-1.0);
      if (ytry <= y[ilo])
        ytry=amotry(p,y,psum,ndim,ihi,2.0);
      else if (ytry >= y[inhi]) {
        ysave=y[ihi];
        ytry=amotry(p,y,psum,ndim,ihi,0.5);
        if (ytry >= ysave) {
          for (i=1;i<=mpts;i++) {
            if (i != ilo) {
              for (j=1;j<=ndim;j++)
                p[i][j]=psum[j]=0.5*(p[i][j]+p[ilo][j]);
              y[i]=calc_correlation(psum);
            }
          }
          *nfunk += ndim;
          GET_PSUM
            }
      } else --(*nfunk);
    }
  delete [] psum;
}

#undef SWAP
#undef GET_PSUM
#undef NMAX

float OFTrack::amotry(
	float **p,           // INOUT Series of points used in evalulating the function
	float y[],           // INOUT Value of the function at the points p
	float psum[],        // INOUT
	int ndim,            // IN The number of dimensions
	int ihi,             // IN
	float fac           // IN Factor extrapolate by
       ) {
  //***00000000000000000000000000000000000000000000000000000000000000000000
  //
  // MODULE NAME: optical_flow.cpp
  //
  // TYPE: float
  //
  // LANGUAGE: c++
  //
  // COMPILER OPTIONS:
  //
  //***11111111111111111111111111111111111111111111111111111111111111111111
  //
  // PURPOSE:
  //
  // Used by amoeba in the function minimisation.
  //
  //***22222222222222222222222222222222222222222222222222222222222222222222
  //
  // CHANGE HISTORY
  //
  // VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
  //
  // 1.00         A Programmer    dd/mm/yy     Initial version
  // 1.01         A Programmer    dd/mm/yy     Change to ....
  // 1.02         etc....
  //
  //***33333333333333333333333333333333333333333333333333333333333333333333
  //
  // REVIEW HISTORY
  //
  // VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
  //
  // 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
  // 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
  // 1.02         etc....
  //
  //***44444444444444444444444444444444444444444444444444444444444444444444
  //
  // STRUCTURE:
  //
  //
  // NOTES:
  //
  // Note: Has been fixed to only run with the function correlation_translate
  //
  // REFERENCES:
  //
  // Numerical recipes
  //
  //***55555555555555555555555555555555555555555555555555555555555555555555
  //
  // RETURN CODES
  //
  // Returns ...
  //
  //***66666666666666666666666666666666666666666666666666666666666666666666
  //
  // FILES USED:
  //
  // None
  //
  //***77777777777777777777777777777777777777777777777777777777777777777777
  //
  // DYNAMIC ARRAYS ALLOCATED
  //
  // None
  //
  //***88888888888888888888888888888888888888888888888888888888888888888888
  //
  // CLASS-WIDE VARIABLES USED:
  //
  // None
  //
  // STANDARD LIBRARIES USED:
  //
  // None
  //
  //***99999999999999999999999999999999999999999999999999999999999999999999
  //
  int j;
  float fac1,fac2,ytry;
  float ptry[256];
  fac1=(1.0-fac)/ndim;
  fac2=fac1-fac;
  for (j=1;j<=ndim;j++) ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
  ytry=calc_correlation(ptry);
  if (ytry < y[ihi]) {
    y[ihi]=ytry;
    for (j=1;j<=ndim;j++) {
      psum[j] += ptry[j]-p[ihi][j];
      p[ihi][j]=ptry[j];
    }
  }
  return ytry;
}

#undef NRANSI

//------------------------------------------------------------------------------
int OFTrack::ludcmp(float **a, int n, int *indx, float *d)
{
  // returns 1 on success, 0 on failure
  int i,imax,j,k;
  float big,dum,sum,temp;
  float vv[256];

  *d=1.0;
  for (i=1;i<=n;i++) {
    big=0.0;
    for (j=1;j<=n;j++)
      if ((temp=fabs(a[i][j])) > big) big=temp;
    if (big == 0.0) return 0;
    vv[i]=1.0/big;
  }
  for (j=1;j<=n;j++) {
    for (i=1;i<j;i++) {
      sum=a[i][j];
      for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
      a[i][j]=sum;
    }
    big=0.0;
    for (i=j;i<=n;i++) {
      sum=a[i][j];
      for (k=1;k<j;k++)
        sum -= a[i][k]*a[k][j];
      a[i][j]=sum;
      if ((dum=vv[i]*fabs(sum)) >= big) {
        big=dum;
        imax=i;
      }
    }
    if (j != imax) {
      for (k=1;k<=n;k++) {
        dum=a[imax][k];
        a[imax][k]=a[j][k];
        a[j][k]=dum;
      }
      *d = -(*d);
      vv[imax]=vv[j];
    }
    indx[j]=imax;
    if (a[j][j] == 0.0) a[j][j]=TINY;
    if (j != n) {
      dum=1.0/(a[j][j]);
      for (i=j+1;i<=n;i++) a[i][j] *= dum;
    }
  }
  return 1;
}

//------------------------------------------------------------------------------
void OFTrack::lubksb(float **a, int n, int *indx, float b[])
{
  int i,ii=0,ip,j;
  float sum;

  for (i=1;i<=n;i++) {
    ip=indx[i];
    sum=b[ip];
    b[ip]=b[i];
    if (ii)
      for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
    else if (sum) ii=i;
    b[i]=sum;
  }
  for (i=n;i>=1;i--) {
    sum=b[i];
    for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
    b[i]=sum/a[i][i];
  }
}

float **OFTrack::New2DArray(float outsideValue, int nLines, int nSamples)

{

  float **array = (float **) umalloc2(nLines, nSamples, sizeof(float));
  float *val = *array;
  for (int ii = 0; ii < nLines * nSamples; ii++, val++) {
    *val = outsideValue;
  }

  return array;

}

#undef TINY
#undef SQR
#undef SWAP

