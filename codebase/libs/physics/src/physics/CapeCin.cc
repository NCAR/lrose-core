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
 *  $Id: CapeCin.cc,v 1.23 2016/03/03 18:43:53 dixon Exp $
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
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <stdio.h>

// Local include files
#include <physics/AdiabatTempLookupTable.hh>
#include <physics/PhysicsLib.hh>
#include <physics/physics_macros.h>


// namespaces
using namespace std;

// define any constants


// declare static functions

namespace PhysicsLib {


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
		     fl32 *lcl, const fl32 lcl_missing,
		     fl32 *lfc, const fl32 lfc_missing,
		     fl32 *el, const fl32 el_missing,
		     const heartbeat_t heartbeat_func)
    {
      static const string method_name = "PhysicsLib::calcCapeCin3D()";
      
      if (heartbeat_func != NULL)
	heartbeat_func("Entering PhysicsLib::calcCapeCin3D");

      if (ny <= 0)
	return false;
      
      // Allocate space for the intermediate arrays

      int array_size = 16384;
      
      fl32 *buoyancy = new fl32[array_size];
      fl32 *rel_height = new fl32[array_size];
      fl32 *accum_buoy_energy = new fl32[array_size];
      fl32 *parcel_mr_array = new fl32[array_size];
      fl32 *lift_temp_array = new fl32[array_size];
      fl32 *lift_mr_array = new fl32[array_size];
      fl32 *lift_vtemp_array = new fl32[array_size];
      fl32 *env_temp_array = new fl32[array_size];
      fl32 *env_mr_array = new fl32[array_size];
      fl32 *env_vtemp_array = new fl32[array_size];
      fl32 *height_array = new fl32[array_size];
      
      // Determine the last level to do calculations

      int true_max_calc_level = min(max_calc_level, nz-1);
      true_max_calc_level = max(true_max_calc_level, 0);
      
      int true_min_calc_level = max(min_calc_level, 0);
      true_min_calc_level = min(true_min_calc_level, nz-1);
      
      // Initialize the return values

      int num_output_z_levels = true_max_calc_level - true_min_calc_level + 1;
      
      for (int i = 0; i < nx * ny * num_output_z_levels; ++i)
      {
	cape[i] = cape_missing;
	cin[i] = cin_missing;

	if (lcl != 0)
	  lcl[i] = lcl_missing;
	if (lfc != 0)
	  lfc[i] = lfc_missing;
	if (el != 0)
	  el[i] = el_missing;
      }
      

      // Process the grids

      for (int x = 0; x < nx; ++x)
      {

	for (int y = 0; y < ny; ++y)
	{
	  if (heartbeat_func != NULL)
	    heartbeat_func("PhysicsLib::calcCapeCin3D: Processing grid");
	  //	  cerr << "---> looking at x = " << x << ", y = " << y << endl;

	  for (int parcel_level = true_min_calc_level;
	       parcel_level <= true_max_calc_level; ++parcel_level)
	  {
	    int lcl_level = -1;
	    
	    // Make sure all of the input data is valid.  If it's not,
	    // continue on to the next grid point.

	    int input_parcel_index = _calcIndex(x, y, parcel_level, nx, ny, nz);
	    int output_parcel_index = _calcIndex(x, y, parcel_level - min_calc_level, nx, ny, num_output_z_levels);
	    

	    if (pressure[input_parcel_index] == pressure_missing ||
		pressure[input_parcel_index] == pressure_bad ||
		temperature[input_parcel_index] == temp_missing ||
		temperature[input_parcel_index] == temp_bad ||
		mixing_ratio[input_parcel_index] == mixing_ratio_missing ||
		mixing_ratio[input_parcel_index] == mixing_ratio_bad ||
		height[input_parcel_index] == height_missing ||
		height[input_parcel_index] == height_bad)
	      continue;
	    
	    // Initialize buoyancy arrays

	    memset((void *)buoyancy, 0, array_size * sizeof(fl32));
	    memset((void *)rel_height, 0, array_size * sizeof(fl32));
	    memset((void *)accum_buoy_energy, 0, array_size * sizeof(fl32));
	    memset((void *)parcel_mr_array, 0, array_size * sizeof(fl32));
	    memset((void *)lift_temp_array, 0, array_size * sizeof(fl32));
	    memset((void *)lift_mr_array, 0, array_size * sizeof(fl32));
	    memset((void *)lift_vtemp_array, 0, array_size * sizeof(fl32));
	    memset((void *)env_temp_array, 0, array_size * sizeof(fl32));
	    memset((void *)env_mr_array, 0, array_size * sizeof(fl32));
	    memset((void *)env_vtemp_array, 0, array_size * sizeof(fl32));
	    memset((void *)height_array, 0, array_size * sizeof(fl32));
	    
	    //
	    // Calculate temperature and moisture properties of parcel
	    //

	    double parcel_mixing_ratio =
	      mixing_ratio[input_parcel_index] / 1000.0;  // Convert from g/kg to g/g
	    double parcel_temp = temperature[input_parcel_index];
	    double parcel_pressure = pressure[input_parcel_index];
	    double parcel_height = height[input_parcel_index];
	    double gammam = GAMMA * (1.0 + GAMMAMD * parcel_mixing_ratio);
	    double cpm = CP * (1.0 + CPMD * parcel_mixing_ratio);

	    double e = max(1.0e-20,
			 parcel_mixing_ratio * parcel_pressure /
			 (EPS + parcel_mixing_ratio));
	    double lcl_temp = TLCLC1 /
	      (log(pow(parcel_temp, TLCLC2) / e) - TLCLC3) + TLCLC4;
	    double parcel_theta_e = parcel_temp *
	      pow(1000.0 / parcel_pressure, gammam) *
	      exp((THTECON1 / lcl_temp - THTECON2) * parcel_mixing_ratio *
		  (1.0 + THTECON3 * parcel_mixing_ratio));
	    
	    double lcl_height = parcel_height +
	      (parcel_temp - lcl_temp) / (GRAVITY_CONSTANT / cpm);
	    
	    //
	    // Calculate buoyancy and relative height of lifted parcel at all
	    // levels, and store in bottom up arrays.  Add a level at the LCL,
	    // and at all points where buoyancy is zero.
	    //

	    int buoy_level = 0;
	    
	    rel_height[buoy_level] = 0.0;
	    buoyancy[buoy_level] = 0.0;
	    int lcl_indicator = 0;
	    
	    bool missing_data_found = false;
	    
	    for (int lift_level = parcel_level+1;
		 lift_level < nz; ++lift_level)
	    {
	      // Check for missing input data

	      int lift_index = _calcIndex(x, y, lift_level, nx, ny, nz);
	      int lower_lift_index = _calcIndex(x, y, lift_level-1, nx, ny, nz);
	      
	      if (pressure[lift_index] == pressure_missing ||
		  pressure[lift_index] == pressure_bad ||
		  temperature[lift_index] == temp_missing ||
		  temperature[lift_index] == temp_bad ||
		  mixing_ratio[lift_index] == mixing_ratio_missing ||
		  mixing_ratio[lift_index] == mixing_ratio_bad ||
		  height[lift_index] == height_missing ||
		  height[lift_index] == height_bad)
	      {
		missing_data_found = true;
		
		break;
	      }
	      
	      double lift_mixing_ratio;
	      double lift_temp;
	      double env_temp;
	      double env_mixing_ratio;
	      double env_virt_temp;
	      double lift_virt_temp;
	      double lift_height;
	      
	      ++buoy_level;
	      
	      if (buoy_level >= array_size || buoy_level < 0)
		cerr << "*** Invalid value of buoy_level = " << buoy_level <<
		  ", array_size = " << array_size << endl;
	      
	      if (height[lift_index] < lcl_height)
	      {
		// We are below the LCL level.  Calculate values accordingly

		lift_temp = parcel_temp - GRAVITY_CONSTANT / cpm *
		  (height[lift_index] - parcel_height);
		lift_mixing_ratio = parcel_mixing_ratio;
		lift_virt_temp =
		  virtualTemperature(lift_temp, lift_mixing_ratio);

		env_temp = temperature[lift_index];
		env_mixing_ratio = mixing_ratio[lift_index] / 1000.0;
		env_virt_temp =
		  virtualTemperature(env_temp, env_mixing_ratio);

		lift_height = height[lift_index];
	      }
	      else if (lcl_indicator == 0)
	      {
		// We have just passed the LCL level.  These values will be
		// used to calculate the buoyancy at the LCL level and then
		// this level will be processed again in the next iteration
		// of the for loop by incrementing lift_level at the bottom
		// of the loop.

		lift_temp = lcl_temp;
		lift_mixing_ratio = parcel_mixing_ratio;
		lift_virt_temp =
		  virtualTemperature(lift_temp, lift_mixing_ratio);

		// Interpolate to get the environment temperature and
		// mixing ratio

		double denominator =
		  height[lift_index] - height[lower_lift_index];
		double fac1 =
		  (lcl_height - height[lower_lift_index]) / denominator;
		double fac2 = (height[lift_index] - lcl_height) / denominator;
		env_temp =
		  temperature[lower_lift_index] * fac2 +
		  temperature[lift_index] * fac1;
		env_mixing_ratio =
		  (mixing_ratio[lower_lift_index] / 1000.0) * fac2 +
		  (mixing_ratio[lift_index] / 1000.0) * fac1;
		env_virt_temp = virtualTemperature(env_temp, env_mixing_ratio);

		lift_height = lcl_height;

		lcl_indicator = 1;
	      }
	      else
	      {
		// We are now above the LCL level.

		lift_temp =
		  lookup_table.getTemperature(pressure[lift_index],
					      parcel_theta_e);
		double eslift = EZERO * exp(ESLCON1 * (lift_temp - CELKEL) /
				     (lift_temp - ESLCON2));
		lift_mixing_ratio = EPS * eslift /
		  (pressure[lift_index] - eslift);
		lift_virt_temp =
		  virtualTemperature(lift_temp, lift_mixing_ratio);

		env_temp = temperature[lift_index];
		env_mixing_ratio = mixing_ratio[lift_index] / 1000.0;
		env_virt_temp =
		  virtualTemperature(env_temp, env_mixing_ratio);

		lift_height = height[lift_index];
	      }

	      parcel_mr_array[buoy_level] = parcel_mixing_ratio;
	      lift_temp_array[buoy_level] = lift_temp;
	      lift_mr_array[buoy_level] = lift_mixing_ratio;
	      lift_vtemp_array[buoy_level] = lift_virt_temp;
	      env_temp_array[buoy_level] = env_temp;
	      env_mr_array[buoy_level] = env_mixing_ratio;
	      env_vtemp_array[buoy_level] = env_virt_temp;
	      height_array[buoy_level] = lift_height;
	      
	      buoyancy[buoy_level] =
		GRAVITY_CONSTANT * (lift_virt_temp - env_virt_temp) /
		env_virt_temp;

	      rel_height[buoy_level] = lift_height - parcel_height;

	      // See if we have passed a 0 buoyancy level

	      if (buoyancy[buoy_level] * buoyancy[buoy_level-1] < 0.0)
	      {
		// Move the new buoyancy information up a level so we
		// can add the 0 buoyancy to the array.

		++buoy_level;

		buoyancy[buoy_level] = buoyancy[buoy_level-1];
		rel_height[buoy_level] = rel_height[buoy_level-1];

		// Add the 0 buoyancy information to the arrays

		buoyancy[buoy_level-1] = 0.0;

		rel_height[buoy_level-1] = rel_height[buoy_level-2] +
		  buoyancy[buoy_level-2] /
		  (buoyancy[buoy_level-2] - buoyancy[buoy_level]) *
		  (rel_height[buoy_level] - rel_height[buoy_level-2]);
	      }
	      
	      if (lcl_indicator == 1)
	      {
		// We have just passed the LCL level.  As indicated above,
		// we need to increment the lift_level so that the current
		// level will be processed again.  This is done because in
		// the current pass we really calculate the buoyancy at the
		// LCL level rather than at the current level and have added
		// the LCL level buoyancy to the buoyancy arrays.

		lcl_level = buoy_level;  // Not used -- commented out below
		lcl_indicator = 2;
		--lift_level;
		continue;
	      }
	    } /* endfor - lift_level */

	    if (missing_data_found)
	      continue;
	    
	    int buoy_array_size = buoy_level + 1;
	    
	    if (buoy_array_size >= array_size)
	    {
	      cerr << "ERROR: " << method_name << endl;
	      cerr << "buoy_array_size got too big: " << buoy_array_size << endl;

	      delete[] buoyancy;
	      delete[] rel_height;
	      delete[] accum_buoy_energy;
	      delete[] parcel_mr_array;
	      delete[] lift_temp_array;
	      delete[] lift_mr_array;
	      delete[] lift_vtemp_array;
	      delete[] env_temp_array;
	      delete[] env_mr_array;
	      delete[] env_vtemp_array;
	      delete[] height_array;
	      
	      return false;
	    }

	    //
	    // Get the accumulated buoyant energy from the parcel's starting
	    // point, at all levels up to the top level.
	    //
	    accum_buoy_energy[0] = 0.0;

	    for (int k = 1; k < buoy_array_size; ++k)
	    {
	      double dz = rel_height[k] - rel_height[k-1];
	      accum_buoy_energy[k] = accum_buoy_energy[k-1] + (0.5 * dz * (buoyancy[k-1] + buoyancy[k]));

	    } /* endfor - k */

	    //
	    // Determine equilibrium level (EL), which we define as the highest
	    // layer at which the parcel is positively buoyant.
	    //
	    
	    bool positive_buoyancy_found = false;
	    int equil_level = -1;
	    int lfc_level = -1;            // Never used, only set
	    
	    for (int k = buoy_array_size - 1; k >= 1; --k)
	    {
	      if (buoyancy[k] == 0.0)
	      {
		equil_level = k;
		positive_buoyancy_found = true;
		break;
	      }
	    }

	    // If positive buoyancy was not found anywhere in the sounding, CAPE
	    // is zero.  CIN will be set missing, and lfc_level will be set to
	    // missing.

	    if (!positive_buoyancy_found)
	    {

	      cape[output_parcel_index] = 0.0;

	    // Commenting this out will cause the cin to remain as
	    // the missing data value. This was requested by Dave Ahijevych
	    // and Stan Trier. 

	    //cin[output_parcel_index] = accum_buoy_energy[buoy_array_size-1];
	      
	    //lfc_level = buoy_array_size - 1;
	    
	      //
	      // c   CIN will be set to minus the accumulated buoyant energy
	      // c   at the point at or above the LCL where accumulated buoyant energy
	      // c   is a maximum, i.e. the least negative.
	      // c   LFC will be set to the height of that level.
	      //
	      //c      cape(y,x,parcel_level)=0.0
	      //c      benamax=-9e9
	      //c      do k=lcl_level,buoy_array_size
	      //c         if (accum_buoy_energy(k).gt.benamax) then
	      //c            benamax=accum_buoy_energy(k)
	      //c            lfc_level=k
	      //c         endif
	      //c      enddo
	      //c      cin(y,x,parcel_level)=-benamax
	      //c
	    }
	    else
	    {
	      //
	      // If there is an equilibrium level, then CAPE is positive.
	      // We'll define the level of free convection (LFC) as the point
	      // below the Equilibrium Level where accumulated buoyant energy
	      // is a minimum.  The net positive area (accumulated
	      // buoyant energy) from the LFC up to the EL will be
	      // defined as the CAPE, and the net negative area (negative of
	      // accumulated buoyant energy) from the parcel starting point to
	      // the LFC will be defined as the convective inhibition (CIN).
	      //
	      // First get the LFC (i.e. the level below the EL with minimum
	      // accum. buoyant energy)
	      // 

	      fl32 min_accum_buoy_energy = 9e9;
	      lfc_level = buoy_array_size - 1;

	      for (int k = 0; k <= equil_level; ++k)
	      {
		if (accum_buoy_energy[k] < min_accum_buoy_energy)
		{
		  min_accum_buoy_energy = accum_buoy_energy[k];
		  lfc_level = k;
		}
	      } /* endfor - k */
	      
	      //
	      // Now we can assign values to cape and cin
	      //

	      cape[output_parcel_index] =
		accum_buoy_energy[equil_level] - min_accum_buoy_energy;
	      cin[output_parcel_index] = -min_accum_buoy_energy;
	    }

	    // Set the output levels

	    if (lcl != 0)
	      lcl[output_parcel_index] = lcl_height;
	    
	    if (lfc != 0 && positive_buoyancy_found)
	    {
	      lfc[output_parcel_index] = rel_height[lfc_level];
	    }
	    
	    if (el != 0 && positive_buoyancy_found)
	      el[output_parcel_index] = rel_height[equil_level];
	    

	  } /* endfor - parcel_level */

	} /* endfor - y */
	
      } /* endfor - x */
      
      delete[] buoyancy;
      delete[] rel_height;
      delete[] accum_buoy_energy;
      delete[] parcel_mr_array;
      delete[] lift_temp_array;
      delete[] lift_mr_array;
      delete[] lift_vtemp_array;
      delete[] env_temp_array;
      delete[] env_mr_array;
      delete[] env_vtemp_array;
      delete[] height_array;
      
      return true;
    }
  

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
		    const int nx, const int ny, const int nz)
    {
      static const string method_name = "PhysicsLib::calcThetaE3D()";
      
      if (nx <= 0 || ny <= 0 || nz <= 0)
	return false;
      
      for (int index = 0; index < nx * ny * nz; ++index)
      {
	if (mixing_ratio[index] == mixing_ratio_bad ||
	    mixing_ratio[index] == mixing_ratio_missing ||
	    temperature[index] == temperature_bad ||
	    temperature[index] == temperature_missing ||
	    pressure[index] == pressure_bad ||
	    pressure[index] == pressure_missing)
	{
	  theta_e[index] = theta_e_missing;
	      
	  continue;
	}
	    
	double q = max(mixing_ratio[index] / 1000.0, 1.0e-15);
	double t = temperature[index];
	double p = pressure[index];
	double e = (q * p) / (EPS + q);
	double tlcl = TLCLC1 / (log(pow(t, TLCLC2) / e)) + TLCLC4;
	theta_e[index] = t * pow(1000.0 / p,
				 GAMMA * (1.0 + GAMMAMD * q)) *
	  exp((THTECON1 / tlcl - THTECON2) * q * (1.0 + THTECON3 * q));
	    
      } /* endfor - index */
      
      return true;
    }
  
} /* end --  namespace PhysicsLib */
