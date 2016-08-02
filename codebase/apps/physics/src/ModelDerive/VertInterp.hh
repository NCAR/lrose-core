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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.4 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * VertInterp: 
 *
 * How to add a new interpolation function:
 *  1) Create a new function class declaration in this file. (example below)
 *  2) Create the constructor/interp and destructor class functions
 *     in VertInterp.cc.
 *  3) Add the class to InterpInterface.hh function getInterpClassFromName.
 *  4) Class can now be called by name from the parameter file. Add documentation
 *     about the interpolation function in paramdef.ModelDerive 
 *     interpolate_function variable.
 *
 *  Example class declaration:  Copy and paste as is changing only the class
 *                              name to your interpolation function name.
 *

class LevelFromLvl1: public InterpBase {

public:

  LevelFromLvl1(const float missing, const float bad, int nx, int ny, int nz, 
	       Mdvx::grid_order_indices_t order,
	       float *inlevels, int outz, float *outlevels);

  virtual ~LevelFromLvl1();

  int interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out);
 
};

 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef VertInterp_HH
#define VertInterp_HH

#include "InterpBase.hh"

class AglFromSigma: public InterpBase {

public:

  AglFromSigma(const float missing, const float bad, int nx, int ny, int nz, 
	       Mdvx::grid_order_indices_t order,
	       float *inlevels, int outz, float *outlevels);

  virtual ~AglFromSigma();

  int interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out);
 
};

class IsblFromSigma: public InterpBase {

public:

  IsblFromSigma(const float missing, const float bad, int nx, int ny, int nz, 
	       Mdvx::grid_order_indices_t order,
	       float *inlevels, int outz, float *outlevels);

  virtual ~IsblFromSigma();

  int interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out);
 
};

class FlFromKm: public InterpBase {

public:

  FlFromKm(const float missing, const float bad, int nx, int ny, int nz, 
	       Mdvx::grid_order_indices_t order,
	       float *inlevels, int outz, float *outlevels);

  virtual ~FlFromKm();

  int interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out);
 
};

class SfcFromSigma: public InterpBase {

public:

  SfcFromSigma(const float missing, const float bad, int nx, int ny, int nz, 
	       Mdvx::grid_order_indices_t order,
	       float *inlevels, int outz, float *outlevels);

  virtual ~SfcFromSigma();

  int interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out);
 
};


#endif
