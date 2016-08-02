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
/**
 *
 * @file Physics.cc
 *
 * @class Physics
 *
 * Class containing methods for calculating the physics.
 *  
 * @date 6/11/2010
 *
 */


#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <stdio.h>

#include <rapmath/math_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>

#include "Physics.hh"

// Constants

const double Physics::GRAVITY_CONSTANT = 9.806;    // m/s^2
const double Physics::CELKEL = 273.15;
const double Physics::RGAS = 287.04;      // J/K/kg
const double Physics::RGASMD = 0.608;     // ??
const double Physics::CP = 1004.0;        // J/K/kg  Note: not using Bolton's value of 1005.7 per RIP code comment
const double Physics::CPMD = 0.887;       // ??
const double Physics::GAMMA = RGAS / CP;
const double Physics::GAMMAMD = RGASMD - CPMD;
const double Physics::EPS = 0.622;
const double Physics::EZERO = 6.112;               // hPa
const double Physics::ESLCON1 = 17.67;
const double Physics::ESLCON2 = 29.65;
const double Physics::TLCLC1 = 2840.0;
const double Physics::TLCLC2 = 3.5;
const double Physics::TLCLC3 = 4.805;
const double Physics::TLCLC4 = 55.0;
const double Physics::THTECON1 = 3376.0;           // K
const double Physics::THTECON2 = 2.54;
const double Physics::THTECON3 = 0.81;

/*********************************************************************
 * calcBoundingPressure()
 */

