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
 *   $Revision: 1.7 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * VertInterp.cc: File containing classes to vertically interpolate.
 *
 * How to add a new interpolation function:
 *  1) Create a new function class declaration in VertInterp.hh.
 *  2) Create the constructor/interp and destructor class functions
 *     in this file. (Example below)
 *  3) Add the class to InterpInterface.hh function getInterpClassFromName.
 *  4) Class can now be called by name from the parameter file. Add documentation
 *     about the interpolation function in paramdef.ModelDerive 
 *     interpolate_function variable.
 *
 * Example class interpolation function:
 *
 *
LevelFromLvl1::LevelFromLvl1(const float missing, const float bad, int nx, int ny, int nz, 
			   Mdvx::grid_order_indices_t order,
			   float *inlevels, int outz, float *outlevels)
  : InterpBase(bad, missing, nx, ny, nz, order, inlevels, outz, outlevels)
{
  numberOfInputs = 1;      // Number of inputs can be 1 to N
  // List the output level name (grib2 conventions if possible)
  strncpy(levelName, "HTGL", 25);
}

LevelFromLvl1::~LevelFromLvl1() { }

// The actual interpolation funciton:
int LevelFromLvl1::interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *outputs)
{
  int returnValue;
  if(returnValue = InterpBase::interp(inputs, data, data_out))
    return returnValue;

  // Add your interpolation function here

  return 0;
}

 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#include "VertInterp.hh" 

#define FT2M 0.3048
#define FT_TO_KM 0.0003048
#define KM_TO_FT 3280.8399
#define MB2PA 100

int InterpBase::numberOfInputs = 0;
char *InterpBase::levelName = new char[25];
char *InterpBase::units = new char[25];

AglFromSigma::AglFromSigma(const float missing, const float bad, int nx, int ny, int nz, 
			   Mdvx::grid_order_indices_t order,
			   float *inlevels, int outz, float *outlevels)
  : InterpBase(bad, missing, nx, ny, nz, order, inlevels, outz, outlevels)
{
  numberOfInputs = 1;
  strncpy(levelName, "HTGL", 25);
  strncpy(units, "ft", 25);
}

AglFromSigma::~AglFromSigma() { }

int AglFromSigma::interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out)
{
  int returnValue;
  if(returnValue = InterpBase::interp(inputs, data, data_out))
    return returnValue;

  const float *zPtr = (*inputs)[0];

  for(int a = 0; a < data->size(); a++) {
    float *odata = new float[oPts];
    data_out->push_back(odata);
  }

  for(int i = 0; i < nx; i++) {
    for(int j = 0; j < ny; j++) {
      for(int outk = 0; outk < outz; outk++) {

	float target_alt = float(outlevels[outk])*FT2M + zPtr[index(i,j,0)];
	int ink = 0;
	while(ink <= nz && target_alt > zPtr[index(i,j,ink)]) {
	  ink++;
	}

	if(ink > 0 && ink < nz)
	{
	  float slope = (zPtr[index(i,j,ink)]-target_alt) / (zPtr[index(i,j,ink)]-zPtr[index(i,j,ink-1)]);

	  for(int a = 0; a < data->size(); a++) {
	    (*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink)] -
	      ((*data)[a][index(i,j,ink)] - (*data)[a][index(i,j,ink-1)]) * slope;
	  }
	} else if(ink == 0) {
	  for(int a = 0; a < data->size(); a++)
	    (*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink)];
	} else if(ink == nz) {
	  for(int a = 0; a < data->size(); a++)
	    (*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink-1)];
	}
      }
    }
  }

  return 0;
}

IsblFromSigma::IsblFromSigma(const float missing, const float bad, int nx, int ny, int nz, 
			     Mdvx::grid_order_indices_t order,
			     float *inlevels, int outz, float *outlevels)
  : InterpBase(bad, missing, nx, ny, nz, order, inlevels, outz, outlevels)
{
  numberOfInputs = 1;
  strncpy(levelName, "ISBL", 25);
  strncpy(units, "mb", 25);
}

IsblFromSigma::~IsblFromSigma() { }

