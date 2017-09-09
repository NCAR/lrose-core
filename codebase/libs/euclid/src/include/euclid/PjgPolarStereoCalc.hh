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

/************************************************************************
 * PjgPolarStereoCalc.hh: class implementing projective geometry 
 *                        transformations.
 *
 * If you use the default constructor, the projection will be set 
 * to PolarStereo. You must call one of the init() functions if you want
 * alternative behavior.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PjgPolarStereoCalc_hh
#define PjgPolarStereoCalc_hh


#include <euclid/PjgCalc.hh>
#include <euclid/PjgTypes.hh>


class PjgPolarStereoCalc : public PjgCalc
{

public:

  /**********************************************************************
   * Constructors
   */
  
  PjgPolarStereoCalc(const double tangent_lon,
		     const PjgTypes::pole_type_t pt = PjgTypes::POLE_NORTH,
		     const double central_scale = 1.0,
		     const int nx = 1, const int ny = 1, const int nz = 1,
		     const double dx = 1.0, const double dy = 1.0,
		     const double dz = 1.0,
		     const double minx = 0.0, const double miny = 0.0,
		     const double minz = 0.0);
  
  /**********************************************************************
   * Destructor
   */

  virtual ~PjgPolarStereoCalc();
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * getProjType() - Retrieve the current projection type.
   */

  int getProjType(void) const
  {
    return PjgTypes::PROJ_POLAR_STEREO;
  }

  /// Set polar stereographic tangent longitude, also known as rotation,
  /// also known as the center longitude, also known as orientation.
  virtual inline void setTangentLon(const double tangent_lon)
  {
    _tangentLon = tangent_lon;

    _math->setTangentLon(_tangentLon);
  }

  virtual inline double getTangentLon(void) const
  {
    return _tangentLon;
  }

  virtual inline void setPole(PjgTypes::pole_type_t pole)
  {
    _pole = pole;
    if (_pole == PjgTypes::POLE_NORTH)
      _math->setPole(true);
    else
      _math->setPole(false);
  }

  virtual inline PjgTypes::pole_type_t getPole() const
  {
    return _pole;
  }

  virtual inline void setCentralScale(const double central_scale)
  {
    _centralScale = central_scale;
    _math->setCentralScale(central_scale);
  }
  
  virtual inline double getCentralScale() const
  {
    return _centralScale;
  }
  

  /////////////////////
  // Virtual methods //
  /////////////////////

  /**********************************************************************
   * latlon2xy() - Convert the given lat/lon location to the grid location
   *               in grid units.
   */

  virtual void latlon2xy(const double lat, const double lon,
			 double  &x, double &y) const;

  /**********************************************************************
   * xy2latlon() - Convert the given grid location specified in grid units
   *               to the appropriate lat/lon location.
   */
  
  virtual void xy2latlon(const double x, const double y,
			 double &lat, double &lon,
			 const double z = -9999.0) const;

  /**********************************************************************
   * km2x() - Converts the given distance in kilometers to the same
   *          distance in the units appropriate to the projection.
   */

  virtual double km2x(const double km) const;
  
  /**********************************************************************
   * x2km() - Converts the given distance to kilometers.  The distance
   *          is assumed to be in the units appropriate to the projection.
   */

  virtual double x2km(const double x) const;
  
  /**********************************************************************
   * km2xGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the X axis.
   */

  virtual double km2xGrid(const double x_km) const;
  
  /**********************************************************************
   * km2yGrid() - Converts the given distance in kilometers to the
   *              appropriate number of grid spaces along the Y axis.
   */

  virtual double km2yGrid(const double y_km) const;
  
  /**********************************************************************
   * xGrid2km() - Converts the given distance in number of grid spaces
   *              along the X axis to kilometers.  If y_index is non-negative,
   *              the conversion is done at that point in the grid;
   *              otherwise, the conversion is done at the center of the
   *              grid.
   */

  virtual double xGrid2km(const double x_grid,
			  const int y_index = -1) const;
  
  /**********************************************************************
   * yGrid2km() - Converts the given distance in number of grid spaces
   *              along the Y axis to kilometers.
   */

  virtual double yGrid2km(const double y_grid) const;


  ////////////////////
  // Output methods //
  ////////////////////

  /**********************************************************************
   * print() - Print the projection parameters to the given stream
   */

  virtual void print(ostream &stream) const;
  

protected:

  PjgPolarStereoMath *_math;
  
  double _tangentLon;
  PjgTypes::pole_type_t _pole;
  double _centralScale;
  

  ///////////////////////////////
  // Virtual protected methods //
  ///////////////////////////////

private:

};

#endif
