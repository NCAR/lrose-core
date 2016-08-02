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
 *  $Id: PhysicsLib.hh,v 1.24 2016/03/03 19:23:21 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header: physics
// 
// Author: G M Cunning
// 
// Date:	Fri Jun 16 11:08:50 2000
// 
// Description:	This header contains functions to calculate derived 
//		physical quantities. Unless otherwise specified the 
//		units for input and output are MKS.
// 
// 


# ifndef    PHYSICSLIB_H
# define    PHYSICSLIB_H

// C++ include files
#include <cmath>

// System/RAP include files
#include <dataport/port_types.h>

// Local include files

#include <physics/AdiabatTempLookupTable.hh>

using namespace std;

namespace PhysicsLib {
  static const double EPSILON = 0.00001;
  
  static const double COREOLIS_FORCE = 7.292e-5;   // rad/sec
  static const double GAS_CONSTANT = 287.0;        // m^2 / ( s^2 * degC )
  static const double GRAVITY_CONSTANT = 9.806;    // m/s^2

  // Constants from RIP code

  static const double RGAS = 287.04;               // J/K/kg
  static const double RGASMD = 0.608;              // ??
  static const double CP = 1004.0;                 // J/K/kg  Note: not using Bolton's value of 1005.7 per RIP code comment
  static const double CPMD = 0.887;                // ??
  static const double GAMMA = RGAS / CP;
  static const double GAMMAMD = RGASMD - CPMD;
  static const double EPS = 0.622;
  static const double EZERO = 6.112;               // hPa
  static const double CELKEL = 273.15;
  static const double ESLCON1 = 17.67;
  static const double ESLCON2 = 29.65;
  static const double THTECON1 = 3376.0;           // K
  static const double THTECON2 = 2.54;
  static const double THTECON3 = 0.81;
  static const double TLCLC1 = 2840.0;
  static const double TLCLC2 = 3.5;
  static const double TLCLC3 = 4.805;
  static const double TLCLC4 = 55.0;
  
  // Type definitions

  typedef void (*heartbeat_t)(const char *label);


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	celsius2Kelvin
  //
  // Description:	This function converts a temperature value given
  //                    in Celsius to the equivalent temperature value
  //                    given in Kelvin.
  //
  // Returns:		the Kelvin temperature value.
  //
  // Notes:		
  //
  //

  template<class T> inline T celsius2Kelvin(const T celsius_temp)
    {
      return celsius_temp + 273.16;
    }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	kelvin2Celsius
  //
  // Description:	This function converts a temperature value given
  //                    in Kelvin to the equivalent temperature value
  //                    given in Celsius.
  //
  // Returns:		the Celcius temperature value.
  //
  // Notes:		
  //
  //

  template<class T> inline T kelvin2Celsius(const T kelvin_temp)
    {
      return kelvin_temp - 273.16;
    }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	virtualTemp
  //
  // Description:	This method calculates virtual temperature in K
  //                    given temperature in K and mixing ratio in kg/kg.
  //
  // Returns:		the virtual temperature value.
  //
  // Notes:		
  //
  //

  template<class T> inline T virtualTemperature(const T temperature,
						const T mixing_ratio)
  {
    return temperature * (EPS + mixing_ratio) / (EPS * (1.0 + mixing_ratio));
  }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	potentialTemp
  //
  // Description:	This method calculates potential temperature in C
  //                    given temperature in C and pressure in millibars.
  //
  // Returns:		the virtual temperature value.
  //
  // Notes:		
  //
  //

  template<class T> inline T potentialTemperature(const T temperature,
						  const T pressure)
  {
    return temperature * pow((1000.0/pressure), (2.0/7.0));
  }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	mb2Pascals
  //
  // Description:	This function converts a pressure value given
  //                    in millibars to the equivalent pressure value
  //                    given in Pascals.
  //
  // Returns:		the Pascals pressure value.
  //
  // Notes:		
  //
  //