int IsblFromSigma::interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out)
{
  int returnValue;
  if(returnValue = InterpBase::interp(inputs, data, data_out))
    return returnValue;

  const float *pPtr = (*inputs)[0];

  for(int a = 0; a < data->size(); a++) {
    float *odata = new float[oPts];
    data_out->push_back(odata);
  }

  for(int i = 0; i < nx; i++) {
    for(int j = 0; j < ny; j++) {
      for(int outk = 0; outk < outz; outk++) {

	float target_alt = float(outlevels[outk])*MB2PA;
	int ink = 0;
	while(ink <= nz && target_alt < pPtr[index(i,j,ink)]) {
	  ink++;
	}

	if(ink > 0 && ink < nz)
	{
	  float slope = (pPtr[index(i,j,ink)]-target_alt) / (pPtr[index(i,j,ink)]-pPtr[index(i,j,ink-1)]);

	  for(int a = 0; a < data->size(); a++) {
	    (*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink)] -
	      ((*data)[a][index(i,j,ink)] - (*data)[a][index(i,j,ink-1)]) * slope;
	  }
	} else if(ink == 0) {
	  for(int a = 0; a < data->size(); a++)
	    (*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink)];
	} else if(ink == nz) {
	  for(int a = 0; a < data->size(); a++)
	    (*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink-1)];
	}
      }
    }
  }

  return 0;
}
 
FlFromKm::FlFromKm(const float missing, const float bad, int nx, int ny, int nz, 
			   Mdvx::grid_order_indices_t order,
			   float *inlevels, int outz, float *outlevels)
  : InterpBase(bad, missing, nx, ny, nz, order, inlevels, outz, outlevels)
{
  numberOfInputs = 1;
  strncpy(levelName, "HTGL", 25);
  strncpy(units, "100ft", 25);
}

FlFromKm::~FlFromKm() { }

int FlFromKm::interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out)
{
  int returnValue;
  if(returnValue = InterpBase::interp(inputs, data, data_out))
    return returnValue;

  for(int a = 0; a < data->size(); a++) {
    float *odata = new float[oPts];
    data_out->push_back(odata);
  }

  for(int i = 0; i < nx; i++) {
    for(int j = 0; j < ny; j++) {
      for(int outk = 0; outk < outz; outk++) {

	float target_alt = float(outlevels[outk]) * 100.0 * FT_TO_KM;
	int ink = 0;
	while(ink < nz && target_alt > inlevels[ink]) {
	  ink++;
	}
	float zd = (inlevels[ink-1] - target_alt) / 
	         ( (inlevels[ink-1] - target_alt) + (target_alt - inlevels[ink]) );

	if(ink > 0 && zd >= 0.0 && zd <= 1.0)
	{
	  for(int a = 0; a < data->size(); a++) {
	    if( (*data)[a][index(i,j,ink-1)] == missing ||
		(*data)[a][index(i,j,ink)] == missing)
	      (*data_out)[a][oindex(i,j,outk)] = missing;
	    else
	      (*data_out)[a][oindex(i,j,outk)] = ( (*data)[a][index(i,j,ink-1)] * (1 - zd) ) +
		( (*data)[a][index(i,j,ink)] * zd);
	  }
	} else {
	  return -1;
	}
      }
    }
  }
  return 0;
}
 
SfcFromSigma::SfcFromSigma(const float missing, const float bad, int nx, int ny, int nz, 
			   Mdvx::grid_order_indices_t order,
			   float *inlevels, int outz, float *outlevels)
  : InterpBase(bad, missing, nx, ny, nz, order, inlevels, outz, outlevels)
{
  numberOfInputs = 1;
  strncpy(levelName, "SFC", 25);
  strncpy(units, "none", 25);
}

SfcFromSigma::~SfcFromSigma() { }

int SfcFromSigma::interp(vector<float*> *inputs, vector<float*> *data, vector<float*> *data_out)
{
  int returnValue;
  if(returnValue = InterpBase::interp(inputs, data, data_out))
    return returnValue;

  if(outz != 1)
    return -1;

  for(int a = 0; a < data->size(); a++) {
    float *odata = new float[oPts];
    data_out->push_back(odata);
  }

  int outk = 0;
  int ink = 0;

  for(int i = 0; i < nx; i++) {
    for(int j = 0; j < ny; j++) {
      
      for(int a = 0; a < data->size(); a++) {
	(*data_out)[a][oindex(i,j,outk)] = (*data)[a][index(i,j,ink)];
      }

    }
  }

  return 0;
					     
}