MdvxField *Physics::calcBoundingPressure(const MdvxField &pressure_field) //mb
{
  static const string method_name = "Physics::_calcBoundingPressure()";
      
  // Create the blank bounding pressure field

  MdvxField *bounding_pres_field;
  
  if ((bounding_pres_field = _createBlankField(pressure_field,
					       "prsf",
					       "bounding pressure")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank bounding pressure field" << endl;
    
    return 0;
  }
  
  // Fill in the field values

  Mdvx::field_header_t p_fld_hdr = pressure_field.getFieldHeader();
  fl32 *p_data = (fl32 *)pressure_field.getVol();
  
  Mdvx::field_header_t pf_fld_hdr = bounding_pres_field->getFieldHeader();
  fl32 *pf_data = (fl32 *)bounding_pres_field->getVol();
  
  int plane_size = p_fld_hdr.nx * p_fld_hdr.ny;
  
  for (int i = 0; i < plane_size; ++i)
  {
    for (int z = 0; z < p_fld_hdr.nz; ++z)
    {
      int curr_index = (z * plane_size) + i;
      int prev_index = ((z-1) * plane_size) + i;
      int next_index = ((z+1) * plane_size) + i;
      
      if (p_data[curr_index] == p_fld_hdr.bad_data_value ||
	  p_data[curr_index] == p_fld_hdr.missing_data_value)
	continue;
      
      if (z == p_fld_hdr.nz - 1)
      {
	if (p_data[prev_index] == p_fld_hdr.bad_data_value ||
	    p_data[prev_index] == p_fld_hdr.missing_data_value)
	  continue;
	
	pf_data[curr_index] =
	  0.5 * (3.0 * p_data[curr_index] - p_data[prev_index]);
      }
      else
      {
	if (p_data[next_index] == p_fld_hdr.bad_data_value ||
	    p_data[next_index] == p_fld_hdr.missing_data_value)
	  continue;
	
	pf_data[curr_index] =
	  0.5 * (p_data[next_index] + p_data[curr_index]);
      }
      
    } /* endfor - z */
    
  } /* endfor - i */
  
  return bounding_pres_field;
}


/*********************************************************************
 * calcCapeCin()
 */

bool Physics::calcCapeCin(AdiabatTempLookupTable &lookup_table,
			  const MdvxField &pressure_field, //mb
			  const MdvxField &temperature_field, // K
			  const MdvxField &mixing_ratio_field, // g/g
			  const MdvxField &height_field, // gpm
			  const MdvxField &terrain_field, // m
			  const MdvxField &prsf_field, 
			  fl32 *cape,                 // J/kg
			  fl32 *cin,                  // J/kg
			  const int min_calc_level,
			  const int max_calc_level,
			  const bool process_3d,
			  fl32 *lcl_rel_ht,
			  fl32 *lfc_rel_ht,
			  fl32 *el_rel_ht,
			  fl32 *lcl,
			  fl32 *lfc,
			  fl32 *kbmin,
			  fl32 *bmin,
			  fl32 *zbmin,
			  fl32 *tlc,
			  fl32 *tlift,    // same as tv_par
			  fl32 *zpar,
			  fl32 *kpar_orig,
			  fl32 *bmax,
			  const bool debug)
{
  static const string method_name = "Physics::_calcCapeCin()";
      
  PMU_auto_register("Entering Physics::calcCapeCin3D");
  if (debug)
    cerr << "Entering " << method_name << endl;
  
  // Get the needed pointers

  Mdvx::field_header_t pressure_fld_hdr = pressure_field.getFieldHeader();
  fl32 *pressure = (fl32 *)pressure_field.getVol();
  
  Mdvx::field_header_t temperature_fld_hdr = temperature_field.getFieldHeader();
  fl32 *temperature = (fl32 *)temperature_field.getVol();
  
  Mdvx::field_header_t mixing_ratio_fld_hdr =
    mixing_ratio_field.getFieldHeader();
  fl32 *mixing_ratio = (fl32 *)mixing_ratio_field.getVol();
  
  Mdvx::field_header_t height_fld_hdr = height_field.getFieldHeader();
  fl32 *height = (fl32 *)height_field.getVol();
  
  Mdvx::field_header_t terrain_fld_hdr = terrain_field.getFieldHeader();
  fl32 *terrain = (fl32 *)terrain_field.getVol();
  
  Mdvx::field_header_t prsf_fld_hdr = prsf_field.getFieldHeader();
  fl32 *prsf = (fl32 *)prsf_field.getVol();
  
  // Get the grid information

  int nx = pressure_fld_hdr.nx;
  int ny = pressure_fld_hdr.ny;
  int nz = pressure_fld_hdr.nz;
  
  if (ny <= 0)
    return false;
      
  // Allocate space for the intermediate arrays

  int array_size = pressure_fld_hdr.nx * pressure_fld_hdr.ny;
      
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
      
  fl32 *t_tlift = new fl32[nz];
  fl32 *p_tlift = new fl32[nz];
  fl32 *q_tlift = new fl32[nz];
  
  for (int i = 0; i < nz; ++i)
  {
    t_tlift[i] = temperature_fld_hdr.missing_data_value;
    p_tlift[i] = pressure_fld_hdr.missing_data_value;
    q_tlift[i] = mixing_ratio_fld_hdr.missing_data_value;
  }
  
  // Calculate the theta array

  fl32 *theta;
  if ((theta = _calcTheta(pressure, temperature, mixing_ratio,
			  nx * ny * nz)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating theta grid" << endl;

    delete [] buoyancy;
    delete [] rel_height;
    delete [] accum_buoy_energy;
    delete [] parcel_mr_array;
    delete [] lift_temp_array;
    delete [] lift_mr_array;
    delete [] lift_vtemp_array;
    delete [] env_temp_array;
    delete [] env_mr_array;
    delete [] env_vtemp_array;
    delete [] height_array;
    delete [] t_tlift;
    delete [] p_tlift;
    delete [] q_tlift;
    
    return false;
  }
  
  // Determine the actual level limits for the calculations

  int true_max_calc_level = max_calc_level;
  if (true_max_calc_level >= nz)
    true_max_calc_level = nz - 1;
  if (true_max_calc_level < 0)
    true_max_calc_level = 0;
      
  int true_min_calc_level = min_calc_level;
  if (true_min_calc_level >= nz)
    true_min_calc_level = nz - 1;
  if (true_min_calc_level < 0)
    true_min_calc_level = 0;
      
  if (debug)
  {
    cerr << "    true_min_calc_level = " << true_min_calc_level << endl;
    cerr << "    true_max_calc_level = " << true_max_calc_level << endl;
  }
  
  // Initialize the return values.  Note that we assume that the return
  // grids are initialized appropriately to missing data values.

  int num_output_z_levels = true_max_calc_level - true_min_calc_level + 1;
  
  // Process the grids

  int kpar1, kpar2;
  
  double totthe = 0.0;
  double totqvp = 0.0;
  double totprs = 0.0;

  for (int x = 0; x < nx; ++x)
  {
    for (int y = 0; y < ny; ++y)
    {
      PMU_auto_register("Physics::calcCapeCin3D: Processing grid");
      
      // Find the index into the 2D fields for this grid point

      int plane_index = _calcIndex(x, y, nx, ny);

      // Initialize the CAPE and CIN values

      cape[plane_index] = 0.0;
      cin[plane_index] = 0.0;
      
      // Find the levels for doing the processing

      double parcel_mixing_ratio;   // g/g
      double parcel_temp;           // K

      if (process_3d)
      {
	kpar1 = true_min_calc_level;
	kpar2 = true_max_calc_level;
      }
      else
      {
	// Find the parcel with max theta-e in lowest 3 km AGL

	double ethmax = -1.0;
	int klev;
	bool valid_data_found = false;
	
//	for (int k = true_max_calc_level; k >= true_min_calc_level; --k)
	for (int k = 0; k < nz; ++k)
	{
	  int vol_index = _calcIndex(x, y, k, nx, ny, nz);
	  
	  // Make sure we have all of the data values we need

	  if (terrain[plane_index] == terrain_fld_hdr.missing_data_value ||
	      terrain[plane_index] == terrain_fld_hdr.bad_data_value ||
	      height[vol_index] == height_fld_hdr.missing_data_value ||
	      height[vol_index] == height_fld_hdr.bad_data_value ||
	      mixing_ratio[vol_index] == mixing_ratio_fld_hdr.missing_data_value ||
	      mixing_ratio[vol_index] == mixing_ratio_fld_hdr.bad_data_value ||
	      temperature[vol_index] == temperature_fld_hdr.missing_data_value ||
	      temperature[vol_index] == temperature_fld_hdr.bad_data_value ||
	      pressure[vol_index] == pressure_fld_hdr.missing_data_value ||
	      pressure[vol_index] == pressure_fld_hdr.bad_data_value)
	    continue;
	  
	  // Calculate the tlift values

	  t_tlift[k] = theta[vol_index] *
	    pow(pressure[vol_index] / 1000.0, 287.0 / 1004.0);
	  q_tlift[k] = mixing_ratio[vol_index];
	  p_tlift[k] = pressure[vol_index];
	  
	  // We only want to look in the lowest 3km

	  if (height[vol_index] - terrain[plane_index] >= 3000.0)
	    continue;
	  
	  valid_data_found = true;
	  
	  double q = MAX(mixing_ratio[vol_index], 1.0e-15);
	  double t = temperature[vol_index];
	  double p = pressure[vol_index];
	  double e = q * p / (EPS + q);
	  double tlcl = TLCLC1 / (log(pow(t, TLCLC2) / e) - TLCLC3) + TLCLC4;
	  double eth = t * pow(1000.0 / p, GAMMA * (1.0 + GAMMAMD * q)) *
	    exp((THTECON1 / tlcl - THTECON2) * q * (1.0 + THTECON3 * q));
	  
	  if (eth > ethmax)
	  {
	    klev = k;
	    ethmax = eth;
	    tlc[plane_index] = tlcl;
	  }
	  
	} /* endfor - k */
	
	if (!valid_data_found)
	  continue;
	
	if (klev >= nz || klev < 0)
	{
	  cerr << "---> invalid klev: " << klev << endl;
	  exit(0);
	}
	
	kpar1 = klev;
	kpar2 = klev;
	kpar_orig[plane_index] = klev;
	// NOTE: Dave's lastest version has the subtracting of terrain
	// commented out, but putting it back gives me an image that matches
	// Dave's image very closely.
//	zpar[plane_index] =
//	  height[_calcIndex(x, y, klev, nx, ny, nz)] /* - terrain[plane_index] */;
	zpar[plane_index] =
	  height[_calcIndex(x, y, klev, nx, ny, nz)] - terrain[plane_index];
	
	// Establish average properties of that parcel (over depth of
	// approximately davg meters)

//	// NOTE: We are using max_prsf instead of prsf(i,j,mkzh).  I'm guessing
//	// I was told to do this, but don't have notes on it.
//
//	double max_prsf = -1.0;
//	
//	for (int z = true_min_calc_level; z <= true_max_calc_level; ++z)
//	{
//	  int index = _calcIndex(x, y, z, nx, ny, nz);
//	  
//	  if (prsf[index] != prsf_fld_hdr.missing_data_value &&
//	      prsf[index] != prsf_fld_hdr.bad_data_value)
//	  {
//	    max_prsf = prsf[index];
//	    break;
//	  }
//	}
//	
//	if (max_prsf < 0.0)
//	{
//	  cerr << "*** max_prsf = " << max_prsf << endl;
//	  continue;
//	}
	
	int kpar1_index = _calcIndex(x, y, kpar1, nx, ny, nz);

	double davg = 500.0;
	double pavg = davg * pressure[kpar1_index] * GRAVITY_CONSTANT /
	  (RGAS * _virtualTemperature(temperature[kpar1_index],
				      mixing_ratio[kpar1_index]));
//	double p2 = MIN(pressure[kpar1_index] + 0.5 * pavg,
//			max_prsf);
	double p2 = MIN(pressure[kpar1_index] + 0.5 * pavg,
			prsf[_calcIndex(x, y, 0, nx, ny, nz)]);
	double p1 = p2 - pavg;
	
	totthe = 0.0;
	totqvp = 0.0;
	totprs = 0.0;
	bool data_found = false;
	
	for (int k = true_min_calc_level + 1; k <= true_max_calc_level; ++k)
	{
	  int index = _calcIndex(x, y, k, nx, ny, nz);
	  int prev_index = _calcIndex(x, y, k+1, nx, ny, nz);
	  
	  if (prsf[index] == prsf_fld_hdr.missing_data_value ||
	      prsf[index] == prsf_fld_hdr.bad_data_value)
	  {
	    continue;
	  }
	  
	  if (prsf[prev_index] == prsf_fld_hdr.missing_data_value ||
	      prsf[prev_index] == prsf_fld_hdr.bad_data_value)
	  {
	    continue;
	  }
	  
	  if (prsf[index] <= p1)
	    break;
	  
	  if (prsf[prev_index] >= p2)
	    continue;
	  
	  double p = pressure[index];
	  double pup = prsf[index];
	  double pdn = prsf[prev_index];
	  double q = MAX(mixing_ratio[index], 1.0e-15);
	  double th = temperature[index] *
	    pow(1000.0 / p, GAMMA * (1.0 + GAMMAMD * q));
	  double pp1 = MAX(p1, pdn);
	  double pp2 = MIN(p2, pup);
	  
	  if (pp2 > pp1)
	  {
	    double deltap = pp2 - pp1;
	    totqvp += q * deltap;
	    totthe += th * deltap;
	    totprs += deltap;
	    data_found = true;
	  }
	  
	} /* endfor - k */
	
	if (!data_found)
	  continue;
	
	parcel_mixing_ratio = totqvp / totprs;
	parcel_temp = (totthe / totprs) *
	  pow(pressure[kpar1_index] / 1000.0,
	      GAMMA * (1.0 + GAMMAMD * mixing_ratio[kpar1_index]));
      } /* endelse - process_3d */
      
//      for (int parcel_level = kpar1; parcel_level <= kpar2; ++parcel_level)
      for (int parcel_level = kpar2; parcel_level >= kpar1; --parcel_level)
      {
	int lcl_level = -1;
	    
	// Make sure all of the input data is valid.  If it's not,
	// continue on to the next grid point.

	int input_parcel_index = _calcIndex(x, y, parcel_level, nx, ny, nz);
	int output_parcel_index =
	  _calcIndex(x, y, parcel_level - kpar1, nx, ny, num_output_z_levels);
	    
	if (pressure[input_parcel_index] == pressure_fld_hdr.missing_data_value ||
	    pressure[input_parcel_index] == pressure_fld_hdr.bad_data_value ||
	    temperature[input_parcel_index] == temperature_fld_hdr.missing_data_value ||
	    temperature[input_parcel_index] == temperature_fld_hdr.bad_data_value ||
	    mixing_ratio[input_parcel_index] == mixing_ratio_fld_hdr.missing_data_value ||
	    mixing_ratio[input_parcel_index] == mixing_ratio_fld_hdr.bad_data_value ||
	    height[input_parcel_index] == height_fld_hdr.missing_data_value ||
	    height[input_parcel_index] == height_fld_hdr.bad_data_value)
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
	    
	// Calculate temperature and moisture properties of parcel

	if (process_3d)
	{
	  // parcel_mixing_ratio is in g/g

	  parcel_mixing_ratio =
	    mixing_ratio[input_parcel_index];
	  parcel_temp = temperature[input_parcel_index];
	}
	
	double parcel_pressure = pressure[input_parcel_index];
	double parcel_height = height[input_parcel_index];
//	double gammam = GAMMA * (1.0 + GAMMAMD * parcel_mixing_ratio);
	double cpm = CP * (1.0 + CPMD * parcel_mixing_ratio);

	double e = max(1.0e-20,
		       parcel_mixing_ratio * parcel_pressure /
		       (EPS + parcel_mixing_ratio));
	double lcl_temp = TLCLC1 /
	  (log(pow(parcel_temp, TLCLC2) / e) - TLCLC3) + TLCLC4;
//	double parcel_theta_e = parcel_temp *
//	  pow(1000.0 / parcel_pressure, gammam) *
//	  exp((THTECON1 / lcl_temp - THTECON2) * parcel_mixing_ratio *
//	      (1.0 + THTECON3 * parcel_mixing_ratio));
	double parcel_theta_e = parcel_temp *
	  pow(1000.0 / parcel_pressure,
	      GAMMA * (1.0 + GAMMAMD * parcel_mixing_ratio)) *
	  exp((THTECON1 / lcl_temp - THTECON2) * parcel_mixing_ratio *
	      (1.0 + THTECON3 * parcel_mixing_ratio));
	    
	double lcl_height = parcel_height +
	  (parcel_temp - lcl_temp) / (GRAVITY_CONSTANT / cpm);
	    
	// Calculate buoyancy and relative height of lifted parcel at all
	// levels, and store in bottom up arrays.  Add a level at the LCL,
	// and at all points where buoyancy is zero.

	int buoy_level = 0;
	    
//	rel_height[buoy_level] = 0.0;
//	buoyancy[buoy_level] = 0.0;
	int lcl_indicator = 0;
	int lfc_indicator = 0;
	int klcl;
	
	// Initialize the bouyancy bounds to extreme values

	bmin[plane_index] = 100.0;
	bmax[plane_index] = -100.0;
	
	// See if the initial parcel is alreay saturated or supersaturated

	if (parcel_height >= lcl_height)
	{
	  lcl_indicator = 2;
	  klcl = 1;
	}
	
	bool missing_data_found = false;
	    
	for (int lift_level = parcel_level+1; lift_level < nz; ++lift_level)
	{
	  // Check for missing input data

	  int lift_index = _calcIndex(x, y, lift_level, nx, ny, nz);
	  int lower_lift_index = _calcIndex(x, y, lift_level-1, nx, ny, nz);
	      
	  if (pressure[lift_index] == pressure_fld_hdr.missing_data_value ||
	      pressure[lift_index] == pressure_fld_hdr.bad_data_value ||
	      temperature[lift_index] == temperature_fld_hdr.missing_data_value ||
	      temperature[lift_index] == temperature_fld_hdr.bad_data_value ||
	      mixing_ratio[lift_index] == mixing_ratio_fld_hdr.missing_data_value ||
	      mixing_ratio[lift_index] == mixing_ratio_fld_hdr.bad_data_value ||
	      height[lift_index] == height_fld_hdr.missing_data_value ||
	      height[lift_index] == height_fld_hdr.bad_data_value)
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

	    lift_mixing_ratio = parcel_mixing_ratio;
	    lift_temp = parcel_temp - GRAVITY_CONSTANT / cpm *
	      (height[lift_index] - parcel_height);
	    env_virt_temp =
	      _virtualTemperature(temperature[lift_index],
				  mixing_ratio[lift_index]);
	    lift_virt_temp =
	      _virtualTemperature(lift_temp, lift_mixing_ratio);
	    lift_height = height[lift_index];

//	    env_temp = temperature[lift_index];
//	    env_mixing_ratio = mixing_ratio[lift_index];
	  }
	  else if (height[lift_index]  >= lcl_height &&
		   lcl_indicator == 0)
	  {
	    // This model and the previous model level straddle the LCL,
	    // so first create a new level in the bottom-up array, at
	    // the LCL.

	    lift_temp = lcl_temp;
	    lift_mixing_ratio = parcel_mixing_ratio;

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
	      (mixing_ratio[lower_lift_index]) * fac2 +
	      (mixing_ratio[lift_index]) * fac1;
	    env_virt_temp = _virtualTemperature(env_temp, env_mixing_ratio);
	    lift_virt_temp =
	      _virtualTemperature(lift_temp, lift_mixing_ratio);

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
	    env_virt_temp =
	      _virtualTemperature(temperature[lift_index],
				  mixing_ratio[lift_index]);
	    lift_virt_temp =
	      _virtualTemperature(lift_temp, lift_mixing_ratio);

//	    env_temp = temperature[lift_index];
//	    env_mixing_ratio = mixing_ratio[lift_index];

	    lift_height = height[lift_index];
	  }

//	  parcel_mr_array[buoy_level] = parcel_mixing_ratio;
//	  lift_temp_array[buoy_level] = lift_temp;
//	  lift_mr_array[buoy_level] = lift_mixing_ratio;
//	  lift_vtemp_array[buoy_level] = lift_virt_temp;
//	  env_temp_array[buoy_level] = env_temp;
//	  env_mr_array[buoy_level] = env_mixing_ratio;
//	  env_vtemp_array[buoy_level] = env_virt_temp;
//	  height_array[buoy_level] = lift_height;
	      
	  buoyancy[buoy_level] =
	    GRAVITY_CONSTANT * (lift_virt_temp - env_virt_temp) /
	    env_virt_temp;

	  rel_height[buoy_level] = lift_height - parcel_height;

	  if (lift_virt_temp - env_virt_temp < bmin[plane_index] &&
	      lfc_indicator == 0)
	  {
	    // See if the current parcel temperature deficit is less than bmin.
	    // ilfc must be zero, and the height AGL must also be below 7km
	    // in order to assign a new value to bmin. In the daytime with a
	    // moist surface, the parcel is buoyant all the way to the
	    // equilibrium level.  We don't want bmin values above 7km AGL.
	    // In these cases, ilfc is ill-defined. Technically the LFC would
	    // be at the parcel origin.  But because the buoyancy never passes
	    // from negative to positive territory, ilfc never changes from
	    // zero to 1.           

	    bmin[plane_index] = lift_virt_temp - env_virt_temp;
	    zbmin[plane_index] = height[lift_index] - terrain[plane_index];
	    tlift[plane_index] = lift_virt_temp;
	    kbmin[plane_index] = lift_level;
	  }
	  
	  if (lift_virt_temp - env_virt_temp > bmax[plane_index])
	    bmax[plane_index] = lift_virt_temp - env_virt_temp;
	  
	  // See if we have passed a 0 buoyancy level

	  if (buoy_level > 0 &&
	      buoyancy[buoy_level] * buoyancy[buoy_level-1] < 0.0)
	  {
	    // Parcel ascent curve crosses sounding curve, so create a
	    // new level in the bottom-up array at the crossing.

	    // Make sure we are crossing from neg to pos buoyancy for LFC

	    if (buoyancy[buoy_level] > 0.0)
	      lfc_indicator = 1;

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
	if (buoy_array_size > 150)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "buoy_array_size got too big" << endl;
	  cerr << "buoy_array_size = " << buoy_array_size << endl;
	  
	  return false;
	}

	if (buoy_array_size >= array_size)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "buoy_array_size got too big: " << buoy_array_size << endl;

	  delete [] buoyancy;
	  delete [] rel_height;
	  delete [] accum_buoy_energy;
	  delete [] parcel_mr_array;
	  delete [] lift_temp_array;
	  delete [] lift_mr_array;
	  delete [] lift_vtemp_array;
	  delete [] env_temp_array;
	  delete [] env_mr_array;
	  delete [] env_vtemp_array;
	  delete [] height_array;
	  delete [] t_tlift;
	  delete [] p_tlift;
	  delete [] q_tlift;
	  
	  return false;
	}

	// If no LCL was found, set klcl to buoy_array_size.  It is probably
	// not really at buoy_array_size, but this will make the rest of the
	// routine behave properly.

	if (lcl_indicator == 0)
	  klcl = buoy_array_size;
	
	// Get the accumulated buoyant energy from the parcel's starting
	// point, at all levels up to the top level.

	accum_buoy_energy[0] = 0.0;
	double min_accum_buoy_energy = 9e9;
	
	for (int k = 1; k < buoy_array_size; ++k)
	{
	  double dz = rel_height[k] - rel_height[k-1];
	  accum_buoy_energy[k] = accum_buoy_energy[k-1] +
	    (0.5 * dz * (buoyancy[k-1] + buoyancy[k]));
	  
	  if (accum_buoy_energy[k] < min_accum_buoy_energy)
	    min_accum_buoy_energy = accum_buoy_energy[k];
	} /* endfor - k */

	// Determine equilibrium level (EL), which we define as the highest
	// level of non-negative buoyancy above the LCL.  Note, this may be
	// the top level if the parcel is still buoyant there.
	    
	bool positive_buoyancy_found = false;
	int equil_level = -1;
	int lfc_level = -1;
	    
	for (int k = buoy_array_size - 1; k >= lcl_level; --k)
	{
	  if (buoyancy[k] >= 0.0)
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
	  // ????? Should these be missing instead??

	  cape[output_parcel_index] = -0.1;
	  cin[output_parcel_index] = -0.1;
	  lfc_level = buoy_array_size - 1;
	}
	else
	{
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

	  double min_accum_buoy_energy = 9e9;
	  lfc_level = buoy_array_size - 1;
	  
	  for (int k = lcl_level; k <= equil_level; ++k)
	  {
	    if (accum_buoy_energy[k] < min_accum_buoy_energy)
	    {
	      min_accum_buoy_energy = accum_buoy_energy[k];
	      lfc_level = k;
	    }
	  } /* endfor - k */
	      
	  // Now we can assign values to cape and cin

	  cape[output_parcel_index] =
	    MAX(accum_buoy_energy[equil_level] - min_accum_buoy_energy, 0.1);
	  cin[output_parcel_index] =
	    MAX(-min_accum_buoy_energy, 0.1);
	}

	// CIN is uninteresting when CAPE is small (<100 J/kg), so set
	// CIN to -0.1 in that case.
	// ??????? use missing?????

	if (cape[output_parcel_index] < 100.0)
	  cin[output_parcel_index] = -0.1;
	
	// Set the extra output fields

	if (lcl_rel_ht != 0)
	  lcl_rel_ht[output_parcel_index] = lcl_height;
	    
	if (lfc_rel_ht != 0 && positive_buoyancy_found)
	  lfc_rel_ht[output_parcel_index] = rel_height[lfc_level];
	    
	if (el_rel_ht != 0 && positive_buoyancy_found)
	  el_rel_ht[output_parcel_index] = rel_height[equil_level];
	    
	if (lcl != 0 &&
	    terrain[plane_index] != terrain_fld_hdr.missing_data_value &&
	    terrain[plane_index] != terrain_fld_hdr.bad_data_value)
	  lcl[output_parcel_index] =
	    rel_height[lcl_level] + parcel_height - terrain[plane_index];
	
	if (lfc != 0 &&
	    terrain[plane_index] != terrain_fld_hdr.missing_data_value &&
	    terrain[plane_index] != terrain_fld_hdr.bad_data_value)
	  lfc[output_parcel_index] =
	    rel_height[lfc_level] + parcel_height - terrain[plane_index];
	
      } /* endfor - parcel_level */

      double tl;
      double tm;
      double dummy = 0.0;
      
      for (int mm = kpar_orig[plane_index]; mm <= true_max_calc_level; ++mm)
      {
	if (t_tlift[mm] == temperature_fld_hdr.missing_data_value)
	  cerr << "---> t value missing" << endl;
	if (p_tlift[mm] == pressure_fld_hdr.missing_data_value)
	  cerr << "---> p value missing" << endl;
	if (q_tlift[mm] == mixing_ratio_fld_hdr.missing_data_value)
	  cerr << "---> q value missing" << endl;
      }
      
      if (!_tlift(t_tlift, 0.0, zpar[plane_index], p_tlift, q_tlift, 0.0,
//		  tl, tm, x, y, kpar_orig[plane_index], true_max_calc_level,
//		  dummy))
		  tl, tm, x, y, kpar_orig[plane_index], nz, dummy))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error returned from tlift" << endl;
	
	return false;
      }
      
      bmin[plane_index] = tl;
      bmax[plane_index] = tm;

    } /* endfor - y */
  } /* endfor - x */
      
  if (debug)
    cerr << "Finished processing grid" << endl;
  
  delete [] buoyancy;
  delete [] rel_height;
  delete [] accum_buoy_energy;
  delete [] parcel_mr_array;
  delete [] lift_temp_array;
  delete [] lift_mr_array;
  delete [] lift_vtemp_array;
  delete [] env_temp_array;
  delete [] env_mr_array;
  delete [] env_vtemp_array;
  delete [] height_array;
  delete [] t_tlift;
  delete [] p_tlift;
  delete [] q_tlift;
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcTheta()
 */

fl32 *Physics::_calcTheta(const fl32 *pressure, const fl32 *temperature,
			  const fl32 *mixing_ratio,
			  const int volume_size)
{
  static const string method_name = "Physics::_calcTheta()";

  // Allocate space for the grid

  fl32 *theta = new fl32[volume_size];
  
  // Calculate theta

  for (int i = 0; i < volume_size; ++i)
  {
    double gammam = GAMMA * (1.0 + GAMMAMD * mixing_ratio[i]);
    theta[i] = temperature[i] * pow((1000. / pressure[i]), gammam);
  } /* endfor - i */
  
  return theta;
}


/*********************************************************************
 * _createBlankField()
 */

MdvxField *Physics::_createBlankField(const MdvxField &base_field,
				      const string &field_name,
				      const string &field_name_long)
{
  // Create the field header based on the header in the base field

  Mdvx::field_header_t field_hdr = base_field.getFieldHeader();
  
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  
  // Create the vlevel header based on the header in the base field

  Mdvx::vlevel_header_t vlevel_hdr = base_field.getVlevelHeader();
  
  // Create and return the new field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _tlift()
 */

bool Physics::_tlift(fl32 *t, const double dt, const double z0,
		     fl32 *p, fl32 *q, const double dq,
		     double &tl, double &tm, int i, int j,
		     const int kk, const int mkzh, double &kminb)
{
  static const string method_name = "Physics::_tlift()";

//  cerr << "*** kk = " << kk << ", mkzh = " << mkzh << endl;
  
  // Allocate temporary arrays

  double *z = new double[mkzh+1];
  memset(z, 0, mkzh * sizeof(double));
  double *tb = new double[mkzh+1];
  memset(tb, 0, mkzh * sizeof(double));
  
  double rd = 287.0;
  double pt = 400.0;
  
  // Highest level to look for neg buoyancy

  double cp = 1005.0;
  int lv = 2.5e6;
  double grav = 9.81;
  double p00 = 1.0e3;
//  double dp = 10.0;
  double bsat = 243.5;
  double asat = 17.67;
  double t00 = 273.15;
  
  // Average parcel through specified pressure depth pavd

  double pavd = 50.0;
  double pavd2 = 25.0;
  int lvlbot = 0;
  double pbot = 0.0;
  double tbot = 0.0;
  double qbot = 0.0;
  
  int lvl;
  
  for (lvl = kk-1; lvl >= 0; --lvl)
  {
    double dp = p[lvl] - p[kk];
    
    if (dp > pavd2)
      break;
  } /* endwhile - true */
  
  lvlbot = lvl + 1;
  pbot = p[lvlbot];
  tbot = t[lvlbot];
  qbot = q[lvlbot];

  double ptop = pbot - pavd;
  
  double pave = 0.0;
  double tave = 0.0;
  double qave = 0.0;
  
  for (lvl = lvlbot + 1; lvl < mkzh; ++lvl)
  {
    if (p[lvl] < ptop)
      break;
  } /* endwhile - true */
  
  if (lvl >= mkzh)
    lvl = mkzh - 1;
  if (lvl < lvlbot)
    lvl = lvlbot;
  
//  cerr << "---> lvl = " << lvl << ", p[lvl] = " << p[lvl] << endl;
//  cerr << "     lvlbot = " << lvlbot << ", p[lvlbot] = " << p[lvlbot] << endl;
  
  double delta = (ptop - p[lvl-1]) / (p[lvl] - p[lvl-1]);
  double ttop = t[lvl-1] + (t[lvl] - t[lvl-1]) * delta;
  double qtop = q[lvl-1] + (q[lvl] - q[lvl-1]) * delta;
  pave = (pbot + ptop) / 2.0;
  tave = (tbot + ttop) / 2.0;
  qave = (qbot + qtop) / 2.0;

  // End parcel average

  double t0 = tave + dt;
  double q0 = MAX(qave + dq, 1.0e-4);
  double p0 = pave;
  
  z[kk] = z0;
  double es00 = 6.11;
  double tminb = 100.0;
  double tmaxb = -100.0;
  
  // Compute geopotential height

  for (int k = kk+1; k < mkzh-1; ++k)
  {
    double tv1 = t[k] * (1.0 + (0.61 * q[k]));
    double tv2 = t[k-1] * (1.0 + (0.61 * q[k-1]));
    z[k] = z[k-1] - rd * 0.5 * (tv1 + tv2) *
      (p[k] - p[k-1]) / (grav * 0.5 * (p[k] + p[k-1]));
  } /* endfor - k */
  
  // Find z_lcl relative to parcel with max MSE.
  // Check to see if parcel already saturated

  double tlc = t0 - t00;
  double esl = es00 * exp(asat * tlc / (bsat + tlc));
  double qsl = 0.622 * esl / (p0 - esl);
  if (q0 > qsl)
    q0 = qsl;
  
  double the0 = t0 * pow(p0 / p00, -rd / cp);
  double zmse0 = cp * t0 + grav * z0 + lv * q0;
  double tlk = t00 + (bsat / asat) * log(q0 * p0 / (0.622 * es00));

  int iter;
  double pl;
  double tlk2;
  
  for (iter = 0; iter < 10; ++iter)
  {
    tlc = tlk - t00;
    pl = p00 * pow(the0 / tlk, -cp / rd);
    esl = es00 * exp(asat * tlc / (bsat + tlc));
    qsl = 0.622 * esl / (pl - esl);
    tlk2 = tlk + ((bsat + tlc) / asat) * (q0 - qsl) / q0;

    if (fabs(tlk2 - tlk) <= 0.01)
      break;
    
    // Iterate for LCL

    tlk = tlk2;
  } /* endfor - iter */
  
//  double zl = z0 - cp * (tlk2 - t0) / grav;
  
  // Height of LCL

  int k1;
  
  for (k1 = kk; k1 < mkzh; ++k1)
  {
    if (p[k1] <= pt)
      break;
  } /* endfor - k1 */
  
  double tp = the0 * pow(p[kk] / p00, rd/cp);
  double t2a = tp;

//  cerr << "    kk = " << kk << endl;
//  cerr << "    k1 = " << k1 << endl;
  
  for (int k2 = kk+1; k2 <= k1; ++k2)
  {
//    cerr << "        k2 = " << k2 << endl;
    
    if (p[k2] > pl)
    {
      tp = the0 * pow(p[k2] / p00, rd/cp);
      tb[k2] = tp * (1.0 + 0.61 * q0) - t[k2] * (1.0 + 0.61 * q[k2]);
      t2a = tp;
      // NOTE: This zmsep isn't used anywhere
//      double zmsep = cp * tp + grav * z[k2] + lv * q0;
      if (tb[k2] < tminb)
      {
	tminb = tb[k2];
	kminb = k2;
      }
      
      if (tb[k2] > tmaxb)
      {
	tmaxb = tb[k2];
	// NOTE: kmaxb is set but not used anywhere
//	int kmaxb = k2;
      }
    }
    else
    {
      // Saturated
      // If here, parcel is saturated at level of max buoyancy

      double t2;
      double q2;
      
      iter = 0;
      while (true)
      {
	double t2c = t2a - 273.15;
	double es2a = 6.11 * exp(asat * t2c / (bsat + t2c));
	double q2a = 0.622 * es2a / (p[k2] - es2a);
	double zmsep = cp * t2a + grav * z[k2] + lv * q2a;
	t2 = t2a + 0.25 * (zmse0 - zmsep) / cp;
	q2 = q2a + 0.25 * (zmse0 - zmsep) / lv;

	++iter;
	if (iter > 10)
	  break;
	
	if (fabs(t2a - t2) > 0.01)
	{
	  // NOTE: This q2a isn't used anywhere
	  q2a = q2;
	  t2a = t2;
	  continue;
//	  tp = t2;
	}
      } /* endwhile - true */

      tb[k2] = t2 * (1.0 + 0.61 * q2) - t[k2] * (1.0 + 0.61 * q[k2]);
      if (tb[k2] < tminb)
      {
	tminb = tb[k2];
	kminb = k2;
      }
      
      if (tb[k2] > tmaxb)
      {
	tmaxb = tb[k2];
	// NOTE: kmaxb is set but isn't used anywhere
//	int kmaxb = k2;
      }
    }
  } /* endfor - k2 */
  
  tl = tminb;
  tm = tmaxb;
  
  // Reclaim space

  delete [] z;
  delete [] tb;
  
  return true;
}
