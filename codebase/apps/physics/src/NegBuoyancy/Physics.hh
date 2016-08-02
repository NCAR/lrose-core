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
 * @file Physics.hh
 *
 * @class Physics
 *
 * Class containing physics methods.
 *  
 * @date 6/14/2010
 *
 */

#ifndef Physics_HH
#define Physics_HH

#include <dataport/port_types.h>
#include <Mdv/MdvxField.hh>
#include <physics/AdiabatTempLookupTable.hh>

/**
 * @class Physics
 */

class Physics
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  static MdvxField *calcBoundingPressure(const MdvxField &pressure_field); //mb


  static bool calcCapeCin(AdiabatTempLookupTable &lookup_table,
			  const MdvxField &pressure_field, // mb
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
			  fl32 *el,
			  fl32 *lcl,
			  fl32 *lfc,
			  fl32 *kbmin,
			  fl32 *bmin,
			  fl32 *zbmin,
			  fl32 *tlc,
			  fl32 *tlift,
			  fl32 *zpar,
			  fl32 *kpar,
			  fl32 *bmax,
			  const bool debug);
  

protected:

  static const double GRAVITY_CONSTANT;    // m/s^2
  static const double CELKEL;
  static const double RGAS;      // J/K/kg
  static const double RGASMD;     // ??
  static const double CP;        // J/K/kg  Note: not using Bolton's value of 1005.7 per RIP code comment
  static const double CPMD;       // ??
  static const double GAMMA;
  static const double GAMMAMD;
  static const double EPS;
  static const double EZERO;               // hPa
  static const double ESLCON1;
  static const double ESLCON2;
  static const double TLCLC1;
  static const double TLCLC2;
  static const double TLCLC3;
  static const double TLCLC4;
  static const double THTECON1;           // K
  static const double THTECON2;
  static const double THTECON3;
  
  static MdvxField *_createBlankField(const MdvxField &base_field,
				      const string &field_name,
				      const string &field_name_long);
  

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

  static inline int _calcIndex(const int x, const int y, const int z,
			       const int nx, const int ny, const int nz)
  {
    return x + (y * nx) + (z * nx * ny);
  }
  
  static inline int _calcIndex(const int x, const int y,
			       const int nx, const int ny)
  {
    return x + (y * nx);
  }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Function Name:	_calcTheta
  //
  // Description:	This function calculates theta from pressure,
  //                    temperature and mixing ration.
  //
  // Returns:		The allocated and calculated grid.
  //
  // Notes:		The calling method is responsible for deleting the
  //                    allocated array.
  //			
  //

  static fl32 *_calcTheta(const fl32 *pressure, const fl32 *temperature,
			  const fl32 *mixing_ratio,
			  const int volume_size);


  static inline double _virtualTemperature(const double temperature,
					   const double mixing_ratio)
  {
    return temperature * (EPS + mixing_ratio) / (EPS * (1.0 + mixing_ratio));
  }


  static bool _tlift(fl32 *t, const double dt, const double z0,
		     fl32 *p, fl32 *q, const double dq,
		     double &tl, double &tm, int i, int j,
		     const int kk, const int mkzh, double &kminb);
  
};


#endif
