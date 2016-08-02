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
 * InterpBase: Base abstract class for all model interpolation operations.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef InterpBase_HH
#define InterpBase_HH

using namespace std;

#include <Mdv/Mdvx.hh>

class InterpBase {

public:

  InterpBase(const float missing, const float bad, int nx, int ny, int nz,
	     Mdvx::grid_order_indices_t order,
	     float *inlevels, int outz, float *outlevels)
    : missing(missing), bad(bad)
  {
    this->nx = nx;
    this->ny = ny;
    this->nz = nz;
    this->nPts = nx * ny * nz;
    this->inlevels = inlevels;
    this->order = order;
    this->outz = outz;
    this->oPts = nx * ny * outz;
    this->outlevels = outlevels;

    this->numberOfInputs = 0;
  }
  
  virtual ~InterpBase() { }

  virtual int interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *outputs)
  {
    if(inputs->size() != this->numberOfInputs) {
      return -2;
    }
    return 0;
  }

  int oindex(int x, int y, int z)
  {
    if(order == Mdvx::ORDER_XYZ)
      return x + (y * nx) + (z * nx * ny);
    if(order == Mdvx::ORDER_YXZ)
      return y + (x * ny) + (z * ny * nx);
    if(order == Mdvx::ORDER_XZY)
      return x + (z * nx) + (y * nx * outz);
    if(order == Mdvx::ORDER_YZX)
      return y + (z * ny) + (x * ny * outz);
    if(order == Mdvx::ORDER_ZXY)
      return z + (x * outz) + (y * outz * nx);
    if(order == Mdvx::ORDER_ZYX)
      return z + (y * outz) + (x * outz * ny);
    return 0;
  }

  int index(int x, int y, int z)
  {
    if(order == Mdvx::ORDER_XYZ)
      return x + (y * nx) + (z * nx * ny);
    if(order == Mdvx::ORDER_YXZ)
      return y + (x * ny) + (z * ny * nx);
    if(order == Mdvx::ORDER_XZY)
      return x + (z * nx) + (y * nx * nz);
    if(order == Mdvx::ORDER_YZX)
      return y + (z * ny) + (x * ny * nz);
    if(order == Mdvx::ORDER_ZXY)
      return z + (x * nz) + (y * nz * nx);
    if(order == Mdvx::ORDER_ZYX)
      return z + (y * nz) + (x * nz * ny);
    return 0;
  }

  static int getNumberOfInputs() {
    return numberOfInputs;
  }

  static char *getOutputLevelName() {
    return levelName;
  }

  static char *getOutputUnits() {
    return units;
  }

protected:

  const float missing;
  const float bad;
  float *inlevels;
  int nx, ny, nz;
  int nPts;
  Mdvx::grid_order_indices_t order;
  float *outlevels;
  int outz;
  int oPts;

  static int numberOfInputs;
  static char* levelName;
  static char* units;

private:

};

#endif
