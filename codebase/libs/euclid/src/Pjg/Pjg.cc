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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:19:27 $
//   $Id: Pjg.cc,v 1.13 2016/03/03 18:19:27 dixon Exp $
//   $Revision: 1.13 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Pjg.cc: class implementing projective geometry calculations
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <euclid/PjgFlatCalc.hh>
#include <euclid/PjgLatlonCalc.hh>
#include <euclid/PjgLc1Calc.hh>
#include <euclid/PjgLc2Calc.hh>
#include <euclid/PjgPolarRadarCalc.hh>
#include <euclid/PjgPolarStereoCalc.hh>
#include <euclid/PjgObliqueStereoCalc.hh>
#include <euclid/PjgMercatorCalc.hh>
#include <euclid/euclid_macros.h>
#include <math.h>
using namespace std;

//////////////////
// Constructors //
//////////////////

/**********************************************************************
 * Default constructor
 */

Pjg::Pjg()
{
  _calculator = new PjgLatlonCalc();
}


/**********************************************************************
 * Copy constructor.
 */

Pjg::Pjg(const Pjg &rhs)
{
  this->_calculator = PjgCalc::copyCalc(rhs._calculator);

  if (this->_calculator == 0)
    this->_calculator =
      new PjgLatlonCalc();
}


/**********************************************************************
 * Destructor
 */

Pjg::~Pjg()
{
  delete _calculator;
}


/**********************************************************************
 * initFlat() - Initialize flat earth projection.
 */

void Pjg::initFlat(const double origin_lat, const double origin_lon,
		   const double rotation,
		   const int nx, const int ny, const int nz,
		   const double dx, const double dy, const double dz,
		   const double minx, const double miny, const double minz)
{
  delete _calculator;
  
  _calculator = new PjgFlatCalc(origin_lat, origin_lon, rotation,
				nx, ny, nz,
				dx, dy, dz,
				minx, miny, minz);
}


/**********************************************************************
 * initPolarRadar() - Initialize polar radar projection.
 */

void Pjg::initPolarRadar(const double origin_lat, const double origin_lon,
			 const int nx, const int ny, const int nz,
			 const double dx, const double dy, const double dz,
			 const double minx, const double miny,
			 const double minz)
{
  delete _calculator;
  
  _calculator = new PjgPolarRadarCalc(origin_lat, origin_lon,
				      nx, ny, nz,
				      dx, dy, dz,
				      minx, miny, minz);
}


/**********************************************************************
 * initLc1() - Initialize lambert conformal projection with one lats.
 */

void Pjg::initLc1(const double origin_lat, const double origin_lon,
		  const double lat1,
		  const int nx, const int ny, const int nz,
		  const double dx, const double dy, const double dz,
		  const double minx, const double miny, const double minz)
{
  delete _calculator;
  
  _calculator = new PjgLc1Calc(origin_lat, origin_lon, lat1,
			       nx, ny, nz,
			       dx, dy, dz,
			       minx, miny, minz);
}


/**********************************************************************
 * initLc2() - Initialize lambert conformal projection with two lats.
 */

void Pjg::initLc2(const double origin_lat, const double origin_lon,
		  const double lat1, const double lat2,
		  const int nx, const int ny, const int nz,
		  const double dx, const double dy, const double dz,
		  const double minx, const double miny, const double minz)
{
  delete _calculator;
  
  // Check to make sure the two lats are tangent.

  if (fabs(lat2 - lat1) > PjgCalc::TINY_ANGLE)
    _calculator = new PjgLc2Calc(origin_lat, origin_lon, lat1, lat2,
				 nx, ny, nz,
				 dx, dy, dz,
				 minx, miny, minz);
  else
    _calculator = new PjgLc1Calc(origin_lat, origin_lon, lat1,
				 nx, ny, nz,
				 dx, dy, dz,
				 minx, miny, minz);
}


/**********************************************************************
 * initLatlon() - Initialize latlon projection.
 */

void Pjg::initLatlon(const int nx, const int ny, const int nz,
		     const double dx, const double dy, const double dz,
		     const double minx, const double miny, const double minz)
{
  delete _calculator;
  
  _calculator = new PjgLatlonCalc(nx, ny, nz,
				  dx, dy, dz,
				  minx, miny, minz);
}

/**********************************************************************
 * initPolarStereo() - Initialize polar stereographic projection.
 */

void Pjg::initPolarStereo(const double tangent_lon,
			  const PjgTypes::pole_type_t pt /* = PjgTypes::POLE_NORTH*/,
			  const double central_scale /* = 1.0 */,
			  const int nx /* = 1*/,
			  const int ny /* = 1*/,
			  const int nz /* = 1*/,
			  const double dx /* = 1.0*/,
			  const double dy /* = 1.0*/,
			  const double dz /* = 1.0*/,
			  const double minx /* = 0.0*/,
			  const double miny /* = 0.0*/,
			  const double minz /* = 0.0*/ )
  
{

  delete _calculator;
  
  _calculator = new PjgPolarStereoCalc(tangent_lon, pt, central_scale,
				       nx, ny, nz, dx, dy, dz, 
				       minx, miny, minz);
}

/**********************************************************************
 * initObliqueStereo() - Initialize oblique stereographic projection.
 */

void Pjg::initObliqueStereo(const double origin_lat, const double origin_lon,
			    const double tangent_lat, const double tangent_lon,
			    const int nx /* = 1*/,
			    const int ny /* = 1*/,
			    const int nz /* = 1*/,
			    const double dx /* = 1.0*/,
			    const double dy /* = 1.0*/,
			    const double dz /* = 1.0*/,
			    const double minx /* = 0.0*/,
			    const double miny /* = 0.0*/,
			    const double minz /* = 0.0*/ )
{
  delete _calculator;
  
  _calculator = new PjgObliqueStereoCalc(origin_lat, origin_lon, tangent_lat, 
					 tangent_lon, nx, ny, nz, dx, dy, dz, 
					 minx, miny, minz);
}

/**********************************************************************
 * initMercator() - Initialize Mercator projection.
 */

void Pjg::initMercator(const double origin_lat, const double origin_lon,
		   const int nx, const int ny, const int nz,
		   const double dx, const double dy, const double dz,
		   const double minx, const double miny, const double minz)
{
  delete _calculator;
  
  _calculator = new PjgMercatorCalc(origin_lat, origin_lon,
				nx, ny, nz,
				dx, dy, dz,
				minx, miny, minz);
}

