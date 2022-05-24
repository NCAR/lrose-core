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
#include <toolsa/copyright.h>
#include <toolsa/TaArray.hh>
#include <cstdlib>
#include <cstring>

/************************************************************************

Module:	Algs

Author:	Dave Albo

Date:	Mon Jan 30 13:48:39 2006

Description:   The actual filtering algorithm code for all filters

************************************************************************/



/* System include files / Local include files */
#include <cstdio>
#include <cmath>
#include "Algs.hh"
using namespace std;

/* Constant definitions / Macro definitions / Type definitions */

/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
//
// Comparison routine for fl32 in ascending order.
//
static int _Fl32Compare(const void *a, const void *b)
{
  fl32 *x = (fl32 *)a;  fl32 *y = (fl32 *)b;
  if (*x >  *y) return 1;
  if (*x <  *y) return -1;
  return 0;
}

/*----------------------------------------------------------------*/
Algs::Algs(void)
{
}

/*----------------------------------------------------------------*/
Algs::~Algs()
{
}

/*----------------------------------------------------------------*/
void Algs::filter_thresh(fl32 *data, int n, fl32 missing, fl32 bad,
			 Params::mdv_thresh_parm_t &p)
{
  filter_thresh(data, n, missing, bad, p.MinThresh, p.MaxThresh, 
		p.ReplacePassValues, p.PassValue);
}

/*----------------------------------------------------------------*/
void Algs::filter_thresh(fl32 *data, int n, fl32 missing, fl32 bad,
			 fl32 min, fl32 max, bool replace, fl32 repval)
{
  fl32 *d = data;

  for(int index = 0; index < n; index++,++d)
  {
    // If the data point is bad, leave it.
    if (*d == missing || *d == bad)
      continue;

    //
    // See if data passes threshold, if not, mark it missing.
    //
    if (*d < min || *d > max)
      *d = missing;
    else
    {
      //
      // Replace passed values with a single value, if desired.
      //
      if (replace)
	*d = repval;
    }
  }
}

/*----------------------------------------------------------------*/
void Algs::filter_smooth(fl32 *data, int nx, int ny, int nz, int n, 
			 fl32 missing, fl32 bad, Params::mdv_smooth_parm_t &p)
{
  filter_smooth(data, nx, ny, nz, n, missing, bad, p.HalfWin, 
		p.SmoothingMethod);
}

/*----------------------------------------------------------------*/
void Algs::filter_smooth(fl32 *data, int nx, int ny, int nz, int n, 
			 fl32 missing, fl32 bad, int halfwin,
			 Params::smoothing_method_t method)
  
{
  // store to local members:
  _data = data;
  _bad = bad;
  _missing = missing;
  _n = n;
  _nx = nx;
  _ny = ny;
  _nz = nz;

  // need temp space to do the smoothing
  fl32 *OutData = (fl32 *)calloc(n, sizeof(fl32));
  fl32 *Buffer = (fl32 *)calloc(n, sizeof(fl32));

  //
  // Initialize the output grid to missing values.
  //
  for(int l=0; l < n; l++)
    OutData[l]=missing;

  //
  // Loop through the vertical planes of the volume.
  //
  for(int iz=0; iz < nz; iz++)
    for(int iy=0; iy < _ny; iy++)
      for(int ix=0; ix < _nx; ix++)
	_filter_smooth_plane_xy(iz, iy, ix, halfwin, method, Buffer, OutData);

  free(Buffer);

  // copy this data back into input data.
  memcpy(data, OutData, n*sizeof(fl32));
  free(OutData);
}

/*----------------------------------------------------------------*/
bool Algs::needs_synch(fl32 *data, int n, fl32 missing, fl32 bad,
		       fl32 *data2, int n2, fl32 missing2, fl32 bad2,
		       fl32 &newbad)
{
  // we have two sets of two values, want each value to to equal
  // one value from the other set.
  if ((missing == missing2 || missing == bad2) &&
      (bad == missing2 || bad == bad2) &&
      (missing2 == missing || missing2 == bad) &&
      (bad2 == missing || bad2 == bad))
    return false;

  // that didn't happen, so need to pull out a unique value that
  // is not in either data's span.
  double min, max, mean, min2, max2, mean2;
  bool is_data, is_data2;

  is_data = evaluate(data, n, missing, bad, min, max, mean);
  is_data2 = evaluate(data2, n2, missing2, bad2, min2, max2, mean2);

  if (is_data && is_data2)
  {
    // take one greater than biggest value.
    if (max >= max2)
      newbad = max + 1;
    else
      newbad = max2 + 1;
  }
  else if (is_data && !is_data2)
    // second input all missing, use first input missing.
    newbad = missing;
  else if ((!is_data) && is_data2)
    // first input all missing, use 2nd input missing.
    newbad = missing2;
  else
    // either will do.
    newbad = missing;
  return true;
}

/*----------------------------------------------------------------*/
void Algs::filter_combine(fl32 *data, int n, fl32 missing, fl32 bad, 
			  fl32 *data2, int n2, fl32 missing2, fl32 bad2,
			  Params::mdv_combine_parm_t &p)
{
  fl32 newbad;
  if (needs_synch(data, n, missing, bad, data2, n2, missing2, bad2, newbad))
  {
    cerr << "Fata error combine data not synched" << endl;
    exit(-1);
  }
  if (n != n2)
  {
    cerr << "Fatal error combine data sizes unequal " << n << "," << n2 << endl;
    exit(-1);
  }

  // do the combining.
  fl32 *d, *d2;
  d = data;
  d2 = data2;
  for (int i=0; i<n; ++i,++d,++d2)
    _combine_1(d, bad, missing, d2, bad2, missing2, p);
}

