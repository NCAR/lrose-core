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
#ifndef SVD_DATA_HH
#define SVD_DATA_HH

#include "ReorderInterp.hh"
#include <vector>

class SvdData
{
public:
  SvdData(void);
  SvdData(const std::vector<ReorderInterp::radar_point_t> &pts, double x0,
	  double y0, double z0);
  ~SvdData(void);

  void init(const std::vector<ReorderInterp::radar_point_t> &pts, double x0,
	    double y0, double z0);

  bool compute(const std::vector<double> &b);
  bool computeWithMissing(const std::vector<double> &b, double missing);

  double getTerm(int index) const;

  bool isOk(void) const {return _ok;}

  void print(void) const;

private:

  bool _ok;
  double **_a, **_u, **_v;
  double *_w, *_b, *_x;
  int _nrow;
  int _ncol;


  void _apply(double** u,	/* I - U matrix from SVD decomposition	*/ 
	    double *w, 	/* I - W diag array from SVD decomposition	*/ 
	    double **v, /* I - V matrix from SVD decomposition		*/
	    int ndata, 	/* I - number of rows in U			*/
	    int nvar,	/* I - number of cols in U, order of V, W size	*/
	    double* b, 	/* I - known values to solve for		*/
	    double *x 	/* O - values solved for			*/
	    );
  void _apply(double** u,	/* I - U matrix from SVD decomposition	*/ 
	    double *w, 	/* I - W diag array from SVD decomposition	*/ 
	    double **v, /* I - V matrix from SVD decomposition		*/
	    int ndata, 	/* I - number of rows in U			*/
	    int nvar,	/* I - number of cols in U, order of V, W size	*/
	    double* b, 	/* I - known values to solve for		*/
	      double missing,
	    double *x 	/* O - values solved for			*/
	    );
  void _free(void);
  void _alloc(const std::vector<ReorderInterp::radar_point_t> &pts,
	      double x0, double y0, double z0);
};

#endif
