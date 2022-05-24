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
/************************************************************************

Header: Algs.hh

Author: Dave Albo

Date:   Mon Jan 30 13:48:39 2006

Description: The actual filtering algorithm code for all filters, contains
             no data, only does processing.

*************************************************************************/

# ifndef    Algs_HH
# define    Algs_HH

/* System include files / Local include files */
#include "Params.hh"
#include <dataport/port_types.h>
using namespace std;

/* Class definition */
class Algs
{
public:

  Algs();
  virtual ~Algs();

  // filter data to within a range of thresholds.
  void filter_thresh(fl32 *data, int n, fl32 missing, fl32 bad,
		     Params::mdv_thresh_parm_t &p);
  void filter_thresh(fl32 *data, int n, fl32 missing, fl32 bad,
		     fl32 min, fl32 max, bool replace, fl32 repval);

  // filter data by smoothing it.
  void filter_smooth(fl32 *data, int nx, int ny, int nz, int n, 
		     fl32 missing, fl32 bad, Params::mdv_smooth_parm_t &p);
  void filter_smooth(fl32 *data, int nx, int ny, int nz, int n, 
		     fl32 missing, fl32 bad, int halfwin,
		     Params::smoothing_method_t method);
  
  //
  // on exit:
  //  true: newbad is a bad value not present in either data input, and sych.
  //        is needed.
  //  false:current bad/missing values are ok as is.
  bool needs_synch(fl32 *data, int n, fl32 missing, fl32 bad,
		   fl32 *data2, int n2, fl32 missing2, fl32 bad2,
			fl32 &newbad);

  // on entry data and data are 'synchronized'.
  void filter_combine(fl32 *data, int n, fl32 missing, fl32 bad, 
		      fl32 *data2, int n2, fl32 missing2, fl32 bad2,
		      Params::mdv_combine_parm_t &p);

  // overwrite data with overwrite data where not missing or bad.
  void filter_overwrite(fl32 *data, int n, fl32 missing, fl32 bad, 
			fl32 *overwrite_data, int n2, fl32 missing2, fl32 bad2);

  // return true if data has non-bad/missing values. Computes min/max/mean.
  bool evaluate(fl32 *data, int n, fl32 missing, fl32 bad,
		double &min, double &max, double &mean);

protected:
private:  

  ////////////////////////////////////////////////////////////////
  /////////////////////// private members ////////////////////////
  ////////////////////////////////////////////////////////////////

  // copies of inputs for a few algs.
  fl32 *_data, _bad, _missing;
  int _n, _nx, _ny, _nz;
  
  ////////////////////////////////////////////////////////////////
  /////////////////////// private methods ////////////////////////
  ////////////////////////////////////////////////////////////////

  void _filter_smooth_plane_xy(int iz, int iy, int ix, 
			       int halfwin, Params::smoothing_method_t method,
			       fl32 *Buffer, fl32 *OutData);
  void _fill_buffer(int &num, fl32 *Buffer, int hwin,  int ix, int iy, int iz);
  fl32 _GetMedian(fl32 *Buffer, int num);
  void _combine_1(fl32 *d, fl32 bad, fl32 missing, fl32 *d2,
		  fl32 bad2, fl32 missing2, 
		  Params::mdv_combine_parm_t &p);
};


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

# endif     /* Algs_HH */
