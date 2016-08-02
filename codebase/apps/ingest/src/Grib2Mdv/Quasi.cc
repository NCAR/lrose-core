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
///////////////////////////////////////////////
// Quasi.cc
//////////////////////////////////////////////


#include "Quasi.hh"
using namespace std;


Quasi::Quasi()
{
}

Quasi::~Quasi() 
{
}


/*
 * to regrid data from one equally-spaced grid to a different equally-spaced
 * grid.
 */
void Quasi::linear(fl32 y[], int n, fl32 v[], int m, double c[])
{
    int j = 0;
    int k = 0;
    for (int i=0; i<m; i++) {
        v[i] = c[j]*y[k] + c[m-j-1]*y[k+1];
        j += n-1;
        int skip = (j-1)/(m-1);
        if (skip > 0) {
            k +=  skip;
            j = j - (m-1) * skip;
        }
    }
}


void Quasi::qlin(int nrows, int ix[], fl32 *idat, int ni, int nj, fl32 *odat)
{
#if	0
    // Remove this section after some more testing.  Carl Drews - April 23, 2004
    // fill the entire output with 0
    for (int i = 0; i < ni * nj; i++)
        odat[i] = 0.0;

    // just copy over the values that we have for each row
    for (int row = 0; row < nrows; row++) {
        for (int column = ix[row]; column < ix[row + 1]; column++) {
            odat[row * ni + column - ix[row]] = idat[column];
        }
    }
#else
    // allocate temporary local storage
    double *c = new double[ni];
    fl32 *row2 = new fl32[ni];

    // precompute interpolation coefficients
    for (int i=0; i < ni; i++)
        c[i] = (double) (ni - i - 1) / (ni - 1);

    fl32 *outp = odat;
    for (int j=0; j < nj; j++) {
                                /* Does jth output row correspond to one of
                                   input rows, or is it between two?  */
        int inrow = j*(nrows-1)/(nj-1);		// input row to use
        if (inrow * (nj-1) == j*(nrows-1)) { /* one row */
                                /* interpolate inrow row to ni points */
            linear(&idat[ix[inrow]],/* input row */
                   ix[inrow+1] - ix[inrow], /* number of points in row */
                   outp,        /* where to put new output row */
                   ni,          /* number of output values to compute */
                   c);          /* precomputed interpolation coefficients */
            outp += ni;
        } else {                /* between two rows */
                                /* interpolate rows inrow, inrow+1 to ni
                                   points  */
            linear(&idat[ix[inrow]],/* first input row */
                   ix[inrow+1] - ix[inrow], /* # of points in row */
                   outp,        /* where to put new output row */
                   ni,          /* number of output values to compute */
                   c);          /* precomputed interpolation coefficients */
            linear(&idat[ix[inrow+1]],/* second input row */
                   ix[inrow+2] - ix[inrow+1], /* # of points in row */
                   row2,        /* where to put new output row */
                   ni,          /* number of output values to compute */
                   c);          /* precomputed interpolation coefficients */
            for (int ii=0; ii < ni; ii++) {
                double c1 = 1.0 - (j*(nrows - 1.0)/(nj - 1.0) - inrow);
                double c2 = 1.0 - c1;
                *outp = c1 * *outp + c2*row2[ii];
                outp++;
            }
        }
    }
    delete [] row2;
    delete [] c;
#endif
}