  template<class T> inline T mb2Pascals(const T press_mb)
    {
      return press_mb * 100.0;
    }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	pascals2mb
  //
  // Description:	This function converts a pressure value given
  //                    in Pascals to the equivalent pressure value
  //                    given in millibars.
  //
  // Returns:		the millibars pressure value.
  //
  // Notes:		
  //
  //

  template<class T> inline T pascals2mb(const T press_pas)
    {
      return press_pas / 100.0;
    }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	vorticity
  //
  // Description:	This function calculates the relative vorticity, 
  //			dv/dx - du/dy.
  //
  // Returns:		true is successful and false for failure. The
  //			vorticity is return through the pointer vort. 
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
  //
  //

  template<class T> bool vorticity(const T *u, const T *v, T *vort,
				   const T miss, const T bad,
				   const double dx, const double dy,
				   const int nx, const int ny,
				   const int nz = 0,
				   const int span = 1);


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

  template<class T> bool absoluteVorticity(const T *u, const T *v, T *vort,
			 const T miss, const T bad,
			 const double dx, const double dy, const int nx, 
			 const int ny, const int nz = 0);



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
				     const int nx, const int ny,
				     const int nz = 0,
				     const int span = 1);



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
					     const T *q, T *cvrg,
					     const T miss, const T bad,
					     const double dx, const double dy, 
					     const int nx, const int ny,
					     const int nz = 0,
					     const int span = 1);


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
				  double dy = 0.0, int ny = 0, 
				  double dz = 0.0, int nz = 0,
				  const int span = 1);


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
				     const double dy = 0.0, const int ny = 0, 
				     const double dz = 0.0, const int nz = 0,
				     const int span = 1);



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
					  const T *pres_pa, T *mixr,
					  const T miss, const T bad,
					  const int nx, const int ny = 0,
					  const int nz = 0);


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
					  const int nz);
  

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
					      const int nz);
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcRelativeHumidity
  //
  // Description:	This function calculates the relative humidity in %
  //                    from the mixing ratio in g/kg, the pressure in mb
  //                    and the virtual potential temperature in degrees C.
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
					      const int nz);
  

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
				      const int nz);
  

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
				      const int nz);
  

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
				       const int nz);
  

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
					 const int nz);
  

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
					  const int nz);
  

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
					  const int nz);
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcEPOT
  //
  // Description:	This function calculates the equivalent potential
  //                    temperature in degrees C from the pressure in mb
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
				  const int nx, const int ny, const int nz);

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcEPOT2
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

  template<class T> bool calcEPOT2(const T *pressure, const T *temp,
				   const T *dewpt, T *epot,
				   const T miss, const T bad,
				   const int nx, const int ny, const int nz);
  

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
					   const int nx, const int ny, const int nz);
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcSatVaporPressure
  //
  // Description:	This function calculates the saturated vapor pressure
  //			from temperature in degrees C. The pressure units are
  //			Pa.
  //
  // Returns:		true is successful and false for failure. The
  //			vapor pressure in Pa is returned through the pointer 
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
					      const int nx, const int ny = 0,
					      const int nz = 0);


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
					      const int nx, const int ny = 0,
					      const int nz = 0);



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
					      const int nx, const int ny = 0,
					      const int nz = 0);
  

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
					   int nx, int ny = 0, int nz = 0);

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
				       int nx, int ny, int nz);

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcCapeCin3D
  //
  // Description:	This function calculates the 3D CAPE and CIN
  //                    grids in J/kg given pressure in mb, temperature
  //                    in K and mixing ratio in g/kg.
  //
  // Returns:		true is successful and false for failure. The
  //			CAPE is returned through the pointer cape and
  //                    the CIN is returned through the pointer cin.
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  //

  bool calcCapeCin3D(AdiabatTempLookupTable &lookup_table,
		     const fl32 *pressure,        // mb
		     const fl32 pressure_missing, const fl32 pressure_bad,
		     const fl32 *temperature,     // K
		     const fl32 temp_missing, const fl32 temp_bad,
		     const fl32 *mixing_ratio,    // g/kg
		     const fl32 mixing_ratio_missing,
		     const fl32 mixing_ratio_bad,
		     const fl32 *height,          // gpm
		     const fl32 height_missing, const fl32 height_bad,
		     fl32 *cape,                 // J/kg
		     const fl32 cape_missing,
		     fl32 *cin,                  // J/kg
		     const fl32 cin_missing,
		     const int min_calc_level,
		     const int max_calc_level,
		     const float dx, const float dy,
		     const int nx, const int ny, const int nz,
		     fl32 *lcl = 0, const fl32 lcl_missing = 0.0,
		     fl32 *lfc = 0, const fl32 lfc_missing = 0.0,
		     fl32 *el = 0, const fl32 el_missing = 0.0,
		     const heartbeat_t heartbeat_func = 0);
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	calcThetaE3D
  //
  // Description:	This function calculates the 3D theta-e
  //                    grid given pressure in mb, temperature
  //                    in K and mixing ratio in g/kg.
  //
  // Returns:		true is successful and false for failure. The
  //			theta-e is returned through the pointer theta_e.
  //
  // Notes:		The data in all of the grids is expected to have
  //                    the same X-Y-Z ordering.
  //
  //			The calling function is responsible for 
  //			memory managment.
  //
  //

  bool calcThetaE3D(const fl32 *mixing_ratio,    // g/kg
		    const fl32 mixing_ratio_missing,
		    const fl32 mixing_ratio_bad,
		    const fl32 *temperature,     // K
		    const fl32 temperature_missing,
		    const fl32 temperature_bad,
		    const fl32 *pressure,        // mb
		    const fl32 pressure_missing, const fl32 pressure_bad,
		    fl32 *theta_e,                 // J/kg
		    const fl32 theta_e_missing,
		    const int nx, const int ny, const int nz);
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradDuDx
  //
  // Description:	This function calculates the gradient of a 
  //			vector field u in the x-direction.
  //
  // Returns:		true is successful and false for failure
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
				  const int nx, const int ny = 0,
				  const int nz = 0,
				  const int span = 1,
				  const bool divide_by_dx_flag = true);


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradDuDy
  //
  // Description:	This function calculates the gradient of a 
  //			vector fieldbu in the y-direction.
  //
  // Returns:		true is successful and false for failure
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
				  const double dx,
				  const int nx, const int ny, const int nz,
				  const int span = 1,
				  const bool divide_by_dy_flag = true);


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	gradDuDz
  //
  // Description:	This function calculates the gradient of 
  //			field u in the z-direction.
  //
  // Returns:		true is successful and false for failure
  //
  // Notes:		The expected data ordering is x-y-z. In terms of 
  //			array ordering x is the fastest dimension, and 
  //			z is the slowest dimension. 
  //
  //                    span is the number of grid spaces to span on either
  //                    side of the current space when calculating the
  //                    gradient.
  //
  //                    divide_by_dz_flag indicates whether the gradient
  //                    value should be normalized based on the grid spacing.
  //			
  //

  template<class T> bool gradDuDz(const T *u, T *grad,
				  const T miss, const T bad,
				  const double dx,
				  const int nx, const int ny, const int nz,
				  const int span,
				  const bool divide_by_dz_flag = true);


  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	_calcIndex
  //
  // Description:	This function calculates the array index for
  //                    the given x,y,z location.
  //
  // Returns:		The absolute array index for the location
  //
  // Notes:		This method is written for interal use only.
  //			
  //

  inline int _calcIndex(const int x, const int y, const int z,
			const int nx, const int ny, const int nz)
    {
      return x + (y * nx) + (z * nx * ny);
    }
  
  inline int _calcIndex(const int x, const int y,
			const int nx, const int ny)
    {
      return x + (y * nx);
    }
  

}

#include <physics/_PhysicsLibImplementation.hh>


# endif     /* PHYSICSLIB_H */
