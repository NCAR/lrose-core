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
/*
 *  $Id: _PhysicsLibImplementation.hh,v 1.15 2016/03/03 19:23:21 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	PhysicsLib
//
// Author:	G M Cunning
//
// Date:	Mon Jun 19 12:24:29 2000
//
// Description: This libarary implements empiral relationship of 
//		various physical parameters
//
//
//
//
//
//


// C++ include files
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>

// Local include files
#include <physics/physics.h>
#include <physics/PhysicsLib.hh>
#include <physics/physics_macros.h>


// namespaces

using namespace std;

// define any constants


// declare static functions

namespace PhysicsLib {


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	vorticity
  //
  // Description:	This function calculates the relative vorticity, 
  //			dv/dx - du/dy.
  //
  // Returns:		true is successful and false for failure. The
  //			vorticity is return through the pointer vort. 
  //			the calling function is responsible for 
  //			memory managment.
  //
  // Globals:	
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //

  template<class T>bool vorticity(const T *u, const T *v, T *vort,
				  const T miss, const T bad, 
				  const double dx, const double dy,
				  const int nx, const int ny, const int nz,
				  const int span)
    {

      if (ny == 0)
	return false;

      int nPts = nx * ny * max(1,nz);

      T *duDy = new T[nPts];
      bool status =
	gradDuDy(u, duDy, miss, bad, dy, nx, ny, nz, span, true);
      if(!status)
      {
	delete [] duDy;
	return false;
      }
      
      T *dvDx = new T[nPts];
      status =
	gradDuDx(v, dvDx, miss, bad, dx, nx, ny, nz, span, true);
      if(!status)
      {
	delete [] duDy;
	delete [] dvDx;
	return false;
      }
      
      T *vortPtr = vort;
      T *vPtr = dvDx;
      T *uPtr = duDy;

      for(int i = 0; i < nPts; i++, vortPtr++, vPtr++, uPtr++)
      {
	if((*uPtr == miss) || (*uPtr == bad) ||
	   (*vPtr == miss) || (*vPtr == bad))
	  *vortPtr = miss;
	else
	  *vortPtr = *vPtr - *uPtr;
      }

      delete [] duDy;
      delete [] dvDx;

      return true;
    }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	absoluteVorticity
  //
  // Description:	This function calculates the relative vorticity, 
  //			dv/dx - du/dy + Fc, where Fc is the Coreolis
  //			force.
  //
  // Returns:		true is successful and false for failure. The
  //			vorticity is return through the pointer vort. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  //

  template<class T> bool absoluteVorticity(const T *u, const T *v, T *vort,
					   const T miss, const T bad, 
					   const double dx, const double dy,
					   const int nx, 
					   const int ny, const int nz)
    {

      if(!ny)
	return false;

      // calculate relative vorticity
      bool status = vorticity(u, v, vort, miss, bad, dx, dy, nx, ny, nz);
      if(!status)
	return false;

      int nPts = nx*ny*max(1,nz);
      T *vortPtr = vort;
      for (int i = 0; i < nPts; ++i, ++vortPtr)
      {
	if(*vortPtr != miss)
	  *vortPtr += COREOLIS_FORCE;
      }

      return true;
    }





  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name: 	convergence
  //
  // Description:	This function calculates the convergence of a 2-D 
  //			vector field, which is describe by a pair of 
  //			orthogonal components. The definition of convergence
  //			is du/dx + dv/dy
  //
  // Returns:		true is successful and false for failure. The
  //			vorticity is return through the pointer cvrg. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //			The calling function is responsible for 
  //			memory managment.

  template<class T> bool convergence(const T *u, const T *v, T *cvrg,
				     const T miss, const T bad, 
				     const double dx, const double dy,
				     const int nx, const int ny, const int nz,
				     const int span)
    {

      if (ny == 0)
	return false;

      int locNz = max(1,nz);
      int nPts = nx*ny*locNz;

      T *duDx = new T[nPts];
      bool status =
	gradDuDx(u, duDx, miss, bad, dx, nx, ny, nz, span, true);
      if (!status)
      {
	delete [] duDx;
	return false;
      }
      
      T *dvDy = new T[nPts];
      status =
	gradDuDy(v, dvDy, miss, bad, dy, nx, ny, nz, span, true);
      if (!status)
      {
	delete [] duDx;
	delete [] dvDy;
	return false;
      }
      
      T *cvrgPtr = cvrg;
      T *vPtr = dvDy;
      T *uPtr = duDx;

      for (int i = 0; i < nPts; ++i, ++cvrgPtr, ++vPtr, ++uPtr)
      {
	if((*uPtr == miss) || (*vPtr == miss))
	  *cvrgPtr = miss;
	else
	  *cvrgPtr = *uPtr + *vPtr;
      }

      delete [] duDx;
      delete [] dvDy;

      return true;
    }



  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name: 	moistureConvergence
  //
  // Description:	This function calculates the moisture convergence 
  //			of a 2-D velocity field and specific humidity, which 
  //			is describe by a pair of orthogonal components. The 
  //			definition of convergence is 
  //				u*dq/dx + v*dq/dy q*(du/dx +dv/dy)
  //
  // Returns:		true is successful and false for failure. The
  //			moisture convergence in g/kg/sec is return through
  //                    the pointer cvrg. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.

  template<class T> bool moistureConvergence(const T *u, const T *v,
					     const T *q, T *mc,
					     const T miss, const T bad,
					     const double dx, const double dy, 
					     const int nx, const int ny,
					     const int nz,
					     const int span)
    {

      if (ny == 0)
	return false;

      int locNz = max(1,nz);
      int nPts = nx*ny*locNz;

      T *dqDx = new T[nPts];
      bool status =
	gradDuDx(q, dqDx, miss, bad, dx, nx, ny, nz, span, true);
      if (!status)
      {
	delete [] dqDx;
	return false;
      }
      
      T *dqDy = new T[nPts];
      status =
	gradDuDy(q, dqDy, miss, bad, dy, nx, ny, nz, span, true);
      if (!status)
      {
	delete [] dqDx;
	delete [] dqDy;
	return false;
      }
      
      T *cvrg = new T[nPts];
      status = convergence(u, v, cvrg, miss, bad, dx, dy, nx, ny, nz, span);
      if (!status)
      {
	delete [] dqDx;
	delete [] dqDy;
	delete [] cvrg;
	return false;
      }
      
      T *mcPtr = mc;
      T *cvrgPtr = cvrg;
      T const *vPtr = v;
      T const *uPtr = u;
      T const *qPtr = q;
      T const *qxPtr = dqDx;
      T const *qyPtr = dqDy;

      for (int i = 0; i < nPts; 
	   ++i, ++mcPtr, ++cvrgPtr, ++vPtr, ++uPtr, ++qPtr, ++qxPtr, ++qyPtr)
      {
	if ((*uPtr == miss) || (*qxPtr == miss) || 
	    (*vPtr == miss) || (*qyPtr == miss) ||
	    (*qPtr == miss) || (*cvrgPtr == miss))
	  *mcPtr = miss;
	else
	  *mcPtr = (*uPtr)*(*qxPtr) + (*vPtr)*(*qyPtr) + (*qPtr)*(*cvrgPtr);
      }

      delete [] dqDx;
      delete [] dqDy;
      delete [] cvrg;

      return true;
    }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradient
  //
  // Description:	This function determines the gradient of given 
  //			scalar field.
  //
  // Returns:		true is successful and false for failure. The
  //			gradient is return through the pointer grad. 
  //
  // Notes:		If dz is set to 0.0, calculates the gradients
  //                    using just the x/y directions.
  //
  //                    The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool gradient(const T *u, T *grad,
				  const T miss, const T bad,
				  double dx, int nx,
				  double dy, int ny, 
				  double dz, int nz,
				  const int span)
  {
    const string method_name = "PhysicsLib::gradient()";
    
    int locNy = max(ny, 1);
    int locNz = max(nz, 1);
    int nPts = nx * locNy * locNz;

    if (!gradDuDx(u, grad, miss, bad, dx, nx, ny, nz, span, true))
    {
      cerr << "Error calculating du/dx in " << method_name << endl;
      
      return false;
    }
    
    if (ny > 1)
    {
      T *duDy = new T[nPts];

      if (!gradDuDy(u, duDy, miss, bad, dy, nx, ny, nz, span, true))
      {
	cerr << "Error calculating du/dy in " << method_name << endl;
	
	delete [] duDy;
	return false;
      }
	
      T *dyPtr = duDy;
      T *gradPtr = grad;
	
      for(int i=0; i<nPts; i++,gradPtr++,dyPtr++)
      {
	if (*dyPtr == miss)
	  *gradPtr = miss;
	else if (*gradPtr != miss)
	  *gradPtr += *dyPtr;
      } /* endfor - i */

      delete [] duDy;
    }

    if (dz != 0.0 && nz > 1)
    {
      T *duDz = new T[nPts];

      if (!gradDuDz(u, duDz, miss, bad, dz, nx, ny, nz, span, false))
      {
	cerr << "Error calculating du/dz in " << method_name << endl;
	
	delete [] duDz;
	return false;
      }
	
      T *dzPtr = duDz;
      T *gradPtr = grad;

      for(int i=0; i<nPts; i++,gradPtr++,dzPtr++)
      {
	if (*dzPtr == miss)
	  *gradPtr = miss;
	else if (*gradPtr != miss)
	  *gradPtr += *dzPtr;
      } /* endfor - i */

      delete [] duDz;
    }

    return true;
  }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	maxGradient
  //
  // Description:	This function determines the maximum gradient in 
  //			one direction given some scalar field.
  //
  // Returns:		true is successful and false for failure. The
  //			vorticity is return through the pointer grad. 
  //
  // Notes:		If dz is set to 0.0, calculates the gradients
  //                    using just the x/y directions.
  //
  //                    The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool maxGradient(const T *u, T *grad,
				     const T miss, const T bad,
				     const double dx, const int nx,
				     const double dy, const int ny, 
				     const double dz, const int nz,
				     const int span)
    {
      // Calculate the number of points in the grid

      int locNy = max(ny,1);
      int locNz = max(nz,1);
      int nPts = nx * locNy * locNz;
 
      // Calculate the positive gradient in the X direction.

      bool status =
	gradDuDx(u, grad, miss, bad, dx, nx, ny, nz, span, false);
      if(!status)
	return false;

      T *gradPtr = grad;
      
      for (int i = 0; i < nPts; ++i, ++gradPtr)
      {
	if (*gradPtr != miss && *gradPtr != bad)
	  if (*gradPtr < 0)
	    *gradPtr = -(*gradPtr);
      } /* endfor - i */
      
      // Calculate the positive gradient in the Y direction, keep the
      // maximum gradient value.

      if (ny > 0)
      {
	T *duDy = new T[nPts];
	status =
	  gradDuDy(u, duDy, miss, bad, dy, nx, ny, nz, span, false);
	if(!status)
	{
	  delete [] duDy;
	  return false;
	}
	
	T *dyPtr = duDy;
	T *gradPtr = grad;
	
	for (int i = 0; i < nPts; ++i, ++gradPtr, ++dyPtr)
	{
	  if (*dyPtr != miss)
	  {
	    T fabs_dy = fabs(*dyPtr);
	    
	    if (*gradPtr == miss && *gradPtr != bad)
	      *gradPtr = fabs_dy;
	    else
	      *gradPtr = max(fabs_dy, *gradPtr);

	  }
	}
	delete [] duDy;
      }

      // Calculate the positive gradient in the Z direction, keep the
      // maximum gradient value.

      if (dz != 0.0 && nz > 0)
      {
	T *duDz = new T[nPts];
	status =
	  gradDuDz(u, duDz, miss, bad, dz, nx, ny, nz, span, false);
	if(!status)
	{
	  delete [] duDz;
	  return false;
	}
	
	T *dzPtr = duDz;
	T *gradPtr = grad;
	
	for (int i = 0; i < nPts; ++i, ++gradPtr, ++dzPtr)
	{
	  if (*dzPtr != miss && *dzPtr != bad)
	  {
	    T fabs_dz = fabs(*dzPtr);
	    
	    if (*gradPtr == miss || *gradPtr == bad)
	      *gradPtr = fabs_dz;
	    else
	      *gradPtr = max(fabs_dz, *gradPtr);
	  }
	  
	}

	delete [] duDz;
      }

      return true;
    }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcMixingRatio1
  //
  // Description:	This function calculates the mixing ratio from 
  //                    inputs with the following units:
  //			      relative humidity in %
  //                          temperature in degrees C 
  //			      pressure in mb.
  //
  // Returns:		true is successful and false for failure. The
  //                    mixing ratio in g/kg is returned in mixr.
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcMixingRatio1(const T *rh, const T *tmp_c,
					  const T *pres, T *mixr,
					  const T miss, const T bad,
					  const int nx, const int ny,
					  const int nz)
    {
      int locNy = max(ny,1);
      int locNz = max(nz,1);
      int nPts = nx * locNy * locNz;

      T const *rhPtr = rh;
      T const *tmpPtr = tmp_c;
      T const *presPtr = pres;
      T *mixrPtr = mixr;

      for(int i = 0; i < nPts; i++, tmpPtr++, presPtr++, rhPtr++, mixrPtr++)
      {
	if((*tmpPtr == miss) || (*tmpPtr == bad) ||
	   (*rhPtr == miss) || (*rhPtr == bad) ||
	   (*presPtr == miss) || (*presPtr == bad))
	{
	  *mixrPtr = miss;
	}
	else
	{
	  T tmpK = celsius2Kelvin(*tmpPtr);
	  T p_pasc = mb2Pascals(*presPtr);
	  T es_td = (0.01 * (*rhPtr)) * PHYe_sub_s(tmpK);
	  *mixrPtr = (0.622 * es_td / (p_pasc - es_td)) * 1000.0;
	}
      }
  
      return true;

    }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcMixingRatio2
  //
  // Description:	This function calculates the mixing ratio from 
  //                    inputs with the following units:
  //			      relative humidity in %
  //                          temperature in degrees K
  //			      pressure in mb.
  //
  // Returns:		true is successful and false for failure. The
  //                    mixing ratio in g/kg is returned in mixr.
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcMixingRatio2(const T *rh, const T *tmp_c,
					  const T *pres, T *mixr,
					  const T miss, const T bad,
					  const int nx, const int ny,
					  const int nz)
    {
      int locNy = max(ny,1);
      int locNz = max(nz,1);
      int nPts = nx * locNy * locNz;

      T const *rhPtr = rh;
      T const *tmpPtr = tmp_c;
      T const *presPtr = pres;
      T *mixrPtr = mixr;

      for(int i = 0; i < nPts; i++, tmpPtr++, presPtr++, rhPtr++, mixrPtr++)
      {
	if((*tmpPtr == miss) || (*tmpPtr == bad) ||
	   (*rhPtr == miss) || (*rhPtr == bad) ||
	   (*presPtr == miss) || (*presPtr == bad))
	{
	  *mixrPtr = miss;
	}
	else
	{
	  T tmpK = *tmpPtr;
	  T p_pasc = mb2Pascals(*presPtr);
	  T es_td = (0.01 * (*rhPtr)) * PHYe_sub_s(tmpK);
	  *mixrPtr = (0.622 * es_td / (p_pasc - es_td)) * 1000.0;
	}
      }
  
      return true;

    }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcRelativeHumidity
  //
  // Description:	This function calculates the relative humidity in %
  //                    from the temperature and dewpoint temperature in
  //                    degrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    relative humidity in % (0-100) is returned in rh.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcRelativeHumidity(const T *temp, const T *dewpt,
					      T *rh,
					      const T miss, const T bad,
					      const int nx, const int ny,
					      const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *tempPtr = temp;
    T const *dewptPtr = dewpt;
    T *rhPtr = rh;

    for(int i = 0; i < nPts; i++, tempPtr++, dewptPtr++, rhPtr++)
    {
      if((*tempPtr == miss) || (*tempPtr == bad) ||
	 (*dewptPtr == miss) || (*dewptPtr == bad))
      {
	*rhPtr = miss;
      }
      else
      {
	double temperature = *tempPtr;
	double dewpt = *dewptPtr;
	
	*rhPtr = (T)PHYrelh(temperature, dewpt);
      }
      
    }
  
    return true;
  }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcRelativeHumidity
  //
  // Description:	This function calculates the relative humidity in %
  //                    from the mixing ratio in g/kg, the pressure in mb
  //                    and the temperature in degrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    relative humidity in % (0-100) is returned in rh.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcRelativeHumidity(const T *mix_ratio,
					      const T *pressure,
					      const T *vpt, T *rh,
					      const T miss, const T bad,
					      const int nx, const int ny,
					      const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *mix_ratio_ptr = mix_ratio;
    T const *pressure_ptr = pressure;
    T const *vpt_ptr = vpt;
    T *rh_ptr = rh;

    for(int i = 0; i < nPts;
	i++, mix_ratio_ptr++, pressure_ptr++, vpt_ptr++, rh_ptr++)
    {
      if((*mix_ratio_ptr == miss) || (*mix_ratio_ptr == bad) ||
	 (*pressure_ptr == miss) || (*pressure_ptr == bad) ||
	 (*vpt_ptr == miss) || (*vpt_ptr == bad))
	*rh_ptr = miss;
      else {
	T theta = celsius2Kelvin(*vpt_ptr) /
	  (1.0 + 0.61 * (*mix_ratio_ptr) / 1000.0);
	theta /= pow( ( 1000.0 / (*pressure_ptr) ), 0.286 );
	T saturationMR = ( 3.8 / (*pressure_ptr) ) *
		    exp(17.27*kelvin2Celsius(theta)/(theta - 35.86));
	*rh_ptr = 100.0 * (*mix_ratio_ptr) / 1000.0 / saturationMR;
      }
    }
  
    return true;
  }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcDewPoint
  //
  // Description:	This function calculates the dew point temperature 
  //			in degrees C from the mixing ratio in g/kg,
  //			the virtual potential temperature in degrees C
  //			and pressure in millibars.
  //
  // Returns:		true is successful and false for failure. The
  //                    dew point temperature in degrees C is returned in 
  //			dewpt.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcDewPoint(const T *mix_ratio, const T *vpt,
				      const T *pressure, T *dewpt,
				      const T miss, const T bad,
				      const int nx, const int ny,
				      const int nz) 
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *mix_ratio_ptr = mix_ratio;
    T const *vpt_ptr = vpt;
    T const *pressure_ptr = pressure;
    T *dewpt_ptr = dewpt;

    for(int i = 0; i < nPts;
	i++, mix_ratio_ptr++, vpt_ptr++, pressure_ptr++, dewpt_ptr++)
    {
      if((*mix_ratio_ptr == miss) || (*mix_ratio_ptr == bad) ||
	 (*vpt_ptr == miss) || (*vpt_ptr == bad) ||
	 (*pressure_ptr == miss) || (*pressure_ptr == bad))
	*dewpt_ptr = miss;
      else {
	T theta = celsius2Kelvin(*vpt_ptr) /
	  (1.0 + 0.61 * (*mix_ratio_ptr) / 1000.0);
	theta /= pow( ( 1000.0 / (*pressure_ptr) ), 0.286 );
	T saturationMR = ( 3.8 / (*pressure_ptr) ) *
		    exp(17.27*kelvin2Celsius(theta)/(theta - 35.86));
	T rh = (*mix_ratio_ptr) / 1000.0 / saturationMR;

	T fabs_rh = fabs(rh);
	
	if(fabs_rh > 10e-07) {
	  *dewpt_ptr = 35.0 * log10(rh) + kelvin2Celsius(theta);
	}
	else {
	  *dewpt_ptr = 0.0;
	}
      }
    }
  
    return true;
  }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcDewPoint
  //
  // Description:	This function calculates the dew point temperature 
  //			in degrees C from the relative humidity in % and 
  //			the temperature in degrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    dew point temperature in degrees C is returned in 
  //			dewpt.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcDewPoint(const T *rh, const T *temp, T *dewpt,
				      const T miss, const T bad,
				      const int nx, const int ny,
				      const int nz) 
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *rh_ptr = rh;
    T const *temp_ptr = temp;
    T *dewpt_ptr = dewpt;

    for(int i = 0; i < nPts;
	i++, rh_ptr++, temp_ptr++, dewpt_ptr++)
    {
      if((*rh_ptr == miss) || (*rh_ptr == bad) ||
	 (*temp_ptr == miss) || (*temp_ptr == bad))
      {
	*dewpt_ptr = miss;
      }
      else
      {
	double temperature = *temp_ptr;
	double rh = *rh_ptr;
	
	*dewpt_ptr = (T)PHYrhdp(temperature, rh);
      }
      
    }
  
    return true;
  }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcDewPoint2
  //
  // Description:	This function calculates the dew point temperature 
  //			in degrees C from the relative humidity in % and 
  //			the temperature in degrees Kelvin.
  //
  // Returns:		true is successful and false for failure. The
  //                    dew point temperature in degrees C is returned in 
  //			dewpt.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcDewPoint2(const T *rh, const T *temp, T *dewpt,
				       const T miss, const T bad,
				       const int nx, const int ny,
				       const int nz) 
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *rh_ptr = rh;
    T const *temp_ptr = temp;
    T *dewpt_ptr = dewpt;

    for(int i = 0; i < nPts;
	i++, rh_ptr++, temp_ptr++, dewpt_ptr++)
    {
      if((*rh_ptr == miss) || (*rh_ptr == bad) ||
	 (*temp_ptr == miss) || (*temp_ptr == bad))
      {
	*dewpt_ptr = miss;
      }
      else
      {
	double temperature = kelvin2Celsius(*temp_ptr);
	double rh = *rh_ptr;
	
	*dewpt_ptr = (T)PHYrhdp(temperature, rh);
      }
      
    }
  
    return true;
  }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcTemperature
  //
  // Description:	This function calculates the temperature in 
  //			degrees C from the the virtual potential
  //			temperature in degrees C, mixing ratio in g/kg and
  //			the pressure in millibars.
  //
  // Returns:		true is successful and false for failure. The
  //                    temperature in degrees C is returned in temp.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcTemperature(const T *vpt, const T *mix_ratio, 
					 const T *pressure, T *temp,
					 const T miss, const T bad,
					 const int nx, const int ny,
					 const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *vpt_ptr = vpt;
    T const *mix_ratio_ptr = mix_ratio;
    T const *pressure_ptr = pressure;
    T *temp_ptr = temp;

    for(int i = 0; i < nPts;
	i++, vpt_ptr++, mix_ratio_ptr++, pressure_ptr++, temp_ptr++)
    {
      if((*vpt_ptr == miss) || (*vpt_ptr == bad) ||
	 (*mix_ratio_ptr == miss) || (*mix_ratio_ptr == bad) ||
	 (*pressure_ptr == miss) || (*pressure_ptr == bad))
	*temp_ptr = miss;
      else {
	T theta = celsius2Kelvin(*vpt_ptr) / (1.0 + 0.61 * (*mix_ratio_ptr) / 1000.0);
	*temp_ptr = theta / pow( ( 1000.0 / (*pressure_ptr) ), 0.286 );
	*temp_ptr = kelvin2Celsius(*temp_ptr);
      }
    }
  
    return true;
  }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcVerticalWind
  //
  // Description:	This function calculates the vertical wind component
  //			in m/s from the vertical velocity in mb/s, virtual
  //			potential temperature in degrees C, pressure
  //			in millibars, and mixing ratio in g/kg.
  //
  // Returns:		true is successful and false for failure. The
  //                    vertical wind component in m/s is returned in 
  //			w_wind.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcVerticalWind(const T *v_vel, const T *vpt,
					  const T *pressure, 
					  const T *mix_ratio, T *w_wind,
					  const T miss, const T bad,
					  const int nx, const int ny,
					  const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *v_vel_ptr = v_vel;
    T const *vpt_ptr = vpt;
    T const *pressure_ptr = pressure;
    T const *mix_ratio_ptr = mix_ratio;
    T *w_wind_ptr = w_wind;

    for(int i = 0; i < nPts;
	i++, v_vel_ptr++, vpt_ptr++, pressure_ptr++,
	  mix_ratio_ptr++, w_wind_ptr++)
    {
      if((*v_vel_ptr == miss) || (*v_vel_ptr == bad) ||
	 (*vpt_ptr == miss) || (*vpt_ptr == bad) ||
	 (*pressure_ptr == miss) || (*pressure_ptr == bad) ||
	 (*mix_ratio_ptr == miss) || (*mix_ratio_ptr == bad))
	*w_wind_ptr = miss;
      else {
	T theta = celsius2Kelvin(*vpt_ptr) /
	  (1.0 + 0.61 * (*mix_ratio_ptr) / 1000.0);
	theta /= pow( ( 1000.0 / mb2Pascals(*pressure_ptr) ), 0.286 );
	*w_wind_ptr = -( (*v_vel_ptr) * GAS_CONSTANT * theta  ) /
	  mb2Pascals(*pressure_ptr);
      }
    }
  
    return true;
  }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcVerticalWind
  //
  // Description:	This function calculates the vertical wind component
  //			in m/s from the vertical velocity in mb/s, pressure
  //			in millibars and the temperature in degrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    vertical wind component in m/s is returned in 
  //			w_wind.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcVerticalWind(const T *v_vel, const T *temp,
					  const T *pressure, T *w_wind,
					  const T miss, const T bad,
					  const int nx, const int ny,
					  const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *v_vel_ptr = v_vel;
    T const *temp_ptr = temp;
    T const *pressure_ptr = pressure;
    T *w_wind_ptr = w_wind;

    for(int i = 0; i < nPts;
	i++, v_vel_ptr++, temp_ptr++, pressure_ptr++, w_wind_ptr++)
    {
      if((*v_vel_ptr == miss) || (*v_vel_ptr == bad) ||
	 (*temp_ptr == miss) || (*temp_ptr == bad) ||
	 (*pressure_ptr == miss) || (*pressure_ptr == bad))
	*w_wind_ptr = miss;
      else {
	*w_wind_ptr = -( (*v_vel_ptr) * GAS_CONSTANT * celsius2Kelvin(*temp_ptr)  ) / 
	  mb2Pascals(*pressure_ptr);
      }
    }
  
    return true;
  }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcVerticalWind
  //
  // Description:	This function calculates the vertical wind component
  //			in m/s from the vertical velocity in mb/s, pressure
  //			in millibars and the temperature in degrees C or K.
  //                    T_units are either DEG_K or DEG_C. If not DEG_K,
  //                    units are assumed to be in DEG_C.
  //
  // Returns:		true is successful and false for failure. The
  //                    vertical wind component in m/s is returned in 
  //			w_wind.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcVerticalWind(const T *v_vel, const T *temp,
					  const T *pressure, T *w_wind,
					  const T miss, const T bad,
					  const int nx, const int ny,
					  const int nz,
					  const string T_units)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *v_vel_ptr = v_vel;
    T const *temp_ptr = temp;
    T const *pressure_ptr = pressure;
    T *w_wind_ptr = w_wind;

    for(int i = 0; i < nPts;
	i++, v_vel_ptr++, temp_ptr++, pressure_ptr++, w_wind_ptr++)
    {
      if((*v_vel_ptr == miss) || (*v_vel_ptr == bad) ||
	 (*temp_ptr == miss) || (*temp_ptr == bad) ||
	 (*pressure_ptr == miss) || (*pressure_ptr == bad))
	*w_wind_ptr = miss;
      else {
	if( T_units == "DEG_K" )
        {
	  *w_wind_ptr = -( (*v_vel_ptr) * GAS_CONSTANT * *temp_ptr  ) / 
	    mb2Pascals(*pressure_ptr);
	}
	else // DEG_C
	{
	  *w_wind_ptr = -( (*v_vel_ptr) * GAS_CONSTANT * celsius2Kelvin(*temp_ptr)  ) / 
	    mb2Pascals(*pressure_ptr);
	}
	
      }
    }
  
    return true;
  }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcEPOT
  //
  // Description:	This function calculates the equivalent potential
  //                    temperature in degrees K from the pressure in mb
  //                    and the temperature and dewpoint temperature in
  //                    degrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    EPOT in C is returned in epot.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcEPOT(const T *pressure, const T *temp,
				  const T *dewpt, T *epot,
				  const T miss, const T bad,
				  const int nx, const int ny, const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *pressurePtr = pressure;
    T const *tempPtr = temp;
    T const *dewptPtr = dewpt;
    T *epotPtr = epot;

    for(int i = 0; i < nPts;
	i++, pressurePtr++, tempPtr++, dewptPtr++, epotPtr++)
    {
      if((*pressurePtr == miss) || (*pressurePtr == bad) ||
	 (*tempPtr == miss) || (*tempPtr == bad) ||
	 (*dewptPtr == miss) || (*dewptPtr == bad))
      {
	*epotPtr = miss;
      }
      else
      {
	double pressure = *pressurePtr;
	double temperature = *tempPtr;
	double dewpt = *dewptPtr;
	
	*epotPtr = (T)PHYthte(pressure, temperature, dewpt);
      }
      
    }
  
    return true;
  }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcEPOT2
  //
  // Description:	This function calculates the equivalent potential
  //                    temperature in degrees K from the pressure in mb
  //                    and the temperature in degrees K and dewpoint
  //                    temperature in degrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    EPOT in C is returned in epot.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcEPOT2(const T *pressure, const T *temp,
				   const T *dewpt, T *epot,
				   const T miss, const T bad,
				   const int nx, const int ny, const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *pressurePtr = pressure;
    T const *tempPtr = temp;
    T const *dewptPtr = dewpt;
    T *epotPtr = epot;

    for(int i = 0; i < nPts;
	i++, pressurePtr++, tempPtr++, dewptPtr++, epotPtr++)
    {
      if((*pressurePtr == miss) || (*pressurePtr == bad) ||
	 (*tempPtr == miss) || (*tempPtr == bad) ||
	 (*dewptPtr == miss) || (*dewptPtr == bad))
      {
	*epotPtr = miss;
      }
      else
      {
	double pressure = *pressurePtr;
	double temperature = kelvin2Celsius(*tempPtr);
	double dewpt = *dewptPtr;
	
	*epotPtr = (T)PHYthte(pressure, temperature, dewpt);
      }
      
    }
  
    return true;
  }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcPotentialTemp
  //
  // Description:	This function calculates the potential temperature
  //                    in degrees C from the pressure in mb and the 
  //                    temperature indegrees C.
  //
  // Returns:		true is successful and false for failure. The
  //                    potential temperature in C is returned in ptemp.
  //
  // Notes:		The data ordering in each of the grids is the same.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //

  template<class T> bool calcPotentialTemp(const T *pressure, const T *temp,
					   T *ptemp, const T miss, const T bad,
					   const int nx, const int ny, const int nz)
  {
    int locNy = max(ny,1);
    int locNz = max(nz,1);
    int nPts = nx * locNy * locNz;

    T const *pressurePtr = pressure;
    T const *tempPtr = temp;
    T *ptempPtr = ptemp;

    for(int i = 0; i < nPts;
	i++, pressurePtr++, tempPtr++, ptempPtr++)
    {
      if((*pressurePtr == miss) || (*pressurePtr == bad) ||
	 (*tempPtr == miss) || (*tempPtr == bad))
      {
	*ptempPtr = miss;
      }
      else
      {
	double pressure = *pressurePtr;
	double temperature = *tempPtr;
	
	*ptempPtr = (T)potentialTemperature(temperature, pressure);
      }
      
    }
  
    return true;
  }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcSatVaporPressure
  //
  // Description:	This function calculates the saturated vapor pressure
  //			from termperature in degrees C. The pressure units are
  //			Pa.
  //
  // Returns:		true is successful and false for failure. The
  //			vapor pressure in Pais returned through the pointer 
  //			svpres. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  //			This function is a wrapper around e_sub_s.
  //

  template<class T> bool calcSatVaporPressure(const T *tmp_c, T *svpres,
					      const T miss, const T bad,
					      const int nx, const int ny,
					      const int nz)
    {
      int locNy = max(ny,1);
      int locNz = max(nz,1);
      int nPts = nx*locNy*locNz;

      const T *tmpPtr = tmp_c;
      T *presPtr = svpres;

      for (int i = 0; i < nPts; i++, tmpPtr++, presPtr++)
      {
	if((*tmpPtr == miss) || (*tmpPtr == bad))
	{
	  *presPtr = miss;
	}
	else
	{
	  T tmpK = celsius2Kelvin(*tmpPtr);
	  *presPtr = (T)PHYe_sub_s(tmpK);
	}
      }
  
      return true;
    }



  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	calcSpecificHumidity
  //
  // Description:	This function calculates the specific humidity 
  //			from pressure in mb, temperature in degrees C
  //                    and relative humidity in % (0-100).
  //
  // Returns:		true is successful and false for failure. The
  //			specific humidity in g/kg is returned through
  //                    the pointer sh. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  
  template<class T> bool calcSpecificHumidity(const T *rh, const T *tmp_c,
					      const T *pres, T *sh,
					      const T miss, const T bad,
					      const int nx, const int ny,
					      const int nz)
    {
      // first calculate the mixing ratio
      //
      // use sh array to avoid any memory allocation
      bool status = calcMixingRatio2(rh, tmp_c, pres, sh,
				    miss, bad,
				    nx, ny, nz);

      if (!status)
	return false;

      int locNy = max(ny,1);
      int locNz = max(nz,1);
      int nPts = nx * locNy * locNz;
      
      T *shPtr = sh;
      for(int i = 0; i < nPts; i++, shPtr++)
      {
	if (*shPtr != miss && *shPtr != bad)
	  *shPtr = *shPtr / (1.0 + *shPtr);
      }

      return true;
    }



  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	calcSpecificHumidity
  //
  // Description:	This function calculates the specific humidity 
  //			from pre-calculated mixing ratio values.
  //
  // Returns:		true is successful and false for failure. The
  //			specific humidity in g/kg is returned through
  //                    the pointer sh. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  
  template<class T> bool calcSpecificHumidity(const T *mixing_ratio, T *sh,
					      const T miss, const T bad,
					      const int nx, const int ny,
					      const int nz)
    {
      int locNy = max(ny,1);
      int locNz = max(nz,1);
      int nPts = nx * locNy * locNz;
      
      T *shPtr = sh;
      const T *mrPtr = mixing_ratio;
      
      for(int i = 0; i < nPts; i++, shPtr++, mrPtr++)
      {
	if (*mrPtr != miss)
	  *shPtr = *mrPtr / (1.0 + *mrPtr);
      }

      return true;
    }



  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	calcWindDirection
  //
  // Description:	This function calculates the meteorlogical wind
  //			direction (i.e. measured from true North in a 
  //			clockwise direction) meteorlogical U/V wind
  //			components. U is positve along y-north axis, and
  //			V is positive along x-east axis.
  //
  // Returns:		true is successful and false for failure. The
  //			wind direction in degrees is returned through the  
  //			pointer wd. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  
  template<class T> bool calcWindDirection(const T *u, const T *v, T *wd,
					   const T miss, const T bad, 
					   int nx, int ny, int nz)
    {
      int nPts = nx * max(ny,1) * max(nz,1);

      const T *uPtr = u;
      const T *vPtr = v;
      T *wdPtr = wd;

      for (int i = 0; i < nPts; i++, uPtr++, vPtr++, wdPtr++)
      {
	if((*uPtr == miss) || (*uPtr == bad) ||
	   (*vPtr == miss) || (*vPtr == bad))
	{
	  *wdPtr = miss;
	}
	else
	{
	  *wdPtr = atan2(-(*uPtr),-(*vPtr))*180.0/M_PI;
	  if (*wdPtr < 0.0)
	    *wdPtr += 360.0;
	}

      } /* endfor - i */

      return true;
    }


  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	calcWindSpeed
  //
  // Description:	This function calculates the wind speed.
  //			direction (i.e. measured from true North in a 
  //			clockwise direction) meteorlogical U/V wind
  //			components. U is positve along y-north axis, and
  //			V is positive along x-east axis.
  //
  // Returns:		true is successful and false for failure. The
  //			wind speed in m/s is returned through the  
  //			pointer ws. 
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  
  template<class T> bool calcWindSpeed(const T *u, const T *v, T *ws,
				       const T miss, const T bad, 
				       int nx, int ny, int nz)
    {
      int nPts = nx * max(ny,1) * max(nz,1);

      const T *uPtr = u;
      const T *vPtr = v;
      T *wsPtr = ws;

      for (int i = 0; i < nPts; i++, uPtr++, vPtr++, wsPtr++)
      {
	if((*uPtr == miss) || (*uPtr == bad) ||
	   (*vPtr == miss) || (*vPtr == bad))
	{
	  *wsPtr = miss;
	}
	else
	{
	  *wsPtr = hypot(*uPtr, *vPtr);
	}

      } /* endfor - i */

      return true;
    }


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradDuDx
  //
  // Description:	This function calculates the gradient of a 
  //			vector field u in the x-direction.
  //
  // Returns:		true is successful and false for failure
  //
  // Globals:	
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //                    divide_by_dx_flag indicates whether the gradient
  //                    value should be normalized based on the grid spacing.
  //			
  //

  template<class T> bool gradDuDx(const T *u, T *grad,
				  const T miss, const T bad,
				  const double dx, 
				  const int nx, const int ny, const int nz,
				  const int span,
				  const bool divide_by_dx_flag)
    {
      // check that there is something to take the derivative of
      // before doing any work
      if ((u == 0) || (grad == 0) || (dx == 0.0))
	return false;

      int locNy = max(1,ny);
      int locNz = max(1,nz);

      for (int x = 0; x < nx; ++x)
      {
	for (int y = 0; y < locNy; ++y)
	{
	  for (int z = 0; z < locNz; ++z)
	  {
	    int cntIdx = _calcIndex(x, y, z, nx, ny, nz);

	    if (x < span || x >= nx - span)
	    {
	      grad[cntIdx] = miss; 
	    }
	    else
	    {
	      int bckIdx = _calcIndex(x - span, y, z, nx, ny, nz);
	      int fwdIdx = _calcIndex(x + span, y, z, nx, ny, nz);

	      if ((u[fwdIdx] == miss) || (u[fwdIdx] == bad) || 
		  (u[bckIdx] == miss) || (u[bckIdx] == bad))
	      {
		grad[cntIdx] = miss; 
	      }
	      else
	      {
		double delta = u[fwdIdx] - u[bckIdx];
		
		if (divide_by_dx_flag)
		  grad[cntIdx] = delta/((2.0*span+1)*dx); 
		else
		  grad[cntIdx] = delta;
	      }
	      
	    }

	  } /* endfor - z */
	} /* endfor - y */
      } /* endfor - x */

      return true;

    }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradDuDy
  //
  // Description:	This function calculates the gradient of a 
  //			vector field u in the y-direction.
  //
  // Returns:		true is successful and false for failure
  //
  // Globals:	
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //                    divide_by_dy_flag indicates whether the gradient
  //                    value should be normalized based on the grid spacing.
  //			
  //

  template<class T> bool gradDuDy(const T *u, T *grad,
				  const T miss, const T bad,
				  const double dy, 
				  const int nx, const int ny, const int nz,
				  const int span,
				  const bool divide_by_dy_flag)
    {
      // check that there is something to take the derivative of
      // before doing any work
      if((u == 0) || (grad == 0) || (dy == 0.0))
	return false;

      int locNz = max(1,nz);

      for (int x = 0; x < nx; ++x)
      {
	for (int y = 0; y < ny; ++y)
	{
	  for (int z = 0; z < locNz; ++z)
	  {
	    int cntIdx = _calcIndex(x, y, z, nx, ny, nz);

	    if (y < span || y >= ny - span)
	    {
	      grad[cntIdx] = miss; 
	    }
	    else
	    {
	      int bckIdx = _calcIndex(x, y - span, z, nx, ny, nz);
	      int fwdIdx = _calcIndex(x, y + span, z, nx, ny, nz);
	      
	      if ((u[fwdIdx] == miss) || (u[fwdIdx] == bad) || 
		  (u[bckIdx] == miss) || (u[bckIdx] == bad))
	      {
		grad[cntIdx] = miss; 
	      }
	      else
	      {
		T delta = u[fwdIdx] - u[bckIdx];
		
		if (divide_by_dy_flag)
		  grad[cntIdx] = delta/((2.0*span+1)*dy);
		else
		  grad[cntIdx] = delta;
	      }
	    }
	  } /* endfor - z */
	} /* endfor - y */
      } /* endfor - x */

      return true;

    }

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradDuDz
  //
  // Description:		This function calculates the gradient of 
  //			field u in the z-direction.
  //
  // Returns:		true is successful and false for failure
  //
  // Globals:	
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //
  //                    divide_by_dz_flag indicates whether the gradient
  //                    value should be normalized based on the grid spacing.
  //                    gradient.
  //			
  //

  template<class T> bool gradDuDz(const T *u, T *grad,
				  const T miss, const T bad,
				  const double dz,
				  const int nx, const int ny, const int nz,
				  const int span,
				  const bool divide_by_dz_flag)
    {
      // check that there is something to take the derivative of
      // before doing any work
      if ((u == 0) || (grad == 0) || (dz == 0.0))
	return false;

      for (int x = 0; x < nx; ++x)
      {
	for (int y = 0; y < ny; ++y)
	{
	  for (int z = 0; z < nz; ++z)
	  {
	    int cntIdx = _calcIndex(x, y, z, nx, ny, nz);

	    if (z < span || z >= nz - span)
	    {
	      grad[cntIdx] = miss; 
	    }
	    else
	    {
	      int bckIdx = _calcIndex(x, y, z - span, nx, ny, nz);
	      int fwdIdx = _calcIndex(x, y, z + span, nx, ny, nz);
	      
	      if ((u[fwdIdx] == miss) || (u[fwdIdx] == bad) || 
		  (u[bckIdx] == miss) || (u[bckIdx] == bad))
	      {
		grad[cntIdx] = miss; 
	      }
	      else
	      {
		double delta = u[fwdIdx] - u[bckIdx];
		
		if (divide_by_dz_flag)
		  grad[cntIdx] = delta/((2.0*span+1)*dz);
		else
		  grad[cntIdx] = delta;
	      }
	      
	    }

	  } /* endfor - z */
	} /* endfor - y */
      } /* endfor - x */

      return true;

    }

} // end namespace PhysicsLib