/*----------------------------------------------------------------*/
void Algs::filter_overwrite(fl32 *data, int n, fl32 missing, fl32 bad, 
			    fl32 *overwrite_data, int n2, fl32 missing2,
			    fl32 bad2)
{
  fl32 newbad;
  if (needs_synch(data, n, missing, bad, overwrite_data, n2, missing2, bad2,
		  newbad))
  {
    cerr << "Fata error overwrite data not synched" << endl;
    exit(-1);
  }
  if (n != n2)
  {
    cerr << "Fatal error overwrite data sizes unequal " << n << "," << n2 << endl;
    exit(-1);
  }

  fl32 *d, *d2;
  d = data;
  d2 = overwrite_data;
  for (int i=0; i<n; ++i,++d,++d2)
  {
    if (*d2 != missing2 && *d2 != bad2)
      *d = *d2;
  }
}

/*----------------------------------------------------------------*/
bool Algs::evaluate(fl32 *data, int n, fl32 missing, fl32 bad,
		    double &min, double &max, double &mean)
{
  int first = 1;
  double total=0.0;
  long numgood = 0;

  min=0.0; 
  max=0.0;
  for(int k=0; k < n; k++){
    if (data[k] == missing || data[k] == bad)
      continue;
    else
    {
      numgood++;
      total = total + data[k];
      if (first){
	min = max = data[k];
	first = 0;
      } else {
	if (data[k] < min) min = data[k];
	if (data[k] > max) max = data[k];
      }
    }
  }
    
  if (numgood == 0)
  {
    mean = 0.0;
    return false;
  }
  else
  {
    mean = total / double(numgood);
    return true;
  }
}

/*----------------------------------------------------------------*/
void Algs::_filter_smooth_plane_xy(int iz, int iy, int ix, int halfwin, 
				   Params::smoothing_method_t method,
				   fl32 *Buffer, fl32 *OutData)
{
  int num;
  double min, max, mean;
  fl32 *out;

  out = &OutData[ix + iy*_nx + iz*_nx*_ny];

  //
  // Gather points to smooth, based at ix, iy and store them in 'Buffer'.
  //
  _fill_buffer(num, Buffer, halfwin, ix, iy, iz);

  if (!evaluate(Buffer, num, _missing, _bad, min, max, mean))
    *out = _missing;
  else
  {
    switch (method)
    {
    case Params::SMOOTH_MIN :
      *out = min;
      break;
    case Params::SMOOTH_MAX :
      *out = max;
      break;
    case Params::SMOOTH_MEAN :
      *out = mean;
      break;
    case Params::SMOOTH_MEDIAN :
      *out = _GetMedian(Buffer, num);
      break;
    default :
      cerr << "Unsupported smoothing option. I cannot cope." << endl;
      exit(-1);
      break;
    }
  }
}

/*----------------------------------------------------------------*/
void Algs::_fill_buffer(int &num, fl32 *Buffer, int hwin, int ix, int iy, int iz)
{
  num = 0;
  for (int ixx = -hwin; ixx <= hwin; ixx++)
  {
    for (int iyy = -hwin; iyy <= hwin; iyy++)
    {
      int ixxx = ix + ixx;
      int iyyy = iy + iyy;
      if ((ixxx > -1) && (iyyy > -1) && (iyyy < _ny) && (ixxx < _nx))
      {
	Buffer[num] = _data[ixxx + iyyy*_nx + iz*_nx*_ny];
	num ++;
      }
    }
  }
}

/*----------------------------------------------------------------*/
//
// Small routine to get the median value.
//
fl32 Algs::_GetMedian(fl32 *Buffer, int num)
{
  //
  // Pick off the non-missing data into another array.
  // Return missing as the median if no data found.
  //

  TaArray<fl32> GoodBuf_;
  fl32 *GoodBuf = GoodBuf_.alloc(num);

  int nGood = 0;
  for(int l=0; l < num; l++)
  {
    if (Buffer[l] != _missing && Buffer[l] != _bad)
    {
      GoodBuf[nGood] = Buffer[l];
      nGood++;
    }
  }

  if (nGood == 0)
    return _missing;

  //
  // Sort them into order.
  //
  qsort(GoodBuf, num, sizeof(fl32), _Fl32Compare);

  //
  // pick median
  //
  int index = (int)floor(double(nGood) / 2.0);
  fl32 ans = GoodBuf[index];
  return ans;
}

/*----------------------------------------------------------------*/
void Algs::_combine_1(fl32 *d, fl32 bad, fl32 missing, fl32 *d2,
		      fl32 bad2, fl32 missing2, 
		      Params::mdv_combine_parm_t &p)
{
  bool isbad = (*d == bad || *d == missing);
  bool isbad2 = (*d2 == bad2 || *d2 == missing2);
  if (isbad && isbad2)
    return;
  else if (isbad && !isbad2)
    *d = *d2;
  else if ((!isbad) && isbad2)
    return;
  else
  {
    switch (p.CombineMethod)
    {
    case Params::COMB_MIN:
      if (*d > *d2)  *d = *d2;
      break;
    case Params::COMB_MAX:
      if (*d < *d2) *d = *d2;
      break;
    case Params::COMB_MEAN:
      *d = ((*d) + (*d2))/2.0;
      break;
    default:
      *d = bad;
    }
  }
}


